/****************************************************************************/
/*

FILE
	__mem.h
	
DESCRIPTION
	
	Standalone Standard C Library Implementation for memory operations.
	This is a target-specific implementation for PowerPC.  It will
	probably work for other target processors as well, but this one
	has been improved specifically for PowerPC.
	
	memcpy and memset are  put into the .init section so that they will
	be available for use in copying sections (e.g. .text) from rom into ram.
	
	Derived from MSL C (CodeWarrior 10).
	
	__memcpy
	__memset

COPYRIGHT

	(c) 1996-7 Metrowerks Corporation
	All rights reserved.
	
*/
/****************************************************************************/

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma section code_type ".init"

void * memcpy(void * dst, const void * src, size_t n);
void * memset(void * dst, int val, size_t n);
void __fill_mem(void * dst, int val, unsigned long n);

#pragma section code_type

#ifdef __cplusplus
}
#endif
