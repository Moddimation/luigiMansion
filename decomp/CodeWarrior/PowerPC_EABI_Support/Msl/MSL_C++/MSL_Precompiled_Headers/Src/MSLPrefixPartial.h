/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:57:44 $ 
 *  $Revision: 1.1.8.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*	This file contains the code that does not get precompiled. */

#ifdef __cplusplus  /* 971006 vss */

//	iostream.h line 58
// 971025  HH  commented out following line.  This line is now in iostream.cpp
//static ios_base::Init  __msipl_ios_init;

//	slist line 474
//	970420 bkoz you will need this also if slist.h is in your .pch
/*
#ifdef MSIPL_MULTITHREAD
	template <class T, class Allocator>
	mutex_arith<bool, mutex> slist<T, Allocator>::separate_copying = false;
#else
	template <class T, class Allocator>
	bool slist<T, Allocator>::separate_copying = false;
#endif
*/

//	bstring line 1301
template <class charT, class traits, class Allocator>
const typename basic_string<charT, traits, Allocator>::size_type
basic_string<charT, traits, Allocator>::npos = (size_t) -1;

#endif  /* ifdef __cplusplus  */

/*  971006 vss  guarantee inclusion in C++ headers only */


