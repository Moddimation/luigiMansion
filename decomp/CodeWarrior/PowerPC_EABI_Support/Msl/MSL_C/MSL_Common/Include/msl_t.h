/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/29 18:02:31 $ 
 *  $Revision: 1.1.2.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	msl_t.h
 */
 
#ifndef __msl_t__
#define __msl_t__

#include <ansi_parms.h>                 

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifndef _INT32
typedef int _INT32 ;
typedef unsigned int _UINT32 ;
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif

/*     Change record
 * vss 990729  New file to define new types introduced into MSL	
 */
