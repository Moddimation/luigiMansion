/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:32 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	locale.c
 *
 *	Routines
 *	--------
 *		setlocale
 *		localeconv
 *
 *
 */

#include <locale.h>
#include <limits.h>
#include <string.h>
#include <stddef.h>
#include "lconv.h"

struct lconv __lconv = {
	"."				/* decimal_point			*/,
	""				/* thousands_sep			*/,
	""				/* grouping					*/,
	""				/* int_curr_symbol			*/,
	""				/* currency_symbol			*/,
	""				/* mon_decimal_point		*/,
	""				/* mon_thousands_sep		*/,
	""				/* mon_grouping				*/,
	""				/* positive_sign			*/,
	""				/* negative_sign			*/,
	CHAR_MAX		/* int_frac_digits			*/,
	CHAR_MAX		/* frac_digits				*/,
	CHAR_MAX		/* p_cs_precedes			*/,
	CHAR_MAX		/* p_sep_by_space			*/,
	CHAR_MAX		/* n_cs_precedes			*/,
	CHAR_MAX		/* n_sep_by_space			*/,
	CHAR_MAX		/* p_sign_posn				*/,
	CHAR_MAX		/* n_sign_posn				*/
};

static struct lconv public_lconv;

char * setlocale(int category, const char *locale)
{
	if (!locale || !strcmp(locale, "C") || !strcmp(locale, ""))
		if (category >= LC_FIRST && category <= LC_LAST)
			return("C");
	
	return(NULL);
}

struct lconv * localeconv(void)
{
	public_lconv = __lconv;
	
	return(&public_lconv);
}

/*  Change Record
 *	12-Jun-95 JFH  First code release.
*/
