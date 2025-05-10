/*  Metrowerks Standard Library  Version 2.1.2b6  1997 April  */

/*
 *	critical_regions.mac.c
 *	
 *		Copyright 1995-1996 Metrowerks, Inc.
 *		All rights reserved.
 *	
 *	Routines
 *	--------
 *		__init_critical_regions
 *		__begin_critical_region
 *		__end_critical_region
 *
 *	Implementation
 *	--------------
 *
 *		For the Mac, these routines are mere stubs. The Thread Manager doesn't
 *		support preemptive threads and the ANSI library code will never yield
 *		cooperatively - ever, much less during critical regions. Interrupt-level
 *		exclusion is not addressed because it is too fraught with the danger of
 *		deadlock.
 *
 *
 */

#include "critical_regions.h"

void __init_critical_regions(void)
{
}

void __kill_critical_regions(void)
{
}

#if _MWMT
void __begin_critical_region(int region)
{
#pragma unused(region)
}

void __end_critical_region(int region)
{
#pragma unused(region)
}
#endif

/*  Change Record
 *	16-Oct-95 JFH  First code release.
 *	990811	mea added _MWMT condition
*/