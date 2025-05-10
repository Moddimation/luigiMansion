/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/04 00:02:01 $
 *  $Revision: 1.12.4.1.2.1 $
 *  $NoKeywords: $
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	ansi_parms.h
 */

#ifndef __ansi_parms__
#define __ansi_parms__

#include <mslGlobals.h> /* hh 980120 added */

/* rjk 980313 added the _MSL_DLLDATA macro
   This macro is set to NULL for targets that link to MSL statically
   and to __declspec(dllimport) for targets that link to MSL in a DLL
   Here is it defaulted to NULL if undefined.
*/

#ifndef _MSL_DLLDATA
#define _MSL_DLLDATA
#endif

#define __MODENALIB__ /*soon to be obsolete...*/

/*  Even though the __MSL__ identifier is a hex value, it is tracking a
 *  more or less decimal identity - that is, for builds 10 - 16, we will
 *  use a decimal appearing number - e.g., 0x4010 means the major Pro
 *  release name as in Pro4, the second digit is the patch level, and
 *  the last two digits refer to the build number within that release.
 *  Hence, the last two digits of 10 mean build 10, and 20 means 20,
 *  and so on.  The identifier, although increasing, is not monatomical
 *  for each build.
 */

/*  990727 vss  MSL 5.1.00 Pro 5, patch 1, Build 00  */
#define __MSL__       0x5100

/*	970415 bkoz
    define this if you would like FPCE functionality in math.h
*/
/* #define __MSL_C9X__ */

#ifdef __cplusplus

#define __extern_c          /*hh 971206	war on macros		extern "C" {*/
#define __end_extern_c      /*		}*/

#ifdef _MSL_USING_NAMESPACE /*hh 971206 need these (bad day for the war on macros)*/
#define __std(ref) std::ref
#define __global() ::
#else
#define __std(ref) ref
#define __global()
#endif

#define __namespace(space)                  /*namespace space {*/
#define __end_namespace(space)              /*}*/

#define __using_namespace(space)            /*using namespace space;*/

#define __stdc_space(space)                 /*__c ## space ## _space*/

#define __import_stdc_into_std_space(space) /*                                                 \
                                            __namespace(std)                                   \
                                            __using_namespace(__stdc_space(space))             \
                                            __end_namespace(std)					*/

#else

#define __extern_c
#define __end_extern_c

#define __std(ref) ref
#define __global()

#define __namespace(space)
#define __end_namespace(space)

#define __using_namespace(space)

#define __stdc_space(space)

#define __import_stdc_into_std_space(space)

#endif                                      /* __cplusplus */

#if (__dest_os == __win32_os || __dest_os == __wince_os)
#define __tls __declspec (thread)
#else
#define __tls
#endif

#endif                                      /* ndef __ansi_parms__ */

/*     Change record
 *	23-Jun-97 MEA  	Added __dest_os __ppc_eabi_bare.  New symbol __no_os
 *					is only defined for bare board embedded systems.
 *					Do not define long long yet for PPC EABI.
 *	15-Jul-97 SCM	Added __nec_eabi_bare and __nec_eabi_os.
 *	20-Jul-97 MEA   Changed __ppc_eabi_bare to __ppc_eabi.
 *	11-Sep-97 MEA   Added support for long long.
 *vss 971015 New version 2.2
 *hh  971206 reworked macro magic for namespace support
 *hh  971206 Added "define OS" code
 *hh  980120 added <mslGlobals.h>
 *hh  980217 added __ANSI_OVERLOAD__
 *rjk 909313 ADDED _MSL_DLLDATA macro
 */
