/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/04 00:02:01 $ 
 *  $Revision: 1.7.14.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	mem_funcs.h
 */
 
#ifndef __mem_funcs__
#define __mem_funcs__

#include <ansi_parms.h>                 /* mm 970905*/

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#define __min_bytes_for_long_copy	32		/* NEVER let this be < 16 */

#ifdef __cplusplus
	extern "C" {
#endif

void	__copy_mem                (void * dst, const void * src, unsigned long n);
void	__move_mem                (void * dst, const void * src, unsigned long n);
void	__copy_longs_aligned      (void * dst, const void * src, unsigned long n);
void	__copy_longs_rev_aligned  (void * dst, const void * src, unsigned long n);
void	__copy_longs_unaligned    (void * dst, const void * src, unsigned long n);
void	__copy_longs_rev_unaligned(void * dst, const void * src, unsigned long n);

void	__fill_mem(void * dst, int val, unsigned long n);

#ifdef __cplusplus
	}
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif
/*     Change record
 * mm 970905  added include of ansi_parms.h to avoid need for prefix file
 */
