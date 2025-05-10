#ifndef _DOLPHIN_CARD_H_
#define _DOLPHIN_CARD_H_

#include <dolphin/dsp.h>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CARD_WORKAREA_SIZE  (5 * 8 * 1024)
#define CARD_READ_SIZE      512
#define CARD_MAX_FILE       127
#define CARD_COMMENT_SIZE   64

// internal API command xfer bytes
#define CARD_XFER_CREATE    (2 * 8 * 1024) // CARDCreate[Async]
#define CARD_XFER_DELETE    (2 * 8 * 1024) // CARD[Fast]Delete[Async]
#define CARD_XFER_MOUNT     (5 * 8 * 1024) // CARDMount[Async]
#define CARD_XFER_FORMAT    (5 * 8 * 1024) // CARDFormat[Async]
#define CARD_XFER_SETSTATUS (1 * 8 * 1024) // CARDSetStatus[Async]
#define CARD_XFER_RENAME    (1 * 8 * 1024) // CARDRename[Async]

#define CARD_ENCODE_ANSI    OS_FONT_ENCODE_ANSI
#define CARD_ENCODE_SJIS    OS_FONT_ENCODE_SJIS

// s32est file name string excluding terminating zero
#define CARD_FILENAME_MAX   32

#define CARD_ICON_MAX       8
#define CARD_ICON_WIDTH     32
#define CARD_ICON_HEIGHT    32
#define CARD_BANNER_WIDTH   96
#define CARD_BANNER_HEIGHT  32

// todo: sort into headers
typedef struct CARDFileInfo
{
    s32 chan;   ///< 0x00
    s32 fileNo; ///< 0x04

    s32 offset; ///< 0x08
    s32 length; ///< 0x0C
    u16 iBlock; ///< 0x10
} CARDFileInfo;

typedef struct CARDStat
{
    // read-only (Set by CARDGetStatus)
    s8  fileName[CARD_FILENAME_MAX]; ///< 0x00
    u32 length;                      ///< 0x20
    u32 time;                        ///< 0x24 // (seconds since 01/01/2000 midnight)
    u8  gameName[4];                 ///< 0x28
    u8  company[2];                  ///< 0x2C

    // read/write (Set by CARDGetStatus/CARDSetStatus)
    u8           bannerFormat; ///< 0x2E
    /*0x30*/ u32 iconAddr;     // (offset to the banner, bannerTlut, icon, iconTlut data set)
    u16          iconFormat;   ///< 0x34
    u16          iconSpeed;    ///< 0x36
    u32          commentAddr;  ///< 0x38 // (offset to the pair of 32 byte character strings)

                               // read-only (Set by CARDGetStatus)
    u32 offsetBanner;              ///< 0x3C
    u32 offsetBannerTlut;          ///< 0x40
    u32 offsetIcon[CARD_ICON_MAX]; ///< 0x44 // per entry (8 entries * = 3)
    u32 offsetIconTlut;            ///< 0x64
    u32 offsetData;                ///< 0x68
} CARDStat;

#define CARDGetBannerFormat(stat)  (((stat)->bannerFormat) & CARD_STAT_BANNER_MASK)
#define CARDGetIconAnim(stat)      (((stat)->bannerFormat) & CARD_STAT_ANIM_MASK)
#define CARDGetIconFormat(stat, n) (((stat)->iconFormat >> (2 * (n))) & CARD_STAT_ICON_MASK)
#define CARDGetIconSpeed(stat, n)  (((stat)->iconSpeed >> (2 * (n))) & CARD_STAT_SPEED_MASK)
#define CARDSetIconFormat(stat, n, f)                                                          \
    ((stat)->iconFormat = (u16)(((stat)->iconFormat & ~(CARD_STAT_ICON_MASK << (2 * (n)))) |   \
                                ((f) << (2 * (n)))))
#define CARDSetIconSpeed(stat, n, f)                                                           \
    ((stat)->iconSpeed = (u16)(((stat)->iconSpeed & ~(CARD_STAT_SPEED_MASK << (2 * (n)))) |    \
                               ((f) << (2 * (n)))))

#define CARD_STAT_ICON_NONE     0
#define CARD_STAT_ICON_C8       1
#define CARD_STAT_ICON_RGB5A3   2
#define CARD_STAT_ICON_MASK     3

#define CARD_STAT_BANNER_NONE   0
#define CARD_STAT_BANNER_C8     1
#define CARD_STAT_BANNER_RGB5A3 2
#define CARD_STAT_BANNER_MASK   3

#define CARD_STAT_ANIM_LOOP     0x00
#define CARD_STAT_ANIM_BOUNCE   0x04
#define CARD_STAT_ANIM_MASK     0x04

#define CARD_STAT_SPEED_END     0
#define CARD_STAT_SPEED_FAST    1
#define CARD_STAT_SPEED_MIDDLE  2
#define CARD_STAT_SPEED_SLOW    3
#define CARD_STAT_SPEED_MASK    3

#define CARD_ATTR_UNK0          0x01u
#define CARD_ATTR_UNK1          0x02u
#define CARD_ATTR_PUBLIC        0x04u
#define CARD_ATTR_NO_COPY       0x08u
#define CARD_ATTR_NO_MOVE       0x10u
#define CARD_ATTR_GLOBAL        0x20u
#define CARD_ATTR_COMPANY       0x40u

typedef void (*CARDCallback) (s32 chan, s32 result);

void CARDInit (void);

s32  CARDCheck (s32 chan);
s32  CARDCheckAsync (s32 chan, CARDCallback callback);
s32  CARDCheckEx (s32 chan, s32* xferBytes);
s32  CARDCheckExAsync (s32 chan, s32* xferBytes, CARDCallback callback);
s32  CARDCreate (s32 chan, char* fileName, u32 size, CARDFileInfo* fileInfo);
s32  CARDCreateAsync (s32           chan,
                      char*         fileName,
                      u32           size,
                      CARDFileInfo* fileInfo,
                      CARDCallback  callback);
s32  CARDDelete (s32 chan, char* fileName);
s32  CARDDeleteAsync (s32 chan, char* fileName, CARDCallback callback);
s32  CARDFastDelete (s32 chan, s32 fileNo);
s32  CARDFastDeleteAsync (s32 chan, s32 fileNo, CARDCallback callback);
s32  CARDFastOpen (s32 chan, s32 fileNo, CARDFileInfo* fileInfo);
s32  CARDFormat (s32 chan);
s32  CARDFormatAsync (s32 chan, CARDCallback callback);
s32  CARDFreeBlocks (s32 chan, s32* byteNotUsed, s32* filesNotUsed);
s32  CARDGetSectorSize (s32 chan, u32* size);
s32  CARDGetEncoding (s32 chan, u16* encode);
s32  CARDGetMemSize (s32 chan, u16* size);
s32  CARDGetResultCode (s32 chan);
s32  CARDGetStatus (s32 chan, s32 fileNo, CARDStat* stat);
s32  CARDGetXferredBytes (s32 chan);
s32  CARDMount (s32 chan, void* workArea, CARDCallback detachCallback);
s32  CARDMountAsync (s32          chan,
                     void*        workArea,
                     CARDCallback detachCallback,
                     CARDCallback attachCallback);
s32  CARDOpen (s32 chan, char* fileName, CARDFileInfo* fileInfo);
BOOL CARDProbe (s32 chan);
s32  CARDProbeEx (s32 chan, s32* memSize, s32* sectorSize);
s32  CARDRename (s32 chan, char* oldName, char* newName);
s32  CARDRenameAsync (s32 chan, char* oldName, char* newName, CARDCallback callback);
s32  CARDSetStatus (s32 chan, s32 fileNo, CARDStat* stat);
s32  CARDSetStatusAsync (s32 chan, s32 fileNo, CARDStat* stat, CARDCallback callback);
s32  CARDUnmount (s32 chan);

s32 CARDCancel (CARDFileInfo* fileInfo);
s32 CARDClose (CARDFileInfo* fileInfo);
s32 CARDRead (CARDFileInfo* fileInfo, void* addr, s32 length, s32 offset);
s32 CARDReadAsync (CARDFileInfo* fileInfo,
                   void*         addr,
                   s32           length,
                   s32           offset,
                   CARDCallback  callback);
s32 CARDWrite (CARDFileInfo* fileInfo, void* addr, s32 length, s32 offset);
s32 CARDWriteAsync (CARDFileInfo* fileInfo,
                    void*         addr,
                    s32           length,
                    s32           offset,
                    CARDCallback  callback);

#define CARDGetFileNo(fileInfo) ((fileInfo)->fileNo)

#define CARD_RESULT_UNLOCKED    1
#define CARD_RESULT_READY       0
#define CARD_RESULT_BUSY        -1
#define CARD_RESULT_WRONGDEVICE -2
#define CARD_RESULT_NOCARD      -3
#define CARD_RESULT_NOFILE      -4
#define CARD_RESULT_IOERROR     -5
#define CARD_RESULT_BROKEN      -6
#define CARD_RESULT_EXIST       -7
#define CARD_RESULT_NOENT       -8
#define CARD_RESULT_INSSPACE    -9
#define CARD_RESULT_NOPERM      -10
#define CARD_RESULT_LIMIT       -11
#define CARD_RESULT_NAMETOOLONG -12
#define CARD_RESULT_ENCODING    -13
#define CARD_RESULT_CANCELED    -14
#define CARD_RESULT_FATAL_ERROR -128

#ifdef __cplusplus
}
#endif

#endif
