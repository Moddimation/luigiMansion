/***************************************************************************/
/*

FILE
	__start.c

DESCRIPTION

	Metrowerks PowerPC EABI Runtime Initialization.	 This file provides
	the first entry point, "__start" which is used by the linker as a
	program entry point.  It initializes the program state (registers,
	stack pointer, data pointers) and copies data images from ROM to RAM
	as necessary.
	
	
	void __start(void)
	
		Main entry point for the PPC EABI program.	It will call main().
	
	This file allows for board-level initialization.  Define a function
	called __init_hardware().

	This file allows for user-level initialization before main().  Define
	a function called __init_user().  If program is in C++, __init_user
	needs to initialize exceptions and static constructors.
	

REFERENCES

	[EABI95]	PowerPC Embedded Application Binary Interface,
				32-bit Implementation.	Version 1.0, Initial Release,
				1/10/95.  Stephen Sobek, Motorola, and Kevin Burke, IBM.

NOTES

	The .PPC.EMB.sdata0 and .PPC.EMB.sbss section pointers are not
	currently supported
	
	Defined, but unused linker-generated symbols:
		_stack_size		Defined but not used yet.  For bounds checking.
		_heap_size		Defined but not used yet.  For user heap.
		_ftext			Defined but not used.  Start of text section.
		_etext			Defined but not used.  Address after end of text.
		

COPYRIGHT	
	(c) 1996-7 Metrowerks Corporation
	All rights reserved.

HISTORY
	96 OCT 20 LLY	Created __start.c
	96 NOV 08 LLY	Initialized .bss and .sbss
	97 FEB 19 LLY	Added __copy_rom function as a future placeholder for
					copying ROM-based data into RAM.  __start changed to
					__init_main for SDSMON07
	97 FEB 24 LLY	Test for zero length .bss and .sbss
	97 FEB 28 LLY	Converted to C code.  Added the rest of the EABI sections
					(.sdata, .sbss, .sdata2)
	97 MAR 02 LLY	Added vector copy
	97 MAR 10 LLY	Converted runtime init for MPC821ADS stationery.
	97 APR 16 LLY	Converted runtime init for PPC EABI tools 4/1/97.
					New linker-generated symbols; moved to
					"__ppc_eabi_linker.h".  Register initialization was
					moved into a function.
	97 APR 19 LLY	Added comments regarding safe usage of memory.
	97 MAY 01 LLY	Added GRC's initial stack frame.
	97 JUL 20 MEA	Removed __exit and replaced it with _ExitProcess.
	97 DEC 7  MEA	Changed linker generated symbols to C type variables.
					Use _rom_copy_info and _bss_copy_info instead of
					individual sections symbols.
	97 DEC 8  MEA	Added section pragma to put all code in this file into
					".init"
					
*/
/***************************************************************************/

	/* main(), __init_user(), and exit() need to have far absolute	*/
	/* addressing if you are flashing to ROM with the default 		*/
	/* address of 0xfe000000 on MPC860.  You can save a couple of	*/
	/* instructions in __start() if you aren't using that address	*/
	/* by commenting out the following define.						*/
#define USE_FAR_ADDRESSING_TO_TEXT_SECTION

#include <__mem.h>
#include <__ppc_eabi_linker.h>		/* linker-generated symbol declarations */
#include <__ppc_eabi_init.h>		/* board- and user-level initialization */

/***************************************************************************/
/*
 *	external declarations
 */
/***************************************************************************/

extern void main();

/***************************************************************************/
/*
 *	function declarations
 */
/***************************************************************************/

#pragma section code_type ".init"

#ifdef __cplusplus
extern "C" {
#endif

extern void __start(void);				/* primary entry point */
extern void exit(int);

#ifdef __cplusplus
}
#endif

static void __init_registers(void);		/* set up PPC regs (r1, r2, r13) */
static void __init_data(void);			/* private ROM-to-RAM copy routine */


/***************************************************************************/
/*
	Memory Map for Motorola MPC821/860ADS Evaluation Board
	
	0x00000000..0x00001FFF	Exception Vector Table
		  0D00				Trace Exception
		  1000				Breakpoint Exception
		  1100				(end of Breakpoint Exception)
	0x00002000..0x0000FFFF	Unused RAM
	0x00010000..0x000FFFFF	Unused RAM (first MB)
		 10000				Start PC (default for downloaded programs)
			  ..0x001FFFFF	Unused RAM (second MB)
			  ..0x002FFFFF	Unused RAM (third MB)
			  ..0x003FFFFF	Unused RAM (fourth MB)
			  ..
			  ..0x01FFFFFF	Unused RAM (32nd MB, maximum for eval board)
			  
*/
/***************************************************************************/

/***************************************************************************/
/*
 *	__start
 *
 *	PowerPC EABI Runtime Initialization.  Initialize pointers,
 *	initialize data, and call main().
 *
 *	This function is the very first location executed in a program, or
 *	the first location called by the board-level initialization.
 *	Memory access is not guaranteed to be safe until after __init_hardware.
 */
/***************************************************************************/
asm void __start(void)
{
	nofralloc							/* MWERKS: explicitly no stack */
										/* frame allocation */

#if __option(little_endian)

	opword 0xA600007C   /* load the MSR */
	opword 0x01000060	/* set the LE mode bit */
	opword 0x2401007C	/* store the new MSR */

#endif


	/*
	 *	PowerPC EABI init registers (stack, small data areas)
	 */
	bl		__init_registers
		
	/*
	 *	board-level initialization
	 */
	bl		__init_hardware

	/*
	 *	Memory access is safe now.
	 */

	/*
	 *	Prepare a terminating stack record.
	 */
	li		r0, 0xFFFF				/* load up r0 with 0xFFFFFFFF */
	stwu	r1, -8(r1)				/* Decrement stack by 8 bytes, (write word)*/
	stw		r0, 4(r1)				/* Make an illegal return address of 0xFFFFFFFF */
	stw		r0,	0(r1)				/* Make an illegal back chain address of 0xFFFFFFFF */

	 
	/*
	 *	Data initialization: copy ROM data to RAM as necessary
	 */

#if __dest_os != __eppc_vxworks
	bl		__init_data
#endif
		
	/*
	 *	initialization before main
	 */
	
#if defined(USE_FAR_ADDRESSING_TO_TEXT_SECTION)
	lis      r3,__init_user@ha
	addi     r3,r3,__init_user@l
	mtlr     r3
	blrl
#else
	bl		__init_user
#endif

	/*
	 *	branch to main program
	 */
	
#if defined(USE_FAR_ADDRESSING_TO_TEXT_SECTION)
	lis      r3,main@ha
	addi     r3,r3,main@l
	mtlr     r3
	blrl
#else
	bl		main
#endif

	/*
	 *	exit program
	 */
	
#if defined(USE_FAR_ADDRESSING_TO_TEXT_SECTION)
	lis      r3,exit@ha
	addi     r3,r3,exit@l
	mtlr     r3
	blrl
#else
	b		exit
#endif
	
}

/***************************************************************************/
/*
 *	__copy_rom_section
 *
 *	Copy the ROM section to RAM if dst and src are different and size
 *	is nonzero.
 *
 *	dst			destination RAM address
 *	src			source ROM address
 *	size		number of bytes to copy
 */
/***************************************************************************/
static void __copy_rom_section(void* dst, const void* src, unsigned long size)
{
	if (size && (dst != src)) {
		memcpy(dst, src, size);
		__flush_cache( dst, size );
	}
}


/***************************************************************************/
/*
 *	__init_bss_section
 *
 *	Initialize the RAM section to zeros if size is greater than zero.
 *
 *	dst			destination RAM address
 *	size		number of bytes to zero
 */
/***************************************************************************/
static void __init_bss_section(void* dst, unsigned long size)
{
	if (size) {
		memset(dst, 0, size);
	}
}

/***************************************************************************/
/*
 *	__init_registers
 *
 *	Initialize PowerPC EABI Registers
 *
 *	Note: this function is guaranteed to not reference any memory; the memory
 *	controller may not be initialized.
 *
 */
/***************************************************************************/
asm static void __init_registers(void)
{
	nofralloc						/* see above on usage of nofralloc */
	/*
	 *	initialize stack pointer
	 */

	lis		r1, _stack_addr@h		/* _stack_addr is generated by linker */
	ori		r1, r1, _stack_addr@l

#if __dest_os != __eppc_vxworks
	/*
	 *	initialize small data area pointers (EABI)
	 */
	lis		r2, _SDA2_BASE_@h		/* __SDA2_BASE_ is generated by linker */
	ori		r2, r2, _SDA2_BASE_@l

	lis		r13, _SDA_BASE_@h		/* _SDA_BASE_ is generated by linker */
	ori		r13, r13, _SDA_BASE_@l
#endif

	blr
}

/***************************************************************************/
/*
 *	__init_data
 *
 *	Initialize all (RAM) data sections, copying ROM sections as necessary.
 *
 *	dst			destination RAM address
 *	size		number of bytes to zero
 */
/***************************************************************************/
static void __init_data(void)
{
	__rom_copy_info *dci;
	__bss_init_info *bii;
	
 	/* Copy from ROM to RAM: */

	dci = _rom_copy_info;
	while (1) {
		if (dci->size == 0) break;
 		__copy_rom_section(dci->addr, dci->rom, dci->size);
 		dci++;
	}
 
 	/* Initialize with zeros: */

	bii = _bss_init_info;
	while (1) {
		if (bii->size == 0) break;
 		__init_bss_section(bii->addr, bii->size);
 		bii++;
	}
}

