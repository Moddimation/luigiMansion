/*  Metrowerks Standard Library  */

/*  $Date: 2000/10/30 22:24:03 $ 
 *  $Revision: 1.7.4.1.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	va_list.h
 */
 
#ifndef __va_list__
#define __va_list__

#include <ansi_parms.h>                 /* mm 970905*/

#ifdef __cplusplus              /* hh  971206  */
	#ifdef _MSL_USING_NAMESPACE
		namespace std {
	#endif
	extern "C" {
#endif

#if __dest_os == __ppc_eabi  || __dest_os ==__dolphin_os
typedef struct {
	char gpr;
	char fpr;
	char reserved[2];
	char *input_arg_area;
	char *reg_save_area;
} __va_list[1];
typedef __va_list va_list;

#elif __MIPS__

	#if __n64abi__
		/* Do not change the layout of this struct.	*/
		/* It is known to the compiler.				*/
		typedef struct {
			char gpr;	
			char fpr;
			char reserved[2];
			char *mem_ptr;
			char *fpreg_ptr;
			char *gpreg_ptr;
		} __va_list;
		
		typedef __va_list va_list;

	#else
		typedef char * va_list;
	#endif	

#else

typedef char * va_list;

#endif

#ifdef __cplusplus             /* hh  971206  */
	}
	#ifdef _MSL_USING_NAMESPACE
		}
	#endif
#endif

#endif /* __va_list__ */

/*     Change record
 *	23-Jun-97 MEA  Merged PPC EABI changes in.
 *	20-Jul-97 MEA  Changed __ppc_eabi_bare to __ppc_eabi.
 * mm 970905  added include of ansi_parms.h to avoid need for prefix file
 * hh 971206  added namespace support.  Is the extern "C" necessary?  Probably not.
 */
 