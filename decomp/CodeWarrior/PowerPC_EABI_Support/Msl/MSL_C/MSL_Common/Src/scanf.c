/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/03 23:59:00 $ 
 *  $Revision: 1.13.4.1.2.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	scanf.c
 *	
 *	Routines
 *	--------
 *		fscanf
 *		scanf
 *		sscanf
 *
 *
 */

#include <ansi_fp.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_io.h"
#include "strtold.h"
#include "strtoul.h"

#define conversion_buff_size	512
#define conversion_max			509
#define bad_conversion			0xFF
enum argument_options 
{
	normal_argument,
	char_argument,                      /* blc 980317 */
	short_argument,
	long_argument,
	long_long_argument,                 /* mm 961219 */
	double_argument,
	long_double_argument,
	wchar_argument						/* mm 990407 */
};

typedef unsigned char char_map[32];

typedef struct 
{
	unsigned char	suppress_assignment;
	unsigned char 	field_width_specified;
	unsigned char 	argument_options;
	unsigned char	conversion_char;
	int				field_width;
#if __ALTIVEC__ && !defined(_No_Altivec)
	int	 vec;							/* doing vector conversion						*/
	int  vecCount;						/* number of vec fields to convert (4, 8, 16) 	*/
	char *vecSeperator;					/* separator string to insert between vec fields*/
	char *c_vecSeperator;				/* separator string to use for %c vec conversion*/
	int	 vecSepLen;						/* length of *vecSeperator or *c_vecSeperator 	*/
#endif
	char_map		char_set;
} scan_format;

#if __ALTIVEC__ && !defined(_No_Altivec)
#if !__VEC__
	#define vec_va_arg(ap)  (* (((MWVector128 *) (ap =  (char *) (( ((unsigned long)ap+15) & ~(15) ) + 16)       ))  -1))
	#define va_arg_replacement2(ap, vec_val) (*va_arg(ap, MWVector128 *) = vec_val)
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

#define set_char_map(map, ch)	 map[(unsigned char)ch>>3] |= (1 << (ch&7))			/* mm 990716 */
#define tst_char_map(map, ch) (map[(unsigned char)ch>>3] &  (1 << (ch&7)))			/* mm 990716 */

#if __ALTIVEC__ && !defined(_No_Altivec)
static const char * parse_format(const char * format_string, va_list * arg, scan_format * format)
#else
static const char * parse_format(const char * format_string, scan_format * format)
#endif
{
	const char *	s = format_string;
	int				c;
	int				flag_found, invert;
#if __ALTIVEC__ && !defined(_No_Altivec)
	int				vec_sep_ok = 1;
	scan_format		f = {0, 0, normal_argument, 0, INT_MAX, 0, 0, "", "", 0, {0}};
#else
	scan_format		f = {0, 0, normal_argument, 0, INT_MAX, {0}};
#endif
	
	if ((c = *++s) == '%')
	{
		f.conversion_char = c;
		*format = f;
		return((const char *) s + 1);
	}
	
	if (c == '*')
	{
		f.suppress_assignment = 1;
		
		c = *++s;
	}
	
#if __ALTIVEC__ && !defined(_No_Altivec)
	GetWidth:			 			/* goto here when we see we had a vec sep char flag */
#endif
	
	if (isdigit(c))
	{
		f.field_width = 0;
		
		do
		{
			f.field_width = (f.field_width * 10) + (c - '0');
			c = *++s;
		}
		while (isdigit(c));
		
		if (f.field_width == 0)
		{
			f.conversion_char = bad_conversion;
			*format = f;						/* mm 981104 */
			return((const char *) s + 1);
		}
		
		f.field_width_specified = 1;
	}
	
#if __ALTIVEC__ && !defined(_No_Altivec)
	if (c == 'v')
	{
		f.vec = 1;
		c = *++s;
	}
#endif

	flag_found = 1;
	
	switch (c)
	{
		case 'h':
			f.argument_options = short_argument;
			
			if (s [1] == 'h')
			{
				f.argument_options = char_argument;
				c = *++s; /* move past second h */
			}
#if __ALTIVEC__ && !defined(_No_Altivec)
			else
				f.vecCount = 8;
#endif
			
			break;
		
		case 'l':
			
			f.argument_options = long_argument;

#ifdef __MSL_LONGLONG_SUPPORT__
			if (s [1] == 'l')
			{
				f.argument_options = long_long_argument;
				c = *++s; /* move past second l */
			}
#endif
#if __ALTIVEC__ && !defined(_No_Altivec)
			else
				f.vecCount = 4;
#endif
			
			break;
		
		case 'L':
			
			f.argument_options = long_double_argument;
			
			break;

		default:
			
			flag_found = 0;
#if __ALTIVEC__ && !defined(_No_Altivec)
			f.vecCount = 16;
#endif
	}
	
	if (flag_found)
		c = *++s;
	
#if __ALTIVEC__ && !defined(_No_Altivec)
	if (!f.vec && c == 'v' && f.vecCount != 0) 
	{
		f.vec = 1;
		c = *++s;
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
			
			if (f.argument_options == long_double_argument)
			{
				f.conversion_char = bad_conversion;
				break;
			}

			break;
		
        case 'a':		/* BLC 980317 -- from C9X */
		case 'f':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
			
			if (f.argument_options == char_argument 
				|| f.argument_options == short_argument
#ifdef __MSL_LONGLONG_SUPPORT__
				|| f.argument_options == long_long_argument
#endif
			)
			{
				f.conversion_char = bad_conversion;
				break;
			}
			
			/* 'l' before float identifier means double */

			if (f.argument_options == long_argument)
				f.argument_options = double_argument;

			break;

		case 'p':
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (!f.vec)
#endif
			f.argument_options = long_argument;
			f.conversion_char  = 'x';
			
			break;
			
		case 'c':
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (!f.vec)
#endif
			if (f.argument_options == long_argument)			/* mm 990407 */
				f.argument_options = wchar_argument;			/* mm 990407 */
			else												/* mm 990407 */
				if (f.argument_options != normal_argument)
					f.conversion_char = bad_conversion;
			
			break;
			
		case 's':
			
			if (f.argument_options == long_argument)			/* mm 990407 */
				f.argument_options = wchar_argument;			/* mm 990407 */
			else												/* mm 990407 */
				if (f.argument_options != normal_argument)
					f.conversion_char = bad_conversion;
			
			{
				int							i;
				unsigned char *	p;
				
				for (i = sizeof(f.char_set), p = f.char_set; i; --i)
					*p++ = 0xFF;
				
				f.char_set[1] = 0xC1;
				f.char_set[4] = 0xFE;
			}
			
			break;
			
		case 'n':
			
			break;
		
		case '[':
			if (f.argument_options == long_argument)			/* mm 990407 */
				f.argument_options = wchar_argument;			/* mm 990407 */
			else												/* mm 990407 */
				if (f.argument_options != normal_argument)
					f.conversion_char = bad_conversion;
			
			c = *++s;
			
			invert = 0;
			
			if (c == '^')
			{
				invert = 1;
				c = *++s;
			}
			
			if (c == ']')
			{
				set_char_map(f.char_set, ']');
				c = *++s;
			}
			
			while (c && c != ']')
			{
				int d;
				
				set_char_map(f.char_set, c);
				
				if (*(s+1) == '-' && (d = *(s+2)) != 0 && d != ']')
				{
					while (++c <= d)
						set_char_map(f.char_set, c);
					
					c = *(s += 3);
				}
				else
					c = *++s;
			}
			
			if (!c)
			{
				f.conversion_char = bad_conversion;
				break;
			}
			
			if (invert)
			{
				int							i;
				unsigned char *	p;
				
				for (i = sizeof(f.char_set), p = f.char_set; i; --i, ++p)
					*p = ~*p;
			}
			
			break;
		
		default:
#if __ALTIVEC__ && !defined(_No_Altivec)
			if (!f.vec && vec_sep_ok)
			{	
				vec_sep_ok = 0;
				if (c == '@') {
					f.vecSeperator = va_arg(*arg, char *);
					f.vecSepLen    = strlen(f.vecSeperator);
					while (f.vecSepLen > 0 && *f.vecSeperator == ' ') {
						++f.vecSeperator;
						--f.vecSepLen;
					}
					while (f.vecSepLen > 0 && f.vecSeperator[f.vecSepLen-1] == ' ')
						--f.vecSepLen;
				} else {
					f.vecSeperator = (char *)s;
					f.vecSepLen    = 1;
				}
				f.c_vecSeperator = f.vecSeperator;
				c = *++s;
				if (!f.suppress_assignment && c == '*')
				{
					f.suppress_assignment = 1;
					c = *++s;
				}
				goto GetWidth;
			}
#endif
			f.conversion_char = bad_conversion;
			
			break;
	}
	
	*format = f;
	
	#if 0
	fprintf(stderr, "scan_format:\n"
					"  suppress_assignment   = %d\n"
					"  field_width_specified = %d\n"
					"  argument_options      = %d\n"
					"  conversion_char       = %c\n"
					"  field_width           = %ld\n"
					"  vec                   = %ld\n"
					"  vecCount              = %ld\n"
					"  vecSeperator          = %s\n"
					"  vecSepLen             = %ld\n",
					f.suppress_assignment,
					f.field_width_specified,
					f.argument_options,
					f.conversion_char,
					f.field_width,
					f.vec,
					f.vecCount,
					f.vecSeperator == 0 ? "<null>" : f.vecSeperator,
					f.vecSepLen);
	#endif
					
	return((const char *) s + 1);
}


static int __sformatter(int (*ReadProc)(void *, int, int), void * ReadProcArg, 
														const char * format_str, va_list arg)  /* mm 990325 */
{
	int					num_chars, chars_read, items_assigned, conversions;
	int					base, negative, overflow;
	const char *		format_ptr;
	char				format_char;           /* mm-961002*/
	char             	c;                     /* mm-961002*//* mm 990409 */
	scan_format			format;
	long				long_num;
	unsigned long		u_long_num;
#ifdef __MSL_LONGLONG_SUPPORT__                /*mm 961219*/
	long long   		long_long_num;         /*mm 961219*/
	unsigned long long 	u_long_long_num;       /*mm 961219*/
#endif                                         /*mm 961219*/
#ifndef _No_Floating_Point
	long double			long_double_num;
#endif
	char *				arg_ptr;
	int terminate  = 0;			/* mm 990608 */
	
#if __ALTIVEC__ && !defined(_No_Altivec)
	int vecIndex;						/* counts vector elements						*/
	MWVector128 vec_val;

	#define GOTO_GET_REMAINING_VEC_FIELDS(lbl) 										\
		{																			\
			if ((c = (*ReadProc)(ReadProcArg, 0, __GetAChar)) != EOF && ++vecIndex < format.vecCount) {			\
				int sepSeen = (format.vecSepLen == 0);								\
				if (!sepSeen) {														\
					int i = 0;														\
					while (isspace(c)) {											\
						c = (*ReadProc)(ReadProcArg, 0, 1);												\
						++chars_read;												\
					}																\
					while (c != EOF) {												\
						sepSeen = (c == format.vecSeperator[i]);					\
						if (!sepSeen)												\
							break;													\
						c = (*ReadProc)(ReadProcArg, 0, __GetAChar);												\
						++chars_read;												\
						if (++i >= format.vecSepLen)								\
							break;													\
					}																\
				}																	\
				if (sepSeen && c != EOF) {											\
					while (isspace(c)) {											\
						c = (*ReadProc)(ReadProcArg, 0, __GetAChar);												\
						++chars_read;												\
					}																\
					(*ReadProc)(ReadProcArg, c, __UngetAChar);												\
					goto lbl;														\
				}																	\
			} else																	\
				(*ReadProc)(ReadProcArg, c, __UngetAChar);													\
		}
#endif

	format_ptr     = format_str;
	chars_read     = 0;
	items_assigned = 0;
	conversions    = 0;
	
	while (!terminate && (format_char = *format_ptr) != 0) /* mm 990311 */	/* mm 990608 */
	{
		if (isspace(format_char))
		{
			do
				format_char = *++format_ptr;
			while (isspace(format_char));
			
			while (isspace(c = (*ReadProc)(ReadProcArg, 0, __GetAChar)))			/* mm 990325 */
				++chars_read;
			
			(*ReadProc)(ReadProcArg, c, __UngetAChar);								/* mm 990325 */
			
			continue;
		}
		
		if (format_char != '%')
		{
			if ((c = (*ReadProc)(ReadProcArg, 0, __GetAChar)) != (unsigned char)format_char)      /* mm 980331 *//* mm 990325 */
			{
				(*ReadProc)(ReadProcArg, c, __UngetAChar);							/* mm 990325 */
				goto exit;
			}
			
			++chars_read;
			++format_ptr;
			
			continue;
		}
#if !(__ALTIVEC__ && !defined(_No_Altivec))
		format_ptr = parse_format(format_ptr, &format);
#else
		format_ptr = parse_format(format_ptr, (va_list *)&arg, &format);
		
		if (format.vec)
		{
			vecIndex 	 	= 0;
			vec_val.u32[0]	= 0;
			vec_val.u32[1]	= 0;
			vec_val.u32[2]	= 0;
			vec_val.u32[3]	= 0;
			
			/* Why aren't leading input blanks skipped in non-vector cases ? */
			
			while (isspace(c = (*ReadProc)(ReadProcArg, 0, __GetAChar)))			/* mm 990325 */
				++chars_read;
			(*ReadProc)(ReadProcArg, c, __UngetAChar);								/* mm 990325 */
		} else
#endif
		if (!format.suppress_assignment && format.conversion_char != '%')
			arg_ptr = va_arg(arg, char *);
		else
			arg_ptr = 0;

		if ((format.conversion_char != 'n') && (*ReadProc)(ReadProcArg, 0, __TestForError))	/* mm 990608 */
		{																					/* mm 990608 */
			terminate = 1;																	/* mm 990608 */
			goto exit;																		/* mm 990608 */
		}																					/* mm 990608 */

		
		switch (format.conversion_char)
		{
			case 'd':
				
				base = 10;
				
				goto signed_int;
				
			case 'i':
				
				base = 0;
				
			signed_int:
				
#ifdef __MSL_LONGLONG_SUPPORT__                                    /*mm 961219*/
                if (format.argument_options == long_long_argument) /*mm 961219*/
                   u_long_long_num =                               /*mm 961219*/
                      __strtoull(base, format.field_width, ReadProc, ReadProcArg, &num_chars, &negative, &overflow);
                else                                               /*mm 961219*/
#endif                                                             /*mm 961219*/
				u_long_num
					= __strtoul(base, format.field_width, ReadProc, ReadProcArg, &num_chars, &negative, &overflow);
				
				if (!num_chars)
					goto exit;
				
				chars_read += num_chars;
				
#ifdef __MSL_LONGLONG_SUPPORT__                                    /*mm 961219*/
                if (format.argument_options == long_long_argument) /*mm 961219*/
                   long_long_num = (negative ? -u_long_long_num : u_long_long_num); /*mm 961219*/
                else                                               /*mm 961219*/
#endif                                                             /*mm 961219*/
				long_num = (negative ? -u_long_num : u_long_num);
				
#if __ALTIVEC__ && !defined(_No_Altivec)
				if (format.vec)
				{
					if (!format.suppress_assignment)
					{
						switch (format.argument_options)
						{
							case normal_argument: vec_val.s8 [vecIndex] = long_num; break;
							case short_argument:  vec_val.s16[vecIndex] = long_num; break;
							case long_argument:   vec_val.s32[vecIndex] = long_num; break;
						}
						GOTO_GET_REMAINING_VEC_FIELDS(signed_int);
						
						#if __VEC__
						*va_arg(arg, MWVector128 *) = vec_val;
						#else
						va_arg_replacement2(arg, vec_val);
						#endif
						++items_assigned;
					}
					else
						GOTO_GET_REMAINING_VEC_FIELDS(signed_int);
				}
				else
#endif

				if (arg_ptr)
				{
					switch (format.argument_options)
					{
						case normal_argument: * (int *)         arg_ptr = long_num; break;
						case char_argument:   * (signed char *) arg_ptr = long_num; break;
						case short_argument:  * (short *)       arg_ptr = long_num; break;
						case long_argument:   * (long *)        arg_ptr = long_num; break;
#ifdef __MSL_LONGLONG_SUPPORT__                                    /*mm 961219*/
                        case long_long_argument: * (long long *) arg_ptr = long_long_num; break;
#endif                                                             /*mm 961219*/
					}
					
					++items_assigned;
				}
				
				++conversions;
				
				break;
				
			case 'o':
				
				base = 8;
				
				goto unsigned_int;
				
			case 'u':
				
				base = 10;
				
				goto unsigned_int;
				
			case 'x':
			case 'X':
				
				base = 16;
				
			unsigned_int:
				
#ifdef __MSL_LONGLONG_SUPPORT__                                    /*mm 961219*/
                if (format.argument_options == long_long_argument) /*mm 961219*/
                   u_long_long_num =                               /*mm 961219*/
                      __strtoull(base, format.field_width, ReadProc, ReadProcArg, &num_chars, &negative, &overflow);
                else                                               /*mm 961219*/
#endif                                                             /*mm 961219*/
				u_long_num
					= __strtoul(base, format.field_width, ReadProc, ReadProcArg, &num_chars, &negative, &overflow);
				
				if (!num_chars)
					goto exit;
				
				chars_read += num_chars;
				
				if (negative)
#ifdef __MSL_LONGLONG_SUPPORT__                                    /*mm 961219*/
                if (format.argument_options == long_long_argument) /*mm 961219*/
					u_long_long_num = -u_long_long_num;            /*mm 961219*/
                else                                               /*mm 961219*/
#endif                                                             /*mm 961219*/
					u_long_num = -u_long_num;
#if __ALTIVEC__ && !defined(_No_Altivec)
				if (format.vec)
				{
					if (!format.suppress_assignment)
					{
						switch (format.argument_options)
						{
							case normal_argument: vec_val.u8 [vecIndex] = u_long_num; break;
							case short_argument:  vec_val.u16[vecIndex] = u_long_num; break;
							case long_argument:   vec_val.u32[vecIndex] = u_long_num; break;
						}
						GOTO_GET_REMAINING_VEC_FIELDS(unsigned_int);
						
						#if __VEC__
						*va_arg(arg, MWVector128 *) = vec_val;
						#else
						va_arg_replacement2(arg, vec_val);
						#endif
						++items_assigned;
					}
					else
						GOTO_GET_REMAINING_VEC_FIELDS(unsigned_int);
				}
				else
#endif
				if (arg_ptr)
				{
					switch (format.argument_options)
					{
						case normal_argument: * (unsigned int *)   arg_ptr = u_long_num; break;
						case char_argument:   * (unsigned char *)  arg_ptr = u_long_num; break;
						case short_argument:  * (unsigned short *) arg_ptr = u_long_num; break;
						case long_argument:   * (unsigned long *)  arg_ptr = u_long_num; break;
#ifdef __MSL_LONGLONG_SUPPORT__                                    /*mm 961219*/
                        case long_long_argument: * (unsigned long long *) arg_ptr = u_long_long_num; break;
#endif                                                             /*mm 961219*/
					}
					
					++items_assigned;
				}
				
				++conversions;
				
				break;

#ifndef _No_Floating_Point

			case 'a':		/* BLC 980317 -- from C9X */
			case 'f':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
			flt:			
				long_double_num = __strtold(format.field_width, ReadProc, ReadProcArg, &num_chars, &overflow);
				
				if (!num_chars)
					goto exit;
				
				chars_read += num_chars;
#if __ALTIVEC__ && !defined(_No_Altivec)
				if (format.vec)
				{
					if (!format.suppress_assignment)
					{
						vec_val.f32[vecIndex] = long_double_num;
						
						if (format.argument_options == normal_argument)
							format.vecCount = 4;
						GOTO_GET_REMAINING_VEC_FIELDS(flt);

						#if __VEC__
						*va_arg(arg, MWVector128 *) = vec_val;
						#else
						va_arg_replacement2(arg, vec_val);
						#endif
						++items_assigned;
					}
					else
						GOTO_GET_REMAINING_VEC_FIELDS(flt);
				}
				else
#endif
				if (arg_ptr)
				{
					switch (format.argument_options)
					{
						case normal_argument:      * (float *)       arg_ptr = long_double_num; break;
						case double_argument:      * (double *)      arg_ptr = long_double_num; break;
						case long_double_argument: * (long double *) arg_ptr = long_double_num; break;
					}
					
					++items_assigned;
				}
				
				++conversions;

				break;

#endif

			case 'c':
				
				if (!format.field_width_specified)
					format.field_width = 1;
#if __ALTIVEC__ && !defined(_No_Altivec)
				if (format.vec)
				{
					if (!format.suppress_assignment)
					{
						char *ptr;
						int width;
						
						if (vecIndex == 0) {
							format.vecSeperator = format.c_vecSeperator;
							format.vecSepLen    = *format.c_vecSeperator ? 1 : 0;
						}
					str1:
						switch (format.argument_options)
						{
							case long_argument:
								if (format.field_width > 4)
									goto exit;
								ptr = (char *)&vec_val.u8[4*vecIndex + 4 - format.field_width];
								break;
							case short_argument:
								if (format.field_width > 2)
									goto exit;
								ptr = (char *)&vec_val.u8[2*vecIndex + 2 - format.field_width];
								break;
							case normal_argument:
								if (format.field_width > 1)
									goto exit;
								ptr = (char *)&vec_val.u8[vecIndex];
								break;
						}
						
						num_chars = 0;
						width = format.field_width;
						while (width-- && (c = (*ReadProc)(ReadProcArg, 0, __GetAChar)) != EOF)	/* mm 990325 */
						{
							/*if  (c == 0) /* mm 971202 */
							/*	return(EOF);                                         /* mm 971202 */
							*ptr++ = c;
							++num_chars;
						}
						
						if (!num_chars)
							goto exit;
						
						chars_read += num_chars;
						GOTO_GET_REMAINING_VEC_FIELDS(str1);
						#if __VEC__
						*va_arg(arg, MWVector128 *) = vec_val;
						#else
						va_arg_replacement2(arg, vec_val);
						#endif
					
						++items_assigned;
					}
					else
					{
						int width;
					str2:
						num_chars = 0;
						width = format.field_width;
						while (format.field_width-- && (c = (*ReadProc)(ReadProcArg, 0, __GetAChar)) != EOF)	/* mm 990325 */
							++num_chars;
						
						if (!num_chars)
							goto exit;
						
						GOTO_GET_REMAINING_VEC_FIELDS(str2);
					}
				}
				else
#endif
				if (arg_ptr)
				{
					num_chars = 0;
					
					while (format.field_width-- && 
						(c = (*ReadProc)(ReadProcArg, 0, __GetAChar)) != EOF)	/* mm 990325 */
					{
						if (format.argument_options == wchar_argument)		/* mm 990407 */
						{
							mbtowc(((wchar_t*)arg_ptr), (char*)&c, 1);
							(wchar_t*)arg_ptr++;
						}
						else												/* mm 990407 */
							*arg_ptr++ = c;
						++num_chars;
					}
					
					if (!num_chars)
						goto exit;
					
					chars_read += num_chars;
					
					++items_assigned;
				}
				else
				{
					num_chars = 0;
					
					while (format.field_width-- && (c = (*ReadProc)(ReadProcArg, 0, __GetAChar)) != EOF)	/* mm 990325 */
						++num_chars;
					
					if (!num_chars)
						goto exit;
				}
				
				++conversions;
				
				break;
			
			case '%':
#if __ALTIVEC__ && !defined(_No_Altivec)
				if (format.vec)
					goto exit;
#endif
				while (isspace(c = (*ReadProc)(ReadProcArg, 0, __GetAChar)))	/* mm 990325 */
					++chars_read;
				
				if (c != '%')
				{
					(*ReadProc)(ReadProcArg, c, __UngetAChar);					/* mm 990325 */
					goto exit;
				}
				
				++chars_read;
				
				break;
			
			case 's':
#if __ALTIVEC__ && !defined(_No_Altivec)
				if (format.vec)
					goto exit;
#endif
				c = (*ReadProc)(ReadProcArg, 0, __GetAChar);			 /* mm 990325 */  /* mm 990413 */
				while(isspace(c))                                        /*mm 970218*/
				{                                                        /*mm 970218*/
					++chars_read;                                        /*mm 970218*/
					c = (*ReadProc)(ReadProcArg, 0, __GetAChar);         /*mm 970218*/	/* mm 990325 */
				}                                                        /*mm 970218*/
				
				(*ReadProc)(ReadProcArg, c, __UngetAChar);				/* mm 990325 */
				
			case '[':
#if __ALTIVEC__ && !defined(_No_Altivec)
				if (format.vec)
					goto exit;
#endif
				if (arg_ptr)
				{
					num_chars = 0;
					
					while (format.field_width-- && 
								(c = (*ReadProc)(ReadProcArg, 0, __GetAChar)) != EOF && tst_char_map(format.char_set, c)) /* mm 990325 */
					{
						if (format.argument_options == wchar_argument)		/* mm 990420 */
						{													/* mm 990420 */
							mbtowc(((wchar_t*)arg_ptr), (char*)&c, 1);		/* mm 990420 */
							arg_ptr = (char*)((wchar_t*)arg_ptr + 1);					/* mm 990420 */
						}													/* mm 990420 */
						else												/* mm 990420 */
							*arg_ptr++ = c;
						++num_chars;;
					}
					
					if (!num_chars)
					{
						(*ReadProc)(ReadProcArg, c, __UngetAChar);               /* mm 961114 */	/* mm 990325 */
						goto exit;
					}
					
					chars_read += num_chars;
					
					if (format.argument_options == wchar_argument)				/* mm 990420 */
						*(wchar_t*)arg_ptr = L'\0';								/* mm 990420 */
					else														/* mm 990420 */
						*arg_ptr = 0;
					
					++items_assigned;
				}
				else
				{
					num_chars = 0;
					
					while (format.field_width-- && 
								(c = (*ReadProc)(ReadProcArg, 0, __GetAChar)) != EOF && tst_char_map(format.char_set, c))	/* mm 990325 */
						++num_chars;
					
					if (!num_chars)
					{
						(*ReadProc)(ReadProcArg, c, __UngetAChar);               /* mm 970513 *//* mm 990325 */
						goto exit;
					}
					chars_read += num_chars;          /* mm 970501 */
				}
				
				if (format.field_width >= 0)
					(*ReadProc)(ReadProcArg, c, __UngetAChar);					/* mm 990325 */
				
				++conversions;
				
				break;
			
			case 'n':
				
				if (arg_ptr)
					switch (format.argument_options)
					{
						case normal_argument: * (int *)   arg_ptr = chars_read; break;
						case short_argument:  * (short *) arg_ptr = chars_read; break;
						case long_argument:   * (long *)  arg_ptr = chars_read; break;
						case char_argument:   * (char *)  arg_ptr = chars_read; break;			/* mm 990414 */
#ifdef __MSL_LONGLONG_SUPPORT__                                    /*mm 961219*/
                        case long_long_argument: * (long long *) arg_ptr = chars_read; break;
#endif                                                             /*mm 961219*/
					}
				
				continue;
				
			case bad_conversion:
			default:
				
				goto exit;
		}
		
	}
	
exit:
	
	if ((*ReadProc)(ReadProcArg, 0, __TestForError) && conversions == 0)			/* mm 990325 */
		return(EOF);
	
	return(items_assigned);
}

/* The remainder of the filoe has been heavily modified  mm 990325 */
int __FileRead(void *File, int ch, int Action)
{
	switch(Action)
	{
		case __GetAChar:
			return(fgetc((FILE *)File));
		case __UngetAChar:
			return(ungetc(ch, (FILE *)File));
		case __TestForError:
			return(ferror((FILE *)File) || feof((FILE *)File));
	}
	return 0;  /* to satisfy compiler */
}

int __StringRead(void * isc, int ch, int Action)
{
	char RetVal;
	__InStrCtrl * Iscp = (__InStrCtrl*)isc;
	switch(Action)
	{
		case __GetAChar:
			RetVal = *(Iscp->NextChar);
			if (RetVal == '\0')
			{
				Iscp->NullCharDetected = 1;
				return (EOF);
			}
			else
			{
				Iscp->NextChar++;
				return(RetVal);
			}
		case __UngetAChar:
			if (!Iscp->NullCharDetected)		/* mm 990413 */
				Iscp->NextChar--;				/* mm 990413 */
			else								/* mm 990413 */
				Iscp->NullCharDetected = 0;		/* mm 990413 */
			return(ch);
		case __TestForError:
			return(Iscp->NullCharDetected);
	}
	return 0;  /* to satisfy compiler */
}


int fscanf(FILE * file, const char * format, ...)
{
#if __PPC_EABI__ || __MIPS__ /*__dest_os == __mips_bare  */
	va_list args;
#ifndef __NO_WIDE_CHAR										/* mm 980205 */
	if ((file == NULL) || (fwide(file, -1) >= 0))			/* mm 990618 */
		return(EOF);
#endif /* __NO_WIDE_CHAR */                                 /* mm 980205 */
	va_start( args, format );
	return(__sformatter(&__FileRead, (void *)file, format, args));
#else
#ifndef __NO_WIDE_CHAR										/* mm 980205 */
	if ((file == NULL) || (fwide(file, -1) >= 0))			/* mm 990618 */
		return(EOF);
#endif /* __NO_WIDE_CHAR */                                 /* mm 980205 */
	return(__sformatter(&__FileRead, (void *)file, format, __va_start(format)));
#endif
}

int scanf(const char * format, ...)
{
#if __PPC_EABI__ || __MIPS__   /*__dest_os == __mips_bare */
	va_list args;
#ifndef __NO_WIDE_CHAR										/* mm 980205 */
	if (fwide(stdin, -1) >= 0)
		return(EOF);
#endif /* __NO_WIDE_CHAR */                                 /* mm 980205 */
	
	va_start( args, format );
	return(__sformatter(&__FileRead, (void *)stdin, format, args));
#else
#ifndef __NO_WIDE_CHAR										/* mm 980205 */
	if (fwide(stdin, -1) >= 0)
		return(EOF);
#endif /* __NO_WIDE_CHAR */                                 /* mm 980205 */
	return(__sformatter(&__FileRead, (void *)stdin, format, __va_start(format)));
#endif
}


int __vsscanf(const char * s, const char * format, va_list arg)
{
	__InStrCtrl isc;
	isc.NextChar         = (char *)s;
	if ((s == NULL) || (*isc.NextChar == '\0'))	/* mm 990617 */
		return(EOF);							/* mm 990617 */
	isc.NullCharDetected = 0;
	
	return(__sformatter(&__StringRead, (void *)&isc, format, arg));
}

int sscanf(const char * s, const char * format, ...)
{
#if __PPC_EABI__ || __MIPS__ /*   __dest_os == __mips_bare */
	va_list args;
	
	va_start( args, format );
	return(__vsscanf(s, format, args));
#else
	return(__vsscanf(s, format, __va_start(format)));
#endif
}

/*  Change Record
 *	26-Sep-95 JFH  First code release.
 *	29-Sep-95 JFH  Fixed misinterpretation of Standard concerning what to do when EOF
 *								 is reached. Now format string continues to be processed until input
 *								 is absolutely necessary (i.e., '%n' still processed even if preceding
 *								 directive hit EOF).
 *	12-Feb-96 JFH  Fixed longstanding (yet undiscovered) bug in parse_format's handling
 *								 of field widths. field_width was initialized to INT_MAX as a default.
 *								 If a digit was seen, field_width was *not* being zeroed before
 *								 accumulating digits in the usual manner.
 *	13-Feb-96 JFH  Changed vfscanf and vsscanf to __vfscanf and __vsscanf, made them
 *								 non-static, and added their prototypes to <stdio.h>.
 *	19-Feb-96 JFH  Added #ifndef around area that handles floating point input. If disabled, you
 *								 can get away with not including MathLib just because you wanted to use scanf.
 *								 There is also some ANSI lib code savings as well.
 *	 4-Mar-96 JFH  Fixed bug in handling of strings (%s) and scansets (%[]) where the last char
 *								 fetched was being ungotten *even if* the conversion ended because the
 *								 field width was exhausted (thus causing the last char of the previous field
 *								 to be rescanned).
 *	21-Apr-96 JFH  Fix bug in scanset inversion (*p++ = ~*p is ill-formed because order of
 *								 evaluation is not guaranteed).
 *	29-aug-96 bkoz added l. you's changes for powertv, no floating point
 *  960930 mm      Removed bitfields from scan_format structure to compile with ANSI strict
 *  961002 mm      Corrected problem recognizing eof
 *  961114 mm      Make sure non matching character is left unread
 *  961220 mm      Additions for support of long long
 *  mm 970218      To make sscanf work with null source string.
 *  mm 970501      To count input characters even when assignment suppressed
 *  mm 970513      Make sure non matching character is left unread even when assignment suppressed
 *  09-Jul-97 SCM  Converted __va_start() calls to va_start() for PPC_EABI.
 *  13-Jan-98 FS   Converted __va_start() calls to va_start() for MIPS_Bare.
 *  mm 971202      To complete mm 970218 
 *  mm 980204      Added wide character support
 *  blc 980317     Added ll (long long)
 *				   Fixed L (long double) prefix
 *                 added a support for decimal input (symmetric to printf)
 *                 added hh (char) prefix
 *  blc 980318     Changed hh to be (signed char) unless explicitly unsigned
 *  mm 980331      Modification to allow characters >= 128, e.g. ァ or ノ to be matched from format
 *                 string correctly. MW06720
 *  mm 981104	   Inserted missing setting of *format IL9810-2059
 *  vss 990203	   Added AltiVec changes (contributed)
 *  mm 990325	Changes to split string scanf functions from file i/o
 *  mm 990407   Added code to handle %lc
 *  mm 990413	Corrected handling of case where source text quits early IL9904-0736
 *  mm 990414	Make %hhn work
 *  mm 990407   Added code to handle %lc
 *  mm 990413	Corrected handling of case where source text quits early IL9904-0736
 *  mm 990527	Corrected of string read function in case of unget and made %n work after end of file
 *  mm 990617	Correction for case where pointer to source string for sscanf == NULL or source string is null
 *  mm 990716	Correction to code for checking format.char_set
 */
