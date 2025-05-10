/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:58:23 $ 
 *  $Revision: 1.8.6.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/************************************************************************/
/*	Project...:	Standard C++ Library									*/
/*	Name......:	MSLstring.cpp											*/
/*	Purpose...:	exception string handling								*/
/************************************************************************/

#include <MSLstring.h>

#ifndef _MSL_NO_EXCEPTIONS

#include <string>
#include <cstring>

_STD::_MSLstring::_MSLstring(const string& value)
	: data_(new char [value.size()+1])
{
	strcpy(const_cast<char*>(static_cast<const char*>(data_)), value.c_str());
}

#endif // _MSL_NO_EXCEPTIONS

// hh 990120 changed name of MSIPL flags
// hh 990314 Added const char* constructor to support nonstandard const char* constructors
//           on all of the standard exceptions.
