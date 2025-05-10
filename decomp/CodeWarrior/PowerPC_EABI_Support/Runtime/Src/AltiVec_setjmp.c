#if __MWERKS__
#pragma ANSI_strict off
#endif

//	Altivec_setjmp.c	-	setjmp() and longjmp() routines for Metrowerks C++ for PowerPC
//
//	Copyright 1999 Metrowerks, Inc.  All Rights Reserved.
//
//
//	THEORY OF OPERATION
//
//	The runtime support routines __setjmp() and longjmp() support the C <setjmp.h>
//	facilities. These routines are (finally) compatible with Apple's StdCLib.
//
//	__setjmp() captures the state of the program in a jmp_buf data structure
//	which has the following C definition:
//
//		typedef struct __jmp_buf {
//			unsigned long	PC;			//	0: saved PC
//			unsigned long	CR;			//	4: saved CR
//			unsigned long	SP;			//  8: saved SP
//			unsigned long	RTOC;		// 12: saved RTOC
//			unsigned long	reserved;	// 16: not used
//			unsigned long	GPRs[19];	// 20: saved r13-r31
//			double			FPRs[18];	// 96: saved fp14-fp31
//			double			FPSCR;		//240: saved FPSCR
//		} *__jmp_buf;
//
//	longjmp() restores the state, effecting a transfer back to the saved PC with
//	appropriate registers, stack, TOC, etc.
//
//	In <setjmp.h> the jmp_buf type must be defined as an array (per ANSI rules):
//
//		typedef long jmp_buf[62];
//
//	setjmp() and longjmp() are defined as follows:
//
//		int __setjmp(jmp_buf env);
//		#define setjmp(env) __setjmp(env)
//		void longjmp(jmp_buf env, int val);
//
//	Because __setjmp() needs to save the RTOC of the calling function, it
//	cannot be called across a fragment boundary. To ensure this, we omit
//	the TVector for setjmp() so that link errors will occur
//	if a fragment does not have a local copy of these routines.
//	(We can't omit the TVector for longjmp() because the Plum Hall suite
//	tests for it by taking its address, which requires a TVector)
//

#include <ansi_parms.h>    /* hh 971207 Added <ansi_parms.h> header */

#if __ALTIVEC__

#pragma scheduling altivec
#pragma altivec_model on

#include "__jmp_buf.h"

/*
 *	Prototypes
 */

#ifdef __cplusplus
extern "C" {
#endif

#pragma tvectors off
#pragma internal on

int __vec_setjmp(__jmp_buf *env);

#pragma internal off

void __vec_longjmp(__jmp_buf *env, int val);

#ifdef __cplusplus
}
#endif


//	__vec_setjmp	-	Vector C setjmp() routine
//
//	On entry R3 points to a jmp_buf struct. On exit, R3 is 0.
// 
//  NOTE: We avoid spilling unused AltiVec vector registers to memory if
//		  the registers are unused as marked by the vrsave register.
//		  AltiVec registers are large (16 bytes) 
//
//		  This code is written for clarity, not for optimum performance.
//		  For better performance you would mix the "stfd" instructions
//		  with the nonvolatile vector register save dispatcher.
//

#pragma altivec_vrsave off				// so we don't save every vector register
asm int __vec_setjmp(register __jmp_buf *env)
{
		nofralloc
		machine altivec
		
		mflr		r7
		mfcr		r6
		stw			SP,env->sp			//	save SP
		stw			r7,env->pc			//	save PC (LR)
		stw			r6,env->cr			//	save CR
		stw			RTOC,env->rtoc		//	save #RTOC
		stmw		r13,env->gprs		//	save R13-R31
#ifndef _No_Floating_Point_Regs
		mffs		fp0
		stfd		fp14,env->fp14		//	save FP14-FP31
		stfd		fp15,env->fp15
		stfd		fp16,env->fp16
		stfd		fp17,env->fp17
		stfd		fp18,env->fp18
		stfd		fp19,env->fp19
		stfd		fp20,env->fp20
		stfd		fp21,env->fp21
		stfd		fp22,env->fp22
		stfd		fp23,env->fp23
		stfd		fp24,env->fp24
		stfd		fp25,env->fp25
		stfd		fp26,env->fp26
		stfd		fp27,env->fp27
		stfd		fp28,env->fp28
		stfd		fp29,env->fp29
		stfd		fp30,env->fp30
		stfd		fp31,env->fp31
		stfd		fp0,env->fpscr		//	save FPSCR
#endif /* ndef _No_Floating_Point_Regs */

		mfvrsave 	r5
		bl			__setLR				// set LR to address of setLR				
__setLR:	

		stw			r5,env->vrsave
		clrlwi		r5,r5,20			// clear left 20 bits
		cntlzw		r5,r5				// find first nonvolatile VR used
		slwi		r5,r5,3				// multiply by 8 for index past __setjmpv20
		mflr		r4
		
		addi		r4,r4,__setjmpv20 - __setLR	- (20 * 8)
		add			r4,r4,r5
		mtlr		r4
		
		blr								// jump to index past __setjmpv20
		
__setjmpv20:		
		la 			r5,env->vr20		// 	save VR20-VR31 
		stvx		vr20,r0,r5
		la 			r5,env->vr21
		stvx		vr21,r0,r5
		la 			r5,env->vr22
		stvx		vr22,r0,r5
		la 			r5,env->vr23
		stvx		vr23,r0,r5
		la 			r5,env->vr24
		stvx		vr24,r0,r5
		la 			r5,env->vr25
		stvx		vr25,r0,r5
		la 			r5,env->vr26
		stvx		vr26,r0,r5
		la 			r5,env->vr27
		stvx		vr27,r0,r5
		la 			r5,env->vr28
		stvx		vr28,r0,r5
		la 			r5,env->vr29
		stvx		vr29,r0,r5
		la 			r5,env->vr30
		stvx		vr30,r0,r5
		la 			r5,env->vr31
		stvx		vr31,r0,r5

		mfvrsave 	r6
		oris	 	r6,r6,0x8000	// indicate vr0 in-use
		mtvrsave 	r6

		mtlr		r7				// set at beginning of routine
		mfvscr		vr0				// mfvscr is context-synchronizing, so this will take many cycles
		la 			r5,env->vscr
		stvewx		vr0,r0,r5		// store 32 bits out of lo 32 bits of v0

		li			r5,0x0000		// indicate that no vector registers are being used now
		mtvrsave	r5				// This is OK because we just saved the non-volatile set and exercised our right
									// to trash all the volatile vector registers
		
		li			r3,0
		blr
}

#pragma altivec_vrsave on


//	__vec_longjmp		-	C longjmp() routine
//
//	On entry R3 points to a jmp_buf struct and R4 contains the return value.
//	On exit, R3 contains 1 if R4 was 0, otherwise it contains the value from R4.
//
//  NOTE: We avoid reading unused AltiVec vector registers from memory if
//		  the registers are unused as marked by the vrsave register.
//		  AltiVec registers are large (16 bytes) 
//
//		  This code is written for clarity, not for optimum performance.
//		  For better performance you would mix the "stfd" instructions
//		  with the nonvolatile vector register save dispatcher.

#pragma altivec_vrsave off
asm void __vec_longjmp(register __jmp_buf *env, register int val)
{
		nofralloc
	
		lwz			r6,env->cr
		mtcrf		255,r6			//	restore CR
		lwz			SP,env->sp		//	restore SP
		lwz			RTOC,env->rtoc	//	restore RTOC
		lmw			r13,env->gprs	//	restore R13-R31
#ifndef _No_Floating_Point_Regs
		lfd			fp14,env->fp14	//	restore FP14-FP31
		lfd			fp15,env->fp15
		lfd			fp16,env->fp16
		lfd			fp17,env->fp17
		lfd			fp18,env->fp18
		lfd			fp19,env->fp19
		lfd			fp20,env->fp20
		lfd			fp21,env->fp21
		lfd			fp22,env->fp22
		lfd			fp23,env->fp23
		lfd			fp24,env->fp24
		lfd			fp25,env->fp25
		lfd			fp26,env->fp26
		lfd			fp27,env->fp27
		lfd			fp28,env->fp28
		lfd			fp29,env->fp29
		lfd			fp30,env->fp30
		lfd			fp0,env->fpscr
		lfd			fp31,env->fp31
#endif /* ndef _No_Floating_Point_Regs */

		lwz			r5,env->vrsave		// mtvrsave must happen after restore of all non-volatiles
		bl			__setLR				// set LR to address of setLR				
__setLR:	

		oris	 	r5,r5,0x8000		// indicate vr0 in-use
		mtvrsave 	r5
		
		clrlwi		r5,r5,20			// clear left 20 bits
		cntlzw		r5,r5				// find first nonvolatile VR used
		slwi		r5,r5,3				// multiply by 8 for index past __setjmpv20
		mflr		r6
		
		addi		r6,r6,__longjmpv20 - __setLR - (20 * 8)
		add			r6,r6,r5
		mtlr		r6
		blr								// jump to index past __longjmpv20
		
__longjmpv20:		
		la			r5,env->vr20
		lvx			vr20,r0,r5
		la			r5,env->vr21
		lvx			vr21,r0,r5
		la			r5,env->vr22
		lvx			vr22,r0,r5
		la			r5,env->vr23
		lvx			vr23,r0,r5
		la			r5,env->vr24
		lvx			vr24,r0,r5
		la			r5,env->vr25
		lvx			vr25,r0,r5
		la			r5,env->vr26
		lvx			vr26,r0,r5
		la			r5,env->vr27
		lvx			vr27,r0,r5
		la			r5,env->vr28
		lvx			vr28,r0,r5
		la			r5,env->vr29
		lvx			vr29,r0,r5
		la			r5,env->vr30
		lvx			vr30,r0,r5
		la			r5,env->vr31
		lvx			vr31,r0,r5

		lwz			r7,env->pc		// restore PC after our jump through the vectors
		la			r5,env->vscr
		lvewx		vr0,r0,r5		// load 32 bits into lo 32 bits of v0
		mtvscr		vr0

		lwz			r5,env->vrsave	// restore vrsave
		mtvrsave 	r5
		
		mtlr		r7				//	restore PC (LR)
		
		cmpwi		val,0
		mr			r3,val
#ifndef _No_Floating_Point_Regs
		mtfsf		255,fp0			//	restore FPSCR
#endif /* ndef _No_Floating_Point_Regs */
		bnelr						//	return 'val'
		li			r3,1			//	return 1
		blr
}
#pragma altivec_vrsave on

#endif /* __ALTIVEC__ */
/*
	Change Record
	
	990723 MEA created from setjmp.c 
*/