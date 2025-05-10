/*  Metrowerks Standard Library  */

/*  $Date: 2000/10/30 22:24:02 $ 
 *  $Revision: 1.7.4.1.2.1.2.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	os_enum.h
*/

#ifndef os_enum_h
#define os_enum_h

#define __undef_os		 0
#define __mac_os		 1
#define __be_os			 2
#define __win32_os		 3
/* #define __powertv_os	 4 */		/* vss 990121 */
#define __wince_os		 5
#define __ppc_eabi		 6
#define __nec_eabi_bare	 7
#define __nec_eabi_os	 8
#define __mips_bare		 9
#define __m56800_os		10          /* mm 981023 */
#define __n64_os		11			/* ad 1.28.99 */
#define __mcore_bare	12			/* vss 990624  */
#define __eppc_vxworks	13			/* mea 990810  */
#define __dolphin_os	14			/* sw 08.21.00 */

#endif

/* Change Records
 * mm  981023  Added entry for __m56800_os
 * vss 990121 Removed powerTV
 * ad 1.28.99 added nintendo support
 * mea 990810 Added entry for __eppc_vxworks
 */
