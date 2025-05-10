
/*	ptmf.c	-	pointer-to-member-function support for Metrowerks C++ for PowerPC
 *
 *	Copyright 1995-1997 Metrowerks, Inc.  All Rights Reserved.
 *
 *
 *	THEORY OF OPERATION
 *
 *	A pointer-to-member-function (PTMF) is represented as a 3-word struct:
 *
 *		+-----------------------------+			+-----------------------------+
 *		|		'this' delta		  |			|		'this' delta		  |
 *		|-----------------------------| 		|-----------------------------|
 *		|			-1				  |			|		 vtbl index 		  |
 *		|-----------------------------| 		|-----------------------------|
 *		|		function pointer	  |			|		 vptr offset		  |
 *		+-----------------------------+			+-----------------------------+
 */

#pragma ANSI_strict off
#pragma tvectors off
#pragma internal on
#pragma options align=power


//
//	Private Types
//

typedef struct PTMF {
	long	this_delta;				//	delta to this pointer
	long	vtbl_offset;			//	offset in vtable (-1: not a virtual function)
	union {
		void	*func_addr;			//	nonvirtual function address
		long	ventry_offset;		//	offset of virtual function entry in vtable
	}	func_data;
}	PTMF;


//
//	Public Data
//

const PTMF __ptmf_null = { 0, 0, 0 };


//
//	Prototypes
//

#ifdef __cplusplus
extern "C" {
#endif

long __ptmf_test(PTMF *ptmf);
long __ptmf_cmpr(PTMF *ptmf1, PTMF *ptmf2);
void __ptmf_call(...);
void __ptmf_call4(...);
void __ptmf_scall(...);
void __ptmf_scall4(...);
PTMF *__ptmf_cast(long offset, const PTMF *ptmfrom, PTMF *ptmto);

#ifdef __cplusplus
}
#endif


//	__ptmf_test	-	test pointer-to-member-function for null
//
//	R3 contains the PTMF. If it is null, we return 0; else we return 1.
//

asm long __ptmf_test(register PTMF *ptmf)
{
#if __PPC_EABI__
		nofralloc
#else
		smclass	GL
#endif
		lwz		r5,PTMF.this_delta(r3)
		lwz		r6,PTMF.vtbl_offset(r3)
		lwz		r7,PTMF.func_data(r3)
		li		r3,1
		cmpwi	cr0,r5,0
		cmpwi	cr6,r6,0
		cmpwi	cr7,r7,0
		bnelr	cr0
		bnelr	cr6
		bnelr	cr7
		li		r3,0
		blr
}


//	__ptmf_cmpr	-	compare two pointer-to-member-functions
//
//	R3 and R4 contain the PTMFs. If equal, we return 0; else we return 1.
//

asm long __ptmf_cmpr(register PTMF *ptmf1, register PTMF *ptmf2)
{
#if __PPC_EABI__
		nofralloc
#else
		smclass	GL
#endif
		lwz		r5,PTMF.this_delta(r3)
		lwz		r6,PTMF.this_delta(r4)
		lwz		r7,PTMF.vtbl_offset(r3)
		lwz		r8,PTMF.vtbl_offset(r4)
		lwz		r9,PTMF.func_data(r3)
		lwz		r10,PTMF.func_data(r4)
		li		r3,1
		cmpw	cr0,r5,r6
		cmpw	cr6,r7,r8
		cmpw	cr7,r9,r10
		bnelr	cr0
		bnelr	cr6
		bnelr	cr7
		li		r3,0
		blr
}


//	__ptmf_call		-	call pointer-to-member-function
//
//	R12 contains the PTMF. R3 contains 'this'.
//

asm void __ptmf_call(...)
{
#if __PPC_EABI__
		nofralloc
#else
		smclass	GL
#endif
		lwz		r0,PTMF.this_delta(r12)
		lwz		r11,PTMF.vtbl_offset(r12)
		lwz		r12,PTMF.func_data(r12)	//	function pointer if not virtual
		cmpwi	cr0,r11,0
		add		r3,r3,r0				//	adjust 'this'
		blt		cr0,@1
		lwzx	r12,r3,r12				//	get vptr
		add		r12,r12,r11				//	point to vtbl entry
		lwz		r0,4(r12)				//	get 'this' delta
		lwz		r12,0(r12)				//	get function pointer
		add		r3,r3,r0				//	adjust 'this' again
#if __PPC_EABI__
@1		mtctr	r12
#else
@1		lwz		r0,0(r12)
		stw		RTOC,20(SP)
		mtctr	r0
		lwz		RTOC,4(r12)
#endif
		bctr
}


//	__ptmf_call4	-	call pointer-to-member-function, 'this' in R4
//
//	R12 contains the PTMF. R4 contains 'this'.
//

asm void __ptmf_call4(...)
{
#if __PPC_EABI__
		nofralloc
#else
		smclass	GL
#endif
		lwz		r0,PTMF.this_delta(r12)
		lwz		r11,PTMF.vtbl_offset(r12)
		lwz		r12,PTMF.func_data(r12)	//	function pointer if not virtual
		cmpwi	cr0,r11,0
		add		r4,r4,r0				//	adjust 'this'
		blt		cr0,@1
		lwzx	r12,r4,r12				//	get vptr
		add		r12,r12,r11				//	point to vtbl entry
		lwz		r0,4(r12)				//	get 'this' delta
		lwz		r12,0(r12)				//	get function pointer
		add		r4,r4,r0				//	adjust 'this' again
#if __PPC_EABI__
@1		mtctr	r12
#else
@1		lwz		r0,0(r12)
		stw		RTOC,20(SP)
		mtctr	r0
		lwz		RTOC,4(r12)
#endif
		bctr
}


//	__ptmf_scall	-	call pointer-to-member-function
//
//	This is used when we know the class tree uses single inheritance only.
//	R12 contains the PTMF. R3 contains 'this'.
//
//

asm void __ptmf_scall(...)
{
#if __PPC_EABI__
		nofralloc
#else
		smclass	GL
#endif
		lwz		r0,PTMF.this_delta(r12)	//	(needed for thunks)
		lwz		r11,PTMF.vtbl_offset(r12)
		lwz		r12,PTMF.func_data(r12)	//	function pointer if not virtual
		add		r3,r3,r0				//	adjust 'this' (needed for thunks)
		cmpwi	cr0,r11,0
		blt		cr0,@1
		lwzx	r12,r3,r12				//	get vptr
		lwzx	r12,r12,r11				//	get function pointer
#if __PPC_EABI__
@1		mtctr	r12
#else
@1		lwz		r0,0(r12)
		stw		RTOC,20(SP)
		mtctr	r0
		lwz		RTOC,4(r12)
#endif
		bctr
}


//	__ptmf_scall4	-	call pointer-to-member-function
//
//	This is used when we know the class tree uses single inheritance only.
//	R12 contains the PTMF. R4 contains 'this'.
//
//

asm void __ptmf_scall4(...)
{
#if __PPC_EABI__
		nofralloc
#else
		smclass	GL
#endif
		lwz		r0,PTMF.this_delta(r12)	//	(needed for thunks)
		lwz		r11,PTMF.vtbl_offset(r12)
		lwz		r12,PTMF.func_data(r12)	//	function pointer if not virtual
		add		r4,r4,r0				//	adjust 'this' (needed for thunks)
		cmpwi	cr0,r11,0
		blt		cr0,@1
		lwzx	r12,r4,r12				//	get vptr
		lwzx	r12,r12,r11				//	get function pointer
#if __PPC_EABI__
@1		mtctr	r12
#else
@1		lwz		r0,0(r12)
		stw		RTOC,20(SP)
		mtctr	r0
		lwz		RTOC,4(r12)
#endif
		bctr
}


/************************************************************************/
/* Purpose..: This function will copy/cast a pointer to func member		*/
/* Input....: offset delta to apply	to pointer to function member		*/
/* Input....: pointer to original pointer to function member			*/
/* Input....: pointer to destiniation pointer to function member		*/
/* Return...: pointer to destiniation pointer to function member		*/
/************************************************************************/
extern PTMF *__ptmf_cast(long offset,const PTMF *ptmfrom,PTMF *ptmto)
{
	ptmto->this_delta	= ptmfrom->this_delta+offset;
	ptmto->vtbl_offset	= ptmfrom->vtbl_offset;
	ptmto->func_data	= ptmfrom->func_data;
	return ptmto;
}