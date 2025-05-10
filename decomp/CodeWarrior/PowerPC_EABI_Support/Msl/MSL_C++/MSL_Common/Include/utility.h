/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:59:47 $ 
 *  $Revision: 1.9.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	utility.h       // hh 971207 Changed filename from utility to utility.h
 */

#ifndef  _UTILITY_H           // hh 971207 added standard include guards
#define  _UTILITY_H

#include <utility>

#ifndef _MSL_NO_CPP_NAMESPACE     // hh 971207 Added backward compatibility
	using namespace std;
	using namespace std::rel_ops;
	namespace std { using namespace rel_ops; }
#endif

#endif
/*     Change record
// hh 971207 Changed filename from utility to utility.h
// hh 971207 added standard include guards
// hh 971207 Added backward compatibility
// hh 990120 changed name of MSIPL flags
*/
