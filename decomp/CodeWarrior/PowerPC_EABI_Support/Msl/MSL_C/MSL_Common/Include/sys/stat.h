/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:32 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

#include <ansi_parms.h>  /* m.f. 042898 */

#if !defined(__dest_os)               
  #error __dest_os undefined 
#endif                                             /*MW-mm 960927a*/

#if __dest_os == __mac_os
#include <stat.mac.h>

#elif (__dest_os == __win32_os || __dest_os == __wince_os)
#include <stat.win32.h>

#elif __dest_os == __powertv_os
#include <stat.powertv.h>

#endif

/*     Change record
//MW-mm 960927a Inserted setting of __dest_os to __mac_os when not otherwise set.
*/
