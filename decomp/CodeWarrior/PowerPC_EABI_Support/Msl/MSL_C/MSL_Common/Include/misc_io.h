/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/04 00:02:01 $ 
 *  $Revision: 1.7.14.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	misc_io.h
 */

#ifndef __stdio_atexit__
#define __stdio_atexit__

#include <ansi_parms.h>

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifdef __cplusplus                  /*hh 971206  expanded _extern macro*/
	extern "C" {
#endif

void	__stdio_atexit(void);

#ifdef __cplusplus               /*hh 971206  expanded _extern macro*/
	}
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif
/*     Change record
 *hh 971206  expanded _extern macro
*/
