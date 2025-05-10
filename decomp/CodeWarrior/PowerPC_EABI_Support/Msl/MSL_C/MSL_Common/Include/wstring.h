/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:32 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	wstring.h
 */
 
#ifndef __wstring_h__
#define __wstring_h__

#include <ansi_parms.h>
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

size_t 		(wcslen)(const wchar_t * str);
wchar_t * 	(wcscpy)(wchar_t * dst, const wchar_t * src);
wchar_t * 	wcsncpy(wchar_t * dst, const wchar_t * src, size_t n);
wchar_t * 	wcscat(wchar_t * dst, const wchar_t * src);
wchar_t * 	wcsncat(wchar_t * dst, const wchar_t * src, size_t n);
int 		wcscmp(const wchar_t * str1, const wchar_t * str2);
int 		wcsncmp(const wchar_t * str1, const wchar_t * str2, size_t n);
wchar_t * 	wcschr(const wchar_t * str, const wchar_t chr);

int 		wcscoll(const wchar_t *str1, const wchar_t * str2);
size_t 		wcsxfrm(wchar_t * str1, const wchar_t * str2, size_t n);
wchar_t * 	wcsrchr(const wchar_t * str, wchar_t chr);
wchar_t * 	wcspbrk(const wchar_t * str, const wchar_t * set);
size_t 		wcsspn(const wchar_t * str, const wchar_t * set);
size_t 		wcscspn(const wchar_t * str, const wchar_t * set);
wchar_t * 	wcsstr(const wchar_t * str, const wchar_t * pat);
wchar_t * 	wcstok(wchar_t * str, const wchar_t * set);

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

#endif /* ifndef __wstring_h__ */

/*  Change Record
 *	980121 	JCM  First code release.
 */
 