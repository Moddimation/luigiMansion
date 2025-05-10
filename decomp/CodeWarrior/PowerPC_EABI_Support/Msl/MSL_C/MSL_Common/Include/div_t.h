/*  Metrowerks Standard Library  */

/*  $Date: 1999/02/22 15:05:51 $ 
 *  $Revision: 1.8 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	div_t.h
 */
 
#ifndef __div_t__
#define __div_t__

#include <ansi_parms.h>                 /* mm 970905*/

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifdef __cplusplus      /* hh  971206 */
	#ifdef _MSL_USING_NAMESPACE				/* mm 990217 */
		namespace std {						/* mm 990217 */
	#endif									/* mm 990217 */
	extern "C" {
#endif

typedef struct {
	int	quot;
	int	rem;
} div_t;

typedef struct {
	long	quot;
	long	rem;
} ldiv_t;

/* hh 980122 added */
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
typedef struct 
{
	long long	quot;
	long long	rem;
} lldiv_t;
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */

#ifdef __cplusplus      /* hh  971206 */
	}
	#ifdef _MSL_USING_NAMESPACE					/* mm 990217 */
		}										/* mm 990217 */
	#endif										/* mm 990217 */
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif

/*     Change record
 * mm 970905  added include of ansi_parms.h to avoid need for prefix file
 * hh  971206 added extern "C" (think maintenance).
 * hh 980122 added long long support
 * mm 981023 added wrappers round long long support
 * mm 990217 added namespace support	
 */
