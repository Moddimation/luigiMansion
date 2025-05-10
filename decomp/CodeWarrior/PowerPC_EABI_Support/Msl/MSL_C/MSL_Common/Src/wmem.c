/*  Metrowerks Standard Library   */

/*  $Date: 1999/07/30 01:02:54 $ 
 *  $Revision: 1.8.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */
 
/*
 *	wcmem.c
 *	
 *	Routines
 *	--------
 *		wmemcpy
 *		wmemmove
 *		wmemset
 *		wmemchr
 *		wmemcmp
 *	
 *
*/

#pragma ANSI_strict off  /*  990729 vss  empty compilation unit illegal in ANSI C */

#ifndef __NO_WIDE_CHAR				/* mm 980204 */
 
#pragma ANSI_strict reset

#include <string.h>
#include <wchar.h>

wchar_t * (wmemcpy)(wchar_t * dst, const wchar_t * src, size_t n)
{
	return memcpy(dst, src, n * 2);
}
	
wchar_t * (wmemmove)(wchar_t * dst, const wchar_t * src, size_t n)
{
	return memmove(dst, src, n * 2);
}

wchar_t * (wmemset)(wchar_t * dst, wchar_t val, size_t n)
{
	wchar_t *save = dst;
	
	while (n)
	{
		*dst++ = val;
		n--;
	}
	
	return(dst);
}

wchar_t * (wmemchr)(const wchar_t * src, wchar_t val, size_t n)
{
	while (n)
	{
		if (*src == val) return (wchar_t *) src;
		src++;
		n--;
	}
	
	return(NULL);
}

int (wmemcmp)(const wchar_t * src1, const wchar_t * src2, size_t n)
{
	wchar_t diff;

	while (n)
	{
		diff = *src1 - *src2;
		if (diff) break;
		src1++;
		src2++;
		n--;
	}

	return(diff);
}

#endif /* #ifndef __NO_WIDE_CHAR */				/* mm 981030 */

/*  Change Record
 * jcm 980126  First code release.
 * mm  980909  Corrected prototypes to refer to wchar_t* instead of void*
 * mm  981030  Added #ifndef __NO_WIDE_CHAR wrappers
 * mm  981111  Corrected wmemchr() and wmemcmp() to return a meaningful result 
 			   and wmemset to work correctly---IL9811-0490	
 * blc 990112 Fixed all the implementation so they actually worked
*/
