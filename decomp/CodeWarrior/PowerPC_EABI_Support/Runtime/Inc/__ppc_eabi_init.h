/***************************************************************************/
/*

FILE
	__ppc_eabi_init.h

DESCRIPTION

	Interface for board-level initialization and user-level initialization.
	
	__start
		(registers initialized)
		__init_hardware called
		(data initialized, .data/.bss/.sdata/.sbss...)
		__init_cpp
		(exceptions initialized and static constructors called)
		__init_user
		main
	
	Define __init_hardware and __init_user or use the default stub functions.
	
	Note: __init_hardware should be written so as to not reference memory
	i.e., it should not be written in C or allocate a stackframe.
		
COPYRIGHT	
	(c) 1997 Metrowerks Corporation
	All rights reserved.

HISTORY
	97 APR 17 LLY	Created.
	97 JUN 21 MEA	added __init_cpp() and __init().
	97 JUL 20 MEA	added _ExitProcess.
	97 JUL 20 MEA	added ADSInit.

*/
/***************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

__declspec(section ".init") extern void __init_hardware(void);
extern void __init_user(void);
extern void _ExitProcess(void);													
__declspec(section ".init") extern void __flush_cache(void *address, unsigned int size);
__declspec(section ".init") extern void ADSInit();
extern void __copy_vectors(void);

#ifdef __cplusplus
}
#endif
