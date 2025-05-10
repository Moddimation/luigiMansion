/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:32 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	strtod.h
 */
 
#ifndef __wtime_h__
#define __wtime_h__

#include <ansi_parms.h>
#include <cstdio>               /* mm 970905 */ /*hh 971206  namespace support*/
#include <ctime>

#include <cwchar>

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifdef __cplusplus          /*hh 971206  namespace support*/
	extern "C" {
#endif

/* extern prototypes */

/* prototypes */

wchar_t * 		wasctime(const struct __std(tm) * tm); /*hh 990121 Fixed __std 7 places*/
wchar_t * 		wctime	(const __std(time_t) * timer);
__std(size_t) 	wcsftime(wchar_t * str, __std(size_t) max_size, const wchar_t * format_str, const struct __std(tm) * timeptr);
static int 		wemit	(wchar_t * str, __std(size_t) size, __std(size_t) * max_size, const wchar_t * format_str, ...);

#ifdef __cplusplus          /*hh 971206  namespace support*/
	}
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif /* #ifndef __wtime_h__ */

/*     Change record
 * mm 970905  added include of stdio.h to allow this header to be included first.
 * hh 971206  namespace support
 * hh 990121  Fixed __std 7 places
*/
 