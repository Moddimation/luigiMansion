/*  Metrowerks Standard Library  Version 2.1.2  1997 May  */

/*
 *	pool_alloc.stub.c
 *	
 *		Copyright 1995-1997 Metrowerks, Inc.
 *		All rights reserved.
 *	
 *	Routines
 *	--------
 *		__sys_alloc
 *		__sys_free
 *
 *	Note
 *	----
 *
 *		These routines, being platform-specific, were split off of pool_alloc.c.
 *	
 *
 */

// Include your OS dependent include files
#include "pool_alloc.h"

#ifdef _No_Alloc_OS_Support
#error pool_alloc.stub.c can't be in your project if _No_Alloc_OS_Support is defined.
#endif

void * __sys_alloc(__std(size_t) size)
{
	return 0; // replace 0 with a call to your OS'es allocator
}

void __sys_free(void *ptr)
{
	// replace 0 with a call to your OS'es deallocator
}
/*  Change Record
 *	20-Jul-97 MEA  First code release.
*/
