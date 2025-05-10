#include <dolphin.h>

#include <string.h>

#include "dvd_private.h"

static u32 status;         // size: 0x4, address: 0x0

enum
{
    STATUS_READING_ID,
    STATUS_READING_BB2,
    STATUS_READING_FST
};

static u8      bb2Buf[63]; // size: 0x3F, address: 0x0
static DVDBB2* bb2;        // size: 0x4, address: 0x4

static DVDDiskID* idTmp;   // size: 0x4, address: 0x8

// functions
static void cb (s32 result, DVDCommandBlock* block);
void        __fstLoad ();

static void
cb (s32 result, DVDCommandBlock* block)
{
    if (result > 0)
    {
        switch (status)
        {
            case 0:                        // read id done
                status = 1;                // next read bb2
                DVDReadAbsAsyncForBS (block, bb2, OSRoundUp32B (sizeof (DVDBB2)), 0x420, cb);
                return;
            case 1:                        // read bb2 done
                status = 2;                // next read fst
                DVDReadAbsAsyncForBS (block,
                                      bb2->FSTAddress,
                                      (s32)((bb2->FSTLength + 0x1F) & 0xFFFFFFE0),
                                      (s32)bb2->FSTPosition,
                                      cb);
            default:
                return;
        }
    }
    if (result == DVD_RESULT_FATAL_ERROR)  // fatal error
    {
        return;
    }
    else if (result == DVD_RESULT_NO_DISK) // cover closed
    {
        status = 0;
        DVDReset();
        DVDReadDiskID (block, idTmp, cb);
    }
}

void
__fstLoad ()
{
    OSBootInfo*            bootInfo;
    DVDDiskID*             id;
    u8                     idTmpBuf[63];
    s32                    state;
    void*                  arenaHi;
    static DVDCommandBlock block;

    arenaHi = OSGetArenaHi();

    bootInfo = (void*)OSPhysicalToCached (0);

    // disk id is not aligned 32b
    idTmp = (void*)OSRoundUp32B (idTmpBuf);
    bb2 = (void*)OSRoundUp32B (bb2Buf);

    DVDReset();
    DVDReadDiskID (&block, idTmp, cb);

    while (1)
    {
        state = DVDGetDriveStatus();
        if (state == DVD_STATE_END)
        {
            break;
        }

        // weird switch that seemingly wont do anything but break out of its
        // own switch. What was this for? Early DVD development pre-hardware?
        switch (state)
        {
            case DVD_STATE_FATAL_ERROR:
                break;
            case DVD_STATE_BUSY:
                break;
            case DVD_STATE_WAITING:
                break;
            case DVD_STATE_COVER_CLOSED:
                break;
            case DVD_STATE_NO_DISK:
                break;
            case DVD_STATE_COVER_OPEN:
                break;
            case DVD_STATE_MOTOR_STOPPED:
                break;
        }
    }

    bootInfo->FSTLocation = (void*)bb2->FSTAddress;
    bootInfo->FSTMaxLength = bb2->FSTMaxLength;

    id = &bootInfo->DVDDiskID;
    memcpy (id, idTmp, 0x20);

    OSReport ("\n");
    OSReport ("  Game Name ... %c%c%c%c\n",
              id->gameName[0],
              id->gameName[1],
              id->gameName[2],
              id->gameName[3]);
    OSReport ("  Company ..... %c%c\n", id->company[0], id->company[1]);
    OSReport ("  Disk # ...... %d\n", id->diskNumber);
    OSReport ("  Game ver .... %d\n", id->gameVersion);
    OSReport ("  Streaming ... %s\n", !(id->streaming) ? "OFF" : "ON");
    OSReport ("\n");
    OSSetArenaHi (bb2->FSTAddress);
}
