/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:31 $ 
 *  $Revision: 1.8 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

#include <ansi_parms.h>  /* m.f. 042898 */

#if !defined(__dest_os)               
	#error __dest_os undefined                                               /*MW-mm 960927a*/

#elif __dest_os == __mac_os
	#include <unistd.mac.h>

#elif ( __dest_os == __win32_os || __dest_os == __wince_os)
	#include <unistd.win32.h>

#endif

/*     Change record
 * MW-mm 960927a Inserted setting of __dest_os to __mac_os when not otherwise set.
 * mf 980428  included ansi_parms.h and use #error instead of mac_os by default
 * vss 990115 removed powerTV
*/