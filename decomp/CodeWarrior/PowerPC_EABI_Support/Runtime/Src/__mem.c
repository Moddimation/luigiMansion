/****************************************************************************/
/*

FILE
	__mem.c
	
DESCRIPTION
	
	Standalone Standard C Library Implementation for memory operations.
	This is a target-specific implementation for PowerPC.  It will
	probably work for other target processors as well, but this one
	has been improved specifically for PowerPC.
	
	memcpy and memset are  put into the .init section so that they will
	be available for use in copying sections (e.g. .text) from rom into ram.

	Derived from MSL C (CodeWarrior 10).

	memcpy
	memset
	strlen

COPYRIGHT

	(c) 1996-7 Metrowerks Corporation
	All rights reserved.
	
*/
/****************************************************************************/

#if __MWERKS__
#pragma ANSI_strict off
#endif

#include <__mem.h>
#include <string.h>

void * memcpy(void * dst, const void * src, size_t n)
{
	const	char * p;
				char * q;
				int		 rev = ((unsigned long) src < (unsigned long) dst);
	
	if (!rev)
	{
		
		for (p = (const char *) src - 1, q = (char *) dst - 1, n++; --n;)
			*++q = *++p;
	
	}
	else
	{
		for (p = (const char *) src + n, q = (char *) dst + n, n++; --n;)
			*--q = *--p;
	}
	return(dst);
}


/*
	mem_funcs.c
*/

#define cps	((unsigned char *) src)
#define cpd	((unsigned char *) dst)
#define lps	((unsigned long *) src)
#define lpd	((unsigned long *) dst)
#define deref_auto_inc(p)	*++(p)

void __fill_mem(void * dst, int val, unsigned long n)
{
	unsigned long			v = (unsigned char) val;
	unsigned long			i;
	
	cpd = ((unsigned char *) dst) - 1;
	
	if (n >= 32)
	{
		i = (~ (unsigned long) dst) & 3;

		if (i)
		{
			n -= i;
			
			do
				deref_auto_inc(cpd) = v;
			while (--i);
		}
	
		if (v)
			v |= v << 24 | v << 16 | v <<  8;
		
		lpd = ((unsigned long *) (cpd + 1)) - 1;
		
		i = n >> 5;
		
		if (i)
			do
			{
				deref_auto_inc(lpd) = v;
				deref_auto_inc(lpd) = v;
				deref_auto_inc(lpd) = v;
				deref_auto_inc(lpd) = v;
				deref_auto_inc(lpd) = v;
				deref_auto_inc(lpd) = v;
				deref_auto_inc(lpd) = v;
				deref_auto_inc(lpd) = v;
			}
			while (--i);
		
		i = (n & 31) >> 2;
		
		if (i)
			do
				deref_auto_inc(lpd) = v;
			while (--i);
		
		cpd = ((unsigned char *) (lpd + 1)) - 1;
		
		n &= 3;
	}
	
	if (n)
		do
			deref_auto_inc(cpd) = v;
		while (--n);
	
	return;
}

void * memset(void * dst, int val, size_t n)
{
	__fill_mem(dst, val, n);
	
	return(dst);
}

#if !__MC68K__ || _No_String_Inlines || !defined(__cplusplus)

#pragma overload size_t (strlen)(const char * str);
size_t (strlen)(const char * str)
{
	size_t	len = -1;
		
	unsigned char * p = (unsigned char *) str - 1;
	
	do
		len++;
	while (*++p);
		
	return(len);
}

#endif /* !__MC68K__ || _No_String_Inlines || !defined(__cplusplus) */
