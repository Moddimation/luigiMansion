/*  Metrowerks Standard Library  */

/*  $Date: 1999/03/08 17:32:44 $ 
 *  $Revision: 1.8 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	ansi_files.h
 */

#ifndef __ansi_files__
#define __ansi_files__

#include <ansi_parms.h>            /*hh 971206  this header must come first*/
#include <cstdio>                  /*hh 971206  namespace support*/

#define set_eof(file)						\
	do										\
	{										\
		(file)->state.io_state = __neutral;	\
		(file)->state.eof = 1;				\
		(file)->buffer_len = 0;				\
	}										\
	while (0)

#define set_error(file)				\
	do								\
	{								\
		(file)->state.error = 1;	\
		(file)->buffer_len = 0;		\
	}								\
	while (0)

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifdef __cplusplus                  /*hh 971206  namespace support*/
	extern "C" {
#endif

__std(FILE) *	__find_unopened_file(void); /* 990121 fixed __std 6 places */
void		__init_file(__std(FILE) * file, __std(__file_modes) mode, char * buff, __std(size_t) size);
void		__close_all(void);
int			__flush_all(void);

/*  mm 981015 If these routines are really needed, they need updating to account for dynamically */
/*  allocated file structures  You can look on this as like the cheshire cat's smile, remaining   */
/*  after the cat has disappeared                                                                 */
/*int			__file_to_num(__std(FILE) * file);
__std(FILE) *	__num_to_file(int file_num);*/
int __flush_line_buffered_output_files(void);

#ifdef __cplusplus                  /*hh 971206  expanded _extern macro*/
	}
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset


#endif
/*     Change record
 * hh 971206  namespace support
 * mm 981015  Commented out __file_to_num and __num_to_file while investigating whether they are really needed.
 * 990121 fixed __std 6 places
 * 990308 blc  fixed tabbing on macro definitions
*/