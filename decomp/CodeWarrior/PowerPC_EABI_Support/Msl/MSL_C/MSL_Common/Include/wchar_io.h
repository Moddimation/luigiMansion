/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:32 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	wchar_io.h
 */

#ifndef __wchar_io__
#define __wchar_io__

#include <ansi_parms.h>
#include <cstdio>
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

wchar_t __put_wchar(wchar_t c, FILE * file);
wchar_t __get_wchar(FILE * file);
wchar_t (getwchar)(void);
wchar_t ungetwc(wchar_t c, FILE * file);

#ifdef __cplusplus           /*  hh  971206 */
	}
	#ifdef _MSL_USING_NAMESPACE
		}
	#endif
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif /* ifndef __wchar_io__ */

/*  Change Record
 *	980121 	JCM  First code release.
 */
 