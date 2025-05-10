#ifndef _DOLPHIN_OSRTC_H_
#define _DOLPHIN_OSRTC_H_

#include <types.h>

#ifdef __cplusplus
extern "C"
{
#endif

// make the assert happy
#define OS_SOUND_MODE_MONO   0
#define OS_SOUND_MODE_STEREO 1

// make the asserts happy
#define OS_VIDEO_MODE_NTSC   0
#define OS_VIDEO_MODE_MPAL   2

typedef struct OSSram
{
    u16 checkSum;           ///< 0x00
    u16 checkSumInv;        ///< 0x02
    u32 ead0;               ///< 0x04
    u32 ead1;               ///< 0x08
    u32 counterBias;        ///< 0x0C
    s8  displayOffsetH;     ///< 0x10
    u8  ntd;                ///< 0x11
    u8  language;           ///< 0x12
    u8  flags;              ///< 0x13
} OSSram;

typedef struct OSSramEx
{
    u8  flashID[2][12];     ///< 0x00
    u32 wirelessKeyboardID; ///< 0x18
    u16 wirelessPadID[4];   ///< 0x1C
    u8  dvdErrorCode;       ///< 0x24
    u8  _padding0;          ///< 0x25
    u8  flashIDCheckSum[2]; ///< 0x26
    u8  _padding1[4];       ///< 0x28
} OSSramEx;

u32  OSGetSoundMode ();
void OSSetSoundMode (u32 mode);
u32  OSGetProgressiveMode ();
void OSSetProgressiveMode (u32 mode);
u32  OSGetVideoMode ();
void OSSetVideoMode (u32 mode);
u16  OSGetLanguage ();
void OSSetLanguage (u16 language);

#ifdef __cplusplus
}
#endif

#endif                      // _DOLPHIN_OSRTC_H_
