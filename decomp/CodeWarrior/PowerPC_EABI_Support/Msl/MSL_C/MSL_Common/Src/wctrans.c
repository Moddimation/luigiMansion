/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 01:02:54 $ 
 *  $Revision: 1.7.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */
 
/*
 *	wctrans.c
 *	
 *	Routines
 *	--------
 *	
 *	towctrans
 *	wctrans
 *
*/

#pragma ANSI_strict off  /*  990729 vss  empty compilation unit illegal in ANSI C */

#ifndef __NO_WIDE_CHAR				/* mm 980204 */
 
#pragma ANSI_strict reset

#include <wctrans.h>

static const struct wctable 
{
	const char *string;
	wctype_t	value;
} 

wtable[] = 
{
	{"tolower", 0},
	{"toupper", 1},
	{(const char *)0, 0}
};

wint_t towctrans(wint_t c, wctrans_t value)
{
	return (value == 1 ? towupper(c) : towlower(c));
}

wctrans_t wctrans(const char *name)
{
	int	i;
	
	for(i=0; wtable[i].string != 0; ++i)
	{
		if(strcmp(wtable[i].string, name) == 0)
		{
			return (wtable[i].value);
		}
	
	}

	return 0;
}

#endif /* #ifndef __NO_WIDE_CHAR */        /* 981030 */

/*  Change Record
 *	980126 JCM  First code release.
 * mm 981030  Added #ifndef __NO_WIDE_CHAR wrappers
*/
