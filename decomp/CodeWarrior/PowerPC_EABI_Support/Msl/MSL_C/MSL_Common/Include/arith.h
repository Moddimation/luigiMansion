/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:30 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	arith.h
 */
 
#ifndef __arith__
#define __arith__

#include <ansi_parms.h>
#include <div_t.h>

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifdef __cplusplus          /*hh 971206  expanded __extern macro*/
	extern "C" {
#endif

int			__add (int  * x, int  y);
int			__ladd(long * x, long y);
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
int __lladd(long long * x, long long y);
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */

int			__mul (int  * x, int  y);
int			__lmul(long * x, long y);
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
int __llmul(long long * x, long long y);
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */

div_t		__div (int  x, int  y);
ldiv_t	__ldiv(long x, long y);
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
lldiv_t __lldiv(long long x, long long y);
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */

#ifndef __MOTO__
int			__mod (int  x, int  y);
long		__lmod(long x, long y);
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
long long __llmod(long long x, long long y);
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */
#endif

#ifdef __cplusplus          /*hh 971206  expanded __extern macro*/
	}
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif
/*     Change record
961221 bkoz lien 30 added mmoss's change
hh 971206  expanded __extern macro
 * mm 981023 added wrappers round long long support
*/
