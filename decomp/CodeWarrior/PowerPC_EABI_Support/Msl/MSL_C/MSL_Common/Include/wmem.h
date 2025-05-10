/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/23 02:22:31 $ 
 *  $Revision: 1.9 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	mem_funcs.h
 */
 
#ifndef __wmem_funcs__
#define __wmem_funcs__

#include <ansi_parms.h>                 /* mm 970905*/
#include <mem_funcs.h>
#include <cstddef>

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifdef __cplusplus
	#ifdef _MSL_USING_NAMESPACE
		namespace std {
	#endif
	extern "C" {
#endif

/* extern prototypes */

wchar_t * (wmemcpy) (wchar_t * s1, const wchar_t * s2, size_t n);	/* mm 980909 */
wchar_t * (wmemmove)(wchar_t *s1, const wchar_t *s2, size_t n);		/* mm 980909 */
wchar_t * (wmemset) (wchar_t *s, wchar_t c, size_t n);				/* mm 980909 */
wchar_t * (wmemchr) (const wchar_t *s, wchar_t c, size_t n);        /* mm 980909 */
int 	  (wmemcmp) (const wchar_t * s1, const wchar_t * s2, size_t n);   /* mm 980909 */

#ifdef __cplusplus
	}
	#ifdef _MSL_USING_NAMESPACE
		}
	#endif
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif	/* #ifndef __wmem_funcs__ */

/*     Change record
 * mm 970905  added include of ansi_parms.h to avoid need for prefix file
 * mm 980909  Corrected prototypes to refer to wchar_t* instead of void*
 */