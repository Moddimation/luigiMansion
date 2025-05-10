/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:58:23 $ 
 *  $Revision: 1.1.6.1 $ 
 *  $NoKeywords: $ 
 *
 *		Portions Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/**
 **  mutex.cpp
 **/

#include <mslconfig>

#ifdef _MSL_MULTITHREAD

	#if __dest_os == __win32_os

		#pragma ANSI_strict off

		#include <WINDEF.H>
		#include <WINBASE.H>

		#pragma ANSI_strict reset

		namespace {

		template <bool b>
		class __compile_assert;
		
		template <>
		class __compile_assert<true>
		{
		};
		
		__compile_assert<sizeof(CRITICAL_SECTION) == 24> check_CRITICAL_SECTION;

		// If you get an error with this object, that means that
		// sizeof(CRITICAL_SECTION) != 24.  You need to find out
		// what the new size of CRITICAL_SECTION is, change the
		// number here, and then go to the windows section of
		// mutex.h.  There is a declaration in the mutex class:
		// long m_[6];  You need to adjust this number so that
		// sizeof(long)*x == sizeof(CRITICAL_SECTION).
		//
		// Note:  The reason for this elaborate song and dance is
		// because including windows headers in our own tends to
		// irritate customers.
		//
		// hh 990520

		}

	#endif

#endif
