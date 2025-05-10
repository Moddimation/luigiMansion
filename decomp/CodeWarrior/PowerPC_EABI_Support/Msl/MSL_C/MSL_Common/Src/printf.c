/*  Metrowerks Standard Library  */

/*  $Date: 2000/03/03 21:19:45 $ 
 *  $Revision: 1.12.4.2.2.3 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */
 
/*
 *	printf.c
 *	
 *	Routines
 *	--------
 *		vfprintf
 *		fprintf
 *
 *		vprintf
 *		printf
 *
 *		vsprintf
 *      vsnprintf
 *		sprintf
 *		snprintf
 *
 *
 */

#include <ansi_fp.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>					/* mm 990325 */
#include "string_io.h"


#define conversion_buff_size	512
#define conversion_max			509
#define bad_conversion			0xFF

enum justification_options {
	left_justification,
	right_justification,
	zero_fill
};

enum sign_options {
	only_minus,
	sign_always,
	space_holder
};

enum argument_options {
	normal_argument,
	char_argument,
	short_argument,
	long_argument,
#ifdef __MSL_LONGLONG_SUPPORT__         /* mm 961219 */
	long_long_argument,                 /* mm 961219 */
#endif                                  /* mm 961219 */
	long_double_argument
#if __ALTIVEC__ && !defined(_No_Altivec)
	,vector_argument
#endif
#ifndef __NO_WIDE_CHAR					/* mm 990325 */
	,wchar_argument						/* mm 990325 */
#endif
};

#if __ALTIVEC__ && !defined(_No_Altivec)
/* depends on the implementation of va_arg */
#if !__VEC__
	#define vec_va_arg(ap)  (*       (((MWVector128 *) (ap =  (char *) (( ((unsigned long)ap+15) & ~(15) ) + 16)       ))  -1))
	#define va_arg_replacement1(ap, vec_val) (vec_val = vec_va_arg(ap))
#endif

union MWVector128 {						/* used to get at the vec field values 				*/
	#if __VEC__
	__vector unsigned char			vuc;
	__vector signed char			vsc;
	__vector bool char				vbc;
	__vector unsigned short			vus;
	__vector signed short			vss;
	__vector unsigned long			vul;
	__vector signed long			vsl;
	__vector float					vf;
	__vector pixel					vp;
	#endif
	unsigned char 		 			u8[16];
	signed   char 		 			s8[16];
	unsigned short		 			u16[8];
	signed   short		 			s16[8];
	unsigned long 		 			u32[4];
	signed   long 		 			s32[4];
	float		   		 			f32[4];
};
typedef union MWVector128 MWVector128;
#endif


#define MAX_SIG_DIG 32                  /* mm 970609 */

typedef struct {
	unsigned char	justification_options;
	unsigned char 	sign_options;
	unsigned char 	precision_specified;
	unsigned char	alternate_form;
	unsigned char 	argument_options;
	unsigned char	conversion_char;
	int			  	field_width;
	int				precision;
#if __ALTIVEC__ && !defined(_No_Altivec)
	int		vecCount;			/* number of vector fields to convert (4, 8, 16) 		*/
	char 	*vecSeperator;		/* separator string to insert between vector fields 	*/
	char 	*c_vecSeperator;	/* separator string to use for %c vector conversion		*/
	int		vecSepLen;			/* length of *vecSeperator or *c_vecSeperator string	*/
	int		useSepString;		/* @ -- asking for a separator string instead of char	*/
	int		fbad;				/* used to control standard flags parsing				*/
	int		vecwidth,vecprec;	/* width and prec value before each conversion 			*/
	int 	useVecDefault;		/* default to "appropriate" vec access if none specified*/
	int		flagAllowed;		/* true means a flag is still possible					*/
	enum {						/* state of vector formatting parse... 					*/ 
		FLAG_OR_Vcode_OK,		/*   initial state: sep flag or 'v' code allowed  		*/
		Vcode_REQUIRED,			/*   set to this state when sep flag seen  reqiring 'v'	*/
		FLAG_OR_Vcode_NOT_OK	/*   set this state when 'v' code qualifier seen 		*/
	} vec_state;
	enum {						/* how to access vector fields (as chars,shorts,longs)	*/
		U8, U16, U32
	} vec_access;
#endif
} print_format;

static const char * parse_format(const char * format_string, va_list * arg, print_format * format)
{
	print_format	f;
	const char *	s = format_string;
	int				c;
	int				flag_found;
	
	f.justification_options	= right_justification;
	f.sign_options			= only_minus;
	f.precision_specified   = 0;
	f.alternate_form		= 0;
	f.argument_options		= normal_argument;
	f.field_width			= 0;
	f.precision				= 0;
	
	if ((c = *++s) == '%')
	{
		f.conversion_char = c;
		*format = f;
		return((const char *) s + 1);
	}
	
#if __ALTIVEC__ && !defined(_No_Altivec)
	f.vec_state 	 = FLAG_OR_Vcode_OK;
	f.vecSeperator   = " ";
	f.c_vecSeperator = "";
	f.vecSepLen	   	 = 1;
getFlags:
	f.useSepString = 0;
#endif
	
	for (;;)
	{
		flag_found = 1;
		
		switch (c)
		{
			case '-':
				
				f.justification_options = left_justification;
				
				break;
			
			case '+':
				
				f.sign_options = sign_always;
				
				break;
			
			case ' ':
				
				if (f.sign_options != sign_always)
					f.sign_options = space_holder;
					
				break;
			
			case '#':
				
				f.alternate_form = 1;
				
				break;
			
			case '0':
				
				if (f.justification_options != left_justification)
					f.justification_options = zero_fill;
				
				break;
#if __ALTIVEC__ && !defined(_No_Altivec)
			case '@':
			
				if (f.vec_state == FLAG_OR_Vcode_OK) {
					f.useSepString = 1;
					break;
				}
				/* fall through */
#endif
			default:
				
				flag_found = 0;
		}
		
		if (flag_found)
		{
#if __ALTIVEC__ && !defined(_No_Altivec)
			f.c_vecSeperator = (char *)s;
			f.vecSepLen	   	 = 1;
#endif
			c = *++s;
		}
		else
			break;
	}

#if __ALTIVEC__ && !defined(_No_Altivec)
	if (f.useSepString) {
		f.vecSeperator = va_arg(*arg, char *);
		f.vecSepLen    = strlen(f.vecSeperator);
	}
#endif
	
	if (c == '*')
	{
		if ((f.field_width = va_arg(*arg, int)) < 0)
		{
			f.justification_options = left_justification;
			f.field_width           = -f.field_width;
		}
		
		c = *++s;
	}
	else
		while (isdigit(c))
		{
			f.field_width = (f.field_width * 10) + (c - '0');
			c = *++s;
		}
	
	if (f.field_width > conversion_max)
	{
		f.conversion_char = bad_conversion;
		*format = f;
		return((const char *) s + 1);
	}
	
	if (c == '.')
	{
		f.precision_specified = 1;
		
		if ((c = *++s) == '*')
		{
			if ((f.precision = va_arg(*arg, int)) < 0)
				f.precision_specified = 0;
			
			c = *++s;
		}
		else
			while (isdigit(c))
			{
				f.precision = (f.precision * 10) + (c - '0');
				c = *++s;
			}
	}
	
	flag_found = 1;

#if __ALTIVEC__ && !defined(_No_Altivec)
	f.useVecDefault = 1;			/* use vec default access if none specified below 	*/
	
	if (c == 'v') {					/* allow 'v' to qualify length or format code 		*/
		f.vec_state = FLAG_OR_Vcode_NOT_OK;
		f.argument_options = vector_argument;
		c = *++s;
	}
#endif		
	
	switch (c)
	{
		case 'h':
			
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (f.argument_options == normal_argument)	
				f.argument_options = short_argument;
			f.vecCount  	= 8;
			f.vec_access	= U16;
			f.useVecDefault = 0;
#else
			f.argument_options = short_argument;
#endif

			if (s [1] == 'h')
			{
				f.argument_options = char_argument;
				c = *++s; /* move past second h */
			}
#if __ALTIVEC__ && !defined(_No_Altivec)
			else if (s [1] == 'v')
			{
				if (f.vec_state != FLAG_OR_Vcode_NOT_OK) {
					f.vec_state = FLAG_OR_Vcode_NOT_OK;
					f.argument_options = vector_argument;
					c = *++s;
				}
			}
#endif
			
			break;
		
		case 'l':
			
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (f.argument_options == normal_argument)	
				f.argument_options = long_argument;
			f.vecCount  	  = 4;
			f.vec_access	  = U32;
			f.useVecDefault = 0;
			if (s [1] == 'v')
			{
				if (f.vec_state != FLAG_OR_Vcode_NOT_OK) {
					f.vec_state = FLAG_OR_Vcode_NOT_OK;
					f.argument_options = vector_argument;
					c = *++s;
				}
			}
#else
			f.argument_options = long_argument;
#endif

#ifdef __MSL_LONGLONG_SUPPORT__
			if (s [1] == 'l')
			{
				f.argument_options = long_long_argument;
				c = *++s; /* move past second l */
#if __ALTIVEC__ && !defined(_No_Altivec)
				f.vec_state = FLAG_OR_Vcode_NOT_OK;
#endif
			}
#endif
			
			break;
		
		case 'L':
			
			f.argument_options = long_double_argument;
#if __ALTIVEC__ && !defined(_No_Altivec)
			f.vec_state = FLAG_OR_Vcode_NOT_OK;
#endif
			
			break;
		
		default:
			
			flag_found = 0;
	}
	
	if (flag_found)
		c = *++s;
		
#if __ALTIVEC__ && !defined(_No_Altivec)
	if (f.vec_state == Vcode_REQUIRED && f.argument_options != vector_argument) {
		f.conversion_char = bad_conversion;
		*format = f;
		return s;
	}
#endif
	
	f.conversion_char = c;
	
	switch (c)
	{
		case 'd':
		case 'i':
		case 'u':
		case 'o':
		case 'x':
		case 'X':
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (f.argument_options != vector_argument)
#endif
			if (f.argument_options == long_double_argument)
			{
				f.conversion_char = bad_conversion;      /*mm 961219*/
				break;                                   /*mm 961219*/
			}
		
			if (!f.precision_specified)
				f.precision = 1;
			else if (f.justification_options == zero_fill)
				f.justification_options = right_justification;
			
			break;
		
		case 'f':
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (f.argument_options != vector_argument)
#endif
		    /* if (f.argument_options == short_argument || f.argument_options == long_argument) */
			if (f.argument_options == short_argument
#ifdef __MSL_LONGLONG_SUPPORT__
				|| f.argument_options == long_long_argument
#endif
			)  /*  defacto standard %lf is a double */
			{
				f.conversion_char = bad_conversion;
				break;
			}
			
			if (!f.precision_specified)
				f.precision = 6;
			
			break;
			
	    case 'g':
		case 'G':
		
			if (!f.precision)
				f.precision = 1;
			
		case 'e':
		case 'E':
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (f.argument_options != vector_argument)
#endif
			if (f.argument_options == short_argument 
#ifdef __MSL_LONGLONG_SUPPORT__
				|| f.argument_options == long_long_argument
#endif
				|| f.argument_options == char_argument)        /* mm 990208 */
			{
				f.conversion_char = bad_conversion;
				break;
			}
			
			if (!f.precision_specified)
				f.precision = 6;
			
			break;
		
		case 'p':
#if 0
			f.argument_options = long_argument;
			f.alternate_form   = 1;
			f.conversion_char  = 'x';
			f.precision        = 8;
#else
			f.conversion_char  = 'x';
			f.alternate_form   = 1;
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (f.argument_options != vector_argument) {
				f.argument_options = long_argument;
				f.precision        = 8;
			} else
				f.precision        = 1;
#else
			f.argument_options = long_argument;
			f.precision        = 8;
#endif		
#endif
			break;
			
		case 'c':
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (f.argument_options != vector_argument)
#endif
#ifndef __NO_WIDE_CHAR				
			if (f.argument_options == long_argument)			/* mm 990325 */
				f.argument_options = wchar_argument;			/* mm 990325 */
			else												/* mm 990325 */
#endif															/* mm 990325 */
			if (f.precision_specified || f.argument_options != normal_argument)
				f.conversion_char = bad_conversion;
			
			break;
			
		case 's':
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (f.argument_options == vector_argument)
				f.argument_options = normal_argument;
			else
#endif
#ifndef __NO_WIDE_CHAR				
			if (f.argument_options == long_argument)			/* mm 990325 */
				f.argument_options = wchar_argument;			/* mm 990325 */
			else												/* mm 990325 */
#endif															/* mm 990325 */
			if (f.argument_options != normal_argument)			/* mm 990325 */

				f.conversion_char = bad_conversion;
			
			break;
			
		case 'n':
			
			if (f.argument_options == long_double_argument)
				f.conversion_char = bad_conversion;         /*mm 961219*/
			
			break;
			
		default:
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (f.vec_state == FLAG_OR_Vcode_OK) {
				f.vec_state = Vcode_REQUIRED;
				f.vecSeperator = f.c_vecSeperator = (char *)s;
				f.vecSepLen = 1;
				c = *++s;
				goto getFlags;
			}
#endif
			
			f.conversion_char = bad_conversion;
			
			break;
	}

#if __ALTIVEC__ && !defined(_No_Altivec)
	f.vecprec  = f.precision;		/* save original prec/width for vec conversions  	*/
	f.vecwidth = f.field_width;
#endif
	
	*format = f;
	
	return((const char *) s + 1);
}

static char * long2str(long num, char * buff, print_format *format)
{
	unsigned long unsigned_num, base;
	char *				p;
	int						n, digits;
	int						minus = 0;
	
	unsigned_num = num;
	minus        =   0;
	
	p = buff;
	
	*--p = 0;
	
	digits = 0;

	if (!num 
		&& !format->precision 
		&& !(format->alternate_form 
		&& format->conversion_char == 'o'))
		return(p);

	switch (format->conversion_char)
	{
		case 'd':
		case 'i':
			
			base =  10;
	
			if (num < 0)
			{
				unsigned_num = -unsigned_num;
				minus        = 1;
			}
			
			break;
			
		case 'o':
			
			base =   8;
			
			format->sign_options = only_minus;
						
			break;
			
		case 'u':
			
			base =  10;
			
			format->sign_options = only_minus;
			
			break;
			
		case 'x':
		case 'X':
			
			base =  16;
			
			format->sign_options = only_minus;
			
			break;
	}
	
	do
	{
		n = unsigned_num % base;
		
		unsigned_num /= base;
		
		if (n < 10)
			n += '0';
		else
		{
			n -= 10;
			
			if (format->conversion_char == 'x')
				n += 'a';
			else
				n += 'A';
		}
		
		*--p = n;
		
		++digits;
	}
	while (unsigned_num != 0);
	
	if (base == 8 && format->alternate_form && *p != '0')
	{
		*--p = '0';
		++digits;
	}
	
	if (format->justification_options == zero_fill)
	{
		format->precision = format->field_width;
		
		if (minus || format->sign_options != only_minus)
			--format->precision;
		
		if (base == 16 && format->alternate_form)
			format->precision -= 2;
	}
	
	if (buff - p + format->precision > conversion_max)
		return(NULL);
	
	while (digits < format->precision)
	{
		*--p = '0';
		++digits;
	}
	
	if (base == 16 && format->alternate_form)
	{
		*--p = format->conversion_char;
		*--p = '0';
	}
	
	if (minus)
		*--p = '-';
	else if (format->sign_options == sign_always)
		*--p = '+';
	else if (format->sign_options == space_holder)
		*--p = ' ';
	
	return(p);
}

#ifdef __MSL_LONGLONG_SUPPORT__                          /*mm 961219*/
static char * longlong2str(long long num, char * buff, print_format *format)
{
	unsigned long long      unsigned_num, base;
	char *				    p;
	int						n, digits;
	int						minus = 0;
	
	unsigned_num = num;
	minus        =   0;
	
	p = buff;
	
	*--p = 0;
	
	digits = 0;
	
	if (!num && !format->precision && !(format->alternate_form && format->conversion_char == 'o'))
		return(p);
	
	switch (format->conversion_char)
	{
		case 'd':
		case 'i':
			
			base =  10;
	
			if (num < 0)
			{
				unsigned_num = -unsigned_num;
				minus        = 1;
			}
			
			break;
			
		case 'o':
			
			base =   8;
			
			format->sign_options = only_minus;
						
			break;
			
		case 'u':
			
			base =  10;
			
			format->sign_options = only_minus;
			
			break;
			
		case 'x':
		case 'X':
			
			base =  16;
			
			format->sign_options = only_minus;
			
			break;
	}
	
	do
	{
		n = unsigned_num % base;
		
		unsigned_num /= base;
		
		if (n < 10)
			n += '0';
		else
		{
			n -= 10;
			
			if (format->conversion_char == 'x')
				n += 'a';
			else
				n += 'A';
		}
		
		*--p = n;
		
		++digits;
	}
	while (unsigned_num != 0);
	
	if (base == 8 && format->alternate_form && *p != '0')
	{
		*--p = '0';
		++digits;
	}
	
	if (format->justification_options == zero_fill)
	{
		format->precision = format->field_width;
		
		if (minus || format->sign_options != only_minus)
			--format->precision;
		
		if (base == 16 && format->alternate_form)
			format->precision -= 2;
	}
	
	if (buff - p + format->precision > conversion_max)
		return(NULL);
	
	while (digits < format->precision)
	{
		*--p = '0';
		++digits;
	}
	
	if (base == 16 && format->alternate_form)
	{
		*--p = format->conversion_char;
		*--p = '0';
	}
	
	if (minus)
		*--p = '-';
	else if (format->sign_options == sign_always)
		*--p = '+';
	else if (format->sign_options == space_holder)
		*--p = ' ';
	
	return(p);
}
#endif    /*__MSL_LONGLONG_SUPPORT__*/

static void round_decimal(decimal * dec, int new_length)
{
	char		c;
	char *	p;
	int			carry;
	
	if (new_length < 0)
	{
return_zero:
		dec->sgn         =  0 ;
		dec->exp         =  0 ;
		dec->sig.length  =  1 ;
		*dec->sig.text   = '0';
		return;
	}
	
	if (new_length >= dec->sig.length)
		return;
	
	p     = (char *) dec->sig.text + new_length + 1;
	c     = *--p - '0';
	/*  -- added round to nearest or even mode; was: carry = (c >= 5);  970614 mm */
	if( c == 5 )                                                     /* 970614 mm */
	{ char *q = &((char *)dec->sig.text)[dec->sig.length];           /* 970614 mm */
	  while( --q > p && *q == '0' ) /* */;                           /* 970614 mm */
	  carry = ( q == p ) ? p[-1] & 1 : 1;                            /* 970614 mm */
	}                                                                /* 970614 mm */
	else                                                             /* 970614 mm */
	  carry = (c > 5);                                               /* 970614 mm */
	
	while (new_length)
	{
		c = *--p - '0' + carry;
		
		if ((carry = (c > 9)) != 0 || c == 0 ) /*  added elimination of trailing zeroes  970614 mm */
			--new_length;
		else
		{
			*p = c + '0';
			break;
		}
	}
	
	if (carry)
	{
		dec->exp        +=  1 ;
		dec->sig.length  =  1 ;
		*dec->sig.text   = '1';
		return;
	}
	else if (new_length == 0)
		goto return_zero;
	
	dec->sig.length = new_length;
}


#ifndef _No_Floating_Point           /*scm 970709 */

static char * float2str(va_list arg, char * buff, print_format *format, int vecIndex)
{
	decimal			dec;
	decform			form;
	char *			p;
	char *			q;
	int				n, digits, sign;
	int				int_digits, frac_digits;
	long double		long_double_num;
#if __ALTIVEC__ && !defined(_No_Altivec)
	static MWVector128 	vec_val;
#endif
	
#if __ALTIVEC__ && !defined(_No_Altivec)
	if (format->argument_options == vector_argument) 
	{
		if (vecIndex == 0) 
		{
			if (format->useVecDefault) 	/* establish default vec access */
			{		
				format->vecCount   = 4;
				format->vec_access = U32;
			}
#if __PPC_EABI__
			vec_val = *((MWVector128*)(&(va_arg(arg, vector unsigned long))));
#else
#if __VEC__
			vec_val = va_arg(arg, MWVector128);
#else
			va_arg_replacement1(arg, vec_val);
#endif
#endif
		}
		switch (format->vec_access) {
			case U8:  long_double_num = vec_val.u8 [vecIndex]; break;
			case U16: long_double_num = vec_val.u16[vecIndex]; break;
			case U32: long_double_num = vec_val.f32[vecIndex]; break;
		}
	}
	else
#endif
	if (format->argument_options == long_double_argument)
		long_double_num = va_arg(arg, long double);
	else
		long_double_num = va_arg(arg, double);

	if (format->precision > conversion_max)							/* might as well punt asap */
		return(NULL);

/*
 *	Note: If you look at <ansi_fp.h> you'll see that __num2dec only supports double.
 *				If you look at <float.h> you'll see that long double == double. Ergo, the
 *				difference is moot *until* a truly long double type is supported.
 */
	
	form.style  = FLOATDECIMAL;
	form.digits = MAX_SIG_DIG;           /* mm 970609 */
	
	__num2dec(&form, long_double_num, &dec);
	
	p = (char *) dec.sig.text + dec.sig.length;					/* strip off trailing zeroes */
	
	while (dec.sig.length > 1 && *--p == '0')
	{
		--dec.sig.length;
		++dec.exp;
	}
	
	switch (*dec.sig.text)
	{
		case '0':
			
			/* dec.sgn = 0;		*/ /* print correctly signed zero --mf 060298 */
			dec.exp = 0;																		/* __num2dec doesn't guarantee */
																											/* this for zeroes             */
			break;
			
		case 'I':
			
			if (long_double_num < 0)       /*mm 970213 */
			{                              /*mm 970213 */
				p = buff - 5;              /*mm 970213 */																		/* special cases */
				strcpy(p, "-Inf");         /*mm 970213 */
			}                              /*mm 970213 */
			else                           /*mm 970213 */
			{                              /*mm 970213 */
				p = buff - 4;              /*mm 970213 */																		/* special cases */
				strcpy(p, "Inf");          /*mm 970213 */
			}                              /*mm 970213 */
			
			return(p);
			
		case 'N':
			
			p = buff - 4;
			
			strcpy(p, "NaN");
			
			return(p);
	}
	
	dec.exp += dec.sig.length - 1;											/* shift decimal point to */
																											/* follow first digit	    */
	p = buff;
	
	*--p = 0;
	
	switch (format->conversion_char)
	{
		case 'g':
		case 'G':
			
			if (dec.sig.length > format->precision)
				round_decimal(&dec, format->precision);
			
			if (dec.exp < -4 || dec.exp >= format->precision)
			{
				if (format->alternate_form)
					--format->precision;
				else
					format->precision = dec.sig.length - 1;
				
				if (format->conversion_char == 'g')
					format->conversion_char = 'e';
				else
					format->conversion_char = 'E';
				
				goto e_format;
			}
			
			if (format->alternate_form)
				format->precision -= dec.exp + 1;
			else
				if ((format->precision = dec.sig.length - (dec.exp + 1)) < 0)
					format->precision = 0;
			
			goto f_format;
		
		case 'e':
		case 'E':
		e_format:
			
			if (dec.sig.length > format->precision + 1)
				round_decimal(&dec, format->precision + 1);
			
			n    = dec.exp;
			sign = '+';
			
			if (n < 0)
			{
				n    = -n;
				sign = '-';
			}
			
			for (digits = 0; n || digits < 2; ++digits)
			{
				*--p  = n % 10 + '0';
				n    /= 10;
			}
			
			*--p = sign;
			*--p = format->conversion_char;
			
			if (buff - p + format->precision > conversion_max)
				return(NULL);
			
			if (dec.sig.length < format->precision + 1)
				for (n = format->precision + 1 - dec.sig.length + 1; --n;)
					*--p = '0';
			
			for (n = dec.sig.length, q = (char *) dec.sig.text + dec.sig.length; --n;)
				*--p = *--q;
			
			if (format->precision || format->alternate_form)
				*--p = '.';
			
			*--p = *dec.sig.text;
			
			if (dec.sgn)
				*--p = '-';
			else if (format->sign_options == sign_always)
				*--p = '+';
			else if (format->sign_options == space_holder)
				*--p = ' ';
			
			break;
		
		case 'f':
		f_format:
			
			if ((frac_digits = -dec.exp + dec.sig.length - 1) < 0)
				frac_digits = 0;
			
			if (frac_digits > format->precision)
			{
				round_decimal(&dec, dec.sig.length - (frac_digits - format->precision));
				
				if ((frac_digits = -dec.exp + dec.sig.length - 1) < 0)
					frac_digits = 0;
			}
			
			if ((int_digits = dec.exp + 1) < 0)
				int_digits = 0;
			
			if (int_digits + frac_digits > conversion_max)
				return(NULL);
			
			q = (char *) dec.sig.text + dec.sig.length;
			
			for (digits = 0; digits < (format->precision - frac_digits); ++digits)
				*--p = '0';
			
			for (digits = 0; digits < frac_digits && digits < dec.sig.length; ++digits)
				*--p = *--q;
			
			for (; digits < frac_digits; ++digits)
				*--p = '0';
			
			if (format->precision || format->alternate_form)
				*--p = '.';
			
			if (int_digits)
			{
				for (digits = 0; digits < int_digits - dec.sig.length; ++digits)
					*--p = '0';
				
				for (; digits < int_digits; ++digits)
					*--p = *--q;
			}
			else
				*--p = '0';
			
			if (dec.sgn)
				*--p = '-';
			else if (format->sign_options == sign_always)
				*--p = '+';
			else if (format->sign_options == space_holder)
				*--p = ' ';
			
			break;
	}
	
	return(p);
}

#endif /* ndef _No_Floating_Point */     /*scm 970709 */

static int __pformatter(void *(*WriteProc)(void *, const char *, size_t), void * WriteProcArg, 
														const char * format_str, va_list arg)	/* mm 990325 */
{
	int				num_chars, chars_written, field_width;
	const char *	format_ptr;
	const char *	curr_format;
	print_format	format;
	long			long_num;
#ifdef __MSL_LONGLONG_SUPPORT__          /*mm 961219*/
	long long   	long_long_num;       /*mm 961219*/
#endif                                   /*mm 961219*/
	char			buff[conversion_buff_size];
	char *			buff_ptr;
	char *			string_end;
	char            fill_char = ' ';      /*mm-960722*/
	int 			vecIndex;			  /* counts vector elements*/
	#if __ALTIVEC__ && !defined(_No_Altivec)
	MWVector128 	vec_val;
#endif

	format_ptr    = format_str;
	chars_written = 0;
	
	while (*format_ptr)
	{
		if (!(curr_format = strchr(format_ptr, '%')))
		{
			num_chars      = strlen(format_ptr);
			chars_written += num_chars;
			
			if (num_chars && !(*WriteProc)(WriteProcArg, format_ptr, num_chars))	/* mm 990325 */
				return(-1);
				
			break;
		}
		
		num_chars      = curr_format - format_ptr;
		chars_written += num_chars;
		
		if (num_chars && !(*WriteProc)(WriteProcArg, format_ptr, num_chars))		/* mm 990325 */
			return(-1);
		
		format_ptr = curr_format;
		
#if __PPC_EABI__
		format_ptr = parse_format(format_ptr, (va_list *)arg, &format);
#else
		format_ptr = parse_format(format_ptr, (va_list *)&arg, &format);		
#endif		
		vecIndex = 0;
#if __ALTIVEC__ && !defined(_No_Altivec)
		do {				/* can be done repeatedly for each vector field */
#endif
		switch (format.conversion_char)
		{
			case 'd':
			case 'i':
#if __ALTIVEC__ && !defined(_No_Altivec)
				if (format.argument_options == vector_argument)
				{
					if (vecIndex == 0)
					{
						if (format.useVecDefault)	/* establish default vec access */
						{	
							format.vecCount   = 16;
							format.vec_access = U8;
						}
#if __PPC_EABI__
						vec_val = *((MWVector128*)(&(va_arg(arg, vector unsigned long))));
#else
						#if __VEC__
						vec_val = va_arg(arg, MWVector128);
						#else
						va_arg_replacement1(arg, vec_val);
						#endif
#endif
					}
					switch (format.vec_access) {
						case U8:  long_num = vec_val.s8 [vecIndex]; break;
						case U16: long_num = vec_val.s16[vecIndex]; break;
						case U32: long_num = vec_val.s32[vecIndex]; break;
					}
				}
				else
#endif
				if (format.argument_options == long_argument)
					long_num = va_arg(arg, long);
#ifdef __MSL_LONGLONG_SUPPORT__                                          /*mm 961219*/
	            else if (format.argument_options == long_long_argument)  /*mm 961219*/
					long_long_num = va_arg(arg, long long);              /*mm 961219*/
#endif                                                                   /*mm 961219*/
				else
					long_num = va_arg(arg, int);
				
				if (format.argument_options == short_argument)
					long_num = (short) long_num;
				if (format.argument_options == char_argument)
					long_num = (char) long_num;
					
#ifdef __MSL_LONGLONG_SUPPORT__                                          /*mm 961219*/
                if (format.argument_options == long_long_argument)       /*mm 961219*/
				{														 /*bb 971019 */
				   if (!(buff_ptr = longlong2str(long_long_num,          /*mm 961219*/
				                   buff + conversion_buff_size, &format)))/*mm 961219*/
					   goto conversion_error;                            /*mm 961219*/
				}														 /*bb 971019 */
				else                                                     /*mm 961219*/
#endif                                                                   /*mm 961219*/
				if (!(buff_ptr = long2str(long_num, buff + conversion_buff_size, &format)))
					goto conversion_error;
				
				num_chars = buff + conversion_buff_size - 1 - buff_ptr;
				
				break;
				
			case 'o':
			case 'u':
			case 'x':
			case 'X':
#if __ALTIVEC__ && !defined(_No_Altivec)
				if (format.argument_options == vector_argument)
				{
					if (vecIndex == 0)
					{
						if (format.useVecDefault)	/* establish default vec access */
						{	
							format.vecCount   = 16;
							format.vec_access = U8;
						}
#if __PPC_EABI__
						vec_val = *((MWVector128*)(&(va_arg(arg, vector unsigned long))));
#else
						#if __VEC__
						vec_val = va_arg(arg, MWVector128);
						#else
						va_arg_replacement1(arg, vec_val);
						#endif
#endif
					}
					switch (format.vec_access) {
						case U8:  long_num = vec_val.u8 [vecIndex]; break;
						case U16: long_num = vec_val.u16[vecIndex]; break;
						case U32: long_num = vec_val.u32[vecIndex]; break;
					}
				}
				else
#endif
				if (format.argument_options == long_argument)
					long_num = va_arg(arg, unsigned long);
#ifdef __MSL_LONGLONG_SUPPORT__                                          /*mm 961219*/
	            else if (format.argument_options == long_long_argument)  /*mm 961219*/
					long_long_num = va_arg(arg, long long);              /*mm 961219*/
#endif                                                                   /*mm 961219*/
				else
					long_num = va_arg(arg, unsigned int);
				
				if (format.argument_options == short_argument)
					long_num = (unsigned short) long_num;
				if (format.argument_options == char_argument)
					long_num = (unsigned char) long_num;
					
#ifdef __MSL_LONGLONG_SUPPORT__                                          /*mm 961219*/
                if (format.argument_options == long_long_argument)       /*mm 961219*/
                {														 /*mf 971020*/
				   if (!(buff_ptr = longlong2str(long_long_num,          /*mm 961219*/
				                   buff + conversion_buff_size, &format)))/*mm 961219*/
					   goto conversion_error;                            /*mm 961219*/
				} 														 /*mf 971020*/
				else                                                     /*mm 961219*/
#endif                                                                   /*mm 961219*/
				if (!(buff_ptr = long2str(long_num, buff + conversion_buff_size, &format)))
					goto conversion_error;
				
				num_chars = buff + conversion_buff_size - 1 - buff_ptr;
				
				break;

#ifndef _No_Floating_Point

			case 'f':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
				if (!(buff_ptr = float2str(arg, buff + conversion_buff_size, &format, vecIndex)))
					goto conversion_error;
				num_chars = buff + conversion_buff_size - 1 - buff_ptr;
				break;
#endif

			case 's':
			  #ifndef __NO_WIDE_CHAR  /*  990624  vss */
				if (format.argument_options == wchar_argument)			/* mm 990325 */
				{														/* mm 990325 */
					wchar_t * wcs_ptr = va_arg(arg, wchar_t*);			/* mm 990325 */
					if (wcs_ptr == NULL)
						wcs_ptr = L"";
					if ((num_chars = wcstombs(buff, wcs_ptr, sizeof(buff))) < 0)
						goto conversion_error;
					buff_ptr = &buff[0];								/* mm 990325 */
				}
				else
			  #endif
					buff_ptr = va_arg(arg, char *);
				if (buff_ptr == NULL)   /*97010mani@be */  /* mm 970708 */
  					buff_ptr = "";      /*97010mani@be */  /* mm 970708 */
				if (format.alternate_form)
				{
					num_chars = (unsigned char) *buff_ptr++;
					
					if (format.precision_specified && num_chars > format.precision)
						num_chars = format.precision;
				}
				else if (format.precision_specified)
				{
					num_chars = format.precision;
					
					if ((string_end = (char *) memchr(buff_ptr, 0, num_chars)) != 0)
						num_chars = string_end - buff_ptr;
				}
				else
					num_chars = strlen(buff_ptr);
				
				break;
			
			case 'n':
				
				buff_ptr = va_arg(arg, char *);
				
				switch (format.argument_options)
				{
					case normal_argument:    * (int *)       buff_ptr = chars_written; break;
					case short_argument:     * (short *)     buff_ptr = chars_written; break;
					case long_argument:      * (long *)      buff_ptr = chars_written; break;
#ifdef __MSL_LONGLONG_SUPPORT__                                          /*mm 961219*/
					case long_long_argument: * (long long *) buff_ptr = chars_written; break;
#endif                                                                   /*mm 961219*/
				}
				
				continue;
				
			case 'c':
				
				buff_ptr = buff;
#if __ALTIVEC__ && !defined(_No_Altivec)
				if (format.argument_options == vector_argument)
				{
					if (vecIndex == 0) 
					{
						if (format.useVecDefault) 	/* establish default vec access */
						{	
							format.vecCount   = 16;
							format.vec_access = U8;
						}
						if (!format.useSepString) {
							format.vecSeperator = format.c_vecSeperator;
							format.vecSepLen    = *format.c_vecSeperator ? 1 : 0;
						}
#if __PPC_EABI__
						vec_val = *((MWVector128*)(&(va_arg(arg, vector unsigned long))));
#else
						#if __VEC__
						vec_val  = va_arg(arg, MWVector128);
						#else
						va_arg_replacement1(arg, vec_val);
						#endif
#endif
					}
					switch (format.vec_access)
					{
						case U8:  *buff_ptr = vec_val.u8 [vecIndex]; break;
						case U16: *buff_ptr = vec_val.u16[vecIndex]; break;
						case U32: *buff_ptr = vec_val.u32[vecIndex]; break;
					}
					
				}
				else
#endif			
				*buff_ptr = va_arg(arg, int);
				
				num_chars = 1;
				
				break;
			
			case '%':
				
				buff_ptr = buff;
				
				*buff_ptr = '%';
				
				num_chars = 1;
				
				break;
				
			case bad_conversion:
			conversion_error:
			default:
				num_chars      = strlen(curr_format);
				chars_written += num_chars;
				
				if (num_chars && !(*WriteProc)(WriteProcArg, curr_format, num_chars))	/* mm 990325 */
					return(-1);
				
				return(chars_written);
		}
		
		field_width = num_chars;
		
		if (format.justification_options != left_justification)
		{                                                                         /*mm-960722*/
			fill_char = (format.justification_options == zero_fill) ? '0' : ' ';  /*mm-960722*/
			if (((*buff_ptr == '+') || (*buff_ptr == '-')) && (fill_char == '0')) /*mm-970206*/
			{	                                                                  /*mm-970206*/
					if ((*WriteProc)(WriteProcArg, buff_ptr, 1) == 0)			  /* mm 990325 */
					return(-1);                                                   /*mm-970206*/
				++buff_ptr;                                                       /*mm-970206*/
				num_chars--;                                                      /*mm-970723*/
			}
			while (field_width < format.field_width)
			{
				if ((*WriteProc)(WriteProcArg, &fill_char, 1) == 0)  				/* mm 990325 */
					return(-1);
					
				++field_width;
			}
		}                                                                       /*mm-960722*/
		if (num_chars && !(*WriteProc)(WriteProcArg, buff_ptr, num_chars))		/* mm 990325 */
			return(-1);
		
		if (format.justification_options == left_justification)
			while (field_width < format.field_width)
			{
				char blank = ' ';
				if ((*WriteProc)(WriteProcArg, &blank, 1) == 0)  				/* mm 990325 */
					return(-1);
					
				++field_width;
			}
		
		chars_written += field_width;

#if __ALTIVEC__ && !defined(_No_Altivec)
		/* Process vec types specially... */
		
		if (format.argument_options == vector_argument && ++vecIndex < format.vecCount) {
			if (format.vecSeperator && format.vecSepLen > 0) {
				if (format.vecSepLen == 1) 
				{
					if ((*WriteProc)(WriteProcArg, format.vecSeperator, 1) == 0)  /* mm 990325 */
						return (-1);
				} else
					if (!(*WriteProc)(WriteProcArg, format.vecSeperator, format.vecSepLen)) /* mm 990325 */
						return (-1);
				chars_written += format.vecSepLen;
			}
			format.field_width = format.vecwidth;
			format.precision   = format.vecprec;
			fill_char = ' ';
		}
		} while (format.argument_options == vector_argument && vecIndex < format.vecCount);
#endif
	}
	
	return(chars_written);
}


/* The remainder of the file has been extensively modified  mm 990325 */
void * __FileWrite(void *File, const char * Buffer, size_t NumChars)
{
	return (fwrite(Buffer, 1, NumChars, (FILE *)File) == NumChars ? File : 0);
}

void * __StringWrite(void * osc, const char * Buffer, size_t NumChars)
{
	size_t CharsToBeWritten;
	void * MemCpyResult;
	__OutStrCtrl * Oscp = (__OutStrCtrl*)osc;
	
	CharsToBeWritten = ((Oscp->CharsWritten + NumChars) <= Oscp->MaxCharCount) ? NumChars : 
															Oscp->MaxCharCount - Oscp->CharsWritten;
	MemCpyResult = (void *)memcpy(Oscp->CharStr + Oscp->CharsWritten, Buffer, CharsToBeWritten);
	Oscp->CharsWritten += CharsToBeWritten;
	return(MemCpyResult);
}

#if __dest_os != __n64_os  /*  ard  990128  */
int printf(const char * format, ...)										
{
#ifndef __NO_WIDE_CHAR										
	if (fwide(stdout, -1) >= 0)
		return(-1);
#endif /* __NO_WIDE_CHAR */                                 

#if __PPC_EABI__ || __MIPS__ 
	{
	va_list args;                            
	
	va_start( args, format );                
	return(__pformatter(&__FileWrite, (void *)stdout, format, args));
	}    
#else                                        
	return(__pformatter(&__FileWrite, (void *)stdout, format, __va_start(format)));
#endif                                       
}
#endif

int fprintf(FILE * file, const char * format, ...)
{
#ifndef __NO_WIDE_CHAR							
	if (fwide(file, -1) >= 0)
		return(-1);
#endif /* __NO_WIDE_CHAR */                       

#if __PPC_EABI__ || __MIPS__ 
	{
	va_list args;                            
	
	va_start( args, format );                
	return(__pformatter(&__FileWrite, (void *)file, format, args)); 
	}   
#else                                        
	return(__pformatter(&__FileWrite, (void *)file, format, __va_start(format)));
#endif                                       
}

int vprintf(const char * format, va_list arg)
{
#ifndef __NO_WIDE_CHAR										
	if (fwide(stdout, -1) >= 0)
		return(-1);
#endif /* __NO_WIDE_CHAR */                                 
	return(__pformatter(&__FileWrite, (void *)stdout, format, arg));
}

int vfprintf(FILE * file, const char * format, va_list arg)	
{
#ifndef __NO_WIDE_CHAR										
	if (fwide(file, -1) >= 0)
		return(-1);
#endif /* __NO_WIDE_CHAR */                                 

	return(__pformatter(&__FileWrite, (void *)file, format, arg));
}


int vsnprintf(char * s, size_t n, const char * format, va_list arg)	
{
	int		end;
	__OutStrCtrl osc;
	
	osc.CharStr      = s;
	osc.MaxCharCount = n;
	osc.CharsWritten = 0;
	
	end = __pformatter(&__StringWrite, &osc, format, arg);   					              
	
	s[(end < n) ? end : n-1] = '\0';		
	
	return(end);
}

int vsprintf(char * s, const char * format, va_list arg)		
{
	return(vsnprintf(s, ULONG_MAX, format, arg));			
}

int snprintf(char * s, size_t n, const char * format, ...)	
{
#if __PPC_EABI__ || __MIPS__ 
	va_list args;                        
	
	va_start( args, format );            
	return(vsnprintf(s, n, format, args));					
#else                                    
	return(vsnprintf(s, n, format, __va_start(format)));	
#endif                                   
}

#if __dest_os != __n64_os   /* ard 990126  */
int sprintf(char * s, const char * format, ...)                   
{
#if __PPC_EABI__ || __MIPS__ 
	va_list args;                        
	
	va_start( args, format );            
	return(vsnprintf(s, ULONG_MAX, format, args));			
#else                                    
	return(vsnprintf(s, ULONG_MAX, format, __va_start(format)));	
#endif                                   
}
#endif



/*  Change Record
 *	18-Sep-95 JFH  First code release.
 *	29-Sep-95 JFH  Fixed bug in 'f' format where a zero fractional part would be suppressed
 *								 regardless of the precision.
 *	10-Oct-95 JFH  Changed long2str to comply with Standard saying: "The result of converting
 *								 a zero value with a precision of zero is no characters."
 *	30-Oct-95 JFH  Corrected handling of 'g','G' conversions using the alternate form (#).
 *  22-Jan-96 JFH  Added casts from (void *) for C++ compatibility.
 *	19-Feb-96 JFH  Added #ifndef around area that handles floating point output. If disabled, you
 *								 can get away with not including MathLib just because you wanted to use printf.
 *								 There is also some ANSI lib code savings as well.
 *	 7-Mar-96 JFH  Fixed bug in round_decimal where it *wasn't* rounding anything if all the
 *								 precision digits were zero.
 *	19-Mar-96 JFH  Changed handling of '+' flag so it doesn't affect how unsigned values (i.e.,
 *								 o,u,x,X) are displayed.
 *	24-Mar-96 JFH  Changed "round_decimal" to do rounding using more conventional rules of
 *								 arithmetic. :-)
 *  96Jul22   MM   Fill with zeros when required.
 *  96aug29   bkoz add l.you's changes for powertv, defines for no floating point
 *  960930    mm   Removed bitfields from printformat structure to compile with ANSI strict
 *  961220    mm   Support for long long.
 *	970101	mani@be	Don't choke if a NULL string is passed to %s. Just print "(null)".
 *  970206    mm   Corrected position of sign and left zero fill
 *  970213    mm   Made to print -Inf for negative infinite values
 *  970609    mm   Set the max number of significant digits tolarger than DBL_DIG
 *  970614    mm   Modifications to remove trailing blanks for g format and to round to nearest or even
 *  970702    mm   Modified Mani's fix to conform to Standard
 * mm 970708  Inserted Be changes
 *	970709	  SCM  Converted __va_start() to va_start() for PPC_EABI
 * mm-970723       Further correction to change of 970206  BW01232
 * 	971019	  beb  Cleaned up confusing nested if statements
 *  971020    mf   found another bad if block under long long support
 *	980113	  FS   Converted __va_start() to va_start() for MIPS_bare 
 * mm 980206       Added wide-character support
 *  980301    BLC  added vsnprintf, snprintf functions
 *  980309    BLC  added format string support for 'll' and 'hh' type mods from C9X
 *	980320	  MEA  Changed the call to parse_format for PPC_EABI
 *   mf--060298  -- should print 0 with correct sign(i.e. -0 when the number is minus zero(0x80000000)
 * mm 980715	   Corrected placement of null char in result of vsnprintf MW07942
 * ad 1.28.99 guarded printf() and sprintf() with if __dest_os != __n64_os
 * vss 990203	   Add AltiVec changes (contributed)
 *  mm 990208	   Change to make %lE, %le, %lG, and %lg be accepted but ignore the l (C9x Clause 7.13.6.1 para #3)
 *  mm 990317	ReCorrected placement of null char in result of vsnprintf to match C9x (IL9903-1264)
 *  mm 990325	Modified to make split between string functions andfile i/o
 */
