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

#ifndef __fwscanf_h__
#define __fwscanf_h__

#include <ansi_parms.h>
#include <cstdio>

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

int 	fwscanf(FILE * file, const wchar_t * format, ...);
int 	wscanf(const wchar_t * format, ...);
int 	swscanf(const wchar_t * s, const wchar_t * format, ...);
int 	__vswscanf(const wchar_t * s, const wchar_t * format, va_list arg);
int 	__vfwscanf(FILE * file, const wchar_t * format_str, va_list arg);
int 	vwscanf(const wchar_t * format, va_list arg);

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

#endif /* ifndef __fwscanf_h__ */

/*  Change Record
 *	980121 	JCM  First code release.
 */
 