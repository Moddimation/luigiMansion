/***************************************************************************/
/*

FILE
	__init_cpp_exceptions.c

DESCRIPTION

	Runtime file.  Contains calls to initialize c++ exceptions.  Contents
	will be included only if your project has exceptions.


COPYRIGHT	
	(c) 1999 Metrowerks Corporation
	All rights reserved.

HISTORY
	99 JAN 22 MEA	Created.

*/
/***************************************************************************/

#if __MWERKS__
#pragma exceptions off
#pragma internal on
#endif

#include <__ppc_eabi_linker.h>		/* linker-generated symbol declarations */
#include <NMWException.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void __init_cpp_exceptions(void);
extern void __fini_cpp_exceptions(void);
extern void suspend(void);

#ifdef __cplusplus
}
#endif

static int fragmentID = -2;			/* ID given to fragment by exception-handling		*/
									/* valid numbers are 0 - MAX_INT; if -1 then		*/
									/* too many fragments were loaded					*/

static asm char *GetR2(void)
{
	/*
	 *	Get the contents of the r2 register
	 */
	nofralloc
	mr      r3,r2
	blr
}

extern void __init_cpp_exceptions(void)
{
	char *R2;				/* r2 register contents								*/

	if (fragmentID == -2) {
	/* use suspend with some OSes */
#ifdef TERMINATE_WITH_SUSPEND
		set_terminate(suspend);
#endif

		R2 = GetR2();

		/*
		 *	initialize exception tables
		 */
		fragmentID = __register_fragment(_eti_init_info, R2);
	}
}

extern void __fini_cpp_exceptions(void)
{
	if (fragmentID != -2) {
		__unregister_fragment(fragmentID);
		fragmentID = -2;
	}
}

#if __MWERKS__ && __PPC_EABI__
__declspec(section ".ctors") 
	static const void * const __init_cpp_exceptions_reference = __init_cpp_exceptions;
#if __dest_os != __eppc_vxworks
__declspec(section ".dtors") 
	static void * const __destroy_global_chain_reference = __destroy_global_chain;
__declspec(section ".dtors") 
	static void * const __fini_cpp_exceptions_reference = __fini_cpp_exceptions;
#endif
#endif
