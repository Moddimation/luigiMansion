/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:57:44 $ 
 *  $Revision: 1.2.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*	This file contains the code that gets precompiled.	*/

#if macintosh
	/*	option 1
	 *	to have ansi_prefix not include MacHeaders set MSL_USE_PRECOMPILED_HEADERS to something other than 0 or 1
	 *	uncommenting the following line will achieve this.
	 */

	/*	#define MSL_USE_PRECOMPILED_HEADERS 2 */

	/* 	option 2
	 *	to generate MacHeaders as part of the MSL pch leave everything as is.
	 */

	#ifndef MSL_USE_PRECOMPILED_HEADERS
	#define MSL_USE_PRECOMPILED_HEADERS 1	
	#endif
	
	#include <ansi_prefix.mac.h>

#elif __INTEL__ && !__BEOS__

	#include <ansi_prefix.win32.h>

#elif __PPC_EABI__

	/* prefix file is in Language preference panel */
	
#else
	#error "OS currently unsupported"
#endif

#include <iosfwd>
#if 0
// Support
#include <exception>
#include <new>
#include <limits>
#include <typeinfo>
// Diagnostics
#include <stdexcept>
// Iterators
#include <iterator>
// Utilities
#include <functional>
#include <memory>
#include <utility>
// Algorithms
#include <algorithm>
// Strings
#include <string>
// Containers
#include <bitset>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <stack>
// Localization
#include <locale>
// Input/Output
#include <ios>
#include <streambuf>
#include <istream>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
// Numerics
#include <complex>
#include <numeric>
#include <valarray>

//#include <algobase.h>
//#include <extbasic.h>
//#include <extfunc.h>
//#include <extmath.h>
//#include <hashfun.h>
//#include <hashmap.h>
//#include <hashmmap.h>
//#include <hashmset.h>
//#include <hashset.h>
//#include <hashtbl.h>
//#include <heap.h>
//#include <mtools.h>
//#include <mutex.h>
//#include <slist.h>
//#include <tree.h>
#endif