/*  Metrowerks Standard Library  */

/*  $Date: 1999/03/09 17:42:15 $ 
 *  $Revision: 1.10 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	rand.c
 *	
 *	Routines
 *	--------
 *		rand
 *		srand
 *
 *	Note
 *	----
 *
 *		This is exactly the function described in the standard, including the
 *		assumption that RAND_MAX == 32767.
 *
 *
 */

#include <stdlib.h>

#if _MWMT && (__dest_os	== __win32_os || __dest_os	== __wince_os)
	#include <ThreadLocalData.h>
#else

	__tls static unsigned long int next = 1;

#endif

int rand(void)
{
#if _MWMT && (__dest_os	== __win32_os || __dest_os	== __wince_os)
	_GetThreadLocalData()->random_next =
			_GetThreadLocalData()->random_next * 1103515245 + 12345;
	return((_GetThreadLocalData()->random_next >> 16) & 0x7FFF);
#else
	next = next * 1103515245 + 12345;
	return((next >> 16) & 0x7FFF);
#endif
}

void srand(unsigned int seed)
{
#if _MWMT && (__dest_os	== __win32_os || __dest_os	== __wince_os)
	_GetThreadLocalData()->random_next = seed;
#else
	next = seed;
#endif
}

/*  Change Record
 *	07-Jun-95 JFH  First code release.
 *	29-Apr-96 JFH  Merged Win32 changes in.
 *						CTV
 *	19-Dec-96	KO		Changed this file to use the thread local data structure instead
 *              		of a global.
 * mf  980512       wince changes
 * mf  990309       use _MWMT instead of _MT for single threaded lib
*/
