/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 00:31:27 $ 
 *  $Revision: 1.7.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	console_io.h
 */

#ifndef __console_io__
#define __console_io__

#include <ansi_parms.h>            /*hh 971206  this header must come first*/
#include <cstdio>                  /*hh 971206  namespace support*/

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifdef __cplusplus                  /*hh 971206  namespace support*/
	extern "C" {
#endif

/*hh 990121 Fixed __std 7 places*/
#ifndef _No_Console		/* ad 1.28.99  */
int		__read_console	  (__std(__file_handle) handle, unsigned char * buffer, __std(size_t) * count, __std(__idle_proc) idle_proc);
int		__write_console	  (__std(__file_handle) handle, unsigned char * buffer, __std(size_t) * count, __std(__idle_proc) idle_proc);
int		__close_console	  (__std(__file_handle) handle);
#endif

#ifdef __cplusplus                  /*hh 971206  expanded _extern macro*/
	}
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif
/*     Change record
 *hh 971206  namespace support
 *hh 990121 Fixed __std 7 places
*/
