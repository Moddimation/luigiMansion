#ifndef _DOLPHIN_DVD_H_
#define _DOLPHIN_DVD_H_

#include <types.h>

typedef struct DVDDiskID
{
    u8 gameName[4];
    u8 company[2];
    u8 diskNumber;
    u8 gameVersion;
    u8 streaming;
    u8 streamingBufSize;
    u8 padding[22];
} DVDDiskID;

typedef struct DVDCommandBlock DVDCommandBlock;
typedef void                   (*DVDCBCallback) (s32 result, DVDCommandBlock* block);

struct DVDCommandBlock
{
    DVDCommandBlock* next;             ///< 0x00
    DVDCommandBlock* prev;             ///< 0x04
    u32              command;          ///< 0x08
    s32              state;            ///< 0x0C
    u32              offset;           ///< 0x10
    u32              length;           ///< 0x14
    void*            addr;             ///< 0x18
    u32              currTransferSize; ///< 0x1C
    u32              transferredSize;  ///< 0x20
    DVDDiskID*       id;               ///< 0x24
    DVDCBCallback    callback;         ///< 0x28
    void*            userData;         ///< 0x2C
};

typedef struct DVDFileInfo DVDFileInfo;

typedef void (*DVDCallback) (s32 result, DVDFileInfo* fileInfo);

struct DVDFileInfo
{
    DVDCommandBlock cb;                ///< 0x00
    u32             startAddr;         ///< 0x30
    u32             length;            ///< 0x34
    DVDCallback     callback;          ///< 0x38
};

typedef struct
{
    u32 entryNum;
    u32 location;
    u32 next;
} DVDDir;

typedef struct
{
    u32  entryNum;
    BOOL isDir;
    u8*  name;
} DVDDirEntry;

typedef struct DVDBB2
{
    u32   bootFilePosition;            ///< 0x00
    u32   FSTPosition;                 ///< 0x04
    u32   FSTLength;                   ///< 0x08
    u32   FSTMaxLength;                ///< 0x0C
    void* FSTAddress;                  ///< 0x10
    u32   userPosition;                ///< 0x14
    u32   userLength;                  ///< 0x18
    u32   padding0;                    ///< 0x1C
} DVDBB2;

typedef struct DVDDriveInfo
{
    u16 revisionLevel;                 ///< 0x00
    u16 deviceCode;                    ///< 0x02
    u32 releaseDate;                   ///< 0x04
    u8  padding[24];                   ///< 0x08
} DVDDriveInfo;

typedef void (*DVDLowCallback) (u32);

void DVDDumpWaitingQueue (void);
int  DVDLowRead (void* addr, u32 length, u32 offset, DVDLowCallback callback);
int  DVDLowSeek (u32 offset, DVDLowCallback callback);
int  DVDLowWaitCoverClose (DVDLowCallback callback);
int  DVDLowReadDiskID (DVDDiskID* diskID, DVDLowCallback callback);
int  DVDLowStopMotor (DVDLowCallback callback);
int  DVDLowRequestError (DVDLowCallback callback);
int  DVDLowInquiry (struct DVDDriveInfo* info, DVDLowCallback callback);
int  DVDLowAudioStream (u32 subcmd, u32 length, u32 offset, DVDLowCallback callback);
int  DVDLowRequestAudioStatus (u32 subcmd, DVDLowCallback callback);
int  DVDLowAudioBufferConfig (int enable, u32 size, DVDLowCallback callback);
void DVDLowReset ();
void (*DVDLowSetResetCoverCallback (DVDLowCallback callback)) (u32);
int  DVDLowBreak ();
void (*DVDLowClearCallback()) (u32);
u32  DVDLowGetCoverStatus ();

// dvd.c
void DVDInit ();
int  DVDReadAbsAsyncPrio (DVDCommandBlock* block,
                          void*            addr,
                          s32              length,
                          s32              offset,
                          DVDCBCallback    callback,
                          s32              prio);
int  DVDSeekAbsAsyncPrio (DVDCommandBlock* block, s32 offset, DVDCBCallback callback, s32 prio);
int  DVDReadAbsAsyncForBS (DVDCommandBlock* block,
                           void*            addr,
                           s32              length,
                           s32              offset,
                           DVDCBCallback    callback);
int  DVDReadDiskID (DVDCommandBlock* block, DVDDiskID* diskID, DVDCBCallback callback);
int  DVDPrepareStreamAbsAsync (DVDCommandBlock* block,
                               u32              length,
                               u32              offset,
                               DVDCBCallback    callback);
int  DVDCancelStreamAsync (DVDCommandBlock* block, DVDCBCallback callback);
s32  DVDCancelStream (DVDCommandBlock* block);
int  DVDStopStreamAtEndAsync (DVDCommandBlock* block, DVDCBCallback callback);
s32  DVDStopStreamAtEnd (DVDCommandBlock* block);
int  DVDGetStreamErrorStatusAsync (DVDCommandBlock* block, DVDCBCallback callback);
s32  DVDGetStreamErrorStatus (DVDCommandBlock* block);
int  DVDGetStreamPlayAddrAsync (DVDCommandBlock* block, DVDCBCallback callback);
s32  DVDGetStreamPlayAddr (DVDCommandBlock* block);
int  DVDGetStreamStartAddrAsync (DVDCommandBlock* block, DVDCBCallback callback);
s32  DVDGetStreamStartAddr (DVDCommandBlock* block);
int  DVDGetStreamLengthAsync (DVDCommandBlock* block, DVDCBCallback callback);
s32  DVDGetStreamLength (DVDCommandBlock* block);
int  DVDChangeDiskAsyncForBS (DVDCommandBlock* block, DVDCBCallback callback);
int  DVDChangeDiskAsync (DVDCommandBlock* block, DVDDiskID* id, DVDCBCallback callback);
s32  DVDChangeDisk (DVDCommandBlock* block, DVDDiskID* id);
int DVDInquiryAsync (DVDCommandBlock* block, struct DVDDriveInfo* info, DVDCBCallback callback);
s32 DVDInquiry (DVDCommandBlock* block, struct DVDDriveInfo* info);
void       DVDReset ();
int        DVDResetRequired ();
s32        DVDGetCommandBlockStatus (DVDCommandBlock* block);
s32        DVDGetDriveStatus ();
int        DVDSetAutoInvalidation (int autoInval);
void       DVDPause ();
void       DVDResume ();
int        DVDCancelAsync (DVDCommandBlock* block, DVDCBCallback callback);
s32        DVDCancel (volatile DVDCommandBlock* block);
int        DVDCancelAllAsync (DVDCBCallback callback);
s32        DVDCancelAll ();
DVDDiskID* DVDGetCurrentDiskID ();

BOOL DVDCheckDisk ();
void __DVDPrepareResetAsync (DVDCBCallback cb);

// dvdfs.c
s32   DVDConvertPathToEntrynum (u8* pathPtr);
BOOL  DVDFastOpen (s32 entrynum, DVDFileInfo* fileInfo);
BOOL  DVDOpen (u8* fileName, DVDFileInfo* fileInfo);
BOOL  DVDClose (DVDFileInfo* fileInfo);
BOOL  DVDGetCurrentDir (u8* path, u32 maxlen);
BOOL  DVDChangeDir (u8* dirName);
BOOL  DVDReadAsyncPrio (DVDFileInfo* fileInfo,
                        void*        addr,
                        s32          length,
                        s32          offset,
                        DVDCallback  callback,
                        s32          prio);
s32   DVDReadPrio (DVDFileInfo* fileInfo, void* addr, s32 length, s32 offset, s32 prio);
int   DVDSeekAsyncPrio (DVDFileInfo* fileInfo,
                        s32          offset,
                        void         (*callback) (s32, DVDFileInfo*),
                        s32          prio);
s32   DVDSeekPrio (DVDFileInfo* fileInfo, s32 offset, s32 prio);
s32   DVDGetFileInfoStatus (DVDFileInfo* fileInfo);
int   DVDOpenDir (u8* dirName, DVDDir* dir);
int   DVDReadDir (DVDDir* dir, DVDDirEntry* dirent);
int   DVDCloseDir (DVDDir* dir);
void* DVDGetFSTLocation ();
BOOL  DVDPrepareStreamAsync (DVDFileInfo* fileInfo,
                             u32          length,
                             u32          offset,
                             DVDCallback  callback);
s32   DVDPrepareStream (DVDFileInfo* fileInfo, u32 length, u32 offset);
s32   DVDGetTransferredSize (DVDFileInfo* fileinfo);

#define DVDReadAsync(fileInfo, addr, length, offset, callback)                                 \
    DVDReadAsyncPrio ((fileInfo), (addr), (length), (offset), (callback), 2)

#define DVD_MIN_TRANSFER_SIZE           32

#define DVD_STATE_FATAL_ERROR           -1
#define DVD_STATE_END                   0
#define DVD_STATE_BUSY                  1
#define DVD_STATE_WAITING               2
#define DVD_STATE_COVER_CLOSED          3
#define DVD_STATE_NO_DISK               4
#define DVD_STATE_COVER_OPEN            5
#define DVD_STATE_WRONG_DISK            6
#define DVD_STATE_MOTOR_STOPPED         7
#define DVD_STATE_PAUSING               8
#define DVD_STATE_IGNORED               9
#define DVD_STATE_CANCELED              10
#define DVD_STATE_RETRY                 11

#define DVD_RESULT_FATAL_ERROR          -1
#define DVD_RESULT_COVER_CLOSED         -2
#define DVD_RESULT_COVER_OPEN           -3
#define DVD_RESULT_NO_DISK              -4
#define DVD_RESULT_IGNORED              -5
#define DVD_RESULT_CANCELED             -6

#define DVD_INTTYPE_TC                  0x00000001
#define DVD_INTTYPE_DE                  0x00000002
#define DVD_INTTYPE_CVR                 0x00000004
#define DVD_INTTYPE_BRK                 0x00000008
#define DVD_INTTYPE_TIMEOUT             0x00000010
#define DVD_INTTYPE_SECURITY_ERROR      0x00000020

// DVD Commands

#define DVD_COMMAND_NONE                0
#define DVD_COMMAND_READ                1
#define DVD_COMMAND_SEEK                2
#define DVD_COMMAND_CHANGE_DISK         3
#define DVD_COMMAND_BSREAD              4
#define DVD_COMMAND_READID              5
#define DVD_COMMAND_INITSTREAM          6
#define DVD_COMMAND_CANCELSTREAM        7
#define DVD_COMMAND_STOP_STREAM_AT_END  8
#define DVD_COMMAND_REQUEST_AUDIO_ERROR 9
#define DVD_COMMAND_REQUEST_PLAY_ADDR   10
#define DVD_COMMAND_REQUEST_START_ADDR  11
#define DVD_COMMAND_REQUEST_LENGTH      12
#define DVD_COMMAND_AUDIO_BUFFER_CONFIG 13
#define DVD_COMMAND_INQUIRY             14
#define DVD_COMMAND_BS_CHANGE_DISK      15

// unidentified externs
extern int  DVDReadAbsAsyncForBS (DVDCommandBlock* block,
                                  void*            addr,
                                  s32              length,
                                  s32              offset,
                                  DVDCBCallback    callback);
extern int  DVDReadDiskID (DVDCommandBlock* block, DVDDiskID* diskID, DVDCBCallback callback);
extern void DVDReset (void);

int DVDReadAbsAsyncPrio (DVDCommandBlock* block /* r29 */,
                         void*            addr /* r1+0xC */,
                         s32              length /* r1+0x10 */,
                         s32              offset /* r1+0x14 */,
                         DVDCBCallback    callback /* r1+0x18 */,
                         s32              prio /* r31 */);
int DVDSeekAbsAsyncPrio (DVDCommandBlock* block /* r31 */,
                         s32              offset /* r28 */,
                         DVDCBCallback    callback /* r1+0x10 */,
                         s32              prio);                                 ///< 0x14
int DVDPrepareStreamAbsAsync (DVDCommandBlock* block /* r31 */,
                              u32              length /* r1+0xC */,
                              u32              offset /* r1+0x10 */,
                              void             (*callback) (s32,
                                                DVDCommandBlock*)); ///< 0x14

#endif
