/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:32 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	arith.c
 *	
 *	Routines
 *	--------
 *		abs
 *		div
 *
 *		labs
 *		ldiv
 *
 *		__add
 *		__ladd
 *
 *		__mul
 *		__lmul
 *
 *		__div
 *		__ldiv
 *
 *		__mod
 *		__lmod
 *
 *	Notes
 *	-----
 *
 *		__add and __ladd provide int and long addition with overflow checking. If
 *		overflow occurs, a zero value is returned; otherwise, the sum replaces the
 *		value pointed to by the first argument and a nonzero value is returned.
 *		
 *		__mul and __lmul provide int and long multiplication with overflow checking
 *		in a similar fashion. Note that a result of INT_MIN or LONG_MIN (whose
 *		magnitudes are 1 greater than INT_MAX and LONG_MAX respectively) is
 *		considered an overflow.
 *		
 *		__div and __ldiv provide int and long division/remainder. The definition of
 *		remainder here is different from that of div and ldiv and provides for
 *		more mmathematically consistent modular arithmetic (see Knuth Vol. 1, pp.
 *		37-38).
 *
 *		__mod and __lmod return the same remainder as __div and __ldiv do, but
 *		without the quotient - just a single scalar return value.
 *
 *
 */

#include "arith.h"
#include <limits.h>
#include <stdlib.h>

int (abs)(int n)
{
	if (n < 0)
		return(-n);
	else
		return(n);
}

long (labs)(long n)
{
	if (n < 0)
		return(-n);
	else
		return(n);
}

/* hh 980122 added long long support */
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
long long (llabs)(long long n)
{
	if (n < 0)
		return(-n);
	else
		return(n);
}
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */

div_t div(int numerator, int denominator)
{
	int		n_sign, d_sign;
	div_t	value;
	
	n_sign = 1;
	d_sign = 1;
	
	if (numerator < 0)
	{
		numerator = -numerator;
		n_sign    = -1;
	}
	
	if (denominator < 0)
	{
		denominator = -denominator;
		d_sign      = -1;
	}
	
	value.quot = (numerator / denominator) * (n_sign * d_sign);
	value.rem  = (numerator * n_sign) - (value.quot * denominator * d_sign);
	
	return(value);
}

ldiv_t ldiv(long numerator, long denominator)
{
	long		n_sign, d_sign;
	ldiv_t	value;
	
	n_sign = 1;
	d_sign = 1;
	
	if (numerator < 0)
	{
		numerator = -numerator;
		n_sign    = -1;
	}
	
	if (denominator < 0)
	{
		denominator = -denominator;
		d_sign      = -1;
	}
	
	value.quot = (numerator / denominator) * (n_sign * d_sign);
	value.rem  = (numerator * n_sign) - (value.quot * denominator * d_sign);
	
	return(value);
}

/* hh 980122 added long long support */
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
lldiv_t lldiv(long long numerator, long long denominator)
{
	long long		n_sign, d_sign;
	lldiv_t	value;
	
	n_sign = 1;
	d_sign = 1;
	
	if (numerator < 0)
	{
		numerator = -numerator;
		n_sign    = -1;
	}
	
	if (denominator < 0)
	{
		denominator = -denominator;
		d_sign      = -1;
	}
	
	value.quot = (numerator / denominator) * (n_sign * d_sign);
	value.rem  = (numerator * n_sign) - (value.quot * denominator * d_sign);
	
	return(value);
}
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */

int __add(int * x, int y)
{
	int _x = *x;
	
	if (y < 0)
	{
		if (_x < 0 && y < INT_MIN - _x)
			return(0);
	}
	else
		if (_x > 0 && y > INT_MAX - _x)
			return(0);
	
	*x = _x + y;
	
	return(1);
}

int __ladd(long * x, long y)
{
	long _x = *x;
	
	if (y < 0)
	{
		if (_x < 0 && y < LONG_MIN - _x)
			return(0);
	}
	else
		if (_x > 0 && y > LONG_MAX - _x)
			return(0);
	
	*x = _x + y;
	
	return(1);
}

/* hh 980122 added long long support */
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
int __lladd(long long * x, long long y)
{
	long long _x = *x;
	
	if (y < 0)
	{
		if (_x < 0 && y < LLONG_MIN - _x)
			return(0);
	}
	else
		if (_x > 0 && y > LLONG_MAX - _x)
			return(0);
	
	*x = _x + y;
	
	return(1);
}
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */

int __mul(int * x, int y)
{
	int	_x = *x;
	int	sign;
	
	sign = ((_x < 0) ^ (y < 0)) ? -1 : 1;
	
	if (_x < 0) _x = -_x;
	if ( y < 0)  y = - y;
	
	if (_x > INT_MAX / y)
		return(0);
	
	*x = _x * y * sign;
	
	return(1);
}

int __lmul(long * x, long y)
{
	long	_x = *x;
	int		sign;
	
	sign = ((_x < 0) ^ (y < 0)) ? -1 : 1;
	
	if (_x < 0) _x = -_x;
	if ( y < 0)  y = - y;
	
	if (_x > LONG_MAX / y)
		return(0);
	
	*x = _x * y * sign;
	
	return(1);
}

/* hh 980122 added long long support */
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
int __llmul(long long * x, long long y)
{
	long long	_x = *x;
	int		sign;
	
	sign = ((_x < 0) ^ (y < 0)) ? -1 : 1;
	
	if (_x < 0) _x = -_x;
	if ( y < 0)  y = - y;
	
	if (_x > LLONG_MAX / y)
		return(0);
	
	*x = _x * y * sign;
	
	return(1);
}
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */

div_t __div(int x, int y)
{
	int		q, r, x_sign, y_sign, q_sign;
	div_t	result;
	
	x_sign = 1;
	y_sign = 1;
	
	if (x < 0)
	{
		x      = -x;
		x_sign = -1;
	}
	
	if (y < 0)
	{
		y      = -y;
		y_sign = -1;
	}
	
	q_sign = x_sign * y_sign;
	
	q = (x / y) * q_sign;
	r = (x * x_sign) - (q * y * y_sign);
	
	if (r && q_sign < 0)
	{
		q -= 1;
		r += y * y_sign;
	}
	
	result.quot = q;
	result.rem  = r;
	
	return(result);
}

ldiv_t __ldiv(long x, long y)
{
	long		q, r, x_sign, y_sign, q_sign;
	ldiv_t	result;
	
	x_sign = 1;
	y_sign = 1;
	
	if (x < 0)
	{
		x      = -x;
		x_sign = -1;
	}
	
	if (y < 0)
	{
		y      = -y;
		y_sign = -1;
	}
	
	q_sign = x_sign * y_sign;
	
	q = (x / y) * q_sign;
	r = (x * x_sign) - (q * y * y_sign);
	
	if (r && q_sign < 0)
	{
		q -= 1;
		r += y * y_sign;
	}
	
	result.quot = q;
	result.rem  = r;
	
	return(result);
}

/* hh 980122 added long long support */
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
lldiv_t __lldiv(long long x, long long y)
{
	long long		q, r, x_sign, y_sign, q_sign;
	lldiv_t	result;
	
	x_sign = 1;
	y_sign = 1;
	
	if (x < 0)
	{
		x      = -x;
		x_sign = -1;
	}
	
	if (y < 0)
	{
		y      = -y;
		y_sign = -1;
	}
	
	q_sign = x_sign * y_sign;
	
	q = (x / y) * q_sign;
	r = (x * x_sign) - (q * y * y_sign);
	
	if (r && q_sign < 0)
	{
		q -= 1;
		r += y * y_sign;
	}
	
	result.quot = q;
	result.rem  = r;
	
	return(result);
}
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */

int __mod(int x, int y)
{
	int		q, r, x_sign, y_sign, q_sign;
	
	x_sign = 1;
	y_sign = 1;
	
	if (x < 0)
	{
		x      = -x;
		x_sign = -1;
	}
	
	if (y < 0)
	{
		y      = -y;
		y_sign = -1;
	}
	
	q_sign = x_sign * y_sign;
	
	q = (x / y) * q_sign;
	r = (x * x_sign) - (q * y * y_sign);
	
	if (r && q_sign < 0)
	{
		q -= 1;
		r += y * y_sign;
	}
	
	return(r);
}

long __lmod(long x, long y)
{
	long		q, r, x_sign, y_sign, q_sign;
	
	x_sign = 1;
	y_sign = 1;
	
	if (x < 0)
	{
		x      = -x;
		x_sign = -1;
	}
	
	if (y < 0)
	{
		y      = -y;
		y_sign = -1;
	}
	
	q_sign = x_sign * y_sign;
	
	q = (x / y) * q_sign;
	r = (x * x_sign) - (q * y * y_sign);
	
	if (r && q_sign < 0)
	{
		q -= 1;
		r += y * y_sign;
	}
	
	return(r);
}

/* hh 980122 added long long support */
#ifdef __MSL_LONGLONG_SUPPORT__					/* mm 981023 */
long long __llmod(long long x, long long y)
{
	long long		q, r, x_sign, y_sign, q_sign;
	
	x_sign = 1;
	y_sign = 1;
	
	if (x < 0)
	{
		x      = -x;
		x_sign = -1;
	}
	
	if (y < 0)
	{
		y      = -y;
		y_sign = -1;
	}
	
	q_sign = x_sign * y_sign;
	
	q = (x / y) * q_sign;
	r = (x * x_sign) - (q * y * y_sign);
	
	if (r && q_sign < 0)
	{
		q -= 1;
		r += y * y_sign;
	}
	
	return(r);
}
#endif	/* #ifdef __MSL_LONGLONG_SUPPORT__	*/	/* mm 981023 */

/*     Change record
 *	07-Jun-95 JFH  First code release.
 *	05-Oct-95 JFH  Added __add, __ladd, etc. - primarily for time.c
 *	14-Feb-96 JFH  Brackets abs() and labs() by #if !(__POWERPC__ && __cplusplus)
 *								 because they are inlined in <stdlib.h> under those conditions.
 *  hh 980205 undid previous #if because this file must always be compiled C.  __cplusplus is
 *            never defined.
 * hh 980122 added long long support 
 * mm 981023 added wrappers round long long support
*/
