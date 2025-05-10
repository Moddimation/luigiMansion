/*  Metrowerks Standard Library  */

/*  $Date: 1999/03/09 17:16:22 $ 
 *  $Revision: 1.9 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	errno.c
 */

#include <errno.h>

#if !(_MWMT && (__dest_os	== __win32_os  || __dest_os	== __wince_os))
	__tls int errno;
#endif

/*  Change Record
 *	12-Jun-95 JFH  First code release.
 *	15-Apr-96	CTV	 Add TLS support for Win32
 * mf  980512       wince changes
*/
