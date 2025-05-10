/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/09 17:59:44 $ 
 *  $Revision: 1.11.6.1 $ 
 *  $NoKeywords: $ 
 *
 *		Portions Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/**
 **  mutex.h
 **/

#ifndef _MUTEX_H
#define _MUTEX_H

#include <mslconfig>
#include <new>

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifdef _MSL_MULTITHREAD

	#ifdef _USE_PTHREADS

		#include <pthread.h>

		#ifndef _MSL_NO_CPP_NAMESPACE
			namespace std {
		#endif

		class mutex
		{
		public:
			mutex()
			{
			#ifndef _MSL_NO_EXCEPTIONS
				if (pthread_mutex_init(&m_, 0))
					throw bad_alloc();
			#else
				pthread_mutex_init(&m_, 0);
			#endif
			}
			~mutex() {pthread_mutex_destroy(&m_);}
			void lock() {pthread_mutex_lock(&m_);}
			void unlock() {pthread_mutex_unlock(&m_);}
		private:
			pthread_mutex_t m_;

			mutex(const mutex&);             // Not defined
			mutex& operator=(const mutex&);  // Not defined
		};
	
	#elif __dest_os == __win32_os

		struct _RTL_CRITICAL_SECTION;
		typedef _RTL_CRITICAL_SECTION CRITICAL_SECTION;
		
		extern "C" void __stdcall InitializeCriticalSection(CRITICAL_SECTION*);
		extern "C" void __stdcall DeleteCriticalSection(CRITICAL_SECTION*);
		extern "C" void __stdcall EnterCriticalSection(CRITICAL_SECTION*);
		extern "C" void __stdcall LeaveCriticalSection(CRITICAL_SECTION*);

		#ifndef _MSL_NO_CPP_NAMESPACE
			namespace std {
		#endif

		class mutex
		{
		public:
			mutex() {InitializeCriticalSection((CRITICAL_SECTION*)&m_);}
			~mutex() {DeleteCriticalSection((CRITICAL_SECTION*)&m_);}
			void lock() {EnterCriticalSection((CRITICAL_SECTION*)&m_);}
			void unlock() {LeaveCriticalSection((CRITICAL_SECTION*)&m_);}
		private:
			long m_[6];

			mutex(const mutex&);             // Not defined
			mutex& operator=(const mutex&);  // Not defined
		};

	#elif __dest_os == __mac_os

		typedef struct OpaqueMPCriticalRegionID*  MPCriticalRegionID;

		extern "C" long MPCreateCriticalRegion(MPCriticalRegionID*);
		extern "C" long MPDeleteCriticalRegion(MPCriticalRegionID);
		extern "C" long MPEnterCriticalRegion(MPCriticalRegionID, long);
		extern "C" long MPExitCriticalRegion(MPCriticalRegionID);

		#ifndef _MSL_NO_CPP_NAMESPACE
			namespace std {
		#endif

		class mutex
		{
		public:
			mutex() {MPCreateCriticalRegion(&m_);}
			~mutex() {MPDeleteCriticalRegion(m_);}
			void lock() {MPEnterCriticalRegion(m_, 0x7FFFFFFF);}
			void unlock() {MPExitCriticalRegion(m_);}
		private:
			MPCriticalRegionID m_;

			mutex(const mutex&);             // Not defined
			mutex& operator=(const mutex&);  // Not defined
		};

	#elif __dest_os == __eppc_vxworks
	
		#include <semLib.h>

		#ifndef _MSL_NO_CPP_NAMESPACE
			namespace std {
		#endif

		class mutex
		{
		public:
			mutex()
				: m_(semBCreate(SEM_Q_PRIORITY, SEM_FULL))
			{
			#ifndef _MSL_NO_EXCEPTIONS
				if (m_ == 0)
					throw bad_alloc();
			#endif
			}
			~mutex() {semDelete(m_);}
			void lock() {semTake(m_, WAIT_FOREVER);}
			void unlock() {semGive(m_);}
		private:
			SEM_ID m_;

			mutex(const mutex&);             // Not defined
			mutex& operator=(const mutex&);  // Not defined
		};

	#elif __dest_os == __be_os

		#include <OS.h>
		#include <be_rw_lock.h>

		#ifndef _MSL_NO_CPP_NAMESPACE
			namespace std {
		#endif

		class mutex
		{
		public:
			mutex() : m_(0) {}
			~mutex() {m_ = 0;}
			void lock() {acquire_spinlock(&m_);}
			void unlock() {release_spinlock(&m_);}
		private:
			long m_;

			mutex(const mutex&);             // Not defined
			mutex& operator=(const mutex&);  // Not defined
		};
	
	#else

		#error OS not supported in mutex.h

	#endif

	class mutex_lock
	{
	public:
		explicit mutex_lock(mutex& m) : m_(m) {m_.lock();}
		~mutex_lock() {m_.unlock();}
	private:
		mutex& m_;

		mutex_lock(const mutex_lock&);             // Not defined
		mutex_lock& operator=(const mutex_lock&);  // Not defined
	};

	#define _MSL_DECLARE_MUTEX(mut) mutex mut;
	#define _MSL_LOCK_MUTEX(lock, mut) mutex_lock lock(mut);

	#ifndef _MSL_NO_CPP_NAMESPACE
		} // namespace std 
	#endif

#else  // !_MSL_MULTITHREAD

	#ifndef _MSL_NO_CPP_NAMESPACE
		namespace std {
	#endif

	class mutex
	{
	public:
		mutex() {}
		void lock() {}
		void unlock() {}
	private:

		mutex(const mutex&);             // Not defined
		mutex& operator=(const mutex&);  // Not defined
	};

	#ifndef _MSL_NO_CPP_NAMESPACE
		}
	#endif

	#define _MSL_DECLARE_MUTEX(mut)
	#define _MSL_LOCK_MUTEX(lock, mut)

#endif // _MSL_MULTITHREAD

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif // _MUTEX_H

// hh 971220 fixed MOD_INCLUDE
// hh 971222 added alignment wrapper
// bs 990121 added BeOS changes
// hh 990318 Rewrote
