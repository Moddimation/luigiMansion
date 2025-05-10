/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/03 23:55:18 $ 
 *  $Revision: 1.12.4.1.2.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	mem.c
 *	
 *	Routines
 *	--------
 *		memcpy
 *		memmove
 *		memset
 *		memchr
 *		memcmp
 *	
 *
*/

#include <string.h>
#include "mem_funcs.h"

#if __INTEL__
	
	#include <mem.x86.h> /* 012099 m.f. changed header file name */

#else  /* Be, MacOS, all others. . */

	#if __dest_os == __mac_os
	#include <MacMemory.h>		/*  990421  vss  */
	#endif

	#if __dest_os == __be_os
	#include <mem.be.h>
	#endif
	
	#if (__dest_os == mac_os) && defined(__POWERPC__)
	#define __min_bytes_for_long_copy	32	/* NEVER let this be < 16 */
	#endif

	#if  (__dest_os != __be_os) && (!__PPC_EABI__)
	
	void * (memcpy)(register void * dst, register const void * src, register size_t n)
	{
		#if __dest_os == __mac_os && !_No_BlockMove
			
				#if __POWERPC__  /*  PowerPC && mac_os optimization  */
		
/*Assumptions:		990129 BLC  
 * 
 * - Miss aligned reads are supported (on a G3 miss aligned reads seem to 
 * preform better than the code for manually aligning the data). To kept the 
 * penalty to a minimum, I use miss-aligned reads but aligned writes.
 * 
 * - The processor supports floating point, and if the both operands can 
 * become double aligned it is much faster to use doubles to copy the data.
 *
 * - The 2.2 compiler is required for optimal performance since it will
 *   unroll the loops.
 * 
 */
    			const   char    *p;
            			char    *q;

    			size_t  i;
    			size_t  n2;

    			if (n < __min_bytes_for_long_copy) 
    			{
        			if (n > 0) 
        			{

            			q = ((char *) dst) - 1;
            			p = ((const char *) src) - 1;

            			for (i = 0; i < n; i++) 
            			{
                			*++q = *++p;
            			}
        			}
        			return dst;
    			}

    			/* doing a large copy */

    			q = (char *) dst;
    			p = ((const char *) src);
    			if (((unsigned long) q) & 0x07) 
    			{

       		 	p = ((char *) p) - 1;

        			do {
            			*q++ = *++p;
            			n--;
        			} while (((unsigned long) q) & 0x07);
        			p++;
    			}

    			/* dst is now double aligned */

    			if ((((unsigned long) p) & 0x07) != 0) 
    			{
        			/* src is not double aligned */

        			q = q - 4;
        			p = p - 4;

        			n2 = n / 4;
        			for (i = 0; i < n2; i++) 
        			{
            			q += sizeof(unsigned long);
            			p += sizeof(unsigned long);
            			*((unsigned long *) q) = *((unsigned long *) p);

            			n = n - 4;
        			}
       	 			p = p + 4;
        			q = q + 4;

    			} 
    			else 
    			{
        			/* dst and src are double aligned */

        			q = q - 8;
        			p = p - 8;

        			n2 = n / 8;
        			for (i = 0; i < n2; i++) 
        			{
            			q += sizeof(double);
            			p += sizeof(double);
            			*((double *) q) = *((double *) p);
            			n -= 8;
        			}
        			p = p + 8;
        			q = q + 8;
    			}

    			p--;
    			q--;
    			for (i = 0; i < n; i++) 
    			{
        			*++q = *++p;
    			}
    			
    			return dst;
			
			#else  /*  do BlockMoveData on non-PPC MacOS architectures */
			
				BlockMoveData(src, dst, n);
			
			#endif  /*  __POWERPC  */

		#else /* __dest_os != __mac_os || _No_BlockMove */

			const char * p	=	(char*)src;
			char  * q		=	(char*)dst;
		
			#ifndef __MIPS__
				if (n >= __min_bytes_for_long_copy)
				{
					if ((((int) dst ^ (int) src)) & 3)
						__copy_longs_unaligned(dst, src, n);
					else
						__copy_longs_aligned(dst, src, n);
			
					return(dst);
				}
			#endif		

			#if !__POWERPC__
		
				/*for (p = src, q = dst, n++; --n;)*/
				for(n++;--n;)
					*q++ = *p++;
		
			#else
		
				for (p = (const char *) src - 1, q = (char *) dst - 1, n++; --n;)
					*++q = *++p;

			#endif

		#endif /* __dest_os == __mac_os && !_No_BlockMove */
		
		return(dst);
	}

	#endif /* (__dest_os != __be_os) && (!__PPC_EABI__) */

	void * (memmove)(void * dst, const void * src, size_t n)
	{
		#if __dest_os == __mac_os && !_No_BlockMove
			
			#if __POWERPC__	
			
/*Assumptions:		990129 BLC  
 * 
 * - Miss aligned reads are supported (on a G3 miss aligned reads seem to 
 * preform better than the code for manually aligning the data). To kept the 
 * penalty to a minimum, I use miss-aligned reads but aligned writes.
 * 
 * - The processor supports floating point, and if the both operands can 
 * become double aligned it is much faster to use doubles to copy the data.
 *
 * - The 2.2 compiler is required for optimal performance since it will
 *   unroll the loops.
 * 
 */				
			
				const	char    *p;
            			char    *q;

    			size_t  i;
    			size_t  n2;

    			if (src > dst) 
    			{
       	 			return memcpy(dst, src, n);
    			}
    			if (dst == src) return dst;

    			p = (const char *) src + n;
    			q = (char *) dst + n;

    			if (n < __min_bytes_for_long_copy) 
    			{

        			for (i = 0; i < n; i++) 
        			{
            			*--q = *--p;
        			}
        			return dst;
    			} 
    			else 
    			{

        		/* we are now handling more the __min_bytes_for_long_copy */

        		/* only align the destination */
        		/* don't use a counting loop here we don't want it unrolled!! */
        			while (((unsigned int) q) & 0x07) 
        			{
            			*--q = *--p;
            			n--;
        			}

        		/* the destination is now double aligned */

        			if ((((unsigned int) p) & 0x07)!=0) 
        			{
            			/* src is not double aligned */

            			n2 = n / 4;
            			for (i = 0; i < n2; i++) 
            			{
                			q -= sizeof(unsigned long);
                			p -= sizeof(unsigned long);
                			*((unsigned long *) q) = *((unsigned long *) p);
                			n -= 4;
            			}

        			} 
        			else 
        			{
            			/* dst and src are double aligned */

            			n2 = n / 8;
            			for (i = 0; i < n2; i++) 
            			{
                			q -= sizeof(double);
                			p -= sizeof(double);
                			*((double *) q) = *((double *) p);
                			n -= 8;
            			}
        			}

        			/* finish any odd bytes */
        			for (i = 0; i < n; i++) 
        			{
           	 			*--q = *--p;
        			}
    			}
    			
    			return dst;

			#else  /* end optimization __POWERPC__ && MacOS architectures */ 
			
				BlockMoveData(src, dst, n);
				
			#endif /* __POWERPC__  */

		#else /* __dest_os != __mac_os || _No_BlockMove */

			const	char * p;
					char * q;
					int	rev = ((unsigned long) src < (unsigned long) dst);
					
			#ifndef __MIPS__
			
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
			
					return(dst);
				}
				
			#endif
			
			if (!rev)
			{
		
				#if !__POWERPC__
		
					for (p = src, q = dst, n++; --n;)
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

		#endif /* __dest_os == __mac_os && !_No_BlockMove */
	
		return(dst);
	}

	#if !__PPC_EABI__
	
		void * memset(void * dst, int val, size_t n)
		{
			__fill_mem(dst, val, n);
	
			return(dst);
		}
		
	#endif

	#ifndef UNDER_CE
	
		void * memchr(const void * src, int val, size_t n)
		{
			const unsigned char *	p;
	
			#if !__POWERPC__
				unsigned long	v = (val & 0xff);                /* hh 980624 */
	
				for (p = (unsigned char *) src, n++; --n;)
					if (*p++ == v)
						return((void *) (p-1));
	
			#else
	
				unsigned long	v = (val & 0xff);                /* mm 980425 */
	
				for (p = (unsigned char *) src - 1, n++; --n;)
					if ((*++p & 0xff) == v)						/* mm 980425 */
						return((void *) p);

			#endif
	
			return(NULL);
		}
	#endif

	void * __memrchr(const void * src, int val, size_t n)
	{
		const unsigned char *	p;
	
		#if !__POWERPC__

			unsigned char	v = (val & 0xff);                /* hh 980624 */
	
			for (p = (unsigned char *) src + n, n++; --n;)
				if (*--p == v)
					return((void *) p);
	
		#else
	
			unsigned long	v = (val & 0xff);                /* hh 980624 */
	
			for (p = (unsigned char *) src + n, n++; --n;)
				if (*--p == v)
					return((void *) p);

		#endif
	
		return(NULL);
	}

	int memcmp(const void * src1, const void * src2, size_t n)
	{
		const	unsigned char * p1;
		const	unsigned char * p2;

		#if !__POWERPC__
	
			for (p1 = (const unsigned char *) src1, p2 = (const unsigned char *) src2, n++; --n;)
				if (*p1++ != *p2++)
					return((*--p1 < *--p2) ? -1 : +1);
	
		#else
	
			for (p1 = (const unsigned char *) src1 - 1, p2 = (const unsigned char *) src2 - 1, n++; --n;)
				if (*++p1 != *++p2)
					return((*p1 < *p2) ? -1 : +1);

		#endif
	
		return(0);
	}

#endif /* __INTEL__ */

/*  Change Record
 *	24-May-95 JFH  First code release.
 *	15-Nov-95 JFH  Sped up memset by having it use a routine which writes longs and,
 *								 when possible, does so in an unrolled loop.
 *	21-Nov-95 JFH  Sped up memcpy and memmove by having them use routines that copy longs
 *								 and, when possible, does so in an unrolled loop.
 *	30-Dec-95 JFH  Added explicit #include of <Memory.h>
 *	09-Jan-96 JFH  Added __memrchr (analogous to strrchr). Used by fwrite for line buffering.
 *	19-Jan-96 JFH  Bracketed #include of <Memory.h> by test for __mac_os
 *  22-Jan-96 JFH  Bracketed memcpy() and memmove() by #if <condition too complex to put here>
 *								 (they are inlined in <string.h> if this condition fails).
 *	 1-Mar-96 JFH  Merged Be code into source. Most of that was actually shuffled off to an
 *								 #include file.
 *	29-Apr-96 JFH  Merged Win32 changes in. As above, most of that was actually shuffled off
 *						CTV	 to an #include file.
 *  mm 961227      Prevented duplicate declaration of memmove under c++/68k
 *	10-Jul-97 SCM  Added support for __PPC_EABI__.
 *  bb 970718      Took out lines that prevented memcpy and memmove from being seen
 *                 in standard library test.
 *  mm 980425      Change to make memchr work for characters of value >127 on PPC.  MW02625 
 *  980429 mf      changed macro __dest_os_== win32_os to __INTEL__ so all intel oses pick
 *                 up optimized string
 *                 functions (eg. wince and beos)           
 *  hh 980624     Change to make memchr work for characters of value >127 on PPC.  MW02625 
 *  990120 mf     changed intel header file name from mem.win32.h to mem.x86.h
 *  990129 BLC	  Optimized the PPC & MacOS memcpy and memmove functions
 * vss 990421	Update to Universal Headers 3.2
*/
