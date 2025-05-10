/*  Metrowerks Standard Library  Version 2.4  1998 March 10  */

/*  $Date: 1999/12/09 22:18:39 $ 
 *  $Revision: 1.3.2.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1998 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	ansi_fp.c
 *	
 *	Routines
 *	--------
 *		These are routines similar to those from MathLib needed by non-Mac systems.
 *		Note that we assume 8-byte doubles here.
 *
 *			__num2dec -	same as num2dec, except that we don't support the FIXEDDECIMAL
 *									style result (all conversions are handled in printf using the
 *									FLOATDECIMAL output)
 *
 *			__dec2num -	same as dec2num
 *
 *	Modification History
 *	--------------------
 *
 *	19-Oct-95 JFH  First code release.
 *	07-Dec-95 JFH  Changed Inf and NaN from zero-divide expressions to bit-pattern
 *								 references. Be's SCO barfed on the expressions.
 *	15-Jul-97 SCM  Disabled when _No_Floating_Point defined.
 */

#ifndef _No_Floating_Point

#include "ansi_fp.h"
#include <errno.h>
#include <float.h>
#include <stdlib.h>

static const double bit_values[] =
{
	1.0E+001,
	1.0E+002,
	1.0E+004,
	1.0E+008,
	1.0E+016,
	1.0E+032,
	1.0E+064,
	1.0E+128,
	1.0E+256
};

#define max_bits	511		/* (2**n)-1 where n is the number of elements in bit_values */

static const double digit_values[] =
{
	1.0E+01,
	1.0E+02,
	1.0E+03,
	1.0E+04,
	1.0E+05,
	1.0E+06,
	1.0E+07,
	1.0E+08
};

extern _INT32 __double_nan	 [];      /* hh 971202  added extern, defined in float.c*/
#define Inf	HUGE_VAL   /*  HUGE_VAL is the double encoding of infinity */
#define NaN	(* (double *) __double_nan )

void __num2dec(const decform *f, double x, decimal *d)
{
	int		exp2;
	long	exp10;
	int		digits = f->digits;
	
/*
 *	Don't try to extract any more significant digits than are there.
 */
	
	if (digits > DBL_DIG + 1)
		digits = DBL_DIG + 1;
	
/*
 *	We first deal with the special cases of zeroes, infinities and NaNs
 */
	
	d->sgn = 0;
	d->exp = 0;
	
	d->sig.length = 1;
	
	if (x == 0.0)
	{
		d->sig.text[0] = '0';
		return;
	}
	
	if (!isfinite(x))
	{
		d->sig.text[0] = isnan(x) ? 'N' : 'I';
		return;
	}
	
	d->sig.length = 0;
	
/*
 *	Now comes the easy part - we extract the sign.
 */
	
	if (x < 0.0)
	{
		d->sgn = 1;
		x = -x;
	}
	
/*
 *	Next, we use frexp() to separate x into a fraction f and an exponent e. The
 *	fraction is a value between 0.5 and 1.0 such that f * (2**e) == x. This is
 *	basically how x is stored internally. All we're interested in is the
 *	exponent to approximate the factor of 10 by which to scale x to get rid of
 *	its exponent.
 */
	
	frexp(x, &exp2);
	
/*
 *	We convert the base-2 exponent to a base-10 exponent by multiplying it by
 *	the base-10 logarithm of 2 (approximately .30102999566398). Rather than
 *	invoke the overhead of conversion to floating point and back, we instead
 *	multiply by 301029 and then divide by 1000000. The largest double exponent
 *	is 1024; 301029 * 1024 == 308253696, which can easily be represented by a
 *	long int.
 */
	
	exp10 = (exp2 * 301029) / 1000000;
	
/*
 *	Now we scale x by exp10. The easy way to do this would be simply to
 *	multiply or divide by 10, exp10 times. However, as noted above, the largest
 *	binary exponent is 1024. It's decimal equivalent is 1024 log 2 ナナ 308.
 *	That's potentially a lot of multiplies/divides. Alternatively, we can scan
 *	the decimal exponent bitwise and, for each bit 2**i, scale by a constant
 *	10**(2**i) if that bit is nonzero. This results in at most nine multiplies
 *	or divides.
 *
 *	Note that division usually consumes more cycles than multiplication.
 *	Therefore, we avoid doing multiple division operations by multiplying the
 *	scale factors together and then dividing only once.
 */
	
	{
		const double *	p = bit_values;
		int							n = exp10;
		
		if (n < 0)
		{
			for (n = -n; n; p++, n >>= 1)
				if (n & 1)
					x *= *p;
		}
		else if (n > 0)
		{
			double f = 1.0;
			
			for (; n; p++, n >>= 1)
				if (n & 1)
					f *= *p;
			
			x /= f;
		}
	}
	
/*
 *	Now we scale up or down as necessary to ensure that x is >= 0.1 and < 1.0.
 *	This puts all the digits to the right of the decimal point.
 */
	
	while (x >= 1.0)
	{
		x *= 0.1;
		++exp10;
	}
	
	while (x < 0.1)
	{
		x *= 10.0;
		--exp10;
	}
	
/*
 *	Now we pick off decimal digits from left to right up to eight at a time. We
 *	do this by shifting digits to the left of the decimal point and converting
 *	to long int to extract the shifted digits. We then decode the long int in
 *	the usual way.
 */
	
	{
		unsigned char *	p = d->sig.text;
		int							i;
		
		while (digits)
		{
			int		n = digits;
			long	long_x;
			
			if (n > 8)
				n = 8;
			
			digits -= n;
			
			exp10 -= n;
			
			d->sig.length += n;
			
			p += n;
			
			x *= digit_values[n-1];
			
			long_x = (long) x;
			
			x -= (double) long_x;
			
			for (i = n + 1; --i;)
			{
				*--p = (long_x % 10) + '0';
				long_x /= 10;
			}
			
			p += n;
		}
	
/*
 *	We fill up any remaining requested digits with zeroes, but no more
 *	than fits in a decimal record.
 */
		
		digits = f->digits;
		
		if (digits > SIGDIGLEN)
			digits = SIGDIGLEN;
		
		digits -= d->sig.length;
		
		if (digits > 0)
		{
			for (i = digits + 1; --i;)
				*p++ = '0';
			
			d->sig.length += digits;
			
			exp10 -= digits;
		}
	}
	
/*
 *	Finally, don't forget to drop off the exponent...
 */
	
	d->exp = exp10;
}

double __dec2num(const decimal *d)
{
	double	x   = 0.0;
	int			sgn = d->sgn;
	long		exp = d->exp;
	int			len = d->sig.length;
	
/*
 *	Handle special cases
 */
	
	{
		char	c = *d->sig.text;
		
		if (len < 1 || c == '0')
			return(0.0);
		
		if (c == 'I')
			return(sgn ? -Inf : Inf);
		
		if (c == 'N')
			return(NaN);
	}
	
/*
 *	Toss insignificant digits
 */
	
	if (len > DBL_DIG + 1)
	{
		exp += len - (DBL_DIG + 1);
		len = DBL_DIG + 1;
	}
	
/*
 *	We're going to accumulate digits eight at a time (enough to fit into a long int).
 *	If the number of digits isn't evenly divisible by eight, the stragglers are on
 *	the left as far as we are concerned.
 */

	{
		int					digits = len % 8;
		double				f;
		const unsigned char *	p = d->sig.text;
		
		if (!digits)
			digits = 8;
	
/*
 *	Once we're done we're going to shift the decimal point from the far right to the
 *	immediate right of the most significant digit. We make some adjustments here toward
 *	that end.
 */
		
		exp += len - 1;
		f    = pow(10.0, (len-1));
		
/*
 *	OK, let's do it...
 */
		
		while (digits)
		{
			int		i;
			long	long_x;
			
			for (i = digits + 1, long_x = 0; --i;)
				long_x = (long_x * 10) + (*p++ - '0');
			
			x = (x * 1e+08) + (double) long_x;
			
			if (!(len -= digits))
				break;
			
			digits = 8;
		}
		
		x /= f;
	}
	
/*
 *	We raise 10 to the exp power, making sure we don't overflow along the
 *	way. Then we multiply or divide x by the result, also guarding against
 *	overflow/underflow.
 */
	
	{
		const double *	p = bit_values;
		double					f = 1.0;
		long						n = labs(exp);
		
		if (n > max_bits)
			if (exp < 0)
				return(0.0);
			else
				return(sgn ? -Inf : Inf);
		
		f = 1.0;
		
		for (; n; p++, n >>= 1)
			if (n & 1)
				if (f > DBL_MAX / *p)
					return(sgn ? -Inf : Inf);
				else
					f *= *p;
		
		if (exp < 0)
		{
			if (x < DBL_MIN * f)
				return(0.0);
			else
				x /= f;
		}
		else if (exp > 0)
		{
			if (x > DBL_MAX / f)
				return(sgn ? -Inf : Inf);
			else
				x *= f;
		}
	}
	
	return(sgn ? -x : x);
}

#endif /* ndef _No_Floating_Point */

/*
	Change Record
 * hh 971206 fixed bug with __double_nan
*/
