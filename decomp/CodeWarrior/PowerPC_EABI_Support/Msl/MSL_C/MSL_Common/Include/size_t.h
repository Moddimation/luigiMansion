/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 00:46:32 $
 *  $Revision: 1.8.4.1 $
 *  $NoKeywords: $
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	size_t.h
 */

#ifndef __size_t__
#define __size_t__

#include <ansi_parms.h>                                                   /* mm 970905*/

#ifdef n__cplusplus                                                       /* hh  971206 */
#ifdef _MSL_USING_NAMESPACE
namespace std
{
#endif
extern "C"
{
#endif

#if __dest_os == __win32_os || __MOTO__ || __dest_os == __wince_os || defined(__m56800__) ||   \
    __MIPS__ ||                                                                                \
    __dest_os == __n64_os /* mm 981014 */ /* mm 981020 */ /* mm 981029 */ /*ad 1.28.99 */
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#else
typedef unsigned long size_t;
#if __dest_os == __be_os                                                  /*  bs 990121  */
typedef long ssize_t;
#endif
#endif

#ifdef n__cplusplus                                                       /* hh  971206 */
}
#ifdef _MSL_USING_NAMESPACE
}
#endif
#endif

#endif                                                                    /* __size_t__ */

/*     Change record
 * mm  970905   added include of ansi_parms.h to avoid need for prefix file
 * hh  971206   added namespace support.
 * mf  980507   added typedef of size_t for wince
 * mm  981014   added typedef of size_t for __DSP568
 * mm  981029   Changed __DSP568 to __m56800__
 * bs  990121   Added BEOS changes
 * ad 1.28.99 added __dest_os == __n64_os
 * vss 990224   MrC needs uint for size_t
 */

