/*  Metrowerks Standard Library  Version 2.1.2  1997 May  */

/*
 *	string.c
 *	
 *		Copyright 1995-1998 Metrowerks, Inc.
 *		All rights reserved.
 *	
 *	Routines
 *	--------
 *		strcpy
 *		strcmp
 *
 *
 */

#include <string.h>

#if (__dest_os	!= __ppc_eabi)
	#error
#endif

#pragma warn_possunwant off

char * (strcpy)(char * dst, const char * src)
{
	
	const	unsigned char * p = (unsigned char *) src - 1;
	unsigned char * q = (unsigned char *) dst - 1;
	
	while (*++q = *++p);
	
	return(dst);
}

int strcmp(const char * str1, const char * str2)
{
	
	const	unsigned char * p1 = (unsigned char *) str1 - 1;
	const	unsigned char * p2 = (unsigned char *) str2 - 1;
	unsigned long		c1, c2;
		
	while ((c1 = *++p1) == (c2 = *++p2))
		if (!c1)
			return(0);

	return(c1 - c2);

}


#pragma warn_possunwant reset

/*  Change Record
 *	980320	MEA		Created.
 */
