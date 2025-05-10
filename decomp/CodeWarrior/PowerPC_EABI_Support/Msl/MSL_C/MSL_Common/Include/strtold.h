/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 00:47:14 $ 
 *  $Revision: 1.9.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */
/*
 *	strtold.h
 */
 
#ifndef __strtold__
#define __strtold__

#include <ansi_parms.h>
#include <cstdio>               /* mm 970905 */ /*hh 971206  namespace support*/

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifdef __cplusplus          /*hh 971206  namespace support*/
	extern "C" {
#endif
#if __INTEL__
#pragma k63d_calls off
#endif
extern long double __strtold(int max_width,
							 int (*ReadProc)(void *, int, int), /* mm 990325 */
							 void * ReadProcArg,				/* mm 990325 */
							 int	* chars_scanned,
							 int	* overflow);

#if __INTEL__
#pragma k63d_calls reset
#endif

#ifdef __cplusplus          /*hh 971206  namespace support*/
	}
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif

/*     Change record
 * mm 970905  added include of stdio.h to allow this header to be included first.
 * hh 971206  namespace support
 * hh 990121  Fixed __std
 *  mm 990325		Made to work with input functions passed by pointers 
 *  mf 990420      had to provide k6 wrap internal function __strtold as well

*/ 