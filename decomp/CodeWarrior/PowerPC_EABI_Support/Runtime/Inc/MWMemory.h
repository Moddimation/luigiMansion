/*
 *  MWMemory.h	-	MW runtime suport definitions
 *
 *  Copyright 1995-1999 Metrowerks, Inc.  All rights reserved.
 *
 */

#ifndef __MWMEMORY_H__
#define __MWMEMORY_H__

typedef char *Ptr;
typedef Ptr *Handle;
typedef int OSErr;

extern pascal Handle NewHandle(long dataSize);
extern pascal void DisposeHandle(Handle h);

#endif /* __MWMEMORY_H__ */
