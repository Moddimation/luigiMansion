#include <dolphin/os/OSSerial.h>
#include <dolphin/pad.h>
#include <dolphin/si.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../os/OSPrivate.h"
#include "pad_private.h"

static OSResetFunctionInfo ResetFunctionInfo = {
    OnReset,
    127,
    NULL,
    NULL,
};

static inline u16
GetWirelessID (s32 chan)
{
    OSSramEx* sram;
    u16       id;

    sram = __OSLockSramEx();
    id = sram->wirelessPadID[chan];
    __OSUnlockSramEx (0);
    return id;
}

static inline void
SetWirelessID (s32 chan, u16 id)
{
    OSSramEx* sram = __OSLockSramEx();

    if (sram->wirelessPadID[chan] != id)
    {
        sram->wirelessPadID[chan] = id;
        __OSUnlockSramEx (1);
        return;
    }
    __OSUnlockSramEx (0);
}

static int
DoReset ()
{
    int rc;
    u32 chanBit;

    rc = 1;
    ResettingChan = __cntlzw (ResettingBits);
    if ((ResettingChan >= 0) && (ResettingChan < 4))
    {
        memset (&Origin[ResettingChan], 0, 0xC);
        Type[ResettingChan] = 0;
        PADType[ResettingChan] = 0;
        rc = SITransfer (ResettingChan,
                         &cmdTypeAndStatus,
                         1,
                         &Type[ResettingChan],
                         3,
                         PADResetCallback,
                         0);
        chanBit = (0x80000000 >> ResettingChan);
        ResettingBits &= ~chanBit;
        if (rc == 0)
        {
            ResettingBits = 0;
            ResettingChan = 0x20;
        }
    }
    return rc;
}

static void
PADEnable (s32 chan)
{
    u32 cmd;
    u32 chanBit;
    u32 data[2];

    chanBit = 0x80000000 >> chan;
    EnabledBits |= chanBit;
    SIGetResponse (chan, &data);
    ASSERTLINE (0x1C4, !(Type[chan] & RES_WIRELESS_LITE));
    cmd = (AnalogMode | 0x400000);
    SISetCommand (chan, cmd);
    SIEnablePolling (EnabledBits);
}

static inline void
ProbeWireless (s32 chan)
{
    u32 cmd;
    u32 chanBit;
    u32 data[2];
    u32 type;
    u8  unused[4];
#pragma unused(unused)

    chanBit = 0x80000000 >> chan;
    EnabledBits |= chanBit;
    ProbingBits |= chanBit;
    SIGetResponse (chan, &data);
    type = Type[chan];
    if (!(type & 0x02000000))
    {
        cmd = (u32)((chan << 0xE) | 0x4D0000 | (__OSWirelessPadFixMode & 0x3FFF));
    }
    else if (((type & 0xC0000) + 0xFFFC0000) == 0)
    {
        cmd = 0x500000;
    }
    else
    {
        cmd = (type & 0x70000) + 0x440000;
    }
    SISetCommand (chan, cmd);
    SIEnablePolling (EnabledBits);
}

static void
PADProbeCallback (s32 chan, u32 error, OSContext* context)
{
#pragma unused(context)
    u32 type;

    ASSERTLINE (0x1F5, 0 <= ResettingChan && ResettingChan < SI_MAX_CHAN);
    ASSERTLINE (0x1F6, chan == ResettingChan);
    if (!(error &
          (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION)))
    {
        type = Type[chan];
        if (!(type & 0x80000) && !(type & 0x40000))
        {
            PADEnable (ResettingChan);
            WaitingBits |= PAD_CHAN0_BIT >> ResettingChan;
        }
        else
        {
            ProbeWireless (ResettingChan);
        }
    }
    DoReset();
}

static void
PADDisable (s32 chan)
{
    int enabled;
    u32 chanBit;

    enabled = OSDisableInterrupts();
    chanBit = 0x80000000 >> chan;
    SIDisablePolling (chanBit);
    EnabledBits &= ~chanBit;
    WaitingBits &= ~chanBit;
    CheckingBits &= ~chanBit;
    ProbingBits &= ~chanBit;
    SetWirelessID (chan, 0);
    OSRestoreInterrupts (enabled);
}

static void
UpdateOrigin (s32 chan)
{
    PADStatus* origin;
    u32        chanBit = 0x80000000 >> chan;

    origin = &Origin[chan];
    switch (AnalogMode & 0x00000700u)
    {
        case 0x00000000u:
        case 0x00000500u:
        case 0x00000600u:
        case 0x00000700u:
            origin->triggerLeft &= ~15;
            origin->triggerRight &= ~15;
            origin->analogA &= ~15;
            origin->analogB &= ~15;
            break;
        case 0x00000100u:
            origin->substickX &= ~15;
            origin->substickY &= ~15;
            origin->analogA &= ~15;
            origin->analogB &= ~15;
            break;
        case 0x00000200u:
            origin->substickX &= ~15;
            origin->substickY &= ~15;
            origin->triggerLeft &= ~15;
            origin->triggerRight &= ~15;
            break;
        case 0x00000300u:
            break;
        case 0x00000400u:
            break;
    }

    origin->stickX -= 128;
    origin->stickY -= 128;
    origin->substickX -= 128;
    origin->substickY -= 128;

    if (XPatchBits & chanBit)
    {
        if (64 < origin->stickX && (Type[chan] & 0xffff0000) == 0x9000000)
        {
            origin->stickX = 0;
        }
    }
}

static void
PADOriginCallback (s32 chan, u32 error, OSContext* context)
{
#pragma unused(chan)
#pragma unused(context)

    ASSERTLINE (0x267, 0 <= ResettingChan && ResettingChan < SI_MAX_CHAN);
    ASSERTLINE (0x268, chan == ResettingChan);
    if (!(error &
          (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION)))
    {
        UpdateOrigin (ResettingChan);
        PADEnable (ResettingChan);
    }
    DoReset();
}

static void
PADOriginUpdateCallback (s32 chan, u32 error, OSContext* context)
{
#pragma unused(context)

    ASSERTLINE (0x285, 0 <= chan && chan < SI_MAX_CHAN);
    if (!(EnabledBits & (PAD_CHAN0_BIT >> chan)))
    {
        return;
    }
    if (!(error &
          (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION)))
    {
        UpdateOrigin (chan);
    }
}

static void
PADFixCallback (s32 chan, u32 error, OSContext* context)
{
#pragma unused(chan)
#pragma unused(context)

    u32 type;
    u32 id;

    ASSERTLINE (0x2A9, 0 <= ResettingChan && ResettingChan < SI_MAX_CHAN);

    if (!(error & 0xF))
    {
        type = Type[ResettingChan];
        id = (u32)(GetWirelessID (ResettingChan) << 8);
        if (!(type & 0x100000) || ((id & 0xCFFF00u) != (type & 0xCFFF00)))
        {
            DoReset();
            return;
        }
        if ((type & 0x40000000) && !(type & 0x80000) && !(type & 0x40000))
        {
            SITransfer (ResettingChan,
                        &cmdReadOrigin,
                        1,
                        &Origin[ResettingChan],
                        offsetof (PADStatus, err),
                        PADOriginCallback,
                        0);
            return;
        }
        SITransfer (ResettingChan,
                    &cmdProbeDevice[ResettingChan],
                    3,
                    &Origin[ResettingChan],
                    8,
                    PADProbeCallback,
                    0);
        return;
    }
    DoReset();
}

u32 __PADFixBits;                                               // size: 0x4, address: 0x24

static void
PADResetCallback (s32 chan, u32 error, OSContext* context)
{
#pragma unused(chan)
#pragma unused(context)

    u32 type;
    u32 id;
    u32 chanBit;
    u32 recalibrate;
    int fix;

    ASSERTLINE (0x2E9, 0 <= ResettingChan && ResettingChan < SI_MAX_CHAN);

    error &= 0xF;
    if (error)
    {
        Type[ResettingChan] = 0;
    }

    PADType[ResettingChan] = type = Type[ResettingChan];
    chanBit = 0x80000000 >> ResettingChan;
    recalibrate = RecalibrateBits & chanBit;
    RecalibrateBits &= ~chanBit;
    fix = (int)(__PADFixBits & chanBit);
    __PADFixBits &= ~chanBit;
    if (error || (((type & 0x18000000) + 0xF8000000) != 0))
    {
        SetWirelessID (ResettingChan, 0);
        DoReset();
        return;
    }
    if (Spec < 2)
    {
        PADEnable (ResettingChan);
        DoReset();
        return;
    }
    if (!(type & 0x80000000) || (type & 0x04000000))
    {
        SetWirelessID (ResettingChan, 0);
        if (!(type & 0x01000000))
        {
            DoReset();
            return;
        }
        if (recalibrate != 0)
        {
            SITransfer (ResettingChan,
                        &cmdCalibrate,
                        3,
                        &Origin[ResettingChan],
                        offsetof (PADStatus, err),
                        PADOriginCallback,
                        0);
            return;
        }
        SITransfer (ResettingChan,
                    &cmdReadOrigin,
                    1,
                    &Origin[ResettingChan],
                    offsetof (PADStatus, err),
                    PADOriginCallback,
                    0);
        return;
    }
    id = (u32)(GetWirelessID (ResettingChan) << 8);
    if ((fix != 0) && (id & 0x100000))
    {
        cmdFixDevice[ResettingChan] = id & 0xcfff00 | 0x4e100000;
        SITransfer (ResettingChan,
                    &cmdFixDevice[ResettingChan],
                    3,
                    &Type[ResettingChan],
                    3,
                    PADFixCallback,
                    0);
        return;
    }
    if (type & 0x100000)
    {
        if ((id & 0xCFFF00) != (type & 0xCFFF00))
        {
            if (!(id & 0x100000))
            {
                id = (type & 0xCFFF00);
                id |= 0x100000;
                SetWirelessID (ResettingChan, (u16)((u16)(id >> 8) & 0xFFFFFF));
            }
            cmdFixDevice[ResettingChan] = id | 0x4e000000;
            SITransfer (ResettingChan,
                        &cmdFixDevice[ResettingChan],
                        3,
                        &Type[ResettingChan],
                        3,
                        PADFixCallback,
                        0);
            return;
        }
        if ((type & 0x40000000) && !(type & 0x80000) && !(type & 0x40000))
        {
            SITransfer (ResettingChan,
                        &cmdReadOrigin,
                        1,
                        &Origin[ResettingChan],
                        offsetof (PADStatus, err),
                        PADOriginCallback,
                        0);
            return;
        }
        SITransfer (ResettingChan,
                    &cmdProbeDevice[ResettingChan],
                    3,
                    &Origin[ResettingChan],
                    8,
                    PADProbeCallback,
                    0);
        return;
    }
    if (type & 0x40000000)
    {
        u32 id = (type & 0xCFFF00);
        id |= 0x100000;
        SetWirelessID (ResettingChan, (u16)((u16)(id >> 8) & 0xFFFFFF));
        cmdFixDevice[ResettingChan] = id | 0x4e000000;
        SITransfer (ResettingChan,
                    &cmdFixDevice[ResettingChan],
                    3,
                    &Type[ResettingChan],
                    3,
                    PADFixCallback,
                    0);
        return;
    }
    SetWirelessID (ResettingChan, 0);
    ProbeWireless (ResettingChan);
    DoReset();
}

int
PADReset (u32 mask)
{
    int enabled;
    int rc;

    rc = 0;
    ASSERTMSGLINE (0x392, !(mask & 0x0FFFFFFF), "PADReset(): invalid mask");
    enabled = OSDisableInterrupts();
    mask = mask & ~(CheckingBits | (ProbingBits | WaitingBits));
    ResettingBits |= mask;
    EnabledBits &= ~mask;
    WaitingBits &= ~mask;
    if (Spec == 4)
    {
        RecalibrateBits |= mask;
    }
    SIDisablePolling (ResettingBits);
    if (ResettingChan == 0x20)
    {
        rc = DoReset();
    }
    OSRestoreInterrupts (enabled);
    return rc;
}

BOOL
PADRecalibrate (u32 mask)
{
    BOOL intrEnabled;
    BOOL ret;

    ret = FALSE;
    ASSERTMSGLINE (0x3BD, !(mask & 0x0FFFFFFF), "PADReset(): invalid mask");
    intrEnabled = OSDisableInterrupts();
    mask &= ~(CheckingBits | (ProbingBits | WaitingBits));
    ResettingBits |= mask;
    EnabledBits &= ~mask;
    RecalibrateBits |= mask;
    SIDisablePolling (ResettingBits);
    if ((s32)ResettingChan == 32)
    {
        ret = DoReset();
    }
    OSRestoreInterrupts (intrEnabled);
    return ret;
}

u32 __PADSpec;                                                  // size: 0x4, address: 0x20

BOOL
PADInit ()
{
    s32 chan;

    if (!Initialized)
    {
        if (__PADSpec)
        {
            PADSetSpec (__PADSpec);
        }

        if (__PADFixBits == 0xFFFFFFFF)
        {
            OSTime time = OSGetTime();
            __OSWirelessPadFixMode = (u16)((((time) & 0xffff) + //
                                            ((time >> 16) & 0xffff) + //
                                            ((time >> 32) & 0xffff) + //
                                            ((time >> 48) & 0xffff)) &
                                           0x3fffu);
        }

        for (chan = 0; chan < SI_MAX_CHAN; ++chan)
        {
            cmdProbeDevice[chan] =
                (u32)(__OSWirelessPadFixMode & 0x3fff) << 8 | (0x4d000000 + (chan << 22));
        }
        Initialized = TRUE;

        PADSetSamplingRate (0);

        OSRegisterResetFunction (&ResetFunctionInfo);
    }
    return PADReset (PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT);
}

static void
PADReceiveCheckCallback (s32 chan, u32 error, OSContext* context)
{
    u32 type;
    u32 chanBit;
    u32 motor;
#pragma unused(motor)
#pragma unused(context)

    type = Type[chan];
    chanBit = 0x80000000 >> chan;
    WaitingBits &= ~chanBit;
    CheckingBits &= ~chanBit;
    if (EnabledBits & chanBit)
    {
        if (!(error & 0xF) && (type & 0x80000000) && (type & 0x02000000) &&
            (type & 0x40000000) && !(type & 0x04000000))
        {
            SITransfer (chan,
                        &cmdReadOrigin,
                        1,
                        &Origin[chan],
                        offsetof (PADStatus, err),
                        PADOriginUpdateCallback,
                        0);
            return;
        }
        PADDisable (chan);
    }
}

u32
PADRead (struct PADStatus* status)
{
    s32  chan;
    u32  data[2];
    u32  chanBit;
    u32  sr;
    int  chanShift;
    BOOL enabled;
    u32  motor;

    motor = 0;
    for (chan = 0; chan < SI_MAX_CHAN; chan++, status++)
    {
        chanBit = 0x80000000 >> chan;
        chanShift = 8 * (SI_MAX_CHAN - 1 - chan);

        if ((ResettingBits & chanBit) || (ResettingChan == chan))
        {
            status->err = -2;
            memset (status, 0, offsetof (PADStatus, err));
            continue;
        }
        if (!(EnabledBits & chanBit))
        {
            status->err = -1;
            memset (status, 0, offsetof (PADStatus, err));
            continue;
        }

        sr = SIGetStatus();
        if (sr & (8 << chanShift))
        {
            if (WaitingBits & chanBit)
            {
                status->err = 0;
                memset (status, 0, offsetof (PADStatus, err));
                if ((CheckingBits & chanBit))
                {
                    continue;
                }
                enabled = OSDisableInterrupts();
                if (SITransfer (chan,
                                &cmdTypeAndStatus,
                                1,
                                &Type[chan],
                                3,
                                PADReceiveCheckCallback,
                                0) != 0)
                {
                    CheckingBits |= chanBit;
                }
                OSRestoreInterrupts (enabled);
                continue;
            }
            PADDisable (chan);
            status->err = -1;
            memset (status, 0, offsetof (PADStatus, err));
            continue;
        }
        if (!(ProbingBits & chanBit) && !(Type[chan] & 0x20000000))
        {
            motor |= chanBit;
        }
        if (!(sr & (0x20 << chanShift)))
        {
            status->err = -3;
            memset (status, 0, offsetof (PADStatus, err));
            continue;
        }

        SIGetResponse (chan, &data);
        if (data[0] & 0x80000000)
        {
            status->err = -3;
            memset (status, 0, offsetof (PADStatus, err));
            continue;
        }
        if (ProbingBits & chanBit)
        {
            status->err = -1;
            memset (status, 0, offsetof (PADStatus, err));
            continue;
        }

        MakeStatus (chan, status, data);
        if (status->button & 0x2000)
        {
            status->err = -3;
            memset (status, 0, offsetof (PADStatus, err));
            SITransfer (chan, &cmdReadOrigin, 1, &Origin[chan], 10, PADOriginUpdateCallback, 0);
            continue;
        }

        status->err = 0;

        status->button &= 0xFFFFFF7F;
        continue;
    }
    return motor;
}

typedef struct XY
{
    u8 line;                                                          ///< 0x00
    u8 count;                                                         ///< 0x01
} XY;

static XY XYNTSC[12] = {
    { 0xF7, 0x02 },
    { 0x0E, 0x13 },
    { 0x1D, 0x09 },
    { 0x25, 0x07 },
    { 0x34, 0x05 },
    { 0x41, 0x04 },
    { 0x57, 0x03 },
    { 0x57, 0x03 },
    { 0x57, 0x03 },
    { 0x83, 0x02 },
    { 0x83, 0x02 },
    { 0x83, 0x02 }
};

static XY XYPAL[12] = {
    { 0x94, 0x03 },
    { 0x0D, 0x18 },
    { 0x1A, 0x0C },
    { 0x27, 0x08 },
    { 0x34, 0x06 },
    { 0x3E, 0x05 },
    { 0x4E, 0x04 },
    { 0x68, 0x03 },
    { 0x68, 0x03 },
    { 0x68, 0x03 },
    { 0x68, 0x03 },
    { 0x9C, 0x02 }
};

void
PADSetSamplingRate (u32 msec)
{
    u32 tv;
    XY* xy;

    ASSERTMSGLINE (0x4CE, (msec <= 0xB), "PADSetSamplingRate(): out of rage (0 <= msec <= 11)");
    if (msec > 0xB)
    {
        msec = 0xB;
    }
    tv = VIGetTvFormat();
    switch (tv)
    {
        case VI_NTSC:
        case VI_MPAL:
            xy = XYNTSC;
            break;
        case VI_PAL:
            xy = XYPAL;
            break;
        default:
            OSPanic ("Pad.c", 0x510, "PADSetSamplingRate: unknown TV format");
    }
    SISetXY (xy[msec].line, xy[msec].count);
    SIEnablePolling (EnabledBits);
}
#if DEBUG
void
__PADTestSamplingRate (u32 tvmode)
{
    u32        msec;
    u32        line;
    u32        count;
    struct XY* xy;

    switch (tvmode)
    {
        case VI_NTSC:
        case VI_MPAL:
            xy = XYNTSC;
            for (msec = 0; msec <= 0xB; msec++)
            {
                line = xy[msec].line;
                count = xy[msec].count;
                OSReport (
                    "%2d[msec]: count %3d, line %3d, last %3d, diff0 %2d.%03d, "
                    "diff1 %2d.%03d\n",
                    msec,
                    count,
                    line,
                    (line * (count - 1)) + LATENCY,
                    (line * 636) / 10000,
                    (line * 636) % 10000,
                    636 * (263 - line * (count - 1)) / 10000,
                    636 * (263 - line * (count - 1)) % 10000);
                ASSERTLINE (0x505, line * (count - 1) + LATENCY < 263);
                if (msec != 0)
                {
                    ASSERTLINE (0x508, 636 * line < msec * 10000);
                    ASSERTLINE (0x509, 636 * (263 - line * (count - 1)) < msec * 10000);
                }
            }
            break;
        case VI_PAL:
            xy = XYPAL;
            for (msec = 0; msec <= 0xB; msec++)
            {
                line = xy[msec].line;
                count = xy[msec].count;
                OSReport (
                    "%2d[msec]: count %3d, line %3d, last %3d, diff0 %2d.%03d, "
                    "diff1 %2d.%03d\n",
                    msec,
                    count,
                    line,
                    (line * (count - 1)) + LATENCY,
                    (line * 640) / 10000,
                    (line * 640) % 10000,
                    640 * (313 - line * (count - 1)) / 10000,
                    640 * (313 - line * (count - 1)) % 10000);
                ASSERTLINE (0x51D, line * (count - 1) + LATENCY < 313);
                if (msec != 0)
                {
                    ASSERTLINE (0x520, 640 * line < msec * 10000);
                    ASSERTLINE (0x521, 640 * (313 - line * (count - 1)) < msec * 10000);
                }
            }
            break;
    }
}
#endif
void
PADControlAllMotors (const u32* commandArray)
{
    BOOL enabled;
    int  chan;
    u32  command;
    BOOL commit;
    u32  chanBit;

    enabled = OSDisableInterrupts();
    commit = FALSE;

    for (chan = 0; chan < SI_MAX_CHAN; chan++, commandArray++)
    {
        chanBit = PAD_CHAN0_BIT >> chan;
        if ((EnabledBits & chanBit) && !(ProbingBits & chanBit) && !(Type[chan] & 0x20000000))
        {
            command = *commandArray;
            ASSERTMSGLINE (0x545,
                           !(command & 0xFFFFFFFC),
                           "PADControlAllMotors(): invalid command");
            if (Spec < PAD_SPEC_2 && command == PAD_MOTOR_STOP_HARD)
            {
                command = PAD_MOTOR_STOP;
            }
            SISetCommand (chan,
                          (0x40 << 16) | AnalogMode | (command & (0x00000001 | 0x00000002)));
            commit = TRUE;
        }
    }
    if (commit)
    {
        SITransferCommands();
    }
    OSRestoreInterrupts (enabled);
}

void
PADControlMotor (s32 chan, u32 command)
{
    BOOL enabled;
    u32  chanBit;

    ASSERTMSGLINE (0x568, !(command & 0xFFFFFFFC), "PADControlMotor(): invalid command");

    enabled = OSDisableInterrupts();
    chanBit = PAD_CHAN0_BIT >> chan;
    if ((EnabledBits & chanBit) && !(ProbingBits & chanBit) && !(Type[chan] & 0x20000000))
    {
        if (Spec < PAD_SPEC_2 && command == PAD_MOTOR_STOP_HARD)
        {
            command = PAD_MOTOR_STOP;
        }
        SISetCommand (chan, (0x40 << 16) | AnalogMode | (command & (0x00000001 | 0x00000002)));
        SITransferCommands();
    }
    OSRestoreInterrupts (enabled);
}

void
PADSetSpec (u32 spec)
{
    ASSERTLINE (0x58C, !Initialized);
    __PADSpec = 0;
    switch (spec)
    {
        case PAD_SPEC_0:
            MakeStatus = SPEC0_MakeStatus;
            break;
        case PAD_SPEC_1:
            MakeStatus = SPEC1_MakeStatus;
            break;
        case PAD_SPEC_2:
        case PAD_SPEC_3:
        case PAD_SPEC_4:
        case PAD_SPEC_5:
            MakeStatus = SPEC2_MakeStatus;
            break;
    }
    Spec = spec;
}

u32
PADGetSpec (void)
{
    return Spec;
}

static void
SPEC0_MakeStatus (s32 chan, PADStatus* status, u32 data[2])
{
#pragma unused(chan)

    status->button = 0;
    status->button |= ((data[0] >> 16) & 0x0008) ? PAD_BUTTON_A : 0;
    status->button |= ((data[0] >> 16) & 0x0020) ? PAD_BUTTON_B : 0;
    status->button |= ((data[0] >> 16) & 0x0100) ? PAD_BUTTON_X : 0;
    status->button |= ((data[0] >> 16) & 0x0001) ? PAD_BUTTON_Y : 0;
    status->button |= ((data[0] >> 16) & 0x0010) ? PAD_BUTTON_START : 0;
    status->stickX = (s8)(data[1] >> 16);
    status->stickY = (s8)(data[1] >> 24);
    status->substickX = (s8)(data[1]);
    status->substickY = (s8)(data[1] >> 8);
    status->triggerLeft = (u8)(data[0] >> 8);
    status->triggerRight = (u8)data[0];
    status->analogA = 0;
    status->analogB = 0;
    if (170 <= status->triggerLeft)
    {
        status->button |= PAD_TRIGGER_L;
    }
    if (170 <= status->triggerRight)
    {
        status->button |= PAD_TRIGGER_R;
    }
    status->stickX -= 128;
    status->stickY -= 128;
    status->substickX -= 128;
    status->substickY -= 128;
}

static void
SPEC1_MakeStatus (s32 chan, PADStatus* status, u32 data[2])
{
#pragma unused(chan)

    status->button = 0;
    status->button |= ((data[0] >> 16) & 0x0080) ? PAD_BUTTON_A : 0;
    status->button |= ((data[0] >> 16) & 0x0100) ? PAD_BUTTON_B : 0;
    status->button |= ((data[0] >> 16) & 0x0020) ? PAD_BUTTON_X : 0;
    status->button |= ((data[0] >> 16) & 0x0010) ? PAD_BUTTON_Y : 0;
    status->button |= ((data[0] >> 16) & 0x0200) ? PAD_BUTTON_START : 0;

    status->stickX = (s8)(data[1] >> 16);
    status->stickY = (s8)(data[1] >> 24);
    status->substickX = (s8)(data[1]);
    status->substickY = (s8)(data[1] >> 8);

    status->triggerLeft = (u8)(data[0] >> 8);
    status->triggerRight = (u8)data[0];

    status->analogA = 0;
    status->analogB = 0;

    if (170 <= status->triggerLeft)
    {
        status->button |= PAD_TRIGGER_L;
    }
    if (170 <= status->triggerRight)
    {
        status->button |= PAD_TRIGGER_R;
    }

    status->stickX -= 128;
    status->stickY -= 128;
    status->substickX -= 128;
    status->substickY -= 128;
}

static s8
ClampS8 (s8 var, s8 org)
{
    if (0 < org)
    {
        s8 min = (s8)(-128 + org);
        if (var < min)
        {
            var = min;
        }
    }
    else if (org < 0)
    {
        s8 max = (s8)(127 + org);
        if (max < var)
        {
            var = max;
        }
    }
    return var -= org;
}

static u8
ClampU8 (u8 var, u8 org)
{
    if (var < org)
    {
        var = org;
    }
    return var -= org;
}

static void
SPEC2_MakeStatus (s32 chan, PADStatus* status, u32 data[2])
{
    PADStatus* origin;

    status->button = (u16)((data[0] >> 16) & PAD_ALL);
    status->stickX = (s8)(data[0] >> 8);
    status->stickY = (s8)(data[0]);

    switch (AnalogMode & 0x00000700)
    {
        case 0x00000000:
        case 0x00000500:
        case 0x00000600:
        case 0x00000700:
            status->substickX = (s8)(data[1] >> 24);
            status->substickY = (s8)(data[1] >> 16);
            status->triggerLeft = (u8)(((data[1] >> 12) & 0x0f) << 4);
            status->triggerRight = (u8)(((data[1] >> 8) & 0x0f) << 4);
            status->analogA = (u8)(((data[1] >> 4) & 0x0f) << 4);
            status->analogB = (u8)(((data[1] >> 0) & 0x0f) << 4);
            break;
        case 0x00000100:
            status->substickX = (s8)(((data[1] >> 28) & 0x0f) << 4);
            status->substickY = (s8)(((data[1] >> 24) & 0x0f) << 4);
            status->triggerLeft = (u8)(data[1] >> 16);
            status->triggerRight = (u8)(data[1] >> 8);
            status->analogA = (u8)(((data[1] >> 4) & 0x0f) << 4);
            status->analogB = (u8)(((data[1] >> 0) & 0x0f) << 4);
            break;
        case 0x00000200:
            status->substickX = (s8)(((data[1] >> 28) & 0x0f) << 4);
            status->substickY = (s8)(((data[1] >> 24) & 0x0f) << 4);
            status->triggerLeft = (u8)(((data[1] >> 20) & 0x0f) << 4);
            status->triggerRight = (u8)(((data[1] >> 16) & 0x0f) << 4);
            status->analogA = (u8)(data[1] >> 8);
            status->analogB = (u8)(data[1] >> 0);
            break;
        case 0x00000300:
            status->substickX = (s8)(data[1] >> 24);
            status->substickY = (s8)(data[1] >> 16);
            status->triggerLeft = (u8)(data[1] >> 8);
            status->triggerRight = (u8)(data[1] >> 0);
            status->analogA = 0;
            status->analogB = 0;
            break;
        case 0x00000400:
            status->substickX = (s8)(data[1] >> 24);
            status->substickY = (s8)(data[1] >> 16);
            status->triggerLeft = 0;
            status->triggerRight = 0;
            status->analogA = (u8)(data[1] >> 8);
            status->analogB = (u8)(data[1] >> 0);
            break;
    }

    status->stickX -= 128;
    status->stickY -= 128;
    status->substickX -= 128;
    status->substickY -= 128;

    origin = &Origin[chan];
    status->stickX = ClampS8 (status->stickX, origin->stickX);
    status->stickY = ClampS8 (status->stickY, origin->stickY);
    status->substickX = ClampS8 (status->substickX, origin->substickX);
    status->substickY = ClampS8 (status->substickY, origin->substickY);
    status->triggerLeft = ClampU8 (status->triggerLeft, origin->triggerLeft);
    status->triggerRight = ClampU8 (status->triggerRight, origin->triggerRight);
}

int
PADGetType (s32 chan, u32* type)
{
    u32 chanBit;

    *type = Type[chan];
    chanBit = 0x80000000 >> chan;
    if (ResettingBits & chanBit || ResettingChan == chan || !(EnabledBits & chanBit))
    {
        return 0;
    }
    return 1;
}

BOOL
PADSync (void)
{
    return ResettingBits == 0 && (s32)ResettingChan == 32 && !SIBusy();
}

void
PADSetAnalogMode (u32 mode)
{
    BOOL enabled;
    u32  mask;

    ASSERTMSGLINE (0x6C9, (mode < 8), "PADSetAnalogMode(): invalid mode");

    enabled = OSDisableInterrupts();
    AnalogMode = mode << 8;
    mask = EnabledBits & ~ProbingBits;

    EnabledBits &= ~mask;
    WaitingBits &= ~mask;
    CheckingBits &= ~mask;

    SIDisablePolling (mask);
    OSRestoreInterrupts (enabled);
}

static BOOL
OnReset (BOOL f)
{
    BOOL        sync;
    static BOOL recalibrated = FALSE;

    if (!f)
    {
        sync = PADSync();
        if (!recalibrated && sync)
        {
            recalibrated =
                PADRecalibrate (PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT);
            return FALSE;
        }
        return sync;
    }
    else
    {
        recalibrated = FALSE;
    }

    return TRUE;
}
