/*  Metrowerks Standard Library */

/*  $Date: 1999/07/30 01:02:53 $ 
 *  $Revision: 1.7.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */
 
/*
 *	wstring.c
 *	
 *	Routines
 *	--------
 *		wcslen				
 *
 *		wcscpy				
 *		wcscat				
 *		wcscmp				
 *
 *		wcsncpy				
 *		wcsncat				
 *		wcsncmp				
 *
 *		wcscoll				
 *		wcsxfrm				
 *
 *		wcschr				
 *		wcsrchr				
 *		wcsstr				
 *
 *		wcsspn				
 *		wcscspn				
 *		wcspbrk				
 *	
 *
 */

#pragma ANSI_strict off  /*  990729 vss  empty compilation unit illegal in ANSI C */

#ifndef __NO_WIDE_CHAR				/* mm 980204 */
 
#pragma ANSI_strict reset
#include <cstring>

#include <ansi_parms.h>  

#include <cstdio>
#include <cstddef>      
#include <cerrno>
#include <cstring>
#include <wchar.h>

#if (__dest_os	== __win32_os  || __dest_os	== __wince_os)
	#include <ThreadLocalData.h>
#endif

#pragma warn_possunwant off

/*
	This routine returns the length of the wide character string str

*/
size_t (wcslen)(const wchar_t * str)
{
	size_t	len = -1;
	
#if !__POWERPC__
	
	do
		len++;
	while (*str++);
	
#else
	
	wchar_t * p = (wchar_t *) str - 1;
	
	do
		len++;
	while (*++p);
	
#endif
	
	return(len);
}

wchar_t * (wcscpy)(wchar_t * dst, const wchar_t * src)
{
#if !__POWERPC__
	
	const	wchar_t * p = src;
				wchar_t * q = dst;
	
	while (*q++ = *p++);
	
#else
	
	const	wchar_t * p = (wchar_t *) src - 1;
				wchar_t * q = (wchar_t *) dst - 1;
	
	while (*++q = *++p);

#endif
	
	return(dst);
}



wchar_t * wcsncpy(wchar_t * dst, const wchar_t * src, size_t n)
{
#if !__POWERPC__
	
	const	wchar_t * p = src;
				wchar_t * q = dst;
	
	n++;
	
	while (--n)
		if (!(*q++ = *p++))
		{
			while (--n)
				*q++ = 0;
			break;
		}
	
#else
	
	const	wchar_t * p		= (const wchar_t *) src - 1;
				wchar_t * q		= (wchar_t *) dst - 1;
				wchar_t zero	= 0;
	
	n++;
	
	while (--n)
		if (!(*++q = *++p))
		{
			while (--n)
				*++q = 0;
			break;
		}

#endif
	
	return(dst);
}


wchar_t * wcscat(wchar_t * dst, const wchar_t * src)
{
#if !__POWERPC__
	
	const	wchar_t * p = src;
				wchar_t * q = dst;
	
	while (*q++);
	
	q--;
	
	while (*q++ = *p++);
	
#else
	
	const	wchar_t * p = (wchar_t *) src - 1;
			wchar_t * q = (wchar_t *) dst - 1;
	
	while (*++q);
	
	q--;
	
	while (*++q = *++p);

#endif
	
	return(dst);
}


wchar_t * wcsncat(wchar_t * dst, const wchar_t * src, size_t n)
{
#if !__POWERPC__
	
	const	wchar_t * p = src;
				wchar_t * q = dst;
	
	while (*q++);
	
	q--; n++;
	
	while (--n)
		if (!(*q++ = *p++))
		{
			q--;
			break;
		}
	
	*q = 0;
	
#else
	
	const	wchar_t * p = (wchar_t *) src - 1;
				wchar_t * q = (wchar_t *) dst - 1;
	
	while (*++q);
	
	q--; n++;
	
	while (--n)
		if (!(*++q = *++p))
		{
			q--;
			break;
		}
	
	*++q = 0;

#endif
	
	return(dst);
}


int wcscmp(const wchar_t * str1, const wchar_t * str2)
{
#if !__POWERPC__
	
	const	wchar_t * p1 = (wchar_t *) str1;
	const	wchar_t * p2 = (wchar_t *) str2;
				wchar_t		c1, c2;
	
	while ((c1 = *p1++) == (c2 = *p2++))
		if (!c1)
			return(0);

#else
	
	const	wchar_t * p1 = (wchar_t *) str1 - 1;
	const	wchar_t * p2 = (wchar_t *) str2 - 1;
				wchar_t		c1, c2;
		
	while ((c1 = *++p1) == (c2 = *++p2))
		if (!c1)
			return(0);

#endif
	
	return(c1 - c2);
}


int wcsncmp(const wchar_t * str1, const wchar_t * str2, size_t n)
{
#if !__POWERPC__
	
	const	wchar_t * p1 = (wchar_t *) str1;
	const	wchar_t * p2 = (wchar_t *) str2;
				wchar_t		c1, c2;
	
	n++;
	
	while (--n)
		if ((c1 = *p1++) != (c2 = *p2++))
			return(c1 - c2);
		else if (!c1)
			break;
	
#else
	
	const	wchar_t * p1 = (wchar_t *) str1 - 1;
	const	wchar_t * p2 = (wchar_t *) str2 - 1;
				wchar_t		c1, c2;
	
	n++;
	
	while (--n)
		if ((c1 = *++p1) != (c2 = *++p2))
			return(c1 - c2);
		else if (!c1)
			break;

#endif
	
	return(0);
}


wchar_t * wcschr(const wchar_t * str, const wchar_t chr)
{
#if !__POWERPC__

	const wchar_t * p = str;
	      wchar_t   c = chr;
	      wchar_t   ch;
	
	while(ch = *p++)
		if (ch == c)
			return((wchar_t *) (p - 1));
	
	return(c ? 0 : (wchar_t *) (p - 1));

#else

	const wchar_t * p = (wchar_t *) str - 1;
	      wchar_t   c = (chr & 0xffff);               /*mm 970327*/
	      wchar_t   ch;
	
	while(ch = (*++p & 0xffff))                             /*mm 970327*/
		if (ch == c)
			return((wchar_t *) p);
	
	return(c ? 0 : (wchar_t *) p);

#endif
}


int wcscoll(const wchar_t *str1, const wchar_t * str2)
{
	return(wcscmp(str1, str2));
}

size_t wcsxfrm(wchar_t * str1, const wchar_t * str2, size_t n)
{
	wcsncpy(str1, str2, n);
	return(wcslen(str2));
}


wchar_t * wcsrchr(const wchar_t * str, wchar_t chr)
{
#if !__POWERPC__

	const wchar_t * p = str;
	const wchar_t * q = 0;
	      wchar_t   c = chr;
	      wchar_t   ch;
	
	while(ch = *p++)
		if (ch == c)
			q = p - 1;
	
	if (q)
		return((wchar_t *) q);
	
	return(c ? 0 : (wchar_t *) (p - 1));

#else

	const wchar_t * p = (wchar_t *) str - 1;
	const wchar_t * q = 0;
	      wchar_t   c = (chr & 0xff);				/*bb 970530*/
	      wchar_t   ch;
	
	while(ch = (*++p & 0xff))								/*bb 970530*/
		if (ch == c)
			q = p;
	
	if (q)
		return((wchar_t *) q);
	
	return(c ? 0 : (wchar_t *) p);

#endif
}


typedef wchar_t wchar_map[32];
#define set_wchar_map(map, ch)	 map[ch>>3] |= (1 << (ch&7))
#define tst_wchar_map(map, ch) (map[ch>>3] &  (1 << (ch&7)))


wchar_t * wcspbrk(const wchar_t * str, const wchar_t * set)
{
	const 	wchar_t *	p;
	      	wchar_t								c;
			wchar_map					map = {0};
	
#if !__POWERPC__
	
	p = (wchar_t *) set;

	while (c = *p++)
		set_wchar_map(map, c);
	
	p = (wchar_t *) str;
	
	while (c = *p++)
		if (tst_wchar_map(map, c))
			return((wchar_t *) (p - 1));
			
	return(NULL);

#else
	
	p = (wchar_t *) set - 1;

	while (c = *++p)
		set_wchar_map(map, c);
	
	p = (wchar_t *) str - 1;
	
	while (c = *++p)
		if (tst_wchar_map(map, c))
			return((wchar_t *) p);
			
	return(NULL);

#endif
}


size_t wcsspn(const wchar_t * str, const wchar_t * set)
{
	const wchar_t *	p;
	      wchar_t							c;
				wchar_map				map = {0};
	
#if !__POWERPC__
	
	p = (wchar_t *) set;

	while (c = *p++)
		set_wchar_map(map, c);
	
	p = (wchar_t *) str;
	
	while (c = *p++)
		if (!tst_wchar_map(map, c))
			break;
			
	return(p - (wchar_t *) str - 1);

#else
	
	p = (wchar_t *) set - 1;

	while (c = *++p)
		set_wchar_map(map, c);
	
	p = (wchar_t *) str - 1;
	
	while (c = *++p)
		if (!tst_wchar_map(map, c))
			break;
			
	return(p - (wchar_t *) str);

#endif
}





size_t wcscspn(const wchar_t * str, const wchar_t * set)
{
	const wchar_t *	p;
	      wchar_t							c;
			wchar_map				map = {0};
	
#if !__POWERPC__
	
	p = (wchar_t *) set;

	while (c = *p++)
		set_wchar_map(map, c);
	
	p = (wchar_t *) str;
	
	while (c = *p++)
		if (tst_wchar_map(map, c))
			break;
			
	return(p - (wchar_t *) str - 1);

#else
	
	p = (wchar_t *) set - 1;

	while (c = *++p)
		set_wchar_map(map, c);
	
	p = (wchar_t *) str - 1;
	
	while (c = *++p)
		if (tst_wchar_map(map, c))
			break;
			
	return(p - (wchar_t *) str);

#endif
}





#if (__dest_os	== __win32_os  || __dest_os	== __wince_os)

wchar_t * wcstok(wchar_t * str, const wchar_t * set)
{
	wchar_t *	p, * q;
	wchar_t							c;
	wchar_map				map	= {0};

	if (str)
		_GetThreadLocalData()->strtok_s = (unsigned char*) str;
		
	p = (wchar_t *) set;

	while (c = *p++)
		set_wchar_map(map, c);
	
	p = (wchar_t*)_GetThreadLocalData()->strtok_s;
	
	while (c = *p++)
		if (!tst_wchar_map(map, c))
			break;
	
	if (!c)
	{
		_GetThreadLocalData()->strtok_s = _GetThreadLocalData()->strtok_n;
		return(NULL);
	}
	
	q = p - 1;
	
	while (c = *p++)
		if (tst_wchar_map(map, c))
			break;
	
	if (!c)
		_GetThreadLocalData()->strtok_s = _GetThreadLocalData()->strtok_n;
	else
	{
		_GetThreadLocalData()->strtok_s    = (unsigned char*)p;
		*--p = 0;
	}
	
	return((wchar_t *) q);
}

#else /* if __dest_os != win32 */

wchar_t * wcstok(wchar_t * str, const wchar_t * set)
{
								 wchar_t *	p, * q;
	__tls static	 wchar_t *	n		= ( wchar_t *) L"";
	__tls static	 wchar_t *	s		= ( wchar_t *) L"";
							  wchar_t							c;
								wchar_map				map	= {0};
	
	if (str)
		s = ( wchar_t *) str;
	
#if !__POWERPC__
	
	p = (wchar_t *) set;

	while (c = *p++)
		set_wchar_map(map, c);
	
	p = s;
	
	while (c = *p++)
		if (!tst_wchar_map(map, c))
			break;
	
	if (!c)
	{
		s = n;
		return(NULL);
	}
	
	q = p - 1;
	
	while (c = *p++)
		if (tst_wchar_map(map, c))
			break;
	
	if (!c)
		s = n;
	else
	{
		s    = p;
		*--p = 0;
	}
	
	return(( wchar_t *) q);

#else
	
	p = ( wchar_t *) set - 1;

	while (c = *++p)
		set_wchar_map(map, c);
	
	p = s - 1;
	
	while (c = *++p)
		if (!tst_wchar_map(map, c))
			break;
	
	if (!c)
	{
		s = n;
		return(NULL);
	}
	
	q = p;
	
	while (c = *++p)
		if (tst_wchar_map(map, c))
			break;
	
	if (!c)
		s = n;
	else
	{
		s  = p + 1;
		*p = L'\0';
	}
	
	return(( wchar_t *) q);

#endif
}
#endif /* __dest_os != win32 */





wchar_t * wcsstr(const wchar_t * str, const wchar_t * pat)
{
#if !__POWERPC__

	wchar_t * s1 = (wchar_t *) str;
	wchar_t * p1 = (wchar_t *) pat;
	wchar_t firstc, c1, c2;
	
	if (!(firstc = *p1++))		/* if pat is an empty string we return str */
		return((wchar_t *) str);

	while(c1 = *s1++)
		if (c1 == firstc)
		{
			const wchar_t * s2 = s1;
			const wchar_t * p2 = p1;
			
			while ((c1 = *s2++) == (c2 = *p2++) && c1);
			
			if (!c2)
				return((wchar_t *) s1 - 1);
		}
	
	return(NULL);

#else

	wchar_t * s1 = (wchar_t *) str-1;
	wchar_t * p1 = (wchar_t *) pat-1;
	wchar_t firstc, c1, c2;
	
	if (!(firstc = *++p1))		/* if pat is an empty string we return str */
		return((wchar_t *) str);

	while(c1 = *++s1)
		if (c1 == firstc)
		{
			const wchar_t * s2 = s1-1;
			const wchar_t * p2 = p1-1;
			
			while ((c1 = *++s2) == (c2 = *++p2) && c1);
			
			if (!c2)
				return((wchar_t *) s1);
		}
	
	return(NULL);

#endif
}



#pragma warn_possunwant reset


#endif /* #ifndef __NO_WIDE_CHAR */			/* mm 981030 */

/*  Change Record
 *	980121 	JCM  First code release.
 *  980318  mf   the header file string.win32.h does not implement
                 wide versions of the string functions so these 
                 were not defined at all
 * mf  980512       wince changes                 
 * mm 981030  Added #ifndef __NO_WIDE_CHAR wrappers                 
 */
