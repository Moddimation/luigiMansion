#include <dolphin/os.h>

#include "OSPrivate.h"

static OSResetCallback ResetCallback;

static BOOL Down;

static BOOL LastState;

static OSTime HoldUp;
static OSTime HoldDown;

void
__OSResetSWInterruptHandler (__OSInterrupt exception, OSContext* context)
{
#pragma unused(exception)
#pragma unused(context)

    OSResetCallback callback;

    HoldDown = __OSGetSystemTime();
    while (__OSGetSystemTime() - HoldDown < OSMicrosecondsToTicks (100) &&
           !(__PIRegs[PI_INTSR] & 0x00010000))
    {
        ;
    }
    if (!(__PIRegs[PI_INTSR] & 0x00010000))
    {
        LastState = Down = TRUE;
        __OSMaskInterrupts (OS_INTERRUPTMASK_PI_RSW);
        if (ResetCallback)
        {
            callback = ResetCallback;
            ResetCallback = NULL;
            callback();
        }
    }
    __PIRegs[PI_INTSR] = 2;
}

OSResetCallback
OSSetResetCallback (OSResetCallback callback)
{
    int             enabled;
    OSResetCallback prevCallback;

    enabled = OSDisableInterrupts();
    prevCallback = ResetCallback;
    ResetCallback = callback;

    if (callback)
    {
        __PIRegs[PI_INTSR] = 2;
        __OSUnmaskInterrupts (0x200);
    }
    else
    {
        __OSMaskInterrupts (0x200);
    }
    OSRestoreInterrupts (enabled);

    return prevCallback;
}

int
OSGetResetSwitchState ()
{
    BOOL enabled = OSDisableInterrupts();
    BOOL state;
    u32  reg = __PIRegs[PI_INTSR];

    if (!(reg & 0x00010000))
    {
        if (!Down)
        {
            Down = TRUE;
            state = HoldUp ? TRUE : FALSE;
            HoldDown = __OSGetSystemTime();
        }
        else
        {
            state = (HoldUp || (OSMicrosecondsToTicks (100) < __OSGetSystemTime() - HoldDown))
                        ? TRUE
                        : FALSE;
        }
    }
    else if (Down)
    {
        Down = FALSE;
        state = LastState;
        if (state)
        {
            HoldUp = __OSGetSystemTime();
        }
    }
    else if (HoldUp && (__OSGetSystemTime() - HoldUp < OSMillisecondsToTicks (40)))

    {
        state = TRUE;
    }
    else
    {
        state = FALSE;
        HoldUp = 0;
    }

    LastState = state;

    OSRestoreInterrupts (enabled);

    return state;
}
