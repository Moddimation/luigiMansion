/*  Metrowerks Standard Library  */

/*  $Date: 1999/02/03 21:36:40 $ 
 *  $Revision: 1.11 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	sysenv.c
 *	
 *	Routines
 *	--------
 *		getenv
 *		system
 *
 *
 */

#include <ansi_parms.h>                 /* mm 970904 */
#if (__dest_os != __be_os) && !(__dest_os == __win32_os)				/* mm 970708 */									/* ELR */

#include <stdlib.h>

char * getenv(const char * name)
{
#pragma unused(name)

	return(NULL);
}

int system(const char* cmdLine)
{
	#pragma unused(cmdLine)

	return(NULL);

}

#endif /* __dest_os != __be_os  or win32 */     /* mm 970708 */

/*  Change Record
 *	24-Jul-95 JFH  First code release.
 * mm 970708  Inserted Be changes
 * mm 970904  Added include ansi_parms.h  to allow compilation without prefix
 * vss 990121 Add system call for win32 applications - contributed sources
 * vss 990203 Moved win32 code to sysenv.win32.c
 */
