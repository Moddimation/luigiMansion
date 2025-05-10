/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:31 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

#include <ansi_parms.h>     /* hh  971206  every file needs one of these*/

#if macintosh && !defined(__dest_os)               /*MW-mm 960927a*/
  #define __dest_os __mac_os                       /*MW-mm 960927a*/
#endif                                             /*MW-mm 960927a*/

#if __dest_os == __mac_os

#include <unix.mac.h>

#elif (__dest_os == __win32_os || __dest_os == __wince_os)
#include <unix.win32.h>

#endif

/*     Change record
//MW-mm 960927a Inserted setting of __dest_os to __mac_os when not otherwise set.
 hh  971206  added #include <ansi_parms.h>
*/
