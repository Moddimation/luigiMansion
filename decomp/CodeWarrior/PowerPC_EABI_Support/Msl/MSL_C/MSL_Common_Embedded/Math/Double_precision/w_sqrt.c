#if !(__MIPS__ && __MIPS_ISA2__ && __MIPS_double_fpu__)

#ifndef _No_Floating_Point  
/* @(#)w_sqrt.c 1.2 95/01/04 */
/* $Id: w_sqrt.c,v 1.7.2.1 1999/12/06 19:47:33 fassiott Exp $ */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/* 
 * wrapper sqrt(x)
 */

#include "fdlibm.h"

#if __MIPS__
	#define __OPTION(sqrt) __option(sqrt)
#else
	#define __OPTION(sqrt) 0
#endif

//#if !(__MIPS__ && __MIPS_double_fpu__ && __option(sqrt))
#if !(__MIPS__ && __MIPS_double_fpu__ && __OPTION(sqrt))

#ifdef __STDC__
	double sqrt(double x)		/* wrapper sqrt */
#else
	double sqrt(x)			/* wrapper sqrt */
	double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_sqrt(x);
#else
	double z;
	z = __ieee754_sqrt(x);
	if(_LIB_VERSION == _IEEE_ || isnan(x)) return z;
	if(x<0.0) {
	    #ifdef __STDC__
        errno=EDOM;  /* mf-- added to conform to old ANSI standard */
        #endif
	    return __kernel_standard(x,x,26); /* sqrt(negative) */
	} else
	    return z;
#endif
}
#endif /* __MIPS__	*/
#endif /* _No_Floating_Point  */
#endif /* !(__MIPS__ && __MIPS_ISA2__ && __MIPS_double_fpu__) */
