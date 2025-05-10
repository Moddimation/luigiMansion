/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 01:09:14 $ 
 *  $Revision: 1.8.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	strtold.c
 *	
 *	Routines
 *	--------
 *		__strtold
 *		__strtod
 *
 *	Implementation
 *	--------------
 *			
 *		The string scanner is implemented as an extended Finite State Machine.
 *		A state diagram for it can be found in an accompanying TeachText
 *		document, 'strtod syntax' (too bad pictures can't be imbedded in
 *		comments) in the "MSL Technical Notes" directory. A textual description
 *      of it follows.
 *			
 *		The state transition loop dispatches to the appropriate code for the
 *		current state, while simultaneously watching for terminating
 *		conditions (field width exhausted, EOF encountered, final state
 *		reached).
 *			
 *		start
 *			
 *			Skip leading spaces. Once a non-space is seen, process sign (if any)
 *			and trasition to next state.
 *			
 *		sig_start
 *			
 *			Look for either a digit or a decimal point. If it is a digit zero,
 *			treat it specially.
 *			
 *		leading_sig_zeroes
 *			
 *			Leading zero digits are discarded, as they add nothing to the result.
 *			
 *		int_digit_loop
 *			
 *			Process digits from the integer part of the significand. We accept
 *			only so many significant digits (DBL_DIG), but the ones we discard
 *			have to be accounted for in the exponent.
 *			
 *			If a decimal point is seen, proceed to process a fractional part (if
 *			one is present).
 *			
 *		frac_start
 *			
 *			Having seen a leading decimal point, we must see at least one digit.
 *			If the field width expires before the transition from this state to
 *			the next, we fail.
 *			
 *		frac_digit_loop
 *			
 *			Process digits from the fractional part of the significand. We accept
 *			only so many significant digits (DBL_DIG), but the ones we discard
 *			have to be accounted for in the exponent.
 *		sig_end
 *			
 *			If an 'E' (or 'e') follows we go after an exponent; otherwise we're
 *			done.
 *			
 *		exp_start
 *			
 *			Process the sign (if any).
 *			
 *		leading_exp_digit
 *			
 *			Check the leading exponent digit. If it is a digit zero, treat it
 *			specially.
 *			
 *		leading_exp_zeroes
 *			
 *			Leading zero digits are discarded, as they add nothing to the result.
 *			
 *		exp_digit_loop
 *			
 *			Process digits from the exponent. We watch for short int overflow,
 *			even though the maximum exponent is probably considerably less than
 *			this. The latter will be checked during the actual decimal to binary
 *			conversion.
 *			
 *		finished
 *			
 *			Successful exit.
 *			
 *		failure
 *			
 *			Invalid input exit.
 *
 *		The end product is just the parsed input and its conversion to a
 *		'decimal' record la SANE and MathLib. '__dec2num' is used for conversion
 *		to binary. For other systems that don't provide decimal to binary
 *		conversion in this or a similar way, a routine will be provided.
 *
 *
 */

#ifndef _No_Floating_Point

#include "ansi_fp.h"
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include "lconv.h"
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_io.h"
#include "strtold.h"

enum scan_states {
	start				= 0x0001,
	sig_start			= 0x0002,
	leading_sig_zeroes	= 0x0004,
	int_digit_loop		= 0x0008,
	frac_start			= 0x0010,
	frac_digit_loop		= 0x0020,
	sig_end				= 0x0040,
	exp_start			= 0x0080,
	leading_exp_digit	= 0x0100,
	leading_exp_zeroes	= 0x0200,
	exp_digit_loop		= 0x0400,
	finished			= 0x0800,
	failure				= 0x1000
};

#define MAX_SIG_DIG 20               /* mm 970609  */

#define final_state(scan_state)	(scan_state & (finished | failure))

#define success(scan_state) (scan_state & (	leading_sig_zeroes	|			\
										int_digit_loop			|			\
										frac_digit_loop			|			\
										leading_exp_zeroes		|			\
										exp_digit_loop			|			\
										finished		))

#define fetch()		(count++, (*ReadProc)(ReadProcArg, 0, __GetAChar)) /* mm 990325 */
#define unfetch(c)	(*ReadProc)(ReadProcArg, c, __UngetAChar)			/* mm 990325 */

#if __INTEL__
#pragma k63d_calls off
#endif
long double __strtold(	int			max_width,
						int (*ReadProc)(void *, int, int), /* mm 990325 */
						void * ReadProcArg,					/* mm 990325 */
						int		* chars_scanned,
						int		* overflow)
{
	int						dot				= * (unsigned char *) __lconv.decimal_point;
	int						scan_state		= start;
	int						count			= 0;
	int                     spaces          = 0;     /* mm 970708 */
	int						c;
	decimal					d				= {0, 0, 0, {0, ""}};
	int						sig_negative	= 0;
	int						exp_negative	= 0;
	long					exp_value		= 0;
	int						exp_adjust		= 0;
	long double		result;
	
	*overflow = 0;
	
	c = fetch();
	
	while (count <= max_width && c != EOF && !final_state(scan_state)) 
	{

		switch (scan_state)
		{
			case start:
			
				if (isspace(c))
				{
					c = fetch();
					count--;    /* 01-Jan-97 mani@be */
					spaces++;   /* 01-Jan-97 mani@be */
					
					break;
				}
				
				if (c == '+')
					c = fetch();
				else if (c == '-')
				{
					c = fetch();
					
					sig_negative = 1;
				}
				
				scan_state = sig_start;
				
				break;
			
			case sig_start:
				
				if (c == dot)
				{
					scan_state = frac_start;
					
					c = fetch();
					
					break;
				}
				
				if (!isdigit(c))
				{
					scan_state = failure;
					
					break;
				}
				
				if (c == '0')
				{
					scan_state = leading_sig_zeroes;
					
					c = fetch();
					
					break;
				}
				
				scan_state = int_digit_loop;
				
				break;
			
			case leading_sig_zeroes:
			
				if (c == '0')
				{
					c = fetch();
					
					break;
				}
				
				scan_state = int_digit_loop;
				
				break;
			
			case int_digit_loop:
				
				if (!isdigit(c))
				{
					if (c == dot)
					{
						scan_state = frac_digit_loop;
						
						c = fetch();
					}
					else
						scan_state = sig_end;
					
					break;
				}
				
				if (d.sig.length < MAX_SIG_DIG)          /*mm 970609 */
					d.sig.text[d.sig.length++] = c;
				else
					exp_adjust++;
				
				c = fetch();
				
				break;
			
			case frac_start:
				
				if (!isdigit(c))
				{
					scan_state = failure;
					
					break;
				}
				
				scan_state = frac_digit_loop;
				
				break;
			
			case frac_digit_loop:
				
				if (!isdigit(c))
				{
					scan_state = sig_end;
					
					break;
				}
				
				if (d.sig.length < MAX_SIG_DIG)                /*mm 970609 */
				{
					if ( c != '0' || d.sig.length)				/* __dec2num doesn't like leading zeroes*/
						d.sig.text[d.sig.length++] = c;
					exp_adjust--;
				}
				
				c = fetch();
				
				break;
			
			case sig_end:
				
				if (c == 'E' || c == 'e')
				{
					scan_state = exp_start;
					
					c = fetch();
					
					break;
				}
				
				scan_state = finished;
				
				break;
			
			case exp_start:
				
				if (c == '+')
					c = fetch();
				else if (c == '-')
				{
					c = fetch();
					
					exp_negative = 1;
				}
				
				scan_state = leading_exp_digit;
				
				break;
			
			case leading_exp_digit:
				
				if (!isdigit(c))
				{
					scan_state = failure;
					
					break;
				}
				
				if (c == '0')
				{
					scan_state = leading_exp_zeroes;
					
					c = fetch();
					
					break;
				}
				
				scan_state = exp_digit_loop;
				
				break;
			
			case leading_exp_zeroes:
			
				if (c == '0')
				{
					c = fetch();
					
					break;
				}
				
				scan_state = exp_digit_loop;
				
				break;
			
			case exp_digit_loop:
				
				if (!isdigit(c))
				{
					scan_state = finished;
					
					break;
				}
				
				exp_value = exp_value*10 + (c - '0');
				
				if (exp_value > SHRT_MAX)
					*overflow = 1;
				
				c = fetch();
				
				break;
		}
	}
	
	if (!success(scan_state))
	{
		count = 0;   /* mf 092497 */
		*chars_scanned=0;
	}
	else
	{
		count--;
		*chars_scanned = count + spaces; /* 01-Jan-97 mani@be */
	}
	
	unfetch(c);
	
	if (exp_negative)
		exp_value = -exp_value;
	
	{
		int							n = d.sig.length;
		unsigned char * p = &d.sig.text[n];
		
		while (n-- && *--p == '0')
			exp_adjust++;
		
		d.sig.length = n + 1;
		
		if (d.sig.length == 0)
			d.sig.text[d.sig.length++] = '0';
	}
	
	exp_value += exp_adjust;
	
	if (exp_value < SHRT_MIN || exp_value > SHRT_MAX)
		*overflow = 1;
	
	if (*overflow)
		if (exp_negative)
			return(0.0);
		else
			return(sig_negative ? -HUGE_VAL : HUGE_VAL);
	
	d.exp = exp_value;
	
	result = __dec2num(&d);

/*
 *	Note: If you look at <ansi_fp.h> you'll see that __dec2num only supports double.
 *				If you look at <float.h> you'll see that long double == double. Ergo, the
 *				difference is moot *until* a truly long double type is supported.
 */
	
	if (result != 0.0 && result < LDBL_MIN)
	{
		*overflow = 1;
		result    = 0.0;
	}
	else if (result > LDBL_MAX)
	{
		*overflow = 1;
		result    = HUGE_VAL;
	} 
	
	if (sig_negative)
		result = -result;
	
	return(result);
}

double strtod(const char * str, char ** end)
{
	long double	value, abs_value;
	int					count, overflow;
	
	__InStrCtrl isc;
	isc.NextChar         = (char *)str;
	isc.NullCharDetected = 0;
		
	value = __strtold(INT_MAX, &__StringRead, (void *)&isc, &count, &overflow);
	
	if (end)
		*end = (char *) str + count;
	
	abs_value = fabs(value);
	
	if (overflow || (value != 0.0 && (abs_value < DBL_MIN || abs_value > DBL_MAX)))
		errno = ERANGE;
	
	return(value);
}

double atof(const char * str)
{
	return(strtod(str, NULL));
}
#if __INTEL__
#pragma k63d_calls reset
#endif

#endif /* ndef _No_Floating_Point */

/*  Change Record
 *	22-Jun-95 JFH  First code release.
 *	27-Jul-95 JFH  Removed stray SysBreak(). Added code to make use of the remembered sign of
 *								 of the significand.
 *	29-Sep-95 JFH  Discovered __dec2num doesn't like leading zeroes except for zeroes, so numbers
 *								 like .01 would get interpreted as zero. Fixed by suppressing leading zeroes.
 *	14-Nov-95 JFH  Fixed bug in strtod where value was checked against DBL_MIN and DBL_MAX instead
 *								 of the absolute value.
 *	25-Apr-96 JFH  Changed __strtold to return -HUGE_VAL instead of HUGE_VAL on overflow if a
 *								 minus sign was previously detected.
 *	15-Jul-97 SCM  Disabled when _No_Floating_Point is defined.
 *	01-Jan-97 mani@be	Fix a scanf bug dealing with white space. Things like
 *						scanf("%5lx") weren't working properly when there was
 *						white space.
 *  mm 970609      Changed the max number of significant digits to MAX_SIG_DIG(==32) instead of DBL_DIG
 *  mm 970708      Inserted Be changes
 *  mf 970924      If there are no digits in the string then the value of &endp must remain unchanged
 *                 In this case the variable chars_scanned of strtold should be 0
 *  mm 990325	   Made to work with input functions passed by pointers 
 *  mf 990420      turned off k6 calling convention
 *  mf 990420      had to provide k6 wrap internal function __strtold as well
 */
