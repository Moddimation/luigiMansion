/*  Metrowerks Standard Library  */

/*  $Date: 1999/05/28 16:09:48 $ 
 *  $Revision: 1.8.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */


/*
 *	mem_funcs.c
 *	
 *	Routines
 *	--------
 *		__copy_longs_aligned
 *		__copy_longs_rev_aligned
 *		__copy_longs_unaligned
 *		__copy_longs_rev_unaligned
 *
 *		__fill_mem
 *
 *	Implementation
 *	--------------
 *		Specialized routines to copy memory by transferring longs and using
 *		unrolled loops whenever possible.
 *		
 *		There are two pairs of routines.
 *		
 *		One pair handles the cases where both the source and destination are
 *		mutually aligned, in which case it suffices to transfer bytes until they
 *		are long-aligned, then transfer longs, then finish up transferring any
 *		excess bytes.
 *		
 *		The other pair handles the cases where the source and destination are not
 *		mutually aligned. The handling is similar but requires repeatedly
 *		assembling a destination long from two source longs.
 *		
 *		Each pair consists of one routine to transfer bytes from first byte to last
 *		and another to transfer bytes from last byte to first. The latter should be
 *		used whenever the address of the source is less than the address of the
 *		destination in case they overlap each other.
 *		
 *		There are also two routines here that effectively duplicate memcpy and
 *		memmove, so that these routines may be used without bringing in the entire
 *		StdLib.
 *
 *		Oh, and let's not forget __fill_mem, which fills memory a long at a time
 *		and in an unrolled loop whenever possible. 
 *
 *
 */


#include "mem_funcs.h"


#pragma ANSI_strict off


#if !defined( __MOTO__) && !defined(__MIPS__)
#define cps	((unsigned char *) src)
#define cpd	((unsigned char *) dst)
#define lps	((unsigned long *) src)
#define lpd	((unsigned long *) dst)
#endif


#if !__POWERPC__
#define deref_auto_inc(p)	*(p)++
#else
#define deref_auto_inc(p)	*++(p)
#endif


void __copy_mem(void * dst, const void * src, unsigned long n)
{
	const	char * p;
				char * q;
	
	if (n >= __min_bytes_for_long_copy)
	{
		if ((((int) dst ^ (int) src)) & 3)
			__copy_longs_unaligned(dst, src, n);
		else
			__copy_longs_aligned(dst, src, n);
		
		return;
	}
#if !__POWERPC__
	
	for (p = (const char *) src, q = (char *) dst, n++; --n;)
		*q++ = *p++;
	
#else
	
	for (p = (const char *) src - 1, q = (char *) dst - 1, n++; --n;)
		*++q = *++p;
#endif
}

void __move_mem(void * dst, const void * src, unsigned long n)
{
	const	char * p;
				char * q;
				int		 rev = ((unsigned long) src < (unsigned long) dst);
	
	if (n >= __min_bytes_for_long_copy)
	{
		if ((((int) dst ^ (int) src)) & 3)
			if (!rev)
				__copy_longs_unaligned(dst, src, n);
			else
				__copy_longs_rev_unaligned(dst, src, n);
		else
			if (!rev)
				__copy_longs_aligned(dst, src, n);
			else
				__copy_longs_rev_aligned(dst, src, n);
		
		return;
	}


	if (!rev)
	{
		
#if !__POWERPC__
		
		for (p = (const char *) src, q = (char *) dst, n++; --n;)
			*q++ = *p++;
	
#else
		
		for (p = (const char *) src - 1, q = (char *) dst - 1, n++; --n;)
			*++q = *++p;
	
#endif
		
	}
	else
	{
		for (p = (const char *) src + n, q = (char *) dst + n, n++; --n;)
			*--q = *--p;
	}
}


void __copy_longs_aligned(void * dst, const void * src, unsigned long n)
{
#if( __MOTO__  || __MIPS__)
	unsigned char *	cps	= ((unsigned char *) src);
	unsigned char *	cpd	= ((unsigned char *) dst);
	unsigned long *	lps	= ((unsigned long *) src);
	unsigned long *	lpd	= ((unsigned long *) dst);
#endif

	unsigned long	i;
	
	i = (- (unsigned long) dst) & 3;
	
#if __POWERPC__
	cps = ((unsigned char *) src) - 1;
	cpd = ((unsigned char *) dst) - 1;
#endif
	
	if (i)
	{
		n -= i;
		
		do
			deref_auto_inc(cpd) = deref_auto_inc(cps);
		while (--i);
	}
	
#if __POWERPC__
	lps = ((unsigned long *) (cps + 1)) - 1;
	lpd = ((unsigned long *) (cpd + 1)) - 1;
#endif
	
	i = n >> 5;
	
	if (i)
		do
		{
			deref_auto_inc(lpd) = deref_auto_inc(lps);
			deref_auto_inc(lpd) = deref_auto_inc(lps);
			deref_auto_inc(lpd) = deref_auto_inc(lps);
			deref_auto_inc(lpd) = deref_auto_inc(lps);
			deref_auto_inc(lpd) = deref_auto_inc(lps);
			deref_auto_inc(lpd) = deref_auto_inc(lps);
			deref_auto_inc(lpd) = deref_auto_inc(lps);
			deref_auto_inc(lpd) = deref_auto_inc(lps);
		}
		while (--i);
	
	i = (n & 31) >> 2;
	
	if (i)
		do
			deref_auto_inc(lpd) = deref_auto_inc(lps);
		while (--i);
	
#if __POWERPC__
	cps = ((unsigned char *) (lps + 1)) - 1;
	cpd = ((unsigned char *) (lpd + 1)) - 1;
#endif
	
	n &= 3;
	
	if (n)
		do
			deref_auto_inc(cpd) = deref_auto_inc(cps);
		while (--n);
	
	return;
}


void __copy_longs_rev_aligned(void * dst, const void * src, unsigned long n)
{
#if( __MOTO__  || __MIPS__)
	unsigned char *	cps	= ((unsigned char *) src);
	unsigned char *	cpd	= ((unsigned char *) dst);
	unsigned long *	lps	= ((unsigned long *) src);
	unsigned long *	lpd	= ((unsigned long *) dst);
#endif
	unsigned long			i;
	
	cps = ((unsigned char *) src) + n;
	cpd = ((unsigned char *) dst) + n;
	
	i = ((unsigned long) cpd) & 3;
	
	if (i)
	{
		n -= i;
		
		do
			*--cpd = *--cps;
		while (--i);
	}
	
	i = n >> 5;
	
	if (i)
		do
		{
			*--lpd = *--lps;
			*--lpd = *--lps;
			*--lpd = *--lps;
			*--lpd = *--lps;
			*--lpd = *--lps;
			*--lpd = *--lps;
			*--lpd = *--lps;
			*--lpd = *--lps;
		}
		while (--i);
	
	i = (n & 31) >> 2;
	
	if (i)
		do
			*--lpd = *--lps;
		while (--i);
	
	n &= 3;
	
	if (n)
		do
			*--cpd = *--cps;
		while (--n);
	
	return;
}

void __copy_longs_unaligned(void * dst, const void * src, unsigned long n)
{
#if( __MOTO__  || __MIPS__)
	unsigned char *	cps	= ((unsigned char *) src);
	unsigned char *	cpd	= ((unsigned char *) dst);
	unsigned long *	lps	= ((unsigned long *) src);
	unsigned long *	lpd	= ((unsigned long *) dst);
#endif


	unsigned long	i, v1, v2;
	unsigned int	src_offset, left_shift, right_shift;
	
	i = (- (unsigned long) dst) & 3;
	
#if __POWERPC__
	cps = ((unsigned char *) src) - 1;
	cpd = ((unsigned char *) dst) - 1;
#endif
	
	if (i)
	{
		n -= i;
		
		do
			deref_auto_inc(cpd) = deref_auto_inc(cps);
		while (--i);
	}

#if !__POWERPC__
	src_offset = ((unsigned int) cps) & 3;
#else
	src_offset = ((unsigned int) (cps + 1)) & 3;
#endif
	
	left_shift  = src_offset << 3;
	right_shift = 32 - left_shift;
	
	cps -= src_offset;
	
#if __POWERPC__
	lps = ((unsigned long *) (cps + 1)) - 1;
	lpd = ((unsigned long *) (cpd + 1)) - 1;
#endif
	
	i = n >> 3;
	
	v1 = deref_auto_inc(lps);
	
	do
	{
		v2                  = deref_auto_inc(lps);
		deref_auto_inc(lpd) = (v1 << left_shift) | (v2 >> right_shift);
		v1                  = deref_auto_inc(lps);
		deref_auto_inc(lpd) = (v2 << left_shift) | (v1 >> right_shift);
	}
	while (--i);
	
	if (n & 4)
	{
		v2                  = deref_auto_inc(lps);
		deref_auto_inc(lpd) = (v1 << left_shift) | (v2 >> right_shift);
	}
	
#if __POWERPC__
	cps = ((unsigned char *) (lps + 1)) - 1;
	cpd = ((unsigned char *) (lpd + 1)) - 1;
#endif
	
	n &= 3;
	
	if (n)
	{
		cps -= 4 - src_offset;
		do
			deref_auto_inc(cpd) = deref_auto_inc(cps);
		while (--n);
	}
	
	return;
}


void __copy_longs_rev_unaligned(void * dst, const void * src, unsigned long n)
{
#if( __MOTO__  || __MIPS__)
	unsigned char *	cps	= ((unsigned char *) src);
	unsigned char *	cpd	= ((unsigned char *) dst);
	unsigned long *	lps	= ((unsigned long *) src);
	unsigned long *	lpd	= ((unsigned long *) dst);
#endif

	unsigned long	i, v1, v2;
	unsigned int	src_offset, left_shift, right_shift;
	
	cps = ((unsigned char *) src) + n;
	cpd = ((unsigned char *) dst) + n;
	
	i = ((unsigned long) cpd) & 3;
	
	if (i)
	{
		n -= i;
		
		do
			*--cpd = *--cps;
		while (--i);
	}
	
	src_offset = ((unsigned int) cps) & 3;
	
	left_shift  = src_offset << 3;
	right_shift = 32 - left_shift;
	
	cps += 4 - src_offset;
	
	i = n >> 3;
	
	v1 = *--lps;
	
	do
	{
		v2     = *--lps;
		*--lpd = (v2 << left_shift) | (v1 >> right_shift);
		v1     = *--lps;
		*--lpd = (v1 << left_shift) | (v2 >> right_shift);
	}
	while (--i);
	
	if (n & 4)
	{
		v2     = *--lps;
		*--lpd = (v2 << left_shift) | (v1 >> right_shift);
	}
	
	n &= 3;
	
	if (n)
	{
		cps += src_offset;
		do
			*--cpd = *--cps;
		while (--n);
	}
	
	return;
}


#if !__PPC_EABI__
#if __MIPS__

void __fill_mem(void *dst, int val, unsigned long n)
{
		/*	Note: we always use a long long to make the initialization	*/
		/*	This code will work even on a 32 bit machine.  This is like	*/
		/*	doing loop unrolling.										*/
		
	unsigned char *cpd	= ((unsigned char *) dst);
	unsigned long long *lpd;
	unsigned char c = (unsigned char)val;
	unsigned long long v;
	

		/*	Copy 1 byte at a time until the destination is aligned	*/
	
	while (((unsigned int)cpd & (sizeof(long long) - 1)) && n > 0) {
		*cpd = c;
		--n;
		++cpd;
	}
	
		
		/*	Copy 1 word at a time	*/
		
	lpd = (unsigned long long *)cpd;
	v = c << 8 | c;
	v |= v << 16;
	v |= v << 32;
			
	while (n > sizeof(long long)) {
		*lpd = v;
		n -= sizeof(long long);
		++lpd;
	}	
	
	
		/*	Copy 1 byte at a time	*/
	
	cpd = (unsigned char *)lpd;
	while (n) {
		*cpd = c;
		--n;
		++cpd;
	}	
}


#else
void __fill_mem(void * dst, int val, unsigned long n)
{
#if( __MOTO__  || __MIPS__)
/*	unsigned char *	cps	= ((unsigned char *) src);	*/
	unsigned char *	cpd	= ((unsigned char *) dst);
/*	unsigned long *	lps	= ((unsigned long *) src);	*/
	unsigned long *	lpd	= ((unsigned long *) dst);
#endif


	unsigned long			v = (unsigned char) val;
	unsigned long			i;
	
#if __POWERPC__ 
	cpd = ((unsigned char *) dst) - 1;
#endif
	
	if (n >= 32)
	{
#if __POWERPC__ 
		i = (~ (unsigned long) dst) & 3;
#else		
		i = (-(unsigned long) dst) & 3;
#endif		
		if (i)
		{
			n -= i;
			
			do
				deref_auto_inc(cpd) = v;
			while (--i);
		}
	
		if (v)
			v |= v << 24 | v << 16 | v <<  8;
		
#if __POWERPC__ 
		lpd = ((unsigned long *) (cpd + 1)) - 1;
#endif
		
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
		
#if __POWERPC__ 
		cpd = ((unsigned char *) (lpd + 1)) - 1;
#endif
		
		n &= 3;
	}
	
	if (n)
		do
			deref_auto_inc(cpd) = v;
		while (--n);
	
	return;
}

#endif /*	!__MIPS__	*/
#endif /* !__PPC_EABI__ */


/*  Change Record
 *	15-Nov-95 JFH  First code release.
 *	19-Jan-96 JFH  Fixed bug in the PowerPC version of __copy_longs_unaligned where
 *								 the destination address was being tested for alignment *after*
 *								 decrementing the address in preparation for pre-increment accesses.
 *  22-Jan-96 JFH  Added casts from (void *) for C++ compatibility.
 *	21-Feb-96 JFH  Fixed bug in __copy_longs_aligned analagous to the one in
 *								 __copy_longs_unaligned.
 *	10-Jul-97 SCM  Added support for __PPC_EABI__.
 *	08-Mar-98 MEA  In __fill_mem, changed i = (- (unsigned long) dst) & 3; to
 *									to i = (~ (unsigned long) dst) & 3;
 *  012699    mf   undid change above outside of ppc.  It breaks other platforms badly.
*/
