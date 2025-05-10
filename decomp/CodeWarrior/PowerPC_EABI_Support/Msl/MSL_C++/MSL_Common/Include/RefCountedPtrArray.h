/*  Metrowerks Standard Library  Version 4.0  1998 August 10  */

/*  $Date: 1999/12/09 17:59:45 $ 
 *  $Revision: 1.10.6.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

// RefCountedPtr.h

#ifndef _REFCOUNTEDPTR_H
#define _REFCOUNTEDPTR_H

/*
	WARNING:  This is a non-standard header and class.

	A reference counting handle class.
	
	This is to be used as a pointer to class T.  
	This will feel and smell just like a built-in pointer except:
	1.  There is no need to call delete on the pointer.
	2.  The default copy constructor and assignment implement ref-counting.
	3.  The user may call isNonUnique to determine if this pointer is
	    the only pointer to the data.  This can be used to hide the
	    ref-counting behavior of a class.
	4.  There are two "traits" classes which specify whether the single object
	    form of delete should be used, or the array form.  The default is single.
*/

#include <mslconfig>
#include <new>

#ifndef RC_INVOKED

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifndef _MSL_NO_CPP_NAMESPACE
	namespace std {
#endif

template <class T>
struct _Single
{
	static void destroy(T* ptr) {delete ptr;}
};

template <class T>
struct _Array
{
	static void destroy(T* ptr) {delete [] ptr;}
};

template <class T, class traits = _Single<T> >
class _RefCountedPtr
{
public:
	_RefCountedPtr(T* ptr = 0);  // Accepts responsibility for ptr
	_RefCountedPtr(const _RefCountedPtr<T, traits>& other);
	~_RefCountedPtr();
	_RefCountedPtr<T, traits>& operator=(const _RefCountedPtr<T, traits>& rhs);
	T* operator->();
	const T* operator->() const;
	T& operator*();
	const T& operator*() const;
	operator const T* () const;
	bool isNonUnique() const;
protected:
	T* ptr_;
	int* refCount_;
private:
};

template <class T, class traits>
_RefCountedPtr<T, traits>::_RefCountedPtr(T* ptr)
	: ptr_(ptr),
	refCount_(0)
{
	#ifndef _MSL_NO_EXCEPTIONS
		try
		{
	#endif
		if (ptr_ != 0)
			refCount_ = new int(1);
	#ifndef _MSL_NO_EXCEPTIONS
		}
		catch (...)
		{
			traits::destroy(ptr_);
			throw;
		}
	#else
		if (refCount_ == 0)
		{
			traits::destroy(ptr_);
			__msl_error("_RefCountedPtr out of memory");
		}
	#endif
}

template <class T, class traits>
_RefCountedPtr<T, traits>::_RefCountedPtr(const _RefCountedPtr<T, traits>& other)
	: ptr_(other.ptr_),
	refCount_(other.refCount_)
{
	if (refCount_ != 0)
		++(*refCount_);
}

template <class T, class traits>
_RefCountedPtr<T, traits>::~_RefCountedPtr()
{
	if (refCount_ != 0 && --(*refCount_) == 0)
	{
		traits::destroy(ptr_);
		delete refCount_;
	}
}

template <class T, class traits>
_RefCountedPtr<T, traits>&
_RefCountedPtr<T, traits>::operator=(const _RefCountedPtr<T, traits>& rhs)
{
	if (ptr_ != rhs.ptr_)
	{
		if (refCount_ != 0 && --(*refCount_) == 0)
		{
			traits::destroy(ptr_);
			delete refCount_;
		}
		ptr_ = rhs.ptr_;
		refCount_ = rhs.refCount_;
		if (refCount_ != 0)
			++(*refCount_);
	}
	return *this;
}

template <class T, class traits>
inline
T*
_RefCountedPtr<T, traits>::operator->()
{
	return ptr_;
}

template <class T, class traits>
inline
const T*
_RefCountedPtr<T, traits>::operator->() const
{
	return ptr_;
}

template <class T, class traits>
inline
T&
_RefCountedPtr<T, traits>::operator*()
{
	return *ptr_;
}

template <class T, class traits>
inline
const T&
_RefCountedPtr<T, traits>::operator*() const
{
	return *ptr_;
}

template <class T, class traits>
inline
_RefCountedPtr<T, traits>::operator const T* () const
{
	return ptr_;
}

template <class T, class traits>
inline
bool
_RefCountedPtr<T, traits>::isNonUnique() const
{
	if (refCount_ == 0)
		return false;
	return *refCount_ != 1;
}

#ifndef _MSL_NO_CPP_NAMESPACE
	} // namespace std
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif // RC_INVOKED

#endif // _REFCOUNTEDPTR_H

// hh 980804 changed __std() to _STD::
// hh 990120 changed name of MSIPL flags
