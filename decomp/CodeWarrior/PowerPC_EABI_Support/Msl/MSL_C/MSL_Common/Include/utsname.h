/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/23 00:45:45 $ 
 *  $Revision: 1.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	File:		utsname.h
 *
 *	Content:	Interface file to standard UNIX-style entry points ...
 *
 *	NB:			This file implements some UNIX low level support.  These functions
 *				are not guaranteed to be 100% conformant.
 */

#ifndef	_UTSNAME
#define	_UTSNAME

#include <ansi_parms.h>                 /* mm 970905*/

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif
/* #pragma direct_destruction off */  /* 980807 vss */

#ifdef __cplusplus        /* hh 971207 moved this higher in the file */
	extern "C" {
#endif

#define _UTSNAME_FIELD_LENGTH    32				/* mm 990104 */	
/* struct for uname */
struct utsname {
	char    sysname[_UTSNAME_FIELD_LENGTH];		/* mm 990104 */
	char    nodename[_UTSNAME_FIELD_LENGTH];	/* mm 990104 */
	char    release[_UTSNAME_FIELD_LENGTH];		/* mm 990104 */
	char    version[_UTSNAME_FIELD_LENGTH];		/* mm 990104 */
	char    machine[_UTSNAME_FIELD_LENGTH];		/* mm 990104 */
};

/*
 *	Get information about the current system.
 */
int uname(struct utsname *name);

#ifdef __cplusplus
	}
#endif

/* #pragma direct_destruction reset  */  /*  980807 vss */
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset


#endif
/*     Change record
 * mm 970905    added include of ansi_parms.h to avoid need for prefix file
 * hh 971207    moved this higher in the file
 * vss 980807   remove pragma  - no longer supported by compiler
 * mm 990104	Made field length into defined constant
 */
