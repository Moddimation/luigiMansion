/*  Metrowerks Standard Library  */

/*  $Date: 1999/03/24 05:37:37 $ 
 *  $Revision: 1.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */
 
/*
 *	stdbool.h
 *
 *  based on the 18 January 1999 committee draft
 *
*/

#ifndef _STDBOOL_H
#define _STDBOOL_H

#if !(__option(c9x) && !__option(cplusplus))
#error "stdbool.h relies on C9X language features"
#endif

#define bool							_Bool
#define true							1
#define false							0
#define __bool_true_false_are_defined	1

#endif

/*
 * Change record
 * blc 990323 Created.
 */
