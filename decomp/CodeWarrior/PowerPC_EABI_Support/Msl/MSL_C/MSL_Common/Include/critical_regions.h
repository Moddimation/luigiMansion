/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 23:08:41 $ 
 *  $Revision: 1.11.4.2 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	critical_regions.h
 *
 *		Notes
 *		-----
 *
 *			What we need in certain areas of the library is the ability to "lock"
 *			certain critical regions of code against reentrance by preemptive
 *			threads. For example, when fopen searches for an unused FILE struct, it
 *			would be unfortunate if another thread "found" the same struct before the
 *			first one could mark it as in-use.
 *			
 *			Because the mechanisms used to manage critical regions will vary widely
 *			depending on the underlying hardware and/or system software, *all*
 *			details about how critical regions are locked and released are kept
 *			hidden. Instead, we define a finite number of critical regions that are
 *			of interest to us and leave the details of how they are managed invisible.
 */

#ifndef __critical_regions__
#define __critical_regions__

#include <ansi_parms.h>

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifdef __cplusplus               /*hh 971206  expanded _extern macro*/
	extern "C" {
#endif

enum critical_regions 
{
	atexit_funcs_access		=0,
	malloc_pool_access		=1,
	files_access			=2,
	console_status_access	=3,
	signal_funcs_access		=4,
	num_critical_regions	=5
};

void	__init_critical_regions (void);
void	__kill_critical_regions (void);

#if _MWMT
	void	__begin_critical_region (int region);
	void	__end_critical_region   (int region);
#else
	#define __begin_critical_region(x) 
	#define __end_critical_region(x) 
#endif
 
#ifdef __cplusplus               /*hh 971206  expanded _extern macro*/
	}
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif
/*     Change record
 *hh 971206  expanded _extern macro
  mf 980515  wince changes
  mf 030199  single threaded lib changes
*/
