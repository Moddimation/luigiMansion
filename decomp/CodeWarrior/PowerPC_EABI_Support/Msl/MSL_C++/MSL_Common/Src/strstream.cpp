/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:58:23 $ 
 *  $Revision: 1.6.6.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/************************************************************************/
/*	Project...:	Standard C++ Library									*/
/*	Name......:	strstream.cpp											*/
/*	Purpose...:	<strstream> support										*/
/************************************************************************/

#include <strstream>

#ifndef _MSL_NO_IO

#ifndef _MSL_NO_CPP_NAMESPACE
	namespace std {
#endif

// strstreambuf

strstreambuf::~strstreambuf()
{
	if ((strmode_ & allocated) != 0 && (strmode_ & frozen) == 0)  // hh 990422
	{
		if (pfree_ != 0)
			pfree_(eback());
		else
			delete [] eback();
	}
}

void
strstreambuf::freeze(bool freezefl)
{
	if ((strmode_ & dynamic) != 0)  // hh 990422
	{
		if (freezefl)
			strmode_ |= frozen;
		else
			strmode_ &= (strstate)~frozen;
	}
}

strstreambuf::int_type
strstreambuf::overflow (int_type c)
{
	if (c == EOF)
		return 0;
	if (pptr() < epptr())
	{
		*pptr() = (char)c;
		pbump(1);
		return c;
	}
	if ((strmode_ & dynamic) == 0 || (strmode_ & frozen) != 0)  // hh 990422
		return EOF;
	streamsize getlen = egptr() - eback();
	streamsize getpos = gptr() - eback();
	streamsize putstart = pbase() - eback(); // hh 990423
	streamsize putpos = pptr() - pbase();
	streamsize oldsize = epptr() - eback();  // hh 990423
	streamsize newsize;
	if (oldsize < alsize_)
		newsize = alsize_;
	else
		newsize = 2 * oldsize;
	char* newbuffer;    // hh 990423
	if (palloc_ != 0)
		newbuffer = (char*)palloc_((size_t)newsize);
	else
		newbuffer = new(nothrow) char [newsize];
	if (newbuffer == 0)
		return EOF;
	strmode_ |= allocated;
	if (oldsize != 0)
	{
		char* p = newbuffer;
		char* g = eback();
		for (streamsize i = 0; i < oldsize; ++i)
			*p++ = *g++;
		if (pfree_ != 0)
			pfree_(eback());
		else
			delete [] eback();
	}
	setg(newbuffer, newbuffer + getpos, newbuffer + getlen);
	setp(newbuffer + putstart, newbuffer + newsize);  // hh 980910, 990423
	pbump(putpos);
	*pptr() = (char)c;
	pbump(1);
	return c;
}

strstreambuf::int_type
strstreambuf::pbackfail(int_type c)
{
	if (gptr() <= eback())
		return EOF;
	gbump(-1);
	if (c == EOF)
		return 0;
	if ((char)c == *gptr())
		return c;
	if ((strmode_ & constant) == 0)  // hh 990422
	{
		*gptr() = (char)c;
		return c;
	}
	gbump(1);
	return EOF;
}

strstreambuf::int_type
strstreambuf::underflow()
{
	if (gptr() < egptr())
		return (unsigned char)*gptr();
	if (egptr() < pptr())
	{
		setg(eback(), gptr(), pptr());  // hh 990423
		return (unsigned char)*gptr();
	}
	return EOF;
}

strstreambuf::pos_type
strstreambuf::seekoff(off_type off, ios_base::seekdir way, ios_base::openmode which)
{
	ios_base::openmode inout = ios_base::in | ios_base::out;  // hh 990423
	if ((which & inout) == inout && way == ios_base::cur || (which & inout) == 0)
		return pos_type(-1);
	off_type newoff = -1;
	if (which & ios_base::in)
	{
		switch (way)
		{
		case ios_base::beg:
			newoff = 0;
			break;
		case ios_base::cur:
			newoff = gptr() - eback();
			break;
		case ios_base::end:
			newoff = egptr() - eback();
			break;
		default:
			return pos_type(-1);
		}
		newoff += off;
		if (newoff < 0 || newoff > egptr() - eback())
			return pos_type(-1);
		setg(eback(), eback()+newoff, egptr());
	}
	if (which & ios_base::out)
	{
		switch (way)
		{
		case ios_base::beg:
			newoff = 0;
			break;
		case ios_base::cur:
			newoff = pptr() - pbase();
			break;
		case ios_base::end:
			newoff = epptr() - pbase();
			break;
		default:
			return pos_type(-1);
		}
		newoff += off;
		if (newoff < 0 || newoff > epptr() - pbase())
			return pos_type(-1);
		setp(pbase(), epptr());
		pbump(newoff);
	}
	return pos_type(newoff);
}

strstreambuf::pos_type
strstreambuf::seekpos(pos_type sp, ios_base::openmode which)
{
	off_type newoff = -1;
	if (which & ios_base::in)
	{
		newoff = sp;  // hh 980909
		if (newoff < 0 || newoff > egptr() - eback())
			return pos_type(-1);
		setg(eback(), eback()+newoff, egptr());
	}
	if (which & ios_base::out)
	{
		newoff = sp;  // hh 980909
		if (newoff < 0 || newoff > epptr() - pbase())
			return pos_type(-1);
		setp(pbase(), epptr());
		pbump(newoff);
	}
	return pos_type(newoff);
}

streambuf*
strstreambuf::setbuf(char* s, streamsize n)
{	// hh 990423
	if (s == 0 || n == 0)
		return 0;
	if ((strmode_ & allocated) != 0 && (strmode_ & frozen) == 0)
	{
		if (pfree_ != 0)
			pfree_(eback());
		else
			delete [] eback();
	}
	strmode_ = 0;
	init(s, n, s);
	return this;
}

void
strstreambuf::init(char* gnext_arg, streamsize n, char* pbeg_arg)
{
	streamsize N = n;
	if (N == 0)
		N = (streamsize)strlen(gnext_arg);
	else if (N < 0)
		N = INT_MAX;
	if (pbeg_arg == 0)
		setg(gnext_arg, gnext_arg, gnext_arg + N);
	else
	{
		setg(gnext_arg, gnext_arg, pbeg_arg);
		setp(pbeg_arg, pbeg_arg + N);
	}
}

// istrstream

// ostrstream

#ifndef _MSL_NO_CPP_NAMESPACE
	}
#endif

#endif // _MSL_NO_IO

// hh 980910 Fixed bug in strstreambuf::overflow, pend was being incorrectly set
// hh 990120 changed name of MSIPL flags
// hh 990422 Put parenthesis around (strmode_ & X) 6 places
// hh 990423 Corrected several mistakes concerning the placement and movement of
//           the get and put areas during underflow and overflow.
// hh 990423 Filled out setbuf
