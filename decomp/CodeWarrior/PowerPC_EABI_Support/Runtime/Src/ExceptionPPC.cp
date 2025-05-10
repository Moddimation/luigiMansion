//
//	ExceptionPPC.cp		-	Exception Handling Runtime Support for CodeWarrior(PowerPC)
//
//  Copyright ゥ 1995-1997 Metrowerks, Inc.  All rights reserved.
//
//
//	POWERPC STACK FRAME FORMAT
//
//	The PowerPC runtime architecture uses a grow-down stack with a single stack
//	pointer at the low address end. The stack has the following organization:
//	
//
//	LOW ADDRESSES	|						|
//					|						|
//					|-----------------------|
//					|	GPR save area		|
//					|-----------------------|	Can be used by frameless leaf routines
//					|	FPR save area		|
//	Callee's SP   =>|-----------------------|
//					|		Callee's		|
//					|	  linkage area		|
//					|-----------------------|
//					|		Callee's		|	Storage for parameters passed to
//					|	parameter area		|	routines called by callee
//					|-----------------------|
//					|		Callee's		|
//					|	local variables		|
//					|-----------------------|
//					|	alignment gap 1		|
//					|-----------------------|
//					|	GPR save area		|	Storage for nonvolatile GPRs used by callee
//					|-----------------------|
//					|	alignment gap 2		|
//					|-----------------------|
//					|	FPR save area		|	Storage for nonvolatile FPRs used by callee
//	Caller's SP   =>|-----------------------|
//					|		Caller's		|
//					|	  linkage area		|
//					|-----------------------|
//					|		Caller's		|	Storage for parameters passed to
//					|	parameter area		|	callee
//					|-----------------------|
//					|		Caller's		|
//					|	local variables		|
//					|-----------------------|
//					|						|
//	HIGH ADDRESSES	|						|
//
//
//	where the "linkage area" looks like this:
//
//
//					|-----------------------|
//				0	| Stack Frame Back Link	|	Pointer to caller's frame
//					|-----------------------|
//				4	|		Saved CR		|	Our CR (saved by routines we call)
//					|-----------------------|
//				8	|		Saved LR		|	Our LR (saved by routines we call)
//					|-----------------------|
//				12	|		(Reserved)		|
//					|-----------------------|
//				16	|		(Reserved)		|
//					|-----------------------|
//				20	|	Saved TOC Pointer	|	Our TOC (saved by cross-TOC glue)
//					|-----------------------|
//
//
//	To unwind the stack, we start with a return address and SP for the routine which
//	threw the exception. The next return address can be found by following the
//	back-link at 0(SP) and accessing the saved LR in that frame.
//
//	Restoring registers is slightly more tricky. If 'saved_CR' is true, we restore
//	the CR from the callers linkage area. For saved GPRs and FPRs we start with
//	the callers SP; the <n> saved FPRs will be at <callers SP> - (8 * <n>) and the
//	<m> saved GPRs will be the (<m> * 4) words ending at at <callers SP> - (8 * <n>)
//	adjusted so that the last word ends on a 16-byte boundary. (See below)
//
//

#if __MWERKS__
#pragma exceptions on
#endif

#define __NOSTRING__	//	do not include <string>

#include <stdlib.h>
#include <MWCPlusLib.h>
#include <exception.h>
#include "ExceptionPPC.h"
#include <NMWException.h>	// ra 990322 changed from "" to <>

#if __PPC_EABI__
#include <__ppc_eabi_linker.h>		/* linker-generated symbol declarations */
#define RETURN_ADDRESS 4
#else
#define RETURN_ADDRESS 8
#define nofralloc
#endif

#if 1
#define INLINE			static				//	static for debugging
#else
#define INLINE			inline				//	inline for shipping code
#endif

#if __VEC__
#pragma options align=power
#endif

#if __VEC__
union MWEVector128 {
	__vector unsigned long			vul;
	unsigned long 					l[4];
	unsigned short 					s[8];
	unsigned char 					c[16];
};
typedef union MWEVector128 MWEVector128;
#endif

//	typedefs

typedef struct ThrowContext {
#if __VEC__
	MWEVector128 VR[32];					//  VR0-VR31 (not all are saved/restored)
	MWEVector128 vscr;
	unsigned long vrsave;
#endif
#ifndef _No_Floating_Point_Regs
	double		FPR[32];					//	FPR0-FPR31	(not all are saved/restored)
#endif
	long		GPR[32];					//	GPR0-GPR31	(not all are saved/restored)
	long		CR;							//	CR0-CR7
	char*		SP;							//	stack pointer during unwind (used for linkage)
	char*		FP;							//	frame pointer during unwind (used for locals)
	char*		throwSP;					//	stack pointer at throw
	char*		returnaddr;					//	return address
	char*		throwtype;					//	throw type argument (0L: rethrow: throw; )
	void*		location;					//	location argument (0L: rethrow: throw; )
	void*		dtor;						//	dtor argument
	CatchInfo	*catchinfo;					//	pointer to rethrow CatchInfo (or 0L)	
}	ThrowContext;

typedef struct MWExceptionInfo {
	ExceptionTableSmall*	exception_record;		//	pointer to exception table (small or large)
	char*			current_function;		//	pointer to current function
	char*			action_pointer;			//	pointer to action
	char*			code_section;			//	base of code section for fragment containing table
	char*			data_section;			//	base of data section for fragment containing table
	char*			TOC;					//	TOC pointer for fragment containing table
}	MWExceptionInfo;

typedef struct FragmentInfo {
	ExceptionTableIndex*	exception_start;	//	start of exception table index for fragment
	ExceptionTableIndex*	exception_end;		//	end of exception table index for fragment
	char*					code_start;			//	start of code section for fragment
	char*					code_end;			//	end of code section for fragment
	char*					data_start;			//	start of data section for fragment
	char*					data_end;			//	end of data section for fragment
	char*					TOC;				//	TOC pointer for fragment
	int						active;				//	true: fragmentinfo element is registered
} FragmentInfo;

typedef struct ProcessInfo {
	__eti_init_info*		exception_info;		//	start of exception table index for fragment
	char*					TOC;				//	TOC pointer for fragment
	int						active;				//	true: fragmentinfo element is registered
} ProcessInfo;

typedef struct ActionIterator {
	MWExceptionInfo	info;					//	pointer to exception record
	char*			current_SP;				//	current stack pointer
	char*			current_FP;				//	current frame pointer (SP or R31)
	long			current_R31;			//	current R31
}	ActionIterator;

#if __VEC__
#pragma options align=reset
#endif

#if __PPC_EABI__
#define	MAXFRAGMENTS __CW_MAX_PROCESSES__	//	maximum # of code fragments we can register
static ProcessInfo fragmentinfo[MAXFRAGMENTS];
#else
#define	MAXFRAGMENTS	32					//	maximum # of code fragments we can register
static FragmentInfo fragmentinfo[MAXFRAGMENTS];
#endif


typedef void (*DeleteFunc)(void *);


#if __PPC_EABI__
/************************************************************************/
/* Purpose..: Register a code fragment and its exception tables			*/
/* Input....: pointer to __eti_init_info structure				*/
/* Input....: TOC for fragment											*/
/* Return...: unique ID for __ex_unregister_fragment					*/
/************************************************************************/
int __register_fragment(struct __eti_init_info *info, char *TOC)
{
	ProcessInfo *f;
	int i;
	
	//	find a free entry in the fragment table
	for(i=0,f=fragmentinfo;i<MAXFRAGMENTS;++i,++f) if(f->active==0)
	{
		f->exception_info=info;
		f->TOC=TOC;
		f->active=1;
		return(i);
	}
	//	couldn't register the fragment
	return(-1);
}
#else
/************************************************************************/
/* Purpose..: Register a code fragment and its exception tables			*/
/* Input....: pointer to start of fragments code section				*/
/* Input....: pointer to end   of fragments code section				*/
/* Input....: pointer to start of fragments data section				*/
/* Input....: pointer to end   of fragments data section				*/
/* Input....: pointer to start of fragments exception table index		*/
/* Input....: pointer to end   of fragments exception table index		*/
/* Input....: RTOC for fragment											*/
/* Return...: unique ID for __ex_unregister_fragment					*/
/************************************************************************/
int __register_fragment(char *code_start, char *code_end,
						char *data_start, char *data_end,
						char *exception_start, char *exception_end,
						char *TOC)
{
	FragmentInfo *f;
	int i;
	
	//	find a free entry in the fragment table
	for(i=0,f=fragmentinfo;i<MAXFRAGMENTS;++i,++f) if(f->active==0)
	{
		f->code_start=code_start;
		f->code_end=code_end;
		f->data_start=data_start;
		f->data_end=data_end;
		f->exception_start=(ExceptionTableIndex *)exception_start;
		f->exception_end=(ExceptionTableIndex *)exception_end;
		f->TOC=TOC;
		f->active=1;
		return(i);
	}
	//	couldn't register the fragment
	return(-1);
}
#endif

/************************************************************************/
/* Purpose..: Un-register a code fragment and its exception tables		*/
/* Input....: unique ID assigned in __ex_register_fragment				*/
/* Return...: ---														*/
/************************************************************************/
#if __PPC_EABI__
void __unregister_fragment(int fragmentID)
{
	ProcessInfo *f;
	
	if(fragmentID>=0 && fragmentID<MAXFRAGMENTS)
	{
		f = &fragmentinfo[fragmentID];
		f->exception_info=0;
		f->TOC=0;
		f->active=0;
	}
}
#else
void __unregister_fragment(int fragmentID)
{
	FragmentInfo *f;
	
	if(fragmentID>=0 && fragmentID<MAXFRAGMENTS)
	{
		f = &fragmentinfo[fragmentID];
		f->code_start=0; f->code_end=0;
		f->data_start=0; f->data_end=0;
		f->exception_start=0; f->exception_end=0;
		f->TOC=0;
		f->active=0;
	}
}
#endif

/************************************************************************/
/* Purpose..: Get an exception fragment record pointer					*/
/* Input....: pointer to return address									*/
/* Return...: pointer to FragmentInfo struct 							*/
/************************************************************************/
#if __PPC_EABI__
static int ExPPC_FindExceptionFragment(char *returnaddr, FragmentInfo *frag)
{
	ProcessInfo *f;
	int i;
	__eti_init_info *eti_info;
	
	for(i=0,f=fragmentinfo;i<MAXFRAGMENTS;++i,++f) if(f->active)
	{
		eti_info = f->exception_info;
		while (1) {
			if (eti_info->code_size == 0) break;
			if(returnaddr>=eti_info->code_start && 
					returnaddr<(char*)eti_info->code_start+eti_info->code_size) {
				frag->exception_start = (ExceptionTableIndex *)eti_info->eti_start;
				frag->exception_end = (ExceptionTableIndex *)eti_info->eti_end;
					/* the starts and ends are only used to find the fragment  */ 
				frag->code_start = 0;
				frag->code_end = 0;
				frag->data_start = 0;
				frag->data_end = 0;
				frag->TOC = f->TOC;
				frag->active = f->active;
				return(1);
			}
	 		eti_info++;
		}
	}
	
	return(0);
}
#else
static FragmentInfo *ExPPC_FindExceptionFragment(char *returnaddr)
{
	FragmentInfo *f;
	int i;
	
	for(i=0,f=fragmentinfo;i<MAXFRAGMENTS;++i,++f) if(f->active)
	{
		if(returnaddr>=f->code_start && returnaddr<f->code_end) return(f);
	}
	
	return(0);
}
#endif

/************************************************************************/
/* Purpose..: Get a exception record pointer							*/
/* Input....: pointer to return address									*/
/* Input....: pointer to MWExceptionInfo struct for result				*/
/* Return...: ---														*/
/************************************************************************/
static void ExPPC_FindExceptionRecord(char *returnaddr,MWExceptionInfo *info)
{
	FragmentInfo *fragment;
#if __PPC_EABI__
	FragmentInfo frag;
#endif
	ExceptionTableIndex *exceptionindex,*p;
	unsigned long returnoffset;
	long i,m,n;

	//	so far we haven't found anything
	info->exception_record=0;
	info->action_pointer=0;
	
	//	find the exception table for the fragment containing 'returnaddr'
#if __PPC_EABI__
	if ((ExPPC_FindExceptionFragment(returnaddr, &frag))==0) return;
	fragment = &frag;
#else
	if ((fragment=ExPPC_FindExceptionFragment(returnaddr))==0) return;
#endif
	info->code_section=fragment->code_start;
	info->data_section=fragment->data_start;
	info->TOC=fragment->TOC;
	
	//	binary-search the exception table index for a function containing 'returnaddr'
	returnoffset=returnaddr-fragment->code_start;
	exceptionindex=fragment->exception_start;
	for(i=0,n=fragment->exception_end-fragment->exception_start;;)
	{
		if(i>n) return;
		p=&exceptionindex[m=(i+n)/2];
		if(returnoffset<p->functionoffset) n=m-1;
		else if(returnoffset>p->functionoffset+ETI_GetFunctionSize(p->eti_field)) i=m+1;
		else break;
	}
	info->current_function=fragment->code_start+p->functionoffset;
	info->exception_record=ETI_GetDirectStore(p->eti_field)?(ExceptionTableSmall *)(&p->exceptionoffset)
										:(ExceptionTableSmall *)(fragment->data_start+p->exceptionoffset);
	
	//	find the set of actions to perform for an exception thrown from 'returnaddr'
	returnoffset-=p->functionoffset;
	
	if ( ET_IsLargeTable(info->exception_record->et_field) )
	{
		ExceptionTableLarge *etl = (ExceptionTableLarge *) info->exception_record;
		ExceptionRangeLarge *erl; 
		
		for( erl = etl->ranges; erl->start != 0; erl++ )
		{
			unsigned long range_end = erl->start + (erl->size * 4);		// large range is 256K (64K * 4) ... (+4?)
			
			if ( erl->start <= returnoffset && range_end >= returnoffset )
			{
				info->action_pointer = (char *) etl + erl->action; 
				break;
			}
		}
	}
	else
	{
#if __VEC__
		ExceptionTableSmall *ets 		= (ExceptionTableSmall *) 		info->exception_record;
		ExceptionTableSmallVector *etsv = (ExceptionTableSmallVector *) info->exception_record;
		ExceptionRangeSmall *ers;

		if ( ET_HasVectorInfo(info->exception_record->et_field) ) {
			for( ers = etsv->ranges; ers->start != 0; ers++ ) {
				if( ers->start <= returnoffset && ers->end >= returnoffset) {
					info->action_pointer = (char *) etsv + ers->action; 
					break;
				}
			}
		}
		else {
			for( ers = ets->ranges; ers->start != 0; ers++ ) {
				if( ers->start <= returnoffset && ers->end >= returnoffset) {
					info->action_pointer = (char *) ets + ers->action; 
					break;
				}
			}
		}
#else
		ExceptionTableSmall *ets = (ExceptionTableSmall *) info->exception_record;
		ExceptionRangeSmall *ers;

		for( ers = ets->ranges; ers->start != 0; ers++ )
		{
			if( ers->start <= returnoffset && ers->end >= returnoffset)
			{
				info->action_pointer = (char *) ets + ers->action; 
				break;
			}
		}
#endif

	}
}

/************************************************************************/
/* Purpose..: Find R31 saved in given stack frame						*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to topmost exception record						*/
/* Return...: pointer to return PC										*/
/************************************************************************/
static long ExPPC_PopR31(char *SP,MWExceptionInfo *info)
{
#ifndef _No_Floating_Point_Regs
	double *FPR_save_area;
#endif
	long *GPR_save_area;
	int saved_GPRs, saved_FPRs;
	
#ifndef _No_Floating_Point_Regs
	//	find saved FPRs
	saved_FPRs=ET_GetSavedFPRs(info->exception_record->et_field);
	FPR_save_area=(double *)(SP-saved_FPRs*8);

    //  find saved GPRs
    saved_GPRs=ET_GetSavedGPRs(info->exception_record->et_field);
    GPR_save_area=(long *)FPR_save_area;
    if(saved_FPRs&1) GPR_save_area-=2;  // 8-byte gap if # saved FPRs is odd
#else
    //  find saved GPRs
    saved_GPRs=ET_GetSavedGPRs(info->exception_record->et_field);
    GPR_save_area=(long *)SP;
#endif

	//	return saved R31
	return(GPR_save_area[-1]);	//	R31 is last register saved, has highest address
}

/************************************************************************/
/* Purpose..: Return current exception action type						*/
/* Input....: pointer to ActionIterator									*/
/* Return...: action type												*/
/************************************************************************/
static exaction_type ExPPC_CurrentAction(const ActionIterator *iter)
{
	if(iter->info.action_pointer==0) return EXACTION_ENDOFLIST;
	return ((ex_destroylocal *)iter->info.action_pointer)->action&EXACTION_MASK;
}

/************************************************************************/
/* Purpose..: Move to next action in Exception Table					*/
/* Input....: pointer to ActionIterator									*/
/* Return...: next action type											*/
/************************************************************************/
static exaction_type ExPPC_NextAction(ActionIterator *iter)
{
	exaction_type	action;

	for(;;)
	{
		if( iter->info.action_pointer==0
		||	((action=((ex_destroylocal *)iter->info.action_pointer)->action)&EXACTION_ENDBIT)!=0 )
		{	//	end of action list: find next exception record
			char *return_addr, *callers_SP;
			
			//	get LR saved in linkage area of caller
			callers_SP=*(char **)iter->current_SP;
			if(ET_GetSavedGPRs(iter->info.exception_record->et_field)) iter->current_R31=ExPPC_PopR31(callers_SP,&iter->info);
			return_addr=*(char **)(callers_SP+RETURN_ADDRESS);
			ExPPC_FindExceptionRecord(return_addr,&iter->info);
			if(iter->info.exception_record==0) terminate();	//	cannot find matching exception record
			//	pop down to caller's stack frame
			iter->current_SP=callers_SP;
			iter->current_FP=(ET_GetHasFramePtr(iter->info.exception_record->et_field))?(char *)iter->current_R31:iter->current_SP;
			if(iter->info.action_pointer==0) continue;		//	no actions
		}
		else
		{
			switch(action)
			{
			case EXACTION_DESTROYLOCAL:
				iter->info.action_pointer+=sizeof(ex_destroylocal); break;
			case EXACTION_DESTROYLOCALCOND:
				iter->info.action_pointer+=sizeof(ex_destroylocalcond); break;
			case EXACTION_DESTROYLOCALPOINTER:
				iter->info.action_pointer+=sizeof(ex_destroylocalpointer); break;
			case EXACTION_DESTROYLOCALARRAY:
				iter->info.action_pointer+=sizeof(ex_destroylocalarray); break;
			case EXACTION_DESTROYBASE:
			case EXACTION_DESTROYMEMBER:
				iter->info.action_pointer+=sizeof(ex_destroymember); break;
			case EXACTION_DESTROYMEMBERCOND:
				iter->info.action_pointer+=sizeof(ex_destroymembercond); break;
			case EXACTION_DESTROYMEMBERARRAY:
				iter->info.action_pointer+=sizeof(ex_destroymemberarray); break;
			case EXACTION_DELETEPOINTER:
				iter->info.action_pointer+=sizeof(ex_deletepointer); break;
			case EXACTION_DELETEPOINTERCOND:
				iter->info.action_pointer+=sizeof(ex_deletepointercond); break;
			case EXACTION_CATCHBLOCK:
				iter->info.action_pointer+=sizeof(ex_catchblock); break;
			case EXACTION_CATCHBLOCK_32:
				iter->info.action_pointer+=sizeof(ex_catchblock_32); break;
			case EXACTION_ACTIVECATCHBLOCK:
				iter->info.action_pointer+=sizeof(ex_activecatchblock); break;
			case EXACTION_SPECIFICATION:
				iter->info.action_pointer+=sizeof(ex_specification)+((ex_specification *)iter->info.action_pointer)->specs*sizeof(void *); break;
			default:
				terminate();	//	error
			}
		}
		action=((ex_destroylocal *)iter->info.action_pointer)->action&EXACTION_MASK;
		if(action==EXACTION_BRANCH)
		{	//	skip to target action--we never return EXACTION_BRANCH to caller!
			iter->info.action_pointer=((char *)iter->info.exception_record)+((ex_branch *)iter->info.action_pointer)->target;
			action=((ex_destroylocal *)iter->info.action_pointer)->action&EXACTION_MASK;
		}
		return action;
	}
}

/************************************************************************/
/* Purpose..: Restore registers											*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to topmost exception record						*/
/* Return...: pointer to return PC										*/
/************************************************************************/
static char *ExPPC_PopStackFrame(ThrowContext *context,MWExceptionInfo *info)
{
	char *SP, *callers_SP;
#ifndef _No_Floating_Point_Regs
	double *FPR_save_area;
#endif
	long *GPR_save_area;
	int saved_GPRs, saved_FPRs;
#if __VEC__
	MWEVector128 *VR_save_area;
	int saved_VRs;
#endif
	int i, j;
	
	//	obtain current and callers frame pointers
	SP=context->SP; callers_SP=*(char **)SP;
	
	//	restore saved FPRs
#ifndef _No_Floating_Point_Regs
	saved_FPRs=ET_GetSavedFPRs(info->exception_record->et_field);
	FPR_save_area=(double *)(callers_SP-saved_FPRs*8);
	for(i=32-saved_FPRs, j=0;i<32;++i,++j) context->FPR[i]=FPR_save_area[j];
#endif

    //  restore saved GPRs
    saved_GPRs=ET_GetSavedGPRs(info->exception_record->et_field);
//	GPR_save_area=(long *)(((long)FPR_save_area&0xFFFFFFF0)-saved_GPRs*4);
//	for(i=32-saved_GPRs, j=0;i<32;++i,++j) context->GPR[i]=GPR_save_area[j];
#ifndef _No_Floating_Point_Regs
    GPR_save_area=(long *)FPR_save_area;
#if !__PPC_EABI__
    if(saved_FPRs&1) GPR_save_area-=2;  // 8-byte gap if # saved FPRs is odd
#endif
    GPR_save_area-=saved_GPRs;
#else
	GPR_save_area = (long *)(callers_SP-saved_GPRs*4);
#endif
    for(i=32-saved_GPRs, j=0;i<32;++i,++j) context->GPR[i]=GPR_save_area[j];

#if __VEC__
	//	restore saved VRs
	if ( ET_HasVectorInfo(info->exception_record->et_field) )
	{
		ExceptionTableSmallVector *etsv = (ExceptionTableSmallVector *) info->exception_record;
		int vrsavesize 					= ET_GetSavedVRSAVE(etsv->et_field) ? sizeof(long) : 0;
    	saved_VRs						= ET_GetSavedVRs(etsv->et_field);

		VR_save_area	=	(MWEVector128 *) GPR_save_area;
		VR_save_area	-=	vrsavesize;
		VR_save_area	-= 	saved_VRs;
		
		VR_save_area	=  (MWEVector128 *) ( (unsigned int) VR_save_area & 0xFFFFFFF0 );		// align to 16-byte boundary

		for(i=32-saved_VRs, j=0;i<32;++i,++j) context->VR[i]=VR_save_area[j];
	}
#endif
	
	//	restore saved CR
#if !__PPC_EABI__ // ignore for now (never saved)
	if(ET_GetSavedCR(info->exception_record->et_field)) context->CR=*(long *)(callers_SP+4);
#endif

	//	restore saved SP (back link)
	context->SP=callers_SP;
	
	//	return new return_addr
	return(*(char **)(callers_SP+RETURN_ADDRESS));
}

/************************************************************************/
/* Purpose..: Unwind ex_destroylocal struct								*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to ex_destroylocal struct							*/
/* Return...: ---														*/
/************************************************************************/
INLINE void ExPPC_DestroyLocal(ThrowContext *context,const ex_destroylocal *ex)
{
	DTORCALL_COMPLETE(ex->dtor,context->FP+ex->local);
}

/************************************************************************/
/* Purpose..: Unwind ex_destroylocalcond struct							*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to ex_destroylocalcond struct						*/
/* Return...: ---														*/
/************************************************************************/
INLINE void ExPPC_DestroyLocalCond(ThrowContext *context,const ex_destroylocalcond *ex)
{
	int cond = ex_destroylocalcond_GetRegCond(ex->dlc_field)	? (local_cond_type) context->GPR[ex->cond]
							: *(local_cond_type *)(context->FP+ex->cond);

	if(cond) DTORCALL_COMPLETE(ex->dtor,context->FP+ex->local);
}

/************************************************************************/
/* Purpose..: Unwind ex_destroylocalpointer struct						*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to ex_destroylocalpointer struct					*/
/* Return...: ---														*/
/************************************************************************/
INLINE void ExPPC_DestroyLocalPointer(ThrowContext *context,const ex_destroylocalpointer *ex)
{
	void *pointer = ex_destroylocalpointer_GetRegPointer(ex->dlp_field)	? (void *) context->GPR[ex->pointer]
									: *(void **)(context->FP+ex->pointer);

	DTORCALL_COMPLETE(ex->dtor,pointer);
}

/************************************************************************/
/* Purpose..: Unwind ex_destroylocalarray struct						*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to ex_destroylocalarray struct					*/
/* Return...: ---														*/
/************************************************************************/
INLINE void ExPPC_DestroyLocalArray(ThrowContext *context,const ex_destroylocalarray *ex)
{
	char *ptr	= context->FP+ex->localarray;
	long n		= ex->elements;
	long size	= ex->element_size;

	for(ptr=ptr+size*n; n>0; n--)
	{
		ptr-=size; DTORCALL_COMPLETE(ex->dtor,ptr);
	}
}

/************************************************************************/
/* Purpose..: Unwind ex_destroymember struct							*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to ex_destroymember struct						*/
/* Return...: ---														*/
/************************************************************************/
INLINE void ExPPC_DestroyMember(ThrowContext *context,const ex_destroymember *ex)
{
	char *objectptr = ex_destroymember_GetRegPointer(ex->dm_field)	? (char *) context->GPR[ex->objectptr]
										: *(char **)(context->FP+ex->objectptr);

	DTORCALL_COMPLETE(ex->dtor,objectptr+ex->offset);
}

/************************************************************************/
/* Purpose..: Unwind ex_destroymember struct							*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to ex_destroymember struct						*/
/* Return...: ---														*/
/************************************************************************/
INLINE void ExPPC_DestroyBase(ThrowContext *context,const ex_destroymember *ex)
{
	char *objectptr = ex_destroymember_GetRegPointer(ex->dm_field)	? (char *) context->GPR[ex->objectptr]
										: *(char **)(context->FP+ex->objectptr);

	DTORCALL_PARTIAL(ex->dtor,objectptr+ex->offset);
}

/************************************************************************/
/* Purpose..: Unwind ex_destroymembercond struct						*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to ex_destroymembercond struct					*/
/* Return...: ---														*/
/************************************************************************/
INLINE void ExPPC_DestroyMemberCond(ThrowContext *context,const ex_destroymembercond *ex)
{
	char *objectptr = ex_destroymembercond_GetRegPointer(ex->dmc_field)	? (char *) context->GPR[ex->objectptr]
										: *(char **)(context->FP+ex->objectptr);
	int cond = ex_destroymembercond_GetRegCond(ex->dmc_field)	? (vbase_ctor_arg_type) context->GPR[ex->cond]
							: *(vbase_ctor_arg_type *)(context->FP+ex->cond);

	if(cond) DTORCALL_PARTIAL(ex->dtor,objectptr+ex->offset);
}

/************************************************************************/
/* Purpose..: Unwind ex_destroymemberarray struct						*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to ex_destroymemberarray struct					*/
/* Return...: ---														*/
/************************************************************************/
INLINE void ExPPC_DestroyMemberArray(ThrowContext *context,const ex_destroymemberarray *ex)
{
	char *ptr	= ex_destroymemberarray_GetRegPointer(ex->dma_field)	? (char *) context->GPR[ex->objectptr]
									: *(char **)(context->FP+ex->objectptr);
	long n		= ex->elements;
	long size	= ex->element_size;

	// msw 11/28/97 -	Bug fix.
	ptr += ex->offset;

	for(ptr=ptr+size*n; n>0; n--)
	{
		ptr-=size; DTORCALL_COMPLETE(ex->dtor,ptr);
	}
}

/************************************************************************/
/* Purpose..: Unwind ex_deletepointer struct							*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to ex_deletepointer struct						*/
/* Return...: ---														*/
/************************************************************************/
INLINE void ExPPC_DeletePointer(ThrowContext *context,const ex_deletepointer *ex)
{
	char *objectptr = ex_deletepointer_GetRegPointer(ex->dp_field)	? (char *) context->GPR[ex->objectptr]
										: *(char **)(context->FP+ex->objectptr);

	((DeleteFunc) ex->deletefunc)(objectptr);
}

/************************************************************************/
/* Purpose..: Unwind ex_deletepointercond struct						*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to ex_deletepointercond struct					*/
/* Return...: ---														*/
/************************************************************************/
INLINE void ExPPC_DeletePointerCond(ThrowContext *context,const ex_deletepointercond *ex)
{
	char *objectptr = ex_deletepointercond_GetRegPointer(ex->dpc_field) ? (char *) context->GPR[ex->objectptr]
										: *(char **)(context->FP+ex->objectptr);
	int cond = ex_deletepointercond_GetRegCond(ex->dpc_field)	? (local_cond_type) context->GPR[ex->cond]
							: *(local_cond_type *)(context->FP+ex->cond);

	if(cond) ((DeleteFunc) ex->deletefunc)(objectptr);
}

/************************************************************************/
/* Purpose..: Unwind stack												*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to topmost action									*/
/* Input....: pointer to catcher										*/
/* Return...: ---														*/
/************************************************************************/
static void ExPPC_UnwindStack(ThrowContext *context,MWExceptionInfo *info,void *catcher)
{
	exaction_type	action;

#pragma exception_terminate		//	this will prevent exception exits during unwindind

	for(;;)
	{
		if(info->action_pointer==0)
		{
			char *return_addr;
	
			return_addr=ExPPC_PopStackFrame(context,info);
			ExPPC_FindExceptionRecord(return_addr,info);
			if(info->exception_record==0) terminate();		//	cannot find matching exception record
			context->FP=(ET_GetHasFramePtr(info->exception_record->et_field))?(char *)context->GPR[31]:context->SP;
			continue;
		}

		action=((ex_destroylocal *)info->action_pointer)->action;
		switch(action&EXACTION_MASK)
		{
		case EXACTION_BRANCH:
			info->action_pointer=((char *)info->exception_record)+((ex_branch *)info->action_pointer)->target; break;
		
		case EXACTION_DESTROYLOCAL:
			ExPPC_DestroyLocal(context,(ex_destroylocal *)info->action_pointer);
			info->action_pointer+=sizeof(ex_destroylocal); break;

		case EXACTION_DESTROYLOCALCOND:
			ExPPC_DestroyLocalCond(context,(ex_destroylocalcond *)info->action_pointer);
			info->action_pointer+=sizeof(ex_destroylocalcond); break;

		case EXACTION_DESTROYLOCALPOINTER:
			ExPPC_DestroyLocalPointer(context,(ex_destroylocalpointer *)info->action_pointer);
			info->action_pointer+=sizeof(ex_destroylocalpointer); break;

		case EXACTION_DESTROYLOCALARRAY:
			ExPPC_DestroyLocalArray(context,(ex_destroylocalarray *)info->action_pointer);
			info->action_pointer+=sizeof(ex_destroylocalarray); break;

		case EXACTION_DESTROYBASE:
			ExPPC_DestroyBase(context,(ex_destroymember *)info->action_pointer);
			info->action_pointer+=sizeof(ex_destroymember); break;

		case EXACTION_DESTROYMEMBER:
			ExPPC_DestroyMember(context,(ex_destroymember *)info->action_pointer);
			info->action_pointer+=sizeof(ex_destroymember); break;

		case EXACTION_DESTROYMEMBERCOND:
			ExPPC_DestroyMemberCond(context,(ex_destroymembercond *)info->action_pointer);
			info->action_pointer+=sizeof(ex_destroymembercond); break;

		case EXACTION_DESTROYMEMBERARRAY:
			ExPPC_DestroyMemberArray(context,(ex_destroymemberarray *)info->action_pointer);
			info->action_pointer+=sizeof(ex_destroymemberarray); break;

		case EXACTION_DELETEPOINTER:
			ExPPC_DeletePointer(context,(ex_deletepointer *)info->action_pointer);
			info->action_pointer+=sizeof(ex_deletepointer); break;

		case EXACTION_DELETEPOINTERCOND:
			ExPPC_DeletePointerCond(context,(ex_deletepointercond *)info->action_pointer);
			info->action_pointer+=sizeof(ex_deletepointercond); break;

		case EXACTION_CATCHBLOCK:
			if(catcher==(void *)info->action_pointer) return;	//	finished unwinding
			info->action_pointer+=sizeof(ex_catchblock); break;

		case EXACTION_CATCHBLOCK_32:
			if(catcher==(void *)info->action_pointer) return;	//	finished unwinding
			info->action_pointer+=sizeof(ex_catchblock_32); break;

		case EXACTION_ACTIVECATCHBLOCK:
			{
				CatchInfo		*catchinfo;

				catchinfo=(CatchInfo *)(context->FP+((ex_activecatchblock *)info->action_pointer)->cinfo_ref);
            	if ( catchinfo->dtor )
           		{
                	// re-throwing exception from this block
                	if ( context->location == catchinfo->location )
                	{
                    	// pass destruction responsibility to re-thrown exception
                    	context->dtor = catchinfo->dtor;
                	}
                	else
                	{
                    	DTORCALL_COMPLETE(catchinfo->dtor,catchinfo->location);
                	}
				}
				info->action_pointer+=sizeof(ex_activecatchblock);
			}
			break;

		case EXACTION_SPECIFICATION:
			if(catcher==(void *)info->action_pointer) return;	//	finished unwinding
			info->action_pointer+=sizeof(ex_specification)+((ex_specification *)info->action_pointer)->specs*sizeof(void *);
			break;

		default:
			terminate();			//	error
		}
		if(action&EXACTION_ENDBIT) info->action_pointer=0;
	}
}

/************************************************************************/
/* Purpose..: Check if an exception is in a specification list			*/
/* Input....: pointer to exception type string							*/
/* Input....: pointer to specification list								*/
/* Return...: ---														*/
/************************************************************************/
static int ExPPC_IsInSpecification(char *extype,ex_specification *spec)
{
	long	i,offset;

	for(i=0; i<spec->specs; i++)
	{
		if(__throw_catch_compare(extype,spec->spec[i],&offset)) return 1;
	}
	return 0;
}

/************************************************************************/
/* Purpose..: Unexpected handler										*/
/* Input....: pointer to throw context									*/
/* Return...: --- (this function will never return)						*/
/************************************************************************/
extern void __unexpected(CatchInfo* catchinfo)
{	
	ex_specification *unexp=(ex_specification *)catchinfo->stacktop;

#pragma exception_magic

	try {
		unexpected();
	}
	catch(...)
	{	//	unexpected throws an exception => check if the exception matches the specification
		if(ExPPC_IsInSpecification((char *)((CatchInfo *)&__exception_magic)->typeinfo,unexp))
		{	//	new exception is in specification list => rethrow
			throw;
		}
		if(ExPPC_IsInSpecification("!bad_exception!!",unexp))
		{	//	"bad_exception" is in specification list => throw bad_exception()
			throw bad_exception();
		}
		if(ExPPC_IsInSpecification("!std::bad_exception!!",unexp))
		{	//	"bad_exception" is in specification list => throw bad_exception()
			throw bad_exception();
		}
		//	cannot handle exception => terminate();
	}
	terminate();
}

/************************************************************************/
/* Purpose..: Restore registers and branch to catcher					*/
/* Input....: pointer to throw context									*/
/* Input....: destination RTOC											*/
/* Input....: destination PC											*/
/* Return...: ---														*/
/************************************************************************/
static asm void ExPPC_LongJump(register ThrowContext *context, register void *newRTOC, register void *newPC)
{
#if __ALTIVEC__ || __PPC_EABI__
		nofralloc
#endif

		mr			r8,newPC	
		mr			RTOC,newRTOC		//	restore RTOC
		lwz			r0,context->CR		//	restore CR
		mtcrf		255,r0

#if __VEC__
		lwz			r5,context->vrsave	
		bl			__setLR				// set LR to address of setLR				
__setLR:	

		oris	 	r5,r5,0x8000		// indicate vr0 also in-use
		mtvrsave 	r5
		clrlwi		r5,r5,20			// clear left 20 bits
		cntlzw		r5,r5				// find first nonvolatile VR used
		slwi		r5,r5,3				// multiply by 8 for index past __longjmpv20
		mflr		r6
		
		addi		r6,r6,__longjmpv20 - __setLR - (20 * 8)
		add			r6,r6,r5
		mtlr		r6
		blr								// jump to index past __longjmpv20
		
__longjmpv20:		
		la			r31,context->VR[20]
		lvx			vr20,r0,r31
		la			r31,context->VR[21]
		lvx			vr21,r0,r31
		la			r31,context->VR[22]
		lvx			vr22,r0,r31
		la			r31,context->VR[23]
		lvx			vr23,r0,r31
		la			r31,context->VR[24]
		lvx			vr24,r0,r31
		la			r31,context->VR[25]
		lvx			vr25,r0,r31
		la			r31,context->VR[26]
		lvx			vr26,r0,r31
		la			r31,context->VR[27]
		lvx			vr27,r0,r31
		la			r31,context->VR[28]
		lvx			vr28,r0,r31
		la			r31,context->VR[29]
		lvx			vr29,r0,r31
		la			r31,context->VR[30]
		lvx			vr30,r0,r31
		la			r31,context->VR[31]
		lvx			vr31,r0,r31
#endif

		//	restore R13-R31
#if __PPC_EABI__
	#if !__option(use_lmw_stmw)
		lwz			r14,context->GPR[14]
		lwz			r15,context->GPR[15]
		lwz			r16,context->GPR[16]
		lwz			r17,context->GPR[17]
		lwz			r18,context->GPR[18]
		lwz			r19,context->GPR[19]
		lwz			r20,context->GPR[20]
		lwz			r21,context->GPR[21]
		lwz			r22,context->GPR[22]
		lwz			r23,context->GPR[23]
		lwz			r24,context->GPR[24]
		lwz			r25,context->GPR[25]
		lwz			r26,context->GPR[26]
		lwz			r27,context->GPR[27]
		lwz			r28,context->GPR[28]
		lwz			r29,context->GPR[29]
		lwz			r30,context->GPR[30]
		lwz			r31,context->GPR[31]
	#else
		lmw			r13,context->GPR[13]
	#endif
#else
		lmw			r13,context->GPR[13]
#endif

#ifndef _No_Floating_Point_Regs
		//	restore FP14-FP31
		lfd			fp14,context->FPR[14]
		lfd			fp15,context->FPR[15]
		lfd			fp16,context->FPR[16]
		lfd			fp17,context->FPR[17]
		lfd			fp18,context->FPR[18]
		lfd			fp19,context->FPR[19]
		lfd			fp20,context->FPR[20]
		lfd			fp21,context->FPR[21]
		lfd			fp22,context->FPR[22]
		lfd			fp23,context->FPR[23]
		lfd			fp24,context->FPR[24]
		lfd			fp25,context->FPR[25]
		lfd			fp26,context->FPR[26]
		lfd			fp27,context->FPR[27]
		lfd			fp28,context->FPR[28]
		lfd			fp29,context->FPR[29]
		lfd			fp30,context->FPR[30]
		lfd			fp31,context->FPR[31]
#endif

		
		mtlr		r8						//	restore PC (LR)

#if __VEC__
		la			r5,context->vscr
		lvewx		vr0,r0,r5				// load 32 bits into lo 32 bits of v0
		mtvscr		vr0
		
		lwz			r5,context->vrsave		// restore vrsave, remove usage of bit 0
		mtvrsave 	r5
#endif

		//	restore SP to stack top at throw: discards exception-handling frames
		//	but not exception temporaries
		lwz			SP,context->throwSP
		//	move stack frame back-link from catcher's linkage area to new top of
		//	stack; this effectively pops all intermediate frames.
		lwz			r3,context->SP
		lwz			r3,0(r3)
		stw			r3,0(SP)
		//	jump to exception handler
		blr
}

/************************************************************************/
/* Purpose..: Handle unexpected exception								*/
/* Input....: pointer to throw context									*/
/* Input....: pointer to topmost MWExceptionInfo struct					*/
/* Input....: pointer to specification record							*/
/* Return...: ---														*/
/************************************************************************/
static void ExPPC_HandleUnexpected(ThrowContext *context,MWExceptionInfo *info,ex_specification *unexp)
{
	CatchInfo	*catchinfo;

#pragma exception_terminate		//	this will prevent exception exits during unwinding

	ExPPC_UnwindStack(context,info,unexp);	//	unwind stack to failing specification

	//	initialize catch info struct
	catchinfo=(CatchInfo *)(context->FP+unexp->cinfo_ref);
	catchinfo->location		= context->location;
	catchinfo->typeinfo		= context->throwtype;
	catchinfo->dtor			= context->dtor;
	catchinfo->stacktop		= unexp;		//	the __unexpected will never call __end_catch
											//	so we can resue this field
	//	jump to exception handler
	ExPPC_LongJump(context,info->TOC,info->current_function+unexp->pcoffset);
}

/************************************************************************/
/* Purpose..: Throw (rethrow) current exception							*/
/* Input....: pointer to throw context									*/
/* Return...: ---														*/
/************************************************************************/
static void ExPPC_ThrowHandler(ThrowContext *context)
{
	ActionIterator	iter;
	MWExceptionInfo	info;
	exaction_type	action;
	CatchInfo		*catchinfo;
	long			offset;

	//	find first ExceptionRecord
	
	ExPPC_FindExceptionRecord(context->returnaddr, &info);
	if(info.exception_record==0) terminate();	//	cannot find matching exception record
	context->FP=(ET_GetHasFramePtr(info.exception_record->et_field))?(char *)context->GPR[31]:context->SP;

	if(context->throwtype==0)
	{	//	rethrow, find most recent exception
		iter.info		= info;
		iter.current_SP	= context->SP;
		iter.current_FP	= context->FP;
		iter.current_R31 = context->GPR[31];
		for(action=ExPPC_CurrentAction(&iter);; action=ExPPC_NextAction(&iter))
		{
			switch(action)
			{
			case EXACTION_ACTIVECATCHBLOCK:
				break;
	
			case EXACTION_ENDOFLIST:
			case EXACTION_DESTROYLOCAL:
			case EXACTION_DESTROYLOCALCOND:
			case EXACTION_DESTROYLOCALPOINTER:
			case EXACTION_DESTROYLOCALARRAY:
			case EXACTION_DESTROYBASE:
			case EXACTION_DESTROYMEMBER:
			case EXACTION_DESTROYMEMBERCOND:
			case EXACTION_DESTROYMEMBERARRAY:
			case EXACTION_DELETEPOINTER:
			case EXACTION_DELETEPOINTERCOND:
			case EXACTION_CATCHBLOCK:
			case EXACTION_CATCHBLOCK_32:
			case EXACTION_SPECIFICATION:
				continue;

			case EXACTION_TERMINATE:
			default:
				terminate();			//	cannot find find most recent exception
			}
			break;
		}
		catchinfo=(CatchInfo *)(iter.current_FP+((ex_activecatchblock *)iter.info.action_pointer)->cinfo_ref);
		context->throwtype	= (char *)catchinfo->typeinfo;
		context->location	= catchinfo->location;
		context->dtor		= 0;
		// original active catch block is still responsible for destruction
		context->catchinfo	= catchinfo;
	}
	else context->catchinfo=0L;

	//	find matching exception handler

	iter.info		= info;
	iter.current_SP	= context->SP;
	iter.current_FP	= context->FP;
	iter.current_R31 = context->GPR[31];
	for(action=ExPPC_CurrentAction(&iter);; action=ExPPC_NextAction(&iter))
	{
		switch(action)
		{
		case EXACTION_CATCHBLOCK_32:
			if(__throw_catch_compare(context->throwtype,((ex_catchblock_32 *)iter.info.action_pointer)->catch_type,&offset))
			{
				break;
			}
			continue;
		case EXACTION_CATCHBLOCK:
			if(__throw_catch_compare(context->throwtype,((ex_catchblock *)iter.info.action_pointer)->catch_type,&offset))
			{
				break;
			}
			continue;

		case EXACTION_SPECIFICATION:
			if(!ExPPC_IsInSpecification(context->throwtype,(ex_specification *)iter.info.action_pointer))
			{	//	unexpected specification
				ExPPC_HandleUnexpected(context,&info,(ex_specification *)iter.info.action_pointer);
				//	we will never return from this function call
			}
			continue;

		case EXACTION_ENDOFLIST:
		case EXACTION_DESTROYLOCAL:
		case EXACTION_DESTROYLOCALCOND:
		case EXACTION_DESTROYLOCALPOINTER:
		case EXACTION_DESTROYLOCALARRAY:
		case EXACTION_DESTROYBASE:
		case EXACTION_DESTROYMEMBER:
		case EXACTION_DESTROYMEMBERCOND:
		case EXACTION_DESTROYMEMBERARRAY:
		case EXACTION_DELETEPOINTER:
		case EXACTION_DELETEPOINTERCOND:
		case EXACTION_ACTIVECATCHBLOCK:
			continue;

		case EXACTION_TERMINATE:
		default:
			terminate();			//	cannot find matching catch block
		}
		break;
	}

	//	we have found a matching catch block
	if (action == EXACTION_CATCHBLOCK_32) {
		ex_catchblock_32	*catchblock_32;
		catchblock_32=(ex_catchblock_32 *)iter.info.action_pointer;

		ExPPC_UnwindStack(context,&info,catchblock_32);

		//	initialize catch info struct
		catchinfo=(CatchInfo *)(context->FP+catchblock_32->cinfo_ref);
		catchinfo->location		= context->location;
		catchinfo->typeinfo		= context->throwtype;
		catchinfo->dtor			= context->dtor;

		if(*context->throwtype=='*')
		{
			//	pointer match (create a pointer copy with adjusted offset)
			catchinfo->sublocation	= &catchinfo->pointercopy;
			catchinfo->pointercopy	= *(long *)context->location+offset;
		}
		else
		{	//	traditional or class match (directly adjust offset)
			catchinfo->sublocation	= (char *)context->location+offset;
		}

		//	remember eventual stacktop (restored from catchinfo at end of catch block)
		//	catchinfo->stacktop = context->SP;	//	saved at try { ... } instead

		//	jump to exception handler
		ExPPC_LongJump(context,info.TOC,info.current_function+catchblock_32->catch_pcoffset);
	} else {
		ex_catchblock	*catchblock;
	
		catchblock=(ex_catchblock *)iter.info.action_pointer;
		ExPPC_UnwindStack(context,&info,catchblock);
	
		//	initialize catch info struct
		catchinfo=(CatchInfo *)(context->FP+catchblock->cinfo_ref);
		catchinfo->location		= context->location;
		catchinfo->typeinfo		= context->throwtype;
		catchinfo->dtor			= context->dtor;
		if(*context->throwtype=='*')
		{	//	pointer match (create a pointer copy with adjusted offset)
			catchinfo->sublocation	= &catchinfo->pointercopy;
			catchinfo->pointercopy	= *(long *)context->location+offset;
		}
		else
		{	//	traditional or class match (directly adjust offset)
			catchinfo->sublocation	= (char *)context->location+offset;
		}
	
		//	remember eventual stacktop (restored from catchinfo at end of catch block)
	//	catchinfo->stacktop = context->SP;	//	saved at try { ... } instead
	
		//	jump to exception handler
		ExPPC_LongJump(context,info.TOC,info.current_function+catchblock->catch_pcoffset);
	}
}


#if __VEC__
#pragma altivec_vrsave off

/************************************************************************/
/* Purpose..: Throw (rethrow) current exception							*/
/* Input....: pointer to throw type (0L: rethrow)						*/
/* Input....: pointer to complete throw object (0L: rethrow)			*/
/* Input....: pointer to throw object destructor (0L: no destructor		*/
/* Return...: ---														*/
/************************************************************************/
asm void __throw(char *throwtype, void *location, void *dtor)
{
		ThrowContext 	throwcontext;
			
		nofralloc						// user is responsible for stack frame

//
// PROLOGUE
//
		mflr		r0
		mr			r11,r30				// save high 2 nonvolatiles in r11 and r12
		stw			r31,-4(sp)
		mr			r12,r31
		stw			r30,-8(sp)
		stw			r0,8(sp)
		stw			r3,24(sp)
		stw			r4,28(sp)

		mr			r30,sp				// save callerSP in r30
		stw			r5,32(sp)

		clrrwi		sp,sp,4				// clear the right 4 bits to 16-byte align new frame

										// frame size calculated by:
										// sizeof(ThrowContext) 
										// + 24 bytes for linkage area 
										// + 8 for 16-byte alignment
										// + 32 for parameter area size
										
		stwu		r30,-(sizeof(ThrowContext) + 32 + 24 + 8) (SP)

//
// MAIN FUNCTION BODY
//
		addi		r31,sp,64			// offset of throwcontext calculated by:
										// SP + 24 bytes for linkage area + 8 for 16-byte alignment
										//    + 32 for parameter area size
		

		stmw		r13,ThrowContext.GPR[13](r31)
		stw			r11,ThrowContext.GPR[30](r31)	// restore register state as it was on entry to this routine
		stw			r12,ThrowContext.GPR[31](r31)
		
		mfvrsave 	r5
		stw			r5,ThrowContext.vrsave(r31)

		bl			__setLR				// set LR to address of setLR				
__setLR:	

		stw			r5,ThrowContext.vrsave(r31)
		clrlwi		r5,r5,20			// clear left 20 bits
		cntlzw		r5,r5				// find first nonvolatile VR used
		slwi		r5,r5,3				// multiply by 8 for index past __setjmpv20
		mflr		r4
		
		addi		r4,r4,__setjmpv20 - __setLR	- (20 * 8)
		add			r4,r4,r5
		mtlr		r4
		
		blr								// jump to index past __setjmpv20
		
__setjmpv20:		
		li 			r3,ThrowContext.VR[20]
		stvx		vr20,r3,r31
		li 			r3,ThrowContext.VR[21]
		stvx		vr21,r3,r31
		li 			r3,ThrowContext.VR[22]
		stvx		vr22,r3,r31
		li 			r3,ThrowContext.VR[23]
		stvx		vr23,r3,r31
		li 			r3,ThrowContext.VR[24]
		stvx		vr24,r3,r31
		li 			r3,ThrowContext.VR[25]
		stvx		vr25,r3,r31
		li 			r3,ThrowContext.VR[26]
		stvx		vr26,r3,r31
		li 			r3,ThrowContext.VR[27]
		stvx		vr27,r3,r31
		li 			r3,ThrowContext.VR[28]
		stvx		vr28,r3,r31
		li 			r3,ThrowContext.VR[29]
		stvx		vr29,r3,r31
		li 			r3,ThrowContext.VR[30]
		stvx		vr30,r3,r31
		li 			r3,ThrowContext.VR[31]
		stvx		vr31,r3,r31

		mfvrsave 	r6
		oris	 	r6,r6,0x8000		// indicate vr0 in-use
		mtvrsave 	r6

		mfvscr		vr0					// mfvscr is context-synchronizing, so this will take many cycles
		la 			r5,ThrowContext.vscr(r31)
		stvewx		vr0,r0,r5			// store 32 bits out of lo 32 bits of v0

		li			r5,0x0000			// indicate that no vector registers are being used now
		mtvrsave	r5					// This is OK because we just saved the non-volatile set and exercised our right
										// to trash all the volatile vector registers


		//	save FPRs so we can restore them during stack unwind
#ifndef _No_Floating_Point_Regs
		stfd		fp14,ThrowContext.FPR[14](r31)
		stfd		fp15,ThrowContext.FPR[15](r31)
		stfd		fp16,ThrowContext.FPR[16](r31)
		stfd		fp17,ThrowContext.FPR[17](r31)
		stfd		fp18,ThrowContext.FPR[18](r31)
		stfd		fp19,ThrowContext.FPR[19](r31)
		stfd		fp20,ThrowContext.FPR[20](r31)
		stfd		fp21,ThrowContext.FPR[21](r31)
		stfd		fp22,ThrowContext.FPR[22](r31)
		stfd		fp23,ThrowContext.FPR[23](r31)
		stfd		fp24,ThrowContext.FPR[24](r31)
		stfd		fp25,ThrowContext.FPR[25](r31)
		stfd		fp26,ThrowContext.FPR[26](r31)
		stfd		fp27,ThrowContext.FPR[27](r31)
		stfd		fp28,ThrowContext.FPR[28](r31)
		stfd		fp29,ThrowContext.FPR[29](r31)
		stfd		fp30,ThrowContext.FPR[30](r31)
		stfd		fp31,ThrowContext.FPR[31](r31)
#endif
		//	save CR so we can restore it during stack unwind
		mfcr		r3
		stw			r3,ThrowContext.CR(r31);
		//	tcp->SP = tcp->throwSP = <stack pointer of caller>;
		//	tcp->returnaddr = <return address into caller>;
		lwz			r3,0(sp)
		lwz			r4,8(r3)
		stw			r3,ThrowContext.SP(r31);
		stw			r3,ThrowContext.throwSP(r31);
		stw			r4,ThrowContext.returnaddr(r31);
		//	tcp->throwtype = throwtype;
		lwz			r3,throwtype(r30)	// using callerSP
		stw			r3,ThrowContext.throwtype(r31)
		//	tcp->location = location;
		lwz			r3,location(r30)	// using callerSP
		stw			r3,ThrowContext.location(r31)
		//	tcp->dtor = dtor;
		lwz			r3,dtor(r30)		// using callerSP
		stw			r3,ThrowContext.dtor(r31)
		//	call __ex_throwhandler(&throwcontext);

		mr			r3,r31
		bl			ExPPC_ThrowHandler
		nop

//
// EPILOGUE
//		
		lwz			r0,8(r30)			// fetch old LR from linkage area
		mr			sp,r30				// restore caller's SP from nonvolatile r30
		mtlr		r0					// restore LR so we can return
		lwz			r31,-4(sp)			// restore nonvolatile r30 and r31 callerSP-relative
		lwz			r30,-8(sp)
		blr
}

#pragma debug_listing  off
#pragma altivec_vrsave on

#else

/************************************************************************/
/* Purpose..: Throw (rethrow) current exception							*/
/* Input....: pointer to throw type (0L: rethrow)						*/
/* Input....: pointer to complete throw object (0L: rethrow)			*/
/* Input....: pointer to throw object destructor (0L: no destructor		*/
/* Return...: ---														*/
/************************************************************************/
asm void __throw(char *throwtype, void *location, void *dtor)
{
		ThrowContext throwcontext;
		
		//	allocate a stack frame so we can use symbolic access to locals
		fralloc
		
		//	save GPRs so we can restore them during stack unwind
#if __PPC_EABI__
	#if !__option(use_lmw_stmw)
		stw			r14,throwcontext.GPR[14]
		stw			r15,throwcontext.GPR[15]
		stw			r16,throwcontext.GPR[16]
		stw			r17,throwcontext.GPR[17]
		stw			r18,throwcontext.GPR[18]
		stw			r19,throwcontext.GPR[19]
		stw			r20,throwcontext.GPR[20]
		stw			r21,throwcontext.GPR[21]
		stw			r22,throwcontext.GPR[22]
		stw			r23,throwcontext.GPR[23]
		stw			r24,throwcontext.GPR[24]
		stw			r25,throwcontext.GPR[25]
		stw			r26,throwcontext.GPR[26]
		stw			r27,throwcontext.GPR[27]
		stw			r28,throwcontext.GPR[28]
		stw			r29,throwcontext.GPR[29]
		stw			r30,throwcontext.GPR[30]
		stw			r31,throwcontext.GPR[31]
	#else
		stmw		r13,throwcontext.GPR[13]
	#endif
#else
		stmw		r13,throwcontext.GPR[13]
#endif

#ifndef _No_Floating_Point_Regs
		//	save FPRs so we can restore them during stack unwind
		stfd		fp14,throwcontext.FPR[14]
		stfd		fp15,throwcontext.FPR[15]
		stfd		fp16,throwcontext.FPR[16]
		stfd		fp17,throwcontext.FPR[17]
		stfd		fp18,throwcontext.FPR[18]
		stfd		fp19,throwcontext.FPR[19]
		stfd		fp20,throwcontext.FPR[20]
		stfd		fp21,throwcontext.FPR[21]
		stfd		fp22,throwcontext.FPR[22]
		stfd		fp23,throwcontext.FPR[23]
		stfd		fp24,throwcontext.FPR[24]
		stfd		fp25,throwcontext.FPR[25]
		stfd		fp26,throwcontext.FPR[26]
		stfd		fp27,throwcontext.FPR[27]
		stfd		fp28,throwcontext.FPR[28]
		stfd		fp29,throwcontext.FPR[29]
		stfd		fp30,throwcontext.FPR[30]
		stfd		fp31,throwcontext.FPR[31]
#endif

		//	save CR so we can restore it during stack unwind
		mfcr		r3
		stw			r3,throwcontext.CR;
		//	throwcontext.SP = throwcontext.throwSP = <stack pointer of caller>;
		//	throwcontext.returnaddr = <return address into caller>;
		lwz			r3,0(sp)
		lwz			r4,RETURN_ADDRESS(r3)
		stw			r3,throwcontext.SP;
		stw			r3,throwcontext.throwSP;
		stw			r4,throwcontext.returnaddr;
		//	throwcontext.throwtype = throwtype;
		lwz			r3,throwtype
		stw			r3,throwcontext.throwtype
		//	throwcontext.location = location;
		lwz			r3,location
		stw			r3,throwcontext.location
		//	throwcontext.dtor = dtor;
		lwz			r3,dtor
		stw			r3,throwcontext.dtor
		//	call __ex_throwhandler(&throwcontext);

		la			r3,throwcontext
		bl			ExPPC_ThrowHandler
		nop
		//	(will never get here)
		frfree
		blr
}

#endif


/************************************************************************/
/* Purpose..: Deinitialize CatchInfo struct								*/
/* Input....: pointer to catchinfo struct								*/
/* Return...: ---														*/
/************************************************************************/
void __end__catch(CatchInfo *catchinfo)
{
	if (catchinfo->location && catchinfo->dtor)
		DTORCALL_COMPLETE(catchinfo->dtor,catchinfo->location);
}

