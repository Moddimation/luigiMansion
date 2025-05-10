#if !(__MIPS__ && __MIPS_ISA2__ && __MIPS_double_fpu__)
#ifndef _No_Floating_Point  
/* @(#)s_fabs.c 1.2 95/01/04 */
/* $Id: s_fabs.c,v 1.4.4.1 1999/12/06 19:47:31 fassiott Exp $ */
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
 * fabs(x) returns the absolute value of x.
 */

#if !defined(__MIPS__)

#include "fdlibm.h"

#ifdef __STDC__
	 double   fabs(double x)	/* wrapper pow */
#else
	double fabs(x)
	double x;
#endif
{
	__HI(x) &= 0x7fffffff;
        return x;
}

#endif /* !defined(__MIPS__)	*/

#endif /* _No_Floating_Point  */
#endif /* !(__MIPS__ && __MIPS_ISA2__ && __MIPS_double_fpu__) */
