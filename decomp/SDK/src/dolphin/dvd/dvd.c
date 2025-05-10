#include <string.h>

#include "../os/OSPrivate.h"
#include "dvd_private.h"

#define ERROR_FATAL            1
#define ERROR_RETRY            2
#define ERROR_RETRY_INTERNALLY 3

static u16*            tmpBuffer[32] ATTRIBUTE_ALIGN (32);
static DVDCommandBlock DummyCommandBlock;
static OSAlarm         ResetAlarm;

static int autoInvalidation = 1;

static DVDCommandBlock* executing;
static void*            tmp;
static DVDDiskID*       currID;
static OSBootInfo*      bootInfo;
static volatile BOOL    PauseFlag;
static volatile BOOL    PausingFlag;
static int              AutoFinishing;
static volatile BOOL    FatalErrorFlag;
static vu32             CurrCommand;
static vu32             Canceling;
static DVDCommandBlock* CancelingCommandBlock;
static void             (*CancelCallback) (s32, DVDCommandBlock*);
static vu32             ResumeFromHere;
static vu32             CancelLastError;
static u32              LastError;
static vs32             NumInternalRetry;
static int              ResetRequired;
static int              CancelAllSyncComplete;
static vu32             ResetCount;
static int              FirstTimeInBootrom;
static s32              ResultForSyncCommand;
static int              DVDInitialized;
void                    (*LastState) (DVDCommandBlock*);

static void stateReadingFST ();
static void cbForStateReadingFST (u32 intType);
static void cbForStateError (u32 intType);
static void stateError (u32 error);
static void stateTimeout ();
static void stateGettingError ();
static u32  CategorizeError (u32 error);
static BOOL CheckCancel (u32 resume);
static void cbForStateGettingError (u32 intType);
static void cbForUnrecoveredError (u32 intType);
static void cbForUnrecoveredErrorRetry (u32 intType);
static void stateGoToRetry ();
static void cbForStateGoToRetry (u32 intType);
static void stateCheckID ();
static void stateCheckID3 (DVDCommandBlock* block);
static void stateCheckID2 (DVDCommandBlock* block);
static void cbForStateCheckID1 (u32 intType);
static void cbForStateCheckID2 (u32 intType);
static void cbForStateCheckID3 (u32 intType);
static void stateCoverClosed ();
static void stateCoverClosed_CMD (DVDCommandBlock* block);
static void cbForStateCoverClosed (u32 intType);
static void stateMotorStopped ();
static void cbForStateMotorStopped (u32 intType);
static void stateReady ();
static void stateBusy (DVDCommandBlock* block);
static void cbForStateBusy (u32 intType);
static int  issueCommand (s32 prio, DVDCommandBlock* block);
static void cbForCancelStreamSync (s32 result, DVDCommandBlock* block);
static void cbForStopStreamAtEndSync (s32 result, DVDCommandBlock* block);
static void cbForGetStreamErrorStatusSync (s32 result, DVDCommandBlock* block);
static void cbForGetStreamPlayAddrSync (s32 result, DVDCommandBlock* block);
static void cbForGetStreamStartAddrSync (s32 result, DVDCommandBlock* block);
static void cbForGetStreamLengthSync (s32 result, DVDCommandBlock* block);
static void cbForChangeDiskSync ();
static void cbForInquirySync (s32 result, DVDCommandBlock* block);
static void cbForCancelSync ();
static void cbForCancelAllSync ();

void
DVDInit ()
{
    if (!DVDInitialized)
    {
        OSInitAlarm();
        DVDInitialized = 1;
        __DVDFSInit();
        __DVDClearWaitingQueue();
        __DVDInitWA();
        bootInfo = (void*)OSPhysicalToCached (0);
        currID = &bootInfo->DVDDiskID;
        __OSSetInterruptHandler (0x15, __DVDInterruptHandler);
        __OSUnmaskInterrupts (0x400U);
        OSInitThreadQueue (&__DVDThreadQueue);
        __DIRegs[DI_SR] = 0x2A;
        __DIRegs[DI_CVR] = 0;
        if (bootInfo->magic == OS_BOOTINFO_MAGIC_JTAG)
        {
            OSReport ("app booted via JTAG\n");
            OSReport ("load fst\n");
            __fstLoad();
            return;
        }
        if (bootInfo->magic == OS_BOOTINFO_MAGIC)
        {
            OSReport ("app booted from bootrom\n");
            return;
        }
        FirstTimeInBootrom = 1;
        OSReport ("bootrom\n");
    }
}

static void
stateReadingFST ()
{
    LastState = stateReadingFST;
    ASSERTLINE (0x219, ((u32)(bootInfo->FSTLocation) & (32 - 1)) == 0);
    DVDLowRead (bootInfo->FSTLocation,
                OSRoundUp32B ((tmpBuffer[2])),
                (u32)tmpBuffer[1],
                cbForStateReadingFST);
}

static void
cbForStateReadingFST (u32 intType)
{
    DVDCommandBlock* finished;

    ASSERTLINE (0x229, (intType & DVD_INTTYPE_CVR) == 0);

    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        stateTimeout();
        return;
    }

    if (intType & 1)
    {
        ASSERTLINE (0x22E, (intType & DVD_INTTYPE_DE) == 0);
        NumInternalRetry = 0;
        finished = executing;
        executing = &DummyCommandBlock;
        finished->state = DVD_STATE_END;
        if (finished->callback)
        {
            finished->callback (0, finished);
        }
        stateReady();
        return;
    }
    ASSERTLINE (0x246, intType == DVD_INTTYPE_DE);
    stateGettingError();
}

static void
cbForStateError (u32 intType)
{
    DVDCommandBlock* finished;

    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        stateTimeout();
        return;
    }

    FatalErrorFlag = TRUE;
    finished = executing;
    executing = &DummyCommandBlock;
    if (finished->callback)
    {
        (finished->callback) (-1, finished);
    }
    if (Canceling)
    {
        Canceling = FALSE;
        if (CancelCallback)
            (CancelCallback) (0, finished);
    }

    FORCE_DONT_INLINE

    stateReady();
}

static void
stateError (u32 error)
{
    __DVDStoreErrorCode (error);
    DVDLowStopMotor (cbForStateError);
}

static void
stateTimeout ()
{
    DVDCommandBlock* finishing;

    __DVDStoreErrorCode (DVD_TIMEOUT_ERROR_CODE);
    DVDReset();

    FatalErrorFlag = TRUE;
    finishing = executing;
    executing = &DummyCommandBlock;

    if (finishing->callback)
    {
        (finishing->callback) (-1, finishing);
    }

    if (Canceling)
    {
        Canceling = 0;
        if (CancelCallback)
            (CancelCallback) (0, finishing);
    }

    stateReady();
}

static void
stateGettingError ()
{
    DVDLowRequestError (cbForStateGettingError);
}

static u32
CategorizeError (u32 error)
{
    if (error == 0x20400)
    {
        LastError = error;
        return 1;
    }
    error &= 0xFFFFFF;
    if ((error == 0x62800) || (error == 0x23A00) || (error == 0xB5A01))
    {
        return 0;
    }
    NumInternalRetry += 1;
    if (NumInternalRetry == 2)
    {
        if (error == LastError)
        {
            LastError = error;
            return 1;
        }
        LastError = error;
        return 2;
    }
    LastError = error;
    if ((error == 0x31100) || (executing->command == 5))
    {
        return 2;
    }

    return 3;
}

static BOOL
CheckCancel (u32 resume)
{
    DVDCommandBlock* finishing;
    if (Canceling)
    {
        ResumeFromHere = resume;
        finishing = executing;
        Canceling = FALSE;
        executing = &DummyCommandBlock;
        finishing->state = 10;
        if (finishing->callback)
        {
            (finishing->callback) (-3, finishing);
        }
        if (CancelCallback)
        {
            (CancelCallback) (0, finishing);
        }
        stateReady();
        return TRUE;
    }

    return FALSE;
}

static void
cbForStateGettingError (u32 intType)
{
    u32 error;
    u32 status;
    u32 errorCategory;
    u32 resume;

    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        __DVDStoreErrorCode (DVD_TIMEOUT_ERROR_CODE);
        DVDReset();
        cbForStateError (0);
        return;
    }
    if (intType & DVD_INTTYPE_DE)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        stateError (DVD_DE_INT_ERROR_CODE);
        return;
    }

    ASSERTLINE (0x30F, intType == DVD_INTTYPE_TC);

    error = __DIRegs[DI_IMMBUF];
    status = error & 0xFF000000;

    errorCategory = CategorizeError (error);
    if (errorCategory == ERROR_FATAL)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        stateError (error);
        return;
    }

    if ((errorCategory == ERROR_RETRY) || (errorCategory == ERROR_RETRY_INTERNALLY))
    {
        resume = 0;
    }
    else
    {
        if (status == 0x1000000)
            resume = 4;
        else if (status == 0x2000000)
            resume = 6;
        else if (status == 0x3000000)
            resume = 3;
        else
            resume = 5;
    }
    if (CheckCancel (resume))
        return;

    if (errorCategory == ERROR_RETRY)
    {
        __DVDStoreErrorCode (error);
        stateGoToRetry();
        return;
    }

    else if (errorCategory == ERROR_RETRY_INTERNALLY)
    {
        if ((error & 0xFFFFFF) == 0x31100)
        {
            DVDLowSeek (executing->offset, cbForUnrecoveredError);
        }
        else
        {
            (*LastState) (executing);
        }
        return;
    }

    if (status == 0x1000000)
    {
        executing->state = 5;
        stateMotorStopped();
    }
    else if (status == 0x2000000)
    {
        executing->state = 3;
        stateCoverClosed();
    }
    else if (status == 0x3000000)
    {
        executing->state = 4;
        stateMotorStopped();
    }
    else
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        stateError (DVD_DE_INT_ERROR_CODE);
        return;
    }
}

static void
cbForUnrecoveredError (u32 intType)
{
    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        __DVDStoreErrorCode (DVD_TIMEOUT_ERROR_CODE);
        DVDReset();
        cbForStateError (0);
        return;
    }

    if ((intType & 1))
    {
        stateGoToRetry();
        return;
    }

    DVDLowRequestError (cbForUnrecoveredErrorRetry);
}

static void
cbForUnrecoveredErrorRetry (u32 intType)
{
    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        __DVDStoreErrorCode (DVD_TIMEOUT_ERROR_CODE);
        DVDReset();
        cbForStateError (0);
        return;
    }

    executing->state = DVD_STATE_FATAL_ERROR;

    if (intType & 2)
        stateError (0x1234567);
    else
        stateError (__DIRegs[DI_IMMBUF]);
}

static void
stateGoToRetry ()
{
    DVDLowStopMotor (cbForStateGoToRetry);
}

static void
cbForStateGoToRetry (u32 intType)
{
    DVDCommandBlock* finished;

    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        __DVDStoreErrorCode (DVD_TIMEOUT_ERROR_CODE);
        DVDReset();
        cbForStateError (0);
        return;
    }

    if (intType & 2)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        stateError (DVD_DE_INT_ERROR_CODE);
        return;
    }

    ASSERTLINE (0x3D9, intType == DVD_INTTYPE_TC);

    NumInternalRetry = FALSE;

    if ((CurrCommand == DVD_COMMAND_BSREAD) || (CurrCommand == DVD_COMMAND_READID) ||
        (CurrCommand == DVD_COMMAND_AUDIO_BUFFER_CONFIG) ||
        (CurrCommand == DVD_COMMAND_BS_CHANGE_DISK))
    {
        ResetRequired = TRUE;
    }

    if (!CheckCancel (2))
    {
        executing->state = DVD_STATE_RETRY;
        stateMotorStopped();
    }
}

static void
stateCheckID ()
{
    switch (CurrCommand)
    {
        case DVD_COMMAND_CHANGE_DISK:
            if ((s32)memcmp (&tmpBuffer, executing->id, 0x1C))
            {
                DVDLowStopMotor (cbForStateCheckID1);
                return;
            }
            memcpy (currID, &tmpBuffer, sizeof (DVDDiskID));
            executing->state = DVD_STATE_BUSY;
            DCInvalidateRange (&tmpBuffer, sizeof (DVDDiskID));
            LastState = stateCheckID2;
            stateCheckID2 (executing);
            break;
        default:
            if ((s32)memcmp (&tmpBuffer, currID, sizeof (DVDDiskID)))
            {
                DVDLowStopMotor (cbForStateCheckID1);
                return;
            }
            LastState = stateCheckID3;
            stateCheckID3 (executing);
            ASSERTLINE (0x452, FALSE);
            return;
    }
}

static void
stateCheckID3 (DVDCommandBlock* block)
{
    DVDLowAudioBufferConfig (currID->streaming, 0xAU, cbForStateCheckID3);
}

static void
stateCheckID2 (DVDCommandBlock* block)
{
    DVDLowRead (&tmpBuffer, 0x20U, 0x420, cbForStateCheckID2);
}

static void
cbForStateCheckID1 (u32 intType)
{
    DVDCommandBlock* finishing;

    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        __DVDStoreErrorCode (DVD_TIMEOUT_ERROR_CODE);
        DVDReset();
        cbForStateError (0);
        return;
    }

    if ((intType & DVD_INTTYPE_DE))
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        __DVDStoreErrorCode (0x1234567);
        DVDLowStopMotor (cbForStateError);
        return;
    }

    ASSERTLINE (0x478, intType == DVD_INTTYPE_TC);
    NumInternalRetry = 0;
    if (!CheckCancel (TRUE))
    {
        executing->state = DVD_STATE_WRONG_DISK;
        stateMotorStopped();
    }
}

static void
cbForStateCheckID2 (u32 intType)
{
    ASSERTLINE (0x494, (intType & DVD_INTTYPE_CVR) == 0);

    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        __DVDStoreErrorCode (DVD_TIMEOUT_ERROR_CODE);
        DVDReset();
        cbForStateError (0);
        return;
    }

    if (intType & DVD_INTTYPE_TC)
    {
        ASSERTLINE (0x499, (intType & DVD_INTTYPE_DE) == 0);
        NumInternalRetry = 0;
        stateReadingFST();
        return;
    }
    ASSERTLINE (0x4AA, intType == DVD_INTTYPE_DE);
    stateGettingError();
}

static void
cbForStateCheckID3 (u32 intType)
{
    DVDCommandBlock* finishing;

    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        __DVDStoreErrorCode (DVD_TIMEOUT_ERROR_CODE);
        DVDReset();
        cbForStateError (0);
        return;
    }
    ASSERTLINE (0x478, intType == DVD_INTTYPE_TC);

    if ((intType & 1))
    {
        NumInternalRetry = 0;
        if (!CheckCancel (FALSE))
        {
            executing->state = 1;
            stateBusy (executing);
        }
        return;
    }

    DVDLowRequestError (cbForStateGettingError);
}

static void
AlarmHandler ()
{
    DVDReset();
    DCInvalidateRange (&tmpBuffer, 0x20);
    LastState = stateCoverClosed_CMD;
    stateCoverClosed_CMD (executing);
}

static void
stateCoverClosed ()
{
    DVDCommandBlock* finished;

    switch (CurrCommand)
    {
        case DVD_COMMAND_BSREAD:
        case DVD_COMMAND_READID:
        case DVD_COMMAND_AUDIO_BUFFER_CONFIG:
        case DVD_COMMAND_BS_CHANGE_DISK:
            __DVDClearWaitingQueue();
            finished = executing;
            executing = &DummyCommandBlock;
            if (finished->callback)
            {
                finished->callback (-4, finished);
            }
            stateReady();
            return;
        default:
            DVDReset();
            OSCreateAlarm (&ResetAlarm);
            OSSetAlarm (&ResetAlarm, OSMillisecondsToTicks (1150), AlarmHandler);
    }
    return;
}

static void
stateCoverClosed_CMD (DVDCommandBlock* block)
{
    DVDLowReadDiskID ((void*)&tmpBuffer, cbForStateCoverClosed);
}

static void
cbForStateCoverClosed (u32 intType)
{
    ASSERTLINE (0x510, (intType & DVD_INTTYPE_CVR) == 0);

    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        __DVDStoreErrorCode (DVD_TIMEOUT_ERROR_CODE);
        DVDReset();
        cbForStateError (0);
        return;
    }

    if (intType & DVD_INTTYPE_TC)
    {
        ASSERTLINE (0x515, (intType & DVD_INTTYPE_DE) == 0);
        ASSERTLINE (0x519,
                    (CurrCommand == DVD_COMMAND_READ) || (CurrCommand == DVD_COMMAND_SEEK) ||
                        (CurrCommand == DVD_COMMAND_CHANGE_DISK));
        NumInternalRetry = 0;
        stateCheckID();
        return;
    }
    ASSERTLINE (0x523, intType == DVD_INTTYPE_DE);
    stateGettingError();
}

static void
stateMotorStopped ()
{
    DVDLowWaitCoverClose (cbForStateMotorStopped);
}

static void
cbForStateMotorStopped (u32 intType)
{
    ASSERTLINE (0x540, intType == DVD_INTTYPE_CVR);
    __DIRegs[DI_CVR] = 0;
    executing->state = DVD_STATE_COVER_CLOSED;
    stateCoverClosed();
}

static void
stateReady ()
{
    DVDCommandBlock* finished;

    if (!__DVDCheckWaitingQueue())
    {
        executing = NULL;
        return;
    }
    if (PauseFlag)
    {
        PausingFlag = TRUE;
        executing = NULL;
        return;
    }
    executing = __DVDPopWaitingQueue();

    if (FatalErrorFlag)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        finished = executing;
        executing = &DummyCommandBlock;

        if (finished->callback)
        {
            (finished->callback) (-1, finished);
        }

        stateReady();
        return;
    }

    CurrCommand = executing->command;
    if (ResumeFromHere)
    {
        switch (ResumeFromHere)
        {
            case 1:
                executing->state = DVD_STATE_WRONG_DISK;
                stateMotorStopped();
                break;
            case 2:
                executing->state = DVD_STATE_RETRY;
                stateMotorStopped();
                break;
            case 3:
                executing->state = DVD_STATE_NO_DISK;
                stateMotorStopped();
                break;
            case 7:
                executing->state = DVD_STATE_MOTOR_STOPPED;
                stateMotorStopped();
                break;
            case 4:
                executing->state = DVD_STATE_COVER_OPEN;
                stateMotorStopped();
                break;
            case 6:
                executing->state = DVD_STATE_COVER_CLOSED;
                stateCoverClosed();
                break;
            case 5:
                executing->state = DVD_STATE_FATAL_ERROR;
                stateError (CancelLastError);
                break;
        }
        ResumeFromHere = 0;
    }
    else
    {
        executing->state = DVD_STATE_BUSY;
        stateBusy (executing);
    }
}

static void
stateBusy (DVDCommandBlock* block)
{
    LastState = stateBusy;
    switch (block->command)
    {
        case DVD_COMMAND_READID:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            block->currTransferSize = 0x20;
            DVDLowReadDiskID (block->addr, cbForStateBusy);
            return;
        case DVD_COMMAND_READ:
        case DVD_COMMAND_BSREAD:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            block->currTransferSize = (block->length - block->transferredSize > 0x600000)
                                          ? 0x600000
                                          : (block->length - block->transferredSize);
            DVDLowRead ((char*)block->addr + block->transferredSize,
                        block->currTransferSize,
                        block->offset + block->transferredSize,
                        cbForStateBusy);
            return;
        case DVD_COMMAND_SEEK:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            DVDLowSeek (block->offset, cbForStateBusy);
            return;
        case DVD_COMMAND_CHANGE_DISK:
            DVDLowStopMotor (cbForStateBusy);
            return;
        case DVD_COMMAND_BS_CHANGE_DISK:
            DVDLowStopMotor (cbForStateBusy);
            return;
        case DVD_COMMAND_INITSTREAM:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            if (AutoFinishing != 0)
            {
                executing->currTransferSize = 0;
                DVDLowRequestAudioStatus (0, cbForStateBusy);
                return;
            }
            executing->currTransferSize = 1;
            DVDLowAudioStream (0, block->length, block->offset, cbForStateBusy);
            return;
        case DVD_COMMAND_CANCELSTREAM:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            DVDLowAudioStream (0x10000, 0U, 0U, cbForStateBusy);
            return;
        case DVD_COMMAND_STOP_STREAM_AT_END:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            AutoFinishing = 1;
            DVDLowAudioStream (0, 0U, 0U, cbForStateBusy);
            return;
        case DVD_COMMAND_REQUEST_AUDIO_ERROR:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            DVDLowRequestAudioStatus (0, cbForStateBusy);
            return;
        case DVD_COMMAND_REQUEST_PLAY_ADDR:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            DVDLowRequestAudioStatus (0x10000, cbForStateBusy);
            return;
        case DVD_COMMAND_REQUEST_START_ADDR:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            DVDLowRequestAudioStatus (0x20000, cbForStateBusy);
            return;
        case DVD_COMMAND_REQUEST_LENGTH:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            DVDLowRequestAudioStatus (0x30000, cbForStateBusy);
            return;
        case DVD_COMMAND_AUDIO_BUFFER_CONFIG:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            DVDLowAudioBufferConfig (block->offset, block->length, cbForStateBusy);
            return;
        case DVD_COMMAND_INQUIRY:
            __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
            block->currTransferSize = 0x20;
            DVDLowInquiry (block->addr, cbForStateBusy);
            return;
    }
}

static void
cbForStateBusy (u32 intType)
{
    DVDCommandBlock* finished;
    s32              result;

    if (intType == DVD_INTTYPE_TIMEOUT)
    {
        executing->state = DVD_STATE_FATAL_ERROR;
        __DVDStoreErrorCode (DVD_TIMEOUT_ERROR_CODE);
        DVDReset();
        cbForStateError (0);
        return;
    }

    if ((CurrCommand == DVD_COMMAND_CHANGE_DISK) || (CurrCommand == DVD_COMMAND_BS_CHANGE_DISK))
    {
        if (intType & DVD_INTTYPE_DE)
        {
            executing->state = DVD_STATE_FATAL_ERROR;
            stateError (DVD_DE_INT_ERROR_CODE);
            return;
        }
        ASSERTLINE (0x64B, intType == DVD_INTTYPE_TC);
        NumInternalRetry = 0;
        if (CurrCommand == DVD_COMMAND_BS_CHANGE_DISK)
        {
            ResetRequired = 1;
        }
        if (!CheckCancel (7))
        {
            executing->state = DVD_STATE_MOTOR_STOPPED;
            stateMotorStopped();
        }
        return;
    }
    ASSERTLINE (0x671, (intType & DVD_INTTYPE_CVR) == 0);
    if ((CurrCommand == DVD_COMMAND_READ) || (CurrCommand == DVD_COMMAND_BSREAD) ||
        (CurrCommand == DVD_COMMAND_READID) || (CurrCommand == DVD_COMMAND_INQUIRY))
    {
        executing->transferredSize += executing->currTransferSize - __DIRegs[DI_LENGTH];
    }
    if (intType & 8)
    {
        Canceling = 0;
        finished = executing;
        executing = &DummyCommandBlock;
        finished->state = DVD_STATE_CANCELED;
        if (finished->callback)
        {
            finished->callback (-3, finished);
        }
        if (CancelCallback)
        {
            CancelCallback (0, finished);
        }
        stateReady();
        return;
    }
    if (intType & 1)
    {
        ASSERTLINE (0x697, (intType & DVD_INTTYPE_DE) == 0);
        NumInternalRetry = 0;
        if (CheckCancel (0))
            return;

        if (CurrCommand == DVD_COMMAND_READ || CurrCommand == DVD_COMMAND_BSREAD ||
            CurrCommand == DVD_COMMAND_READID || CurrCommand == DVD_COMMAND_INQUIRY)
        {
            if (executing->transferredSize != executing->length)
            {
                stateBusy (executing);
                return;
            }
            finished = executing;
            executing = &DummyCommandBlock;
            finished->state = DVD_STATE_END;
            if (finished->callback)
            {
                finished->callback (finished->transferredSize, finished);
            }
            stateReady();
            return;
        }
        else if (CurrCommand == DVD_COMMAND_REQUEST_AUDIO_ERROR ||
                 CurrCommand == DVD_COMMAND_REQUEST_PLAY_ADDR ||
                 CurrCommand == DVD_COMMAND_REQUEST_START_ADDR ||
                 CurrCommand == DVD_COMMAND_REQUEST_LENGTH)
        {
            if (CurrCommand == DVD_COMMAND_REQUEST_START_ADDR ||
                CurrCommand == DVD_COMMAND_REQUEST_PLAY_ADDR)
            {
                result = __DIRegs[DI_IMMBUF] * 4;
            }
            else
            {
                result = __DIRegs[DI_IMMBUF];
            }

            finished = executing;
            executing = &DummyCommandBlock;
            finished->state = DVD_STATE_END;
            if (finished->callback)
            {
                finished->callback (result, finished);
            }
            stateReady();
            return;
        }
        else if (CurrCommand == DVD_COMMAND_INITSTREAM)
        {
            if (executing->currTransferSize == 0)
            {
                if (__DIRegs[DI_IMMBUF] & 1)
                {
                    finished = executing;
                    executing = &DummyCommandBlock;
                    finished->state = DVD_STATE_IGNORED;
                    if (finished->callback)
                    {
                        finished->callback (-2, finished);
                    }
                    stateReady();
                    return;
                }
                AutoFinishing = 0;
                executing->currTransferSize = 1;
                DVDLowAudioStream (0, executing->length, executing->offset, cbForStateBusy);
                return;
            }
            finished = executing;
            executing = &DummyCommandBlock;
            finished->state = DVD_STATE_END;
            if (finished->callback)
            {
                finished->callback (0, finished);
            }
            stateReady();
            return;
        }
        else
        {
            finished = executing;
            executing = &DummyCommandBlock;
            finished->state = DVD_STATE_END;
            if (finished->callback)
            {
                finished->callback (0, finished);
            }
            stateReady();
            return;
        }
    }
    else
    {
        if (CurrCommand == 14)
        {
            executing->state = DVD_STATE_FATAL_ERROR;
            stateError (DVD_DE_INT_ERROR_CODE);
            return;
        }
        if ((CurrCommand == DVD_COMMAND_READ || CurrCommand == DVD_COMMAND_BSREAD ||
             CurrCommand == DVD_COMMAND_READID || CurrCommand == DVD_COMMAND_INQUIRY) &&
            executing->transferredSize == executing->length)
        {
            if (CheckCancel (0))
            {
                return;
            }
            finished = executing;
            executing = &DummyCommandBlock;

            finished->state = 0;
            if (finished->callback)
            {
                (finished->callback) ((s32)finished->transferredSize, finished);
            }
            stateReady();
            return;
        }

        ASSERTLINE (0x728, intType == DVD_INTTYPE_DE);
        stateGettingError();
    }
}

static int
issueCommand (s32 prio, DVDCommandBlock* block)
{
    int level;
    int result;

    if (autoInvalidation != 0 &&
        (block->command == DVD_COMMAND_READ || block->command == DVD_COMMAND_BSREAD ||
         block->command == DVD_COMMAND_READID || block->command == DVD_COMMAND_INQUIRY))
    {
        DCInvalidateRange (block->addr, block->length);
    }
    level = OSDisableInterrupts();
#if DEBUG
    if (executing == block ||
        block->state == DVD_STATE_WAITING && __DVDIsBlockInWaitingQueue (block))
    {
        ASSERTMSGLINE (0x758,
                       FALSE,
                       "DVD library: Specified command block (or file info) "
                       "is already in use\n");
    }
#endif
    block->state = DVD_STATE_WAITING;
    result = __DVDPushWaitingQueue (prio, block);
    if (executing == NULL && PauseFlag == 0)
    {
        stateReady();
    }
    OSRestoreInterrupts (level);
    return result;
}

int
DVDReadAbsAsyncPrio (DVDCommandBlock* block,
                     void*            addr,
                     s32              length,
                     s32              offset,
                     void             (*callback) (s32, DVDCommandBlock*),
                     s32              prio)
{
    int idle;

    ASSERTMSGLINE (0x780,
                   block,
                   "DVDReadAbsAsync(): null pointer is specified to command "
                   "block address.");
    ASSERTMSGLINE (0x781, addr, "DVDReadAbsAsync(): null pointer is specified to addr.");
    ASSERTMSGLINE (0x783,
                   !OFFSET (addr, 32),
                   "DVDReadAbsAsync(): address must be aligned with 32 byte "
                   "boundary.");
    ASSERTMSGLINE (0x785,
                   !(length & (32 - 1)),
                   "DVDReadAbsAsync(): length must be a multiple of 32.");
    ASSERTMSGLINE (0x787,
                   !(offset & (4 - 1)),
                   "DVDReadAbsAsync(): offset must be a multiple of 4.");
    ASSERTMSGLINE (0x789,
                   length > 0,
                   "DVD read: 0 or negative value was specified to length of "
                   "the read\n");
    block->command = DVD_COMMAND_READ;
    block->addr = addr;
    block->length = length;
    block->offset = offset;
    block->transferredSize = 0;
    block->callback = callback;
    idle = issueCommand (prio, block);
    ASSERTMSGLINE (0x793,
                   idle,
                   "DVDReadAbsAsync(): command block is used for processing "
                   "previous request.");
    return idle;
}

int
DVDSeekAbsAsyncPrio (DVDCommandBlock* block,
                     s32              offset,
                     void             (*callback) (long, DVDCommandBlock*),
                     s32              prio)
{
    int idle;

    ASSERTMSGLINE (0x7AA,
                   block,
                   "DVDSeekAbs(): null pointer is specified to command "
                   "block address.");
    ASSERTMSGLINE (0x7AC, !(offset & (4 - 1)), "DVDSeekAbs(): offset must be a multiple of 4.");
    block->command = DVD_COMMAND_SEEK;
    block->offset = offset;
    block->callback = callback;
    idle = issueCommand (prio, block);
    ASSERTMSGLINE (0x7B3,
                   idle,
                   "DVDSeekAbs(): command block is used for processing "
                   "previous request.");
    return idle;
}

int
DVDReadAbsAsyncForBS (DVDCommandBlock* block,
                      void*            addr,
                      s32              length,
                      s32              offset,
                      void             (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    ASSERTMSGLINE (0x7D1,
                   block,
                   "DVDReadAbsAsyncForBS(): null pointer is specified to "
                   "command block address.");
    ASSERTMSGLINE (0x7D2, addr, "DVDReadAbsAsyncForBS(): null pointer is specified to addr.");
    ASSERTMSGLINE (0x7D4,
                   !OFFSET (addr, 32),
                   "DVDReadAbsAsyncForBS(): address must be aligned with 32 "
                   "byte boundary.");
    ASSERTMSGLINE (0x7D6,
                   !(length & (32 - 1)),
                   "DVDReadAbsAsyncForBS(): length must be a multiple of 32.");
    ASSERTMSGLINE (0x7D8,
                   !(offset & (4 - 1)),
                   "DVDReadAbsAsyncForBS(): offset must be a multiple of 4.");
    block->command = DVD_COMMAND_BSREAD;
    block->addr = addr;
    block->length = length;
    block->offset = offset;
    block->transferredSize = 0;
    block->callback = callback;
    idle = issueCommand (2, block);
    ASSERTMSGLINE (0x7E2,
                   idle,
                   "DVDReadAbsAsyncForBS(): command block is used for "
                   "processing previous request.");
    return idle;
}

int
DVDReadDiskID (DVDCommandBlock* block,
               DVDDiskID*       diskID,
               void             (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    ASSERTMSGLINE (0x7F9,
                   block,
                   "DVDReadDiskID(): null pointer is specified to command "
                   "block address.");
    ASSERTMSGLINE (0x7FA, diskID, "DVDReadDiskID(): null pointer is specified to id address.");
    ASSERTMSGLINE (0x7FC,
                   !OFFSET (diskID, 32),
                   "DVDReadDiskID(): id must be aligned with 32 byte boundary.");

    block->command = DVD_COMMAND_READID;
    block->addr = diskID;
    block->length = 0x20;
    block->offset = 0;
    block->transferredSize = 0;
    block->callback = callback;
    idle = issueCommand (2, block);
    ASSERTMSGLINE (0x806,
                   idle,
                   "DVDReadDiskID(): command block is used for processing "
                   "previous request.");
    return idle;
}

int
DVDPrepareStreamAbsAsync (DVDCommandBlock* block,
                          u32              length,
                          u32              offset,
                          void             (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    block->command = DVD_COMMAND_INITSTREAM;
    block->length = length;
    block->offset = offset;
    block->callback = callback;
    idle = issueCommand (1, block);
    return idle;
}

int
DVDCancelStreamAsync (DVDCommandBlock* block, void (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    block->command = DVD_COMMAND_CANCELSTREAM;
    block->callback = callback;
    idle = issueCommand (1, block);
    return idle;
}

s32
DVDCancelStream (DVDCommandBlock* block)
{
    int result;
    s32 state;
    int enabled;
    s32 retVal;

    result = DVDCancelStreamAsync (block, cbForCancelStreamSync);
    if (result == 0)
    {
        return -1;
    }
    enabled = OSDisableInterrupts();

    while (TRUE)
    {
        state = ((volatile DVDCommandBlock*)block)->state;
        if (state == DVD_STATE_END || state == DVD_STATE_FATAL_ERROR ||
            state == DVD_STATE_CANCELED)
        {
            retVal = (s32)block->transferredSize;
            break;
        }
        OSSleepThread (&__DVDThreadQueue);
    }
    OSRestoreInterrupts (enabled);
    return retVal;
}

static void
cbForCancelStreamSync (s32 result, DVDCommandBlock* block)
{
    block->transferredSize = result;
    OSWakeupThread (&__DVDThreadQueue);
}

int
DVDStopStreamAtEndAsync (DVDCommandBlock* block, void (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    block->command = DVD_COMMAND_STOP_STREAM_AT_END;
    block->callback = callback;
    idle = issueCommand (1, block);
    return idle;
}

s32
DVDStopStreamAtEnd (DVDCommandBlock* block)
{
    int result;
    s32 state;
    int enabled;
    s32 retVal;

    result = DVDStopStreamAtEndAsync (block, cbForStopStreamAtEndSync);
    if (result == 0)
    {
        return -1;
    }
    enabled = OSDisableInterrupts();

    while (1)
    {
        state = block->state;
        if (state != DVD_STATE_BUSY && state != DVD_STATE_WAITING)
        {
            retVal = ResultForSyncCommand;
            break;
        }
        OSSleepThread (&__DVDThreadQueue);
    }
    OSRestoreInterrupts (enabled);
    return retVal;
}

static void
cbForStopStreamAtEndSync (s32 result, DVDCommandBlock* block)
{
    ResultForSyncCommand = result;
    OSWakeupThread (&__DVDThreadQueue);
}

int
DVDGetStreamErrorStatusAsync (DVDCommandBlock* block, void (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    block->command = DVD_COMMAND_REQUEST_AUDIO_ERROR;
    block->callback = callback;
    idle = issueCommand (1, block);
    return idle;
}

s32
DVDGetStreamErrorStatus (DVDCommandBlock* block)
{
    int result;
    s32 state;
    int enabled;
    s32 retVal;

    result = DVDGetStreamErrorStatusAsync (block, cbForGetStreamErrorStatusSync);
    if (result == 0)
    {
        return -1;
    }
    enabled = OSDisableInterrupts();

    while (1)
    {
        state = block->state;
        if (state != DVD_STATE_BUSY && state != DVD_STATE_WAITING)
        {
            retVal = ResultForSyncCommand;
            break;
        }
        OSSleepThread (&__DVDThreadQueue);
    }
    OSRestoreInterrupts (enabled);
    return retVal;
}

static void
cbForGetStreamErrorStatusSync (s32 result, DVDCommandBlock* block)
{
    ResultForSyncCommand = result;
    OSWakeupThread (&__DVDThreadQueue);
}

int
DVDGetStreamPlayAddrAsync (DVDCommandBlock* block, void (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    block->command = DVD_COMMAND_REQUEST_PLAY_ADDR;
    block->callback = callback;
    idle = issueCommand (1, block);
    return idle;
}

s32
DVDGetStreamPlayAddr (DVDCommandBlock* block)
{
    int result;
    s32 state;
    int enabled;
    s32 retVal;

    result = DVDGetStreamPlayAddrAsync (block, cbForGetStreamPlayAddrSync);
    if (result == 0)
    {
        return -1;
    }
    enabled = OSDisableInterrupts();

    while (1)
    {
        state = block->state;
        if (state != DVD_STATE_BUSY && state != DVD_STATE_WAITING)
        {
            retVal = ResultForSyncCommand;
            break;
        }
        OSSleepThread (&__DVDThreadQueue);
    }
    OSRestoreInterrupts (enabled);
    return retVal;
}

static void
cbForGetStreamPlayAddrSync (s32 result, DVDCommandBlock* block)
{
    ResultForSyncCommand = result;
    OSWakeupThread (&__DVDThreadQueue);
}

int
DVDGetStreamStartAddrAsync (DVDCommandBlock* block, void (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    block->command = DVD_COMMAND_REQUEST_START_ADDR;
    block->callback = callback;
    idle = issueCommand (1, block);
    return idle;
}

s32
DVDGetStreamStartAddr (DVDCommandBlock* block)
{
    int result;
    s32 state;
    int enabled;
    s32 retVal;

    result = DVDGetStreamStartAddrAsync (block, cbForGetStreamStartAddrSync);
    if (result == 0)
    {
        return -1;
    }
    enabled = OSDisableInterrupts();

    while (1)
    {
        state = block->state;
        if (state != DVD_STATE_BUSY && state != DVD_STATE_WAITING)
        {
            retVal = ResultForSyncCommand;
            break;
        }
        OSSleepThread (&__DVDThreadQueue);
    }
    OSRestoreInterrupts (enabled);
    return retVal;
}

static void
cbForGetStreamStartAddrSync (s32 result, DVDCommandBlock* block)
{
    ResultForSyncCommand = result;
    OSWakeupThread (&__DVDThreadQueue);
}

int
DVDGetStreamLengthAsync (DVDCommandBlock* block, void (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    block->command = DVD_COMMAND_REQUEST_LENGTH;
    block->callback = callback;
    idle = issueCommand (1, block);
    return idle;
}

s32
DVDGetStreamLength (DVDCommandBlock* block)
{
    int result;
    s32 state;
    int enabled;
    s32 retVal;

    result = DVDGetStreamLengthAsync (block, cbForGetStreamLengthSync);
    if (result == 0)
    {
        return -1;
    }
    enabled = OSDisableInterrupts();

    while (1)
    {
        state = block->state;
        if (state != DVD_STATE_BUSY && state != DVD_STATE_WAITING)
        {
            retVal = ResultForSyncCommand;
            break;
        }
        OSSleepThread (&__DVDThreadQueue);
    }
    OSRestoreInterrupts (enabled);
    return retVal;
}

static void
cbForGetStreamLengthSync (s32 result, DVDCommandBlock* block)
{
    block->transferredSize = result;
    OSWakeupThread (&__DVDThreadQueue);
}

void
__DVDAudioBufferConfig (DVDCommandBlock* block,
                        u32              enable,
                        u32              size,
                        void             (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    block->command = DVD_COMMAND_AUDIO_BUFFER_CONFIG;
    block->offset = enable;
    block->length = size;
    block->callback = callback;
    idle = issueCommand (2, block);
}

int
DVDChangeDiskAsyncForBS (DVDCommandBlock* block, void (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    ASSERTMSGLINE (0xA1F,
                   block,
                   "DVDChangeDiskAsyncForBS(): null pointer is specified to "
                   "command block address.");
    block->command = DVD_COMMAND_BS_CHANGE_DISK;
    block->callback = callback;
    idle = issueCommand (2, block);
    ASSERTMSGLINE (0xA25,
                   idle,
                   "DVDChangeDiskAsyncForBS(): command block is used for "
                   "processing previous request.");
    return idle;
}

int
DVDChangeDiskAsync (DVDCommandBlock* block,
                    DVDDiskID*       id,
                    void             (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    ASSERTMSGLINE (0xA3A,
                   block,
                   "DVDChangeDisk(): null pointer is specified to command "
                   "block address.");
    ASSERTMSGLINE (0xA3B, id, "DVDChangeDisk(): null pointer is specified to id address.");
    block->command = DVD_COMMAND_CHANGE_DISK;
    block->id = id;
    block->callback = callback;
    DCInvalidateRange (bootInfo->FSTLocation, bootInfo->FSTMaxLength);
    idle = issueCommand (2, block);
    ASSERTMSGLINE (0xA44,
                   idle,
                   "DVDChangeDisk(): command block is used for processing "
                   "previous request.");
    return idle;
}

s32
DVDChangeDisk (DVDCommandBlock* block, DVDDiskID* id)
{
    int result;
    s32 state;
    int enabled;
    s32 retVal;

    result = DVDChangeDiskAsync (block, id, cbForChangeDiskSync);
    if (result == 0)
    {
        return -1;
    }
    enabled = OSDisableInterrupts();
    while (1)
    {
        state = block->state;
        if (state == DVD_STATE_END)
        {
            retVal = 0;
            break;
        }
        else if (state == DVD_STATE_FATAL_ERROR)
        {
            retVal = -1;
            break;
        }
        OSSleepThread (&__DVDThreadQueue);
    }
    OSRestoreInterrupts (enabled);
    return retVal;
}

static void
cbForChangeDiskSync ()
{
    OSWakeupThread (&__DVDThreadQueue);
}

int
DVDInquiryAsync (DVDCommandBlock*     block,
                 struct DVDDriveInfo* info,
                 void                 (*callback) (s32, DVDCommandBlock*))
{
    int idle;

    ASSERTMSGLINE (0xA94, block, "DVDInquiry(): Null address was specified for block");
    ASSERTMSGLINE (0xA95, info, "DVDInquiry(): Null address was specified for info");
    ASSERTMSGLINE (0xA97,
                   !OFFSET (info, 32),
                   "DVDInquiry(): Address for info is not 32 bytes aligned");

    block->command = 0xE;
    block->addr = info;
    block->length = 0x20;
    block->transferredSize = 0;
    block->callback = callback;
    idle = issueCommand (2, block);
    return idle;
}

s32
DVDInquiry (DVDCommandBlock* block, struct DVDDriveInfo* info)
{
    int result;
    s32 state;
    int enabled;
    s32 retVal;

    result = DVDInquiryAsync (block, info, cbForInquirySync);
    if (result == 0)
    {
        return -1;
    }
    enabled = OSDisableInterrupts();
    while (1)
    {
        state = block->state;
        if (state != DVD_STATE_BUSY && state != DVD_STATE_WAITING)
        {
            retVal = ResultForSyncCommand;
            break;
        }
        OSSleepThread (&__DVDThreadQueue);
    }
    OSRestoreInterrupts (enabled);
    return retVal;
}

static void
cbForInquirySync (s32 result, DVDCommandBlock* block)
{
    ResultForSyncCommand = result;
    OSWakeupThread (&__DVDThreadQueue);
}

void
DVDReset ()
{
    DVDLowReset();
    __DIRegs[DI_SR] = 0x2A;
    __DIRegs[DI_CVR] = __DIRegs[DI_CVR];
    ResetRequired = 0;
    ResumeFromHere = 0;
}

int
DVDResetRequired ()
{
    return ResetRequired;
}

s32
DVDGetCommandBlockStatus (DVDCommandBlock* block)
{
    BOOL enabled;
    s32  retVal;
    s32  status;

    enabled = OSDisableInterrupts();

    ASSERTMSGLINE (0xAF9,
                   block,
                   "DVDGetCommandBlockStatus(): null pointer is specified to "
                   "command block address.");

    status = block->state;
    if (status == DVD_STATE_COVER_CLOSED)
    {
        retVal = DVD_STATE_BUSY;
    }
    else
    {
        retVal = status;
    }

    OSRestoreInterrupts (enabled);

    return retVal;
}

s32
DVDGetDriveStatus ()
{
    BOOL enabled;
    s32  retVal;

    enabled = OSDisableInterrupts();

    if (FatalErrorFlag != FALSE)
    {
        retVal = DVD_STATE_FATAL_ERROR;
    }
    else if (PausingFlag != FALSE)
    {
        retVal = DVD_STATE_PAUSING;
    }
    else
    {
        if (executing == (DVDCommandBlock*)NULL)
        {
            retVal = DVD_STATE_END;
        }
        else if (executing == &DummyCommandBlock)
        {
            retVal = DVD_STATE_END;
        }
        else
        {
            retVal = DVDGetCommandBlockStatus (executing);
        }
    }

    OSRestoreInterrupts (enabled);

    return retVal;
}

int
DVDSetAutoInvalidation (int autoInval)
{
    int prev;

    prev = autoInvalidation;
    autoInvalidation = autoInval;
    return prev;
}

void
DVDPause ()
{
    int level;

    level = OSDisableInterrupts();
    PauseFlag = 1;
    if (executing == NULL)
    {
        PausingFlag = 1;
    }
    OSRestoreInterrupts (level);
}

void
DVDResume ()
{
    int level;

    level = OSDisableInterrupts();
    PauseFlag = 0;
    if (PausingFlag != 0)
    {
        PausingFlag = 0;
        stateReady();
    }
    OSRestoreInterrupts (level);
}

int
DVDCancelAsync (DVDCommandBlock* block, void (*callback) (s32, DVDCommandBlock*))
{
    int  enabled;
    void (*old) (u32);

    enabled = OSDisableInterrupts();

    switch (block->state)
    {
        case DVD_STATE_FATAL_ERROR:
        case DVD_STATE_END:
        case DVD_STATE_CANCELED:
            if (callback)
            {
                callback (0, block);
            }
            break;
        case DVD_STATE_BUSY:
            if (Canceling != 0)
            {
                OSRestoreInterrupts (enabled);
                return 0;
            }
            Canceling = TRUE;
            CancelCallback = callback;
            if (block->command == DVD_COMMAND_BSREAD || block->command == DVD_COMMAND_READ)
            {
                DVDLowBreak();
            }
            break;
        case DVD_STATE_WAITING:
            __DVDDequeueWaitingQueue (block);
            block->state = DVD_STATE_CANCELED;
            if (block->callback)
            {
                block->callback (-3, block);
            }
            if (callback)
            {
                callback (0, block);
            }
            break;
        case DVD_STATE_COVER_CLOSED:
            switch (block->command)
            {
                case DVD_COMMAND_READID:
                case DVD_COMMAND_BSREAD:
                case DVD_COMMAND_AUDIO_BUFFER_CONFIG:
                case DVD_COMMAND_BS_CHANGE_DISK:
                    if (callback)
                    {
                        callback (0, block);
                    }
                    break;
                default:
                    if (Canceling)
                    {
                        OSRestoreInterrupts (enabled);
                        return FALSE;
                    }
                    Canceling = TRUE;
                    CancelCallback = callback;
                    break;
            }
            break;
        case DVD_STATE_NO_DISK:
        case DVD_STATE_COVER_OPEN:
        case DVD_STATE_WRONG_DISK:
        case DVD_STATE_MOTOR_STOPPED:
        case DVD_STATE_RETRY:
            old = DVDLowClearCallback();
            ASSERTLINE (0xBDB, old == cbForStateMotorStopped);
            if (old != cbForStateMotorStopped)
            {
                OSRestoreInterrupts (enabled);
                return 0;
            }
            if (block->state == DVD_STATE_NO_DISK)
            {
                ResumeFromHere = 3;
            }
            if (block->state == DVD_STATE_COVER_OPEN)
            {
                ResumeFromHere = 4;
            }
            if (block->state == DVD_STATE_WRONG_DISK)
            {
                ResumeFromHere = 1;
            }
            if (block->state == DVD_STATE_RETRY)
            {
                ResumeFromHere = 2;
            }
            if (block->state == DVD_STATE_MOTOR_STOPPED)
            {
                ResumeFromHere = 7;
            }
            block->state = DVD_STATE_CANCELED;
            if (block->callback)
            {
                block->callback (-3, block);
            }
            if (callback)
            {
                callback (0, block);
            }
            stateReady();
            break;
    }
    OSRestoreInterrupts (enabled);
    return 1;
}

s32
DVDCancel (volatile DVDCommandBlock* block)
{
    int result;
    s32 state;
    u32 command;
    int enabled;

    result = DVDCancelAsync ((void*)block, cbForCancelSync);
    if (result == 0)
    {
        return -1;
    }
    enabled = OSDisableInterrupts();
    while (1)
    {
        state = block->state;
        if (state == DVD_STATE_END || state == DVD_STATE_FATAL_ERROR ||
            state == DVD_STATE_CANCELED)
        {
            break;
        }
        if (state == DVD_STATE_COVER_CLOSED)
        {
            command = block->command;
            if ((command == DVD_COMMAND_BSREAD) || (command == DVD_COMMAND_READID) ||
                (command == DVD_COMMAND_AUDIO_BUFFER_CONFIG) ||
                (command == DVD_COMMAND_BS_CHANGE_DISK))
            {
                break;
            }
        }
        OSSleepThread (&__DVDThreadQueue);
    }
    OSRestoreInterrupts (enabled);
    return 0;
}

static void
cbForCancelSync ()
{
    OSWakeupThread (&__DVDThreadQueue);
}

int
DVDCancelAllAsync (void (*callback) (s32, DVDCommandBlock*))
{
    int              enabled;
    DVDCommandBlock* p;
    int              retVal;

    enabled = OSDisableInterrupts();
    DVDPause();
    while ((p = __DVDPopWaitingQueue())) { DVDCancelAsync (p, NULL); }
    if (executing)
    {
        retVal = DVDCancelAsync (executing, callback);
    }
    else
    {
        retVal = 1;
        if (callback)
        {
            callback (0, NULL);
        }
    }
    DVDResume();
    OSRestoreInterrupts (enabled);
    return retVal;
}

s32
DVDCancelAll ()
{
    int result;
    int enabled;

    enabled = OSDisableInterrupts();
    CancelAllSyncComplete = 0;
    result = DVDCancelAllAsync (cbForCancelAllSync);
    if (result == 0)
    {
        OSRestoreInterrupts (enabled);
        return -1;
    }
    while (1)
    {
        if (CancelAllSyncComplete == 0)
        {
            OSSleepThread (&__DVDThreadQueue);
        }
        else
        {
            break;
        }
    }
    OSRestoreInterrupts (enabled);
    return 0;
}

static void
cbForCancelAllSync ()
{
    CancelAllSyncComplete = 1;
    OSWakeupThread (&__DVDThreadQueue);
}

DVDDiskID*
DVDGetCurrentDiskID ()
{
    return (void*)OSPhysicalToCached (0);
}

BOOL
DVDCheckDisk ()
{
    BOOL enabled;
    s32  retVal;
    s32  state;
    s32  coverReg;

    enabled = OSDisableInterrupts();

    if (FatalErrorFlag != FALSE)
    {
        state = DVD_STATE_FATAL_ERROR;
    }
    else if (PausingFlag != FALSE)
    {
        state = DVD_STATE_PAUSING;
    }
    else
    {
        if (executing == (DVDCommandBlock*)NULL)
        {
            state = DVD_STATE_END;
        }
        else if (executing == &DummyCommandBlock)
        {
            state = DVD_STATE_END;
        }
        else
        {
            state = executing->state;
        }
    }

    switch (state)
    {
        case 1:
        case 9:
        case 10:
        case 2:
            retVal = TRUE;
            break;
        case -1:
        case 11:
        case 7:
        case 3:
        case 4:
        case 5:
        case 6:
            retVal = FALSE;
            break;
        case 0:
        case 8:
            coverReg = __DIRegs[DI_CVR];
            if (((coverReg >> 2) & 1) || (coverReg & 1))
            {
                retVal = FALSE;
            }
            else
            {
                retVal = TRUE;
            }
    }

    OSRestoreInterrupts (enabled);

    return retVal;
}

void
__DVDPrepareResetAsync (DVDCBCallback cb)
{
    BOOL enabled;

    enabled = OSDisableInterrupts();

    __DVDClearWaitingQueue();

    if (Canceling)
    {
        CancelCallback = cb;
    }
    else
    {
        if (executing)
        {
            executing->callback = NULL;
        }

        DVDCancelAllAsync (cb);
    }

    OSRestoreInterrupts (enabled);
}
