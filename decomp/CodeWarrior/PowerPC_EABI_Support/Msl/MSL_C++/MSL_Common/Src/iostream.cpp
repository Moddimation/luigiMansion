/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:58:23 $ 
 *  $Revision: 1.8.6.1 $ 
 *  $NoKeywords: $ 
 *
 *		Portions Copyright 1995-1998 Metrowerks, Inc.
 *		All rights reserved.
 */

/**
 ** iostream.cpp
 **/

#include <mslconfig>

#ifndef _MSL_NO_IO

#include <cstdio>
#include <ios>
#include <istream>
#include <ostream>
#include <fstream>
#include <mutex.h>

#ifndef _MSL_NO_CPP_NAMESPACE
	namespace std {
#endif

int ios_base::index_;
int ios_base::Init::init_cnt_s;

#ifdef __MWERKS__
	#pragma suppress_init_code on
#endif

istream cin;
ostream cout;
ostream clog;
ostream cerr;

#ifndef _MSL_NO_WCHART
	wistream wcin;
	wostream wcout;
	wostream wclog;
	wostream wcerr;
#endif

#ifdef __MWERKS__
	#pragma suppress_init_code off
#endif

#ifdef _MSL_MULTITHREAD

	namespace
	{

		mutex&
		get_Init_lock()
		{
			static mutex lock;
			return lock;
		}

	}

#endif

ios_base::Init::Init ()
{
	_MSL_LOCK_MUTEX(lock, get_Init_lock())
	static filebuf fin(stdin);
	static filebuf fout(stdout);
	static filebuf ferr(stderr);

#ifndef _MSL_NO_WCHART
	static wfilebuf wfin(stdin);
	static wfilebuf wfout(stdout);
	static wfilebuf wferr(stderr);
#endif

	if (init_cnt_s++ == 0)
	{
		new (&cin)istream(&fin);
		new (&cout)ostream(&fout);
		new (&cerr)ostream(&ferr);
		new (&clog)ostream(&ferr);
		cin.tie(&cout);
		cerr.tie(&cout);
		cerr.setf(ios_base::unitbuf);
		clog.tie(&cout);

	#ifndef _MSL_NO_WCHART
		new (&wcin)wistream(&wfin);
		new (&wcout)wostream(&wfout);
		new (&wcerr)wostream(&wferr);
		new (&wclog)wostream(&wferr);
		wcin.tie(&wcout);
		wcerr.tie(&wcout);
		wcerr.setf(ios_base::unitbuf);
		wclog.tie(&wcout);
	#endif
	}
}
 
ios_base::Init::~Init ()
{
	_MSL_LOCK_MUTEX(lock, get_Init_lock())
	if (--init_cnt_s == 0)
	{
		cout.flush();
		cerr.flush();
		clog.flush();
	#ifdef __MWERKS__
		__destroy(&cin);
		__destroy(&cout);
		__destroy(&cerr);
		__destroy(&clog);
	#endif

	#ifndef _MSL_NO_WCHART
		wcout.flush();
		wcerr.flush();
		wclog.flush();
	#ifdef __MWERKS__
		__destroy(&wcin);
		__destroy(&wcout);
		__destroy(&wcerr);
		__destroy(&wclog);
	#endif
	#endif
	}
}

#ifndef _MSL_NO_CPP_NAMESPACE
	} // namespace std
#endif

#endif // _MSL_NO_IO

// hh 971220 fixed MOD_INCLUDE
// hh 980112 Modifications to prevent memory leaks
// hh 981122 Replace #include <iostream> with <istream> and <ostream>
// hh 981124 Added code to insure that the standard streams are constructed and destructed
//           only within ios_base::Init
// hh 990617 Rewrote.
