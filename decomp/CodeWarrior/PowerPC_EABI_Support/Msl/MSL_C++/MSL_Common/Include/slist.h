/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:59:45 $ 
 *  $Revision: 1.8.6.1 $ 
 *  $NoKeywords: $ 
 *
 *		Portions Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/**
 **  slist.h
 **/

#ifndef _SLIST_H
#define _SLIST_H

#include <slist>

#ifndef _MSL_NO_CPP_NAMESPACE
	using std::slist;
#endif

#endif // _SLIST_H 

// hh 971220 fixed MOD_INCLUDE
// hh 971223 added alignment wrapper
// hh 971223 added header <iterator>
// dwa, hh 980306 added qualifier to iterator in two places
// hh 980306 removed unused argument in two places
// hh 980713 Temporarily moved member templates into class definition to support compiler
// hh 980902 #ifdef'd out exception code when ndef MSIPL_EXCEPT
// hh 990629 Rewrote
