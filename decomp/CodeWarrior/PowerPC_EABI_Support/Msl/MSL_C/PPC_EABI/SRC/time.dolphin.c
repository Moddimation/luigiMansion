/*---------------------------------------------------------------------------*
  Project:  Dolphin MSL time suppert
  File:     time.dolphin.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: time.dolphin.c,v $
  Revision 1.1.2.1  2000/10/30 22:27:33  swells
  Updates for R1.0

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*  Metrowerks Standard Library  Version 2.1.2  1997 May  */

// Include your OS dependent include files
#include <time.h>

// This has a dependency on the Dolphin OS existing
// OS.h is needed to get OS_TIMER_CLOCK and OSTicksToSeconds.
// swells on 08-22-00
#include <dolphin/os.h>

#define BIAS    ((100 * 365LU + 24) * 86400)    // Dolphin counts since midnight, 1/1/2000

// __get_clock returns the processor time used so far
// Dolphin returns -1 for now. Use OSGetTick or OSGetTime instead.
clock_t __get_clock(void)
{
   return (clock_t) -1;
}

// __get_time returns the current calendar time in seconds since midnight, 1/1/1900
time_t __get_time(void)
{
    return (time_t) (OSTicksToSeconds(OSGetTime())) + BIAS;
}

// Dolphin does not support time zone and just returns 0
int __to_gm_time(time_t* time)
{
    #pragma unused (time)
    return 0;
}
