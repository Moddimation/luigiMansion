/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:32 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	bsearch.c
 *	
 *	Routines
 *	--------
 *		bsearch
 *
 *
 */

#include <stdlib.h>

#define table_ptr(i)	(((char *) table_base) + (member_size * (i)))

void * bsearch(const void * key,
							 const void * table_base, size_t num_members, size_t member_size,
							 _compare_function compare_members)                   /* mm 961031 */
{
	size_t	l, r, m;
	int			c;
	char *	mp;
	
	if (!key || !table_base || !num_members || !member_size || !compare_members)
		return(NULL);
	
	mp = table_ptr(0);
	
	c = compare_members(key, mp);							/* We have to make sure 'key' doesn't compare   */
																						/* less than the first element in the table.    */
	if (c == 0)																/* As long as we're comparing, if it happens to */
		return(mp);															/* come out equal we'll forego discovering that */
	else if (c < 0)														/* all over again via the binary search.        */
		return(NULL);
	
	l = 1;
	r = num_members - 1;
	
	while (l <= r) {
		
		m = (l + r) / 2;
		
		mp = table_ptr(m);
		
		c = compare_members(key, mp);

		if (c == 0)
			return(mp);
		else if (c < 0)
			r = m - 1;
		else
			l = m + 1;
		
	}
	
	return(NULL);
}

/*  Change Record
 *	19-Jul-95 JFH  First code release.
 *	29-Jan-96 JFH  Fixed bug where the sense of the compare_members arguments was reversed.
 *								 Also made some small performance improvements (courtesy of Fabrizio Oddone).
 *	20-Feb-96 JFH  Fixed small bug in "performance improvement" where 'r' (which was *unsigned*)
 *								 could wrap from zero to ULONG_MAX. Rather than take the performance hit
 *								 back, num_members is constrained to fit in a *signed* long. If it doesn't,
 *								 we return NULL immediately.
 *  27-Feb-96 JFH  Fabrizio convinced me to undo the previous change and fix the bug in a
 *								 different way. We basically preflight to make sure the fatal condition
 *								 doesn't occur.
 *  mm 961031      Improved legibility of function declaration
*/
