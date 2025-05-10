
#include <dolphin/exi.h>
#include <dolphin/os.h>

// BOOLernal include
#include "OSPrivate.h"

#define RTC_SRAM_SIZE 64

struct SramControl
{
    u8   sram[RTC_SRAM_SIZE]; ///< 0x00
    u32  offset;              ///< 0x40
    BOOL enabled;             ///< 0x44
    BOOL locked;              ///< 0x48
    BOOL sync;                ///< 0x4C
    void (*callback)();       ///< 0x50
};
static struct SramControl Scb ATTRIBUTE_ALIGN (32);

static BOOL  GetRTC (u32* rtc);
static BOOL  ReadSram (void* buffer);
static void  WriteSramCallback ();
static BOOL  WriteSram (void* buffer, u32 offset, u32 size);
static void* LockSram (u32 offset);
static BOOL  UnlockSram (BOOL commit, u32 offset);
static void  __OSReadROMCallback (s32 chan);

static BOOL
GetRTC (u32* rtc)
{
    BOOL err;
    u32  cmd;

    if (!EXILock (0, 1, NULL))
    {
        return FALSE;
    }
    if (!EXISelect (0, 1, 3))
    {
        EXIUnlock (0);
        return FALSE;
    }
    cmd = 0x20000000;
    err = FALSE;
    err |= !EXIImm (0, &cmd, 4, 1, 0);
    err |= !EXISync (0);
    err |= !EXIImm (0, &cmd, 4, 0, 0);
    err |= !EXISync (0);
    err |= !EXIDeselect (0);
    EXIUnlock (0);
    rtc[0] = cmd;
    return !err;
}

BOOL
__OSGetRTC (u32* rtc)
{
    BOOL err;
    u32  t0;
    u32  t1;
    BOOL i;

    for (i = 0; i < 16; i++)
    {
        err = FALSE;
        err |= !GetRTC (&t0);
        err |= !GetRTC (&t1);
        if (err)
        {
            break;
        }
        if (t0 == t1)
        {
            rtc[0] = t0;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
__OSSetRTC (u32 rtc)
{
    BOOL err;
    u32  cmd;

    if (!EXILock (0, 1, NULL))
    {
        return FALSE;
    }
    if (!EXISelect (0, 1, 3))
    {
        EXIUnlock (0);
        return FALSE;
    }
    cmd = 0xA0000000;
    err = FALSE;
    err |= !EXIImm (0, &cmd, 4, 1, 0);
    err |= !EXISync (0);
    err |= !EXIImm (0, &rtc, 4, 1, 0);
    err |= !EXISync (0);
    err |= !EXIDeselect (0);
    EXIUnlock (0);
    return !err;
}

static BOOL
ReadSram (void* buffer)
{
    BOOL err;
    u32  cmd;

    DCInvalidateRange (buffer, RTC_SRAM_SIZE);
    if (!EXILock (0, 1, NULL))
    {
        return FALSE;
    }
    if (!EXISelect (0, 1, 3))
    {
        EXIUnlock (0);
        return FALSE;
    }
    cmd = 0x20000100;
    err = FALSE;
    err |= !EXIImm (0, &cmd, 4, 1, 0);
    err |= !EXISync (0);
    err |= !EXIDma (0, buffer, RTC_SRAM_SIZE, 0, NULL);
    err |= !EXISync (0);
    err |= !EXIDeselect (0);
    EXIUnlock (0);
    return !err;
}

static void
WriteSramCallback ()
{
    BOOL unused;
#pragma unused(unused)

    ASSERTLINE (0xF0, !Scb.locked);
    Scb.sync = WriteSram (&Scb.sram[Scb.offset], Scb.offset, RTC_SRAM_SIZE - Scb.offset);
    if (Scb.sync)
    {
        Scb.offset = RTC_SRAM_SIZE;
    }
    ASSERTLINE (0xF6, Scb.sync);
}

static BOOL
WriteSram (void* buffer, u32 offset, u32 size)
{
    BOOL err;
    u32  cmd;

    if (!EXILock (0, 1, WriteSramCallback))
    {
        return FALSE;
    }
    if (!EXISelect (0, 1, 3))
    {
        EXIUnlock (0);
        return FALSE;
    }
    offset <<= 6;
    cmd = ((offset + 0x100) | 0xA0000000);
    err = FALSE;
    err |= !EXIImm (0, &cmd, 4, 1, 0);
    err |= !EXISync (0);
    err |= !EXIImmEx (0, buffer, (s32)size, 1);
    err |= !EXIDeselect (0);
    EXIUnlock (0);
    return !err;
}

void
__OSInitSram ()
{
    Scb.locked = Scb.enabled = 0;
    Scb.sync = ReadSram (&Scb);
    ASSERTLINE (0x12C, Scb.sync);
    Scb.offset = RTC_SRAM_SIZE;
}

static void*
LockSram (u32 offset)
{
    BOOL enabled;

    enabled = OSDisableInterrupts();
    ASSERTLINE (0x140, !Scb.locked);
    if (Scb.locked)
    {
        OSRestoreInterrupts (enabled);
        return NULL;
    }
    Scb.enabled = enabled;
    Scb.locked = TRUE;
    return &Scb.sram[offset];
}

OSSram*
__OSLockSram ()
{
    return (OSSram*)LockSram (0);
}

OSSramEx*
__OSLockSramEx (void)
{
    return (OSSramEx*)LockSram (0x14);
}

static BOOL
UnlockSram (BOOL commit, u32 offset)
{
    u16* p;

    ASSERTLINE (0x162, Scb.locked);
    if (commit)
    {
        if (!offset)
        {
            OSSram* sram = (struct OSSram*)&Scb.sram;
            if ((sram->flags & 3) > 2U)
            {
                sram->flags &= ~3;
            }
            sram->checkSum = sram->checkSumInv = 0;
            for (p = (u16*)&sram->counterBias; p < ((u16*)&Scb.sram[sizeof (OSSram)]); p++)
            {
                sram->checkSum += *p;
                sram->checkSumInv += ~(*p);
            }
        }
        if (offset < Scb.offset)
        {
            Scb.offset = offset;
        }
        Scb.sync = WriteSram (Scb.sram + Scb.offset, Scb.offset, RTC_SRAM_SIZE - Scb.offset);
        if (Scb.sync)
        {
            Scb.offset = RTC_SRAM_SIZE;
        }
    }
    Scb.locked = FALSE;
    OSRestoreInterrupts (Scb.enabled);
    return Scb.sync;
}

BOOL
__OSUnlockSram (BOOL commit)
{
    return UnlockSram (commit, 0);
}

BOOL
__OSUnlockSramEx (BOOL commit)
{
    return UnlockSram (commit, 0x14);
}

BOOL
__OSSyncSram ()
{
    return Scb.sync;
}

BOOL
__OSCheckSram ()
{
    u16*    p;
    u16     checkSum;
    u16     checkSumInv;
    OSSram* sram;
    BOOL    unused;
#pragma unused(unused)

    ASSERTLINE (0x1A9, Scb.locked);

    checkSum = checkSumInv = 0;

    sram = (OSSram*)&Scb.sram[0];

    for (p = (void*)&sram->counterBias; p < (u16*)&Scb.sram[0x14]; p++)
    {
        checkSum += *p;
        checkSumInv += ~(*p);
    }

    return (sram->checkSum == checkSum && sram->checkSumInv == checkSumInv);
}

BOOL
__OSReadROM (void* buffer, s32 length, long offset)
{
    BOOL err;
    u32  cmd;

    ASSERTLINE (0x1C8, length <= 1024);
    DCInvalidateRange (buffer, (u32)length);
    if (!EXILock (0, 1, NULL))
    {
        return FALSE;
    }
    if (!EXISelect (0, 1, 3))
    {
        EXIUnlock (0);
        return FALSE;
    }
    cmd = (u32)(offset << 6);
    err = FALSE;
    err |= !EXIImm (0, &cmd, 4, 1, 0);
    err |= !EXISync (0);
    err |= !EXIDma (0, buffer, length, 0, NULL);
    err |= !EXISync (0);
    err |= !EXIDeselect (0);
    EXIUnlock (0);
    return !err;
}

static void
__OSReadROMCallback (s32 chan)
{
    void (*callback)();

    EXIDeselect (chan);
    EXIUnlock (chan);
    callback = Scb.callback;
    if (callback)
    {
        Scb.callback = NULL;
        callback();
    }
}

BOOL
__OSReadROMAsync (void* buffer, s32 length, long offset, void (*callback)())
{
    BOOL err;
    u32  cmd;

    ASSERTLINE (0x203, length <= 1024);
    ASSERTLINE (0x204, callback);
    DCInvalidateRange (buffer, (u32)length);
    Scb.callback = callback;
    if (!EXILock (0, 1, NULL))
    {
        return FALSE;
    }
    if (!EXISelect (0, 1, 3))
    {
        EXIUnlock (0);
        return FALSE;
    }
    cmd = (u32)(offset << 6);
    err = FALSE;
    err |= !EXIImm (0, &cmd, 4, 1, 0);
    err |= !EXISync (0);
    err |= !EXIDma (0, buffer, length, 0, (void*)__OSReadROMCallback);
    return !err;
}

u32
OSGetSoundMode ()
{
    OSSram* sram = __OSLockSram();
    u32     mode = (u32)(sram->flags & 4) ? (u32)1 : (u32)0;

    __OSUnlockSram (FALSE);
    return mode;
}

void
OSSetSoundMode (u32 mode)
{
    OSSram* sram;
    BOOL    unused;
#pragma unused(unused)

    ASSERTLINE (0x22A, mode == OS_SOUND_MODE_MONO || mode == OS_SOUND_MODE_STEREO);
    mode *= 4;
    mode &= 4;
    sram = __OSLockSram();
    if (mode == (sram->flags & 4))
    {
        __OSUnlockSram (FALSE);
        return;
    }
    sram->flags &= 0xFFFFFFFB;
    sram->flags |= mode;
    __OSUnlockSram (TRUE);
}

u32
OSGetProgressiveMode ()
{
    OSSram* sram;
    BOOL    mode;

    sram = __OSLockSram();
    mode = (sram->flags & 0x80) ? TRUE : FALSE;
    __OSUnlockSram (FALSE);
    return (u32)mode;
}

void
OSSetProgressiveMode (u32 mode)
{
    char    trash[0x2];       // TODO: intermediate vars or inlines?
    OSSram* sram;
#pragma unused(trash)

    mode <<= 7;
    mode &= 0x80;

    sram = __OSLockSram();
    if (mode == (sram->flags & 0x80))
    {
        __OSUnlockSram (FALSE);
        return;
    }

    sram->flags &= ~0x80;
    sram->flags |= mode;
    __OSUnlockSram (TRUE);
}

u32
OSGetVideoMode ()
{
    OSSram* sram = __OSLockSram();
    u32     mode = (u32)(sram->flags & 3);

    __OSUnlockSram (FALSE);
    return mode;
}

void
OSSetVideoMode (u32 mode)
{
    OSSram* sram;
    BOOL    unused;
#pragma unused(unused)

    ASSERTLINE (0x249, OS_VIDEO_MODE_NTSC <= mode && mode <= OS_VIDEO_MODE_MPAL);

    mode &= 3;
    sram = __OSLockSram();
    if (mode == (sram->flags & 3))
    {
        __OSUnlockSram (FALSE);
        return;
    }
    sram->flags &= 0xFFFFFFFC;
    sram->flags |= mode;
    __OSUnlockSram (TRUE);
}

u16
OSGetLanguage ()
{
    OSSram* sram = __OSLockSram();
    u16     language = sram->language;

    __OSUnlockSram (FALSE);
    return language;
}

void
OSSetLanguage (u16 language)
{
    OSSram* sram = __OSLockSram();
    BOOL    unused;
#pragma unused(unused)

    if (language == sram->language)
    {
        __OSUnlockSram (FALSE);
        return;
    }
    sram->language = (u8)language;
    __OSUnlockSram (TRUE);
}

u16
__OSGetBootMode ()
{
    OSSram* sram = __OSLockSram();
    u16     ntd = sram->ntd;

    __OSUnlockSram (FALSE);

    return (u16)(ntd & 0x80);
}

void
__OSSetBootMode (u16 ntd)
{
    OSSram* sram;
    BOOL    unused;
#pragma unused(unused)

    ntd &= 0x80;
    sram = __OSLockSram();
    if (ntd == (sram->ntd & 0x80U))
    {
        __OSUnlockSram (FALSE);
        return;
    }
    sram->ntd &= 0xFFFFFF7F;
    sram->ntd |= ntd;
    __OSUnlockSram (TRUE);
}
