/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 01:00:10 $ 
 *  $Revision: 1.7.4.2 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	float.c
 *	
 *	Modification History
 *	--------------------
 *  rewritten by matt fassiotto on 7/30/97
 *  definitions were based on which architecture when
 *  they should have been based only on "endian-ness"
 *  which is now the case.
 *  This assumes both the "long" and "float" type are 
 *  32 bits whenever  compiled
 */
#include <msl_t.h>
#include <float.h> 

static unsigned long	unused = 0;	/* ELR This is an alignment thing */
/*
long __float_min[]    ={0x00800000};
long __float_max[]    ={0x7F7FFFFF};
long __float_epsilon[]={0x34000000};
long __float_tiny[]   ={0x00000001};
*/
/* these are now obsolete as of pro 3 --m.f. 04/20/98 */
_INT32 __float_nan[]    ={0x7FFFFFFF};
_INT32 __float_huge[]   ={0x7f800000};

#if __MC68K__
 #if !__option(IEEEdoubles)
 
  _INT32 __double_min	 []	= {0x00018000,0,0};
  _INT32 __double_max	 []	= {0x7FFEFFFF, 0xFFFFFFFF,0xFFFF0000 };
  _INT32 __double_epsilon[]	= {0x3FC08000,0,0 } ;
  _INT32 __double_tiny	 []	= {0,0,0x00010000} ;
  _INT32 __double_huge	 []	= {0x7FFF0000,0,0 };
  _INT32 __double_nan	 []	= {0x7FFFFFFF,0xFFFFFFFF,0xFFFF0000 };
  
 #else
 _INT32 __double_min	 [] ={0x00100000,0}; 
 _INT32 __double_max	 [] ={0x7fefffff,0xffffffff};
 _INT32 __double_epsilon [] ={0x3cb00000,0};
 _INT32 __double_tiny	 [] ={0,1};
 _INT32 __double_huge	 [] ={0x7FF00000,0};
 _INT32 __double_nan	 [] ={0x7FFFFFFF,0xFFFFFFFF};
  
 #endif /* IEEEDoubles */
#else  /* MC68K */
 
 #if __option(little_endian) 
                     /* as of now all our little_endian architectures
                        do not support IEEE extended precision, therefore
                        long doubles are 64 bits instead of 80.
                        When our compilers support long double types the 
                        __extended... arrays will be added
                     */
 _INT32 __double_min	  [] ={0,0x00100000}; 
 _INT32 __double_max	  [] ={0xffffffff,0x7fefffff};
 _INT32 __double_epsilon  [] ={0,0x3cb00000};
 _INT32 __double_tiny	  [] ={1,0};
 _INT32 __double_huge	  [] ={0,0x7FF00000} ;
 _INT32 __double_nan	  [] ={0xFFFFFFFF,0x7FFFFFFF};

#else

 _INT32 __double_min	  [] ={0x00100000,0}; 
 _INT32 __double_max	  [] ={0x7fefffff,0xffffffff};
 _INT32 __double_epsilon  [] ={0x3cb00000,0};
 _INT32 __double_tiny	  [] ={0,1};
 _INT32 __double_huge	  [] ={0x7FF00000,0};
 _INT32 __double_nan	  [] ={0x7FFFFFFF,0xFFFFFFFF};

 #endif  /* little_endian */
#endif /* __MC68k__*/

#ifdef __MC68K__
 #if __MC68881__
 
  _INT32 __extended_min	   []	= {0x00010000,0x80000000, 0};
  _INT32 __extended_max	   []	= {0x7FFE0000,0xFFFFFFFF,0xFFFFFFFF};
  _INT32 __extended_epsilon[]	= {0x3FC00000,0x80000000,0x00000000};
  _INT32 __extended_tiny   []	= {0,0,1};
  _INT32 __extended_huge   []	= {0x7FFF0000,0,0};
  _INT32 __extended_nan	   []	= {0x7FFF0000,0xFFFFFFFF,0xFFFFFFFF};
 #else
  _INT32 __extended_min	   []	= {0x00018000,0,0};
  _INT32 __extended_max	   []	= {0x7FFEFFFF, 0xFFFFFFFF,0xFFFF0000 };
  _INT32 __extended_epsilon[]	= {0x3FC08000,0,0 } ;
  _INT32 __extended_tiny   []	= {0,0,0x00010000} ;
  _INT32 __extended_huge   []	= {0x7FFF0000,0,0 };
  _INT32 __extended_nan	   []	= {0x7FFFFFFF,0xFFFFFFFF,0xFFFF0000 };
  
 #endif /* __MC68881__ */
#else
/* our compilers support long double on 68k only */
  #if __option(little_endian)
   _INT32 __extended_min    []= {0,0x00100000} ;
   _INT32 __extended_max    []= {0xffffffff,0x7fefffff} ;
   _INT32 __extended_epsilon[]= {0,0x3cb00000} ;
   _INT32 __extended_tiny   []= {1,0};
   _INT32 __extended_huge   []= {0,0x7FF00000} ;
   _INT32 __extended_nan    []= {0xFFFFFFFF,0x7FFFFFFF} ;
 #else
   _INT32 __extended_min    []= {0x00100000,0} ;
   _INT32 __extended_max    []= {0x7fefffff,0xffffffff} ;
   _INT32 __extended_epsilon[]= {0x3cb00000,0} ;
   _INT32 __extended_tiny   []= {0,1};
   _INT32 __extended_huge   []= {0x7FF00000,0} ;
   _INT32 __extended_nan    []= {0x7FFFFFFF,0xFFFFFFFF} ;
 #endif /* __option(little_endian) */
  
#endif /* __M68K__ */

/*
	Change record
	04/20/98 -- m.f.  no longer need float constants.
	990729  vss  Include msl_t.h instead of cmath
*/
