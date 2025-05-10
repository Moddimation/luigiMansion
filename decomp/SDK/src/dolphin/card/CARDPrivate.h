#ifndef _DOLPHIN_CARD_INTERNAL_H_
#define _DOLPHIN_CARD_INTERNAL_H_

#include <dolphin/card.h>
#include <dolphin/exi.h>

typedef struct CARDID
{
    u8  serial[32];                 ///< 0x000
    u16 deviceID;                   ///< 0x020
    u16 size;                       ///< 0x022
    u16 encode;                     ///< 0x024
    u8  padding[470];               ///< 0x026
    u16 checkSum;                   ///< 0x1FC
    u16 checkSumInv;                ///< 0x1FE
} CARDID;

typedef struct CARDDir
{
    u8  gameName[4];
    u8  company[2];
    u8  _padding0;
    u8  bannerFormat;
    u8  fileName[CARD_FILENAME_MAX];
    u32 time;                       // seconds since 01/01/2000 midnight

    u32 iconAddr;                   // 0xffffffff if not used
    u16 iconFormat;
    u16 iconSpeed;

    u8  permission;
    u8  copyTimes;
    u16 startBlock;
    u16 length;
    u8  _padding1[2];

    u32 commentAddr;                // 0xffffffff if not used
} CARDDir;                          // total size 64 bytes

typedef struct CARDControl
{
    BOOL          attached;         ///< 0x00
    s32           result;           ///< 0x04
    u16           size;             ///< 0x08
    u16           pageSize;         ///< 0x0A
    s32           sectorSize;       ///< 0x0C
    u16           cBlock;           ///< 0x10
    u16           vendorID;         ///< 0x12
    s32           latency;          ///< 0x14
    u8            id[12];           ///< 0x18
    int           mountStep;        ///< 0x24
    u32           scramble;         ///< 0x28
    int           formatStep;       ///< 0x2C
    DSPTaskInfo   task;             ///< 0x30
    void*         workArea;         ///< 0x80
    CARDDir*      currentDir;       ///< 0x84
    u16*          currentFat;       ///< 0x88
    OSThreadQueue threadQueue;      ///< 0x8C
    u8            cmd[9];           ///< 0x94
    u8            _pad9D[3];        ///< 0x9D
    s32           cmdlen;           ///< 0xA0
    vs32          mode;             ///< 0xA4
    int           retry;            ///< 0xA8
    int           repeat;           ///< 0xAC
    u32           addr;             ///< 0xB0
    void*         buffer;           ///< 0xB4
    s32           xferred;          ///< 0xB8
    u16           freeNo;           ///< 0xBC
    u16           startBlock;       ///< 0xBE
    CARDFileInfo* fileInfo;         ///< 0xC0
    CARDCallback  extCallback;      ///< 0xC4
    CARDCallback  txCallback;       ///< 0xC8
    CARDCallback  exiCallback;      ///< 0xCC
    CARDCallback  apiCallback;      ///< 0xD0
    CARDCallback  xferCallback;     ///< 0xD4
    CARDCallback  eraseCallback;    ///< 0xD8
    CARDCallback  unlockCallback;   ///< 0xDC
    OSAlarm       alarm;            ///< 0xE4
} CARDControl;

typedef struct CARDDecParam
{
    u8* inputAddr;                  ///< 0x00
    u32 inputLength;                ///< 0x04
    u32 aramAddr;                   ///< 0x08
    u8* outputAddr;                 ///< 0x0C
} CARDDecParam;

typedef struct CARDDirCheck
{
    u8  padding0[64 - 2 * 4];
    u16 padding1;
    s16 checkCode;
    u16 checkSum;
    u16 checkSumInv;
} CARDDirCheck;                     // total 64 bytes

#define CARD_PAGE_SIZE         128u
#define CARD_SEG_SIZE          512u
#define CARD_MAX_SIZE          (16u * 1024u * 1024u)

#define CARD_NUM_SYSTEM_BLOCK  5
#define CARD_SYSTEM_BLOCK_SIZE (8 * 1024u)

#define CARD_MAX_MOUNT_STEP    (CARD_NUM_SYSTEM_BLOCK + 2)

#define CARD_CUSTOM_ID         0x00000000
#define CARD_CUSTOM_ID_MASK    0xffff0000

#define CARD_ID_SIZE           0x000000fc
#define CARD_ID_CHIPS          0x00000003
#define CARD_ID_LATENCY        0x00000700
#define CARD_ID_LATENCY_SHIFT  8
#define CARD_ID_SECTOR         0x00003800
#define CARD_ID_SECTOR_SHIFT   11

#define CARD_CMD_CUSTOM_ID     0x00000000
#define CARD_CMD_INT_ENABLE    0x81010000
#define CARD_CMD_INT_DISABLE   0x81000000
#define CARD_CMD_CLEAR_STATUS  0x89000000
#define CARD_CMD_READ_STATUS   0x83000000
#define CARD_CMD_VENDOR_ID     0x85000000
#define CARD_CMD_READ          0x52
#define CARD_CMD_WRITE         0xF2
#define CARD_CMD_ERASE         0xF4
#define CARD_CMD_ERASE_SECTOR  0xF1
#define CARD_CMD_WAKEUP        0x87000000
#define CARD_CMD_SLEEP         0x88000000

#define CARD_STS_COMPLETE      (1u << 7)
#define CARD_STS_SECURITY_PASS (1u << 6)
#define CARD_STS_SLEEPING      (1u << 5)
#define CARD_STS_ERASE_FAIL    (1u << 4)
#define CARD_STS_PROGRAM_FAIL  (1u << 3)
#define CARD_STS_NA_FAIL       (1u << 2)
#define CARD_STS_INT_ENABLE    (1u << 1)
#define CARD_STS_READY         (1u << 0)

#define CARD_FAT_AVAIL         0x0000u
#define CARD_FAT_VOID          0x0001u
#define CARD_FAT_EOF           0xFFFFu

#define CARD_FAT_CHECKSUM      0x0000u
#define CARD_FAT_CHECKSUMINV   0x0001u
#define CARD_FAT_CHECKCODE     0x0002u
#define CARD_FAT_FREEBLOCKS    0x0003u
#define CARD_FAT_LASTSLOT      0x0004u

#define CARD_VENDOR_SAMSUNG    0xEC // high byte

#define CARDIsValidBlockNo(card, iBlock)                                                       \
    (CARD_NUM_SYSTEM_BLOCK <= (iBlock) && (iBlock) < (card)->cBlock)

#define CARDGetDirCheck(dir) ((CARDDirCheck*)&(dir)[CARD_MAX_FILE])

// CARDStatEx.c
s32 __CARDGetStatusEx (long chan, long fileNo, struct CARDDir* dirent);
s32 __CARDSetStatusExAsync (long            chan,
                            long            fileNo,
                            struct CARDDir* dirent,
                            void            (*callback) (long, long));
s32 __CARDSetStatusEx (long chan, long fileNo, struct CARDDir* dirent);

// CARDUnlock.c
s32 __CARDUnlock (s32 chan, u8 flashID[12]);

// CARDRead.c
s32 __CARDSeek (CARDFileInfo* fileInfo, s32 length, s32 offset, CARDControl** pcard);

// CARDRdwr.c
s32 __CARDRead (long chan, u32 addr, long length, void* dst, void (*callback) (long, long));
s32 __CARDWrite (long chan, u32 addr, long length, void* dst, void (*callback) (long, long));

// CARDRaw.c
s32 __CARDRawReadAsync (long  chan,
                        void* buf,
                        long  length,
                        long  offset,
                        void  (*callback) (long, long));
s32 __CARDRawRead (long chan, void* buf, long length, long offset);

// CARDOpen.c
BOOL __CARDCompareFileName (CARDDir* ent, const char* fileName);
s32  __CARDAccess (CARDDir* ent);
s32  __CARDIsPublic (CARDDir* ent);
s32  __CARDGetFileNo (CARDControl* card, const char* fileName, s32* pfileNo);
BOOL __CARDIsOpened (CARDControl* card, s32 fileNo);

// CARDMount.c
void __CARDMountCallback (s32 chan, s32 result);

// CARDFormat.c
s32 CARDFormatAsync (s32 chan, CARDCallback callback);

// CARDDir.c
CARDDir* __CARDGetDirBlock (CARDControl* card);
s32      __CARDUpdateDir (s32 chan, CARDCallback callback);

// CARDCheck.c
void __CARDCheckSum (void* ptr, int length, u16* checksum, u16* checksumInv);
s32  __CARDVerify (CARDControl* card);

// CARDBlock.c
void* __CARDGetFatBlock (CARDControl* card);
s32   __CARDAllocBlock (s32 chan, u32 cBlock, CARDCallback callback);
s32   __CARDFreeBlock (s32 chan, u16 nBlock, CARDCallback callback);
s32   __CARDUpdateFatBlock (s32 chan, u16* fat, CARDCallback callback);

// CARDBios.c
extern struct CARDControl __CARDBlock[2];

extern DVDDiskID* __CARDDiskID;
extern DVDDiskID  __CARDDiskNone;

void __CARDDefaultApiCallback (s32 chan, s32 result);
void __CARDSyncCallback (s32 chan, s32 result);
void __CARDExtHandler (s32 chan, OSContext* context);
void __CARDExiHandler (s32 chan, OSContext* context);
void __CARDTxHandler (s32 chan, OSContext* context);
void __CARDUnlockedHandler (s32 chan, OSContext* context);
int  __CARDReadNintendoID (s32 chan, u32* id);
s32  __CARDEnableInterrupt (s32 chan, BOOL enable);
s32  __CARDReadStatus (s32 chan, u8* status);
s32  __CARDClearStatus (s32 chan);
s32  __CARDSleep (long chan);
s32  __CARDWakeup (long chan);
s32  __CARDReadSegment (s32 chan, CARDCallback callback);
s32  __CARDWritePage (s32 chan, CARDCallback callback);
s32  __CARDErase (long chan, void (*callback) (long, long));
s32  __CARDEraseSector (s32 chan, u32 addr, CARDCallback callback);
void __CARDSetDiskID (DVDDiskID* id);
s32  __CARDGetControlBlock (s32 chan, CARDControl** pcard);
s32  __CARDPutControlBlock (CARDControl* card, s32 result);
s32  __CARDSync (s32 chan);

#endif // _DOLPHIN_CARD_INTERNAL_H_
