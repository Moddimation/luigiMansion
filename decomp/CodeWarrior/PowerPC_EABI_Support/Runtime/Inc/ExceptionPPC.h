
/*
 *  ExceptionPPC.h	-	C++ Exception Table Format for CodeWarrior(PowerPC)
 *
 *  Copyright ゥ 1995-1999 Metrowerks, Inc.  All rights reserved.
 *
 */

#ifndef __EXCEPTIONPPC_H__
#define __EXCEPTIONPPC_H__

#if __MWERKS__
#pragma ANSI_strict off
#if __PPC_EABI__
#pragma options align=native
#define _32BIT_EXCEPTION_TABLES 1
#else
#pragma options align=mac68k
#endif
#endif

//	Exception Table Actions

typedef unsigned char exaction_type;

#define EXACTION_ENDBIT					0x80	//	action table end bit
#define EXACTION_MASK					0x7F	//	action mask

#define	EXACTION_ENDOFLIST				0

#define	EXACTION_BRANCH					1
typedef struct ex_branch {
	exaction_type	action;				//	EXACTION_BRANCH
	unsigned char	unused;				//	(reserved for future use)
	unsigned short	target;				//	exception-table-relative offset of branch target
} ex_branch;

#define EXACTION_DESTROYLOCAL			2
typedef struct ex_destroylocal {
	exaction_type	action;				//	EXACTION_DESTROYLOCAL
	unsigned char	unused;				//	(reserved for future use)
	short			local;				//	16-bit frame-relative offset to local variable
	void*			dtor;				//	absolute pointer to TVector of destructor
} ex_destroylocal;

#define EXACTION_DESTROYLOCALCOND		3
typedef struct ex_destroylocalcond {
	exaction_type	action;				//	EXACTION_DESTROYLOCALCOND
		/*   0        1                  7   */
		/*  +----------------------------+  */
		/*  | regcond | unused           |  */
		/*  +----------------------------+  */
	unsigned char   dlc_field;			//	regcond	0 bit (1 -> condition flag is in a GPR)
										//	unused 		1:7 (reserved for future use)
	short			cond;				//	16-bit frame-relative offset or GPR# of condition flag
	short			local;				//	16-bit frame-relative offset to local variable
	void*			dtor;				//	absolute pointer to TVector of destructor
} ex_destroylocalcond;

#define ex_destroylocalcond_MakeField(regcond)		(((regcond)<<7))
#define ex_destroylocalcond_GetRegCond(field)		((field)>>7)

#define EXACTION_DESTROYLOCALPOINTER	4
typedef struct ex_destroylocalpointer {
	exaction_type	action;				//	EXACTION_DESTROYLOCALPOINTER
		/*   0           1                  7   */
		/*  +--------------------------------+  */
		/*  | regpointer | unused            |  */
		/*  +--------------------------------+  */
	unsigned char   dlp_field;			//	regpointer	0 bit (1 -> object pointer is in a GPR)
										//	unused 		1:7 (reserved for future use)
	short			pointer;			//	16-bit frame-relative offset or GPR# of local pointer
	void*			dtor;				//	absolute pointer to TVector of destructor
} ex_destroylocalpointer;

#define ex_destroylocalpointer_MakeField(regpointer)	(((regpointer)<<7))
#define ex_destroylocalpointer_GetRegPointer(field)		((field)>>7)

#define EXACTION_DESTROYLOCALARRAY		5
typedef struct ex_destroylocalarray {
	exaction_type	action;				//	EXACTION_DESTROYLOCALARRAY
	unsigned char	unused;				//	(reserved for future use)
	short			localarray;			//	16-bit frame-relative offset to local array
	unsigned short	elements;			//	number of array elements
	unsigned short	element_size;		//	size of one array element
	void*			dtor;				//	absolute pointer to TVector of destructor
} ex_destroylocalarray;

#define EXACTION_DESTROYBASE			6
#define EXACTION_DESTROYMEMBER			7
typedef struct ex_destroymember {
	exaction_type	action;				//	EXACTION_DESTROYMEMBER or EXACTION_DESTROYBASE
		/*   0           1                  7   */
		/*  +--------------------------------+  */
		/*  | regpointer | unused            |  */
		/*  +--------------------------------+  */
	unsigned char   dm_field;			//	regpointer	0 bit (1 -> object pointer is in a GPR)
										//	unused 		1:7 (reserved for future use)
	short			objectptr;			//	16-bit frame-relative offset or GPR# of object pointer
	long			offset;				//	offset of member in complete object
	void*			dtor;				//	absolute pointer to TVector of destructor
} ex_destroymember;

#define ex_destroymember_MakeField(regpointer)		(((regpointer)<<7))
#define ex_destroymember_GetRegPointer(field)		((field)>>7)


#define EXACTION_DESTROYMEMBERCOND		8
typedef struct ex_destroymembercond {
	exaction_type	action;				//	EXACTION_DESTROYMEMBERCOND
		/*   0        1            2         7 	*/
		/*  +--------------------------------+	*/
		/*  | regcond | regpointer | unused  |	*/
		/*  +--------------------------------+	*/
	unsigned char	dmc_field;			//	regcond 	0 (1 -> condition flag is in a GPR)
										//	regpointer 	1 (1 -> object pointer is in a GPR)
										//	unused	 	2:7 (reserved for future use)
	short			cond;				//	16-bit frame-relative offset or GPR# of condition flag
	short			objectptr;			//	16-bit frame-relative offset or GPR# of object pointer
	long			offset;				//	offset of member in complete object
	void*			dtor;				//	absolute pointer to TVector of destructor
} ex_destroymembercond;

#define ex_destroymembercond_MakeField(regcond,regpointer)	\
			(((regcond)<<7)|(((regpointer)&0x0001)<<6))
#define ex_destroymembercond_GetRegCond(field)			((field)>>7)
#define ex_destroymembercond_GetRegPointer(field)		(((field)>>6)&0x0001)

#define EXACTION_DESTROYMEMBERARRAY		9
typedef struct ex_destroymemberarray {
	exaction_type	action;				//	EXACTION_DESTROYMEMBERARRAY
		/*   0           1                  7   */
		/*  +--------------------------------+  */
		/*  | regpointer | unused            |  */
		/*  +--------------------------------+  */
	unsigned char   dma_field;			//	regpointer	0 bit (1 -> object pointer is in a GPR)
										//	unused 		1:7 (reserved for future use)
	short			objectptr;			//	16-bit frame-relative offset or GPR# of object pointer
	long			offset;				//	offset of member in complete object
	long			elements;			//	number of array elements
	long			element_size;		//	size of one array element
	void*			dtor;				//	absolute pointer to TVector of destructor
} ex_destroymemberarray;

#define ex_destroymemberarray_MakeField(regpointer)		(((regpointer)<<7))
#define ex_destroymemberarray_GetRegPointer(field)		((field)>>7)

#define EXACTION_DELETEPOINTER			10
typedef struct ex_deletepointer {
	exaction_type	action;				//	EXACTION_DELETEPOINTER
		/*   0           1                   7  */
		/*  +--------------------------------+  */
		/*  | regpointer | unused            |  */
		/*  +--------------------------------+  */
	unsigned char   dp_field;			// regpointer	0 bit (1 -> object pointer is in a GPR)
										// unused 		1:7 (reserved for future use)
	short			objectptr;			//	16-bit frame-relative offset or GPR# of object pointer
	void*			deletefunc;			//	absolute pointer to TVector of deletefunc
} ex_deletepointer;

#define ex_deletepointer_MakeField(regpointer)		(((regpointer)<<7))
#define ex_deletepointer_GetRegPointer(field)		((field)>>7)

#define EXACTION_DELETEPOINTERCOND		11
typedef struct ex_deletepointercond {
	exaction_type	action;				//	EXACTION_DELETEPOINTERCOND
		/*   0        1            2         7 	*/
		/*  +--------------------------------+	*/
		/*  | regcond | regpointer | unused  |	*/
		/*  +--------------------------------+	*/
	unsigned char	dpc_field;			//	regcond 	0 (1 -> condition flag is in a GPR)
										//	regpointer 	1 (1 -> object pointer is in a GPR)
										//	unused	 	2:7 (reserved for future use)
	short			cond;				//	16-bit frame-relative offset or GPR# of condition flag
	short			objectptr;			//	16-bit frame-relative offset or GPR# of object pointer
	void*			deletefunc;			//	absolute pointer to TVector of deletefunc
} ex_deletepointercond;

#define ex_deletepointercond_MakeField(regcond,regpointer)	\
			(((regcond)<<7)|(((regpointer)&0x0001)<<6))
#define ex_deletepointercond_GetRegCond(field)			((field)>>7)
#define ex_deletepointercond_GetRegPointer(field)		(((field)>>6)&0x0001)

#define EXACTION_CATCHBLOCK				12
typedef struct ex_catchblock {
	exaction_type	action;				//	EXACTION_CATCHBLOCK
	unsigned char	unused;				//	(reserved for future use)
	char*			catch_type;			//	absolute pointer to catch type info; 0 => catch(...)
	unsigned short	catch_pcoffset;		//	16-bit function-relative offset to catch label
	short			cinfo_ref;			//	16-bit frame-relative offset to local CatchInfo struct
} ex_catchblock;

#define EXACTION_ACTIVECATCHBLOCK		13
typedef struct ex_activecatchblock {
	exaction_type	action;				//	EXACTION_ACTIVECATCHBLOCK
	unsigned char	unused;				//	(reserved for future use)
	short			cinfo_ref;			//	16-bit frame-relative offset to local CatchInfo struct
} ex_activecatchblock;

#define EXACTION_TERMINATE				14
typedef struct ex_terminate {
	exaction_type	action;				//	EXACTION_TERMINATE
	unsigned char	unused;				//	(reserved for future use)
} ex_terminate;

#define EXACTION_SPECIFICATION			15
typedef struct ex_specification {
	exaction_type	action;				//	EXACTION_SPECIFICATION
	unsigned char	unused;				//	(reserved for future use)
	unsigned short	specs;				//	number of specializations (0-n)
	long			pcoffset;			//	32-bit function-relative offset to catch label
	long			cinfo_ref;			//	32-bit frame-relative offset to local CatchInfo struct
	char			*spec[0];
}	ex_specification;

#if _32BIT_EXCEPTION_TABLES
#define EXACTION_CATCHBLOCK_32			16
typedef struct ex_catchblock_32 {
	exaction_type	action;				//	EXACTION_CATCHBLOCK_32
	unsigned char	unused;				//	(reserved for future use)
	char*			catch_type;			//	absolute pointer to catch type info; 0 => catch(...)
	long			catch_pcoffset;		//	32-bit function-relative offset to catch label
	long			cinfo_ref;			//	32-bit frame-relative offset to local CatchInfo struct
} ex_catchblock_32;
#endif

//	Exception Table Ranges

typedef struct ExceptionRangeSmall {
	unsigned short	start;				//	start of PC range where actions apply
	unsigned short	end;				//	end of PC range where actions apply
	unsigned short	action;				//	table-relative offset to actions for an
} ExceptionRangeSmall;						//	exception thrown in range start..end


//	Exception Tables (per-function)

typedef struct ExceptionTableSmall {
		/*   0          5           10        11            12   13    15	*/
		/*  +-----------------------------------------------------------+	*/
		/*  | savedGPRs | savedFPRs | savedCR | hasframeptr | 0 | rsvd	|	*/
		/*  +-----------------------------------------------------------+	*/
	unsigned short	et_field;			//	savedGPRs 	0:4 (# of saved GPRs (rN-r31))
										//	savedFPRs 	5:9 (# of saved FPRs (rN-r31))
										//	savedCR 	10 (1 -> saved CR in frame)
										//	hasframeptr 11 (1 -> frame uses R31 as frame pointer)
										//  large table 12 (always zero)
										//	reserved 	13:15 (reserved for future use)
	ExceptionRangeSmall	ranges[0];			//	table of PC ranges and their actions
//	unsigned short	end = 0;			//	(marks end of range list)
//	unsigned long	actions[];			//	arbitrary action data
} ExceptionTableSmall;

//	Exception Tables (per-function)

typedef struct ExceptionTableSmallVector {
		/*   0          5           10        11            12   13  14  15	*/
		/*  +---------------------------------------------------------------+	*/
		/*  | savedGPRs | savedFPRs | savedCR | hasframeptr | 0 | V | rsvd	|	*/
		/*  +---------------------------------------------------------------+	*/
	unsigned short	et_field;			//	savedGPRs 	0:4 (# of saved GPRs (rN-r31))
										//	savedFPRs 	5:9 (# of saved FPRs (rN-r31))
										//	savedCR 	10 (1 -> saved CR in frame)
										//	hasframeptr 11 (1 -> frame uses R31 as frame pointer)
										//  large table 12 (always zero)
										//  hasvector	13 (vector info in next 2 bytes)
										//	reserved 	14:15 (reserved for future use)

    unsigned short  et_field2;			//  savedVRs	0:4 (# of saved VRs (vrN-vr31))
										//  hasvrsave	5 (frame used vrsave)
	ExceptionRangeSmall	ranges[0];			//	table of PC ranges and their actions
//	unsigned short	end = 0;			//	(marks end of range list)
//	unsigned long	actions[];			//	arbitrary action data
} ExceptionTableSmallVector;

typedef struct ExceptionRangeLarge {
	unsigned long	start;				//	start of PC range where actions apply
	unsigned short	size;				//	size of PC range (div 4)
	unsigned short	action;				//	table-relative offset to actions for an
} ExceptionRangeLarge;					//	exception thrown in range start..start+(size*4)


//	Exception Tables (per-function)

typedef struct ExceptionTableLarge {
		/*   0          5           10        11            12   13    15	*/
		/*  +-----------------------------------------------------------+	*/
		/*  | savedGPRs | savedFPRs | savedCR | hasframeptr | L | rsvd	|	*/
		/*  +-----------------------------------------------------------+	*/
	unsigned short	et_field;			//	savedGPRs 	0:4 (# of saved GPRs (rN-r31))
										//	savedFPRs 	5:9 (# of saved FPRs (rN-r31))
										//	savedCR 	10 (1 -> saved CR in frame)
										//	hasframeptr 11 (1 -> frame uses R31 as frame pointer)
										//  large table 12 (always 1 for backawards compatablity)
										//	hasvector 	13
										//	reserved	14:15 (reserved for future use)
    unsigned short  et_field2;			//  savedVRs	0:4 (# of saved VRs (vrN-vr31))
										//  hasvrsave	5 (frame used vrsave)
	ExceptionRangeLarge	ranges[0];			//	table of PC ranges and their actions
//  unsigned long   start = 0;			//  (marks end of range list)
//	unsigned short	size = 0;			//	(marks end of range list)
//	unsigned long	actions[];			//	arbitrary action data
} ExceptionTableLarge;

#define ET_MakeField(savedGPRs,savedFPRs, savedCR, hasframeptr, isLarge) \
			(((savedGPRs)<<11)|((savedFPRs&0x001f)<<6)|((savedCR&0x0001)<<5)|((hasframeptr&0x0001)<<4)|((isLarge&1)<<3))

#if __ALTIVEC__
#define ET_AddHasVector(field, hasVector) \
			((field)|((hasVector&1)<<2))
#define ET_MakeVectorField(savedVRs,hasVRSAVE) \
			(((savedVRs)<<11)|((hasVRSAVE&0x0001)<<10))
#endif


#define ET_GetSavedGPRs(field)		((field)>>11)
#define ET_GetSavedFPRs(field)		(((field)>>6)&0x001f)
#define ET_GetSavedCR(field)		(((field)>>5)&0x0001)
#define ET_GetHasFramePtr(field)	(((field)>>4)&0x0001)
#define ET_IsLargeTable(field)		(((field)>>3)&0x0001)
#define ET_ClearLargeBit(field)		((field) & ~(1<<3))
#define ET_SetLargeBit(field)		((field) |  (1<<3))

#if __ALTIVEC__
#define ET_GetSavedVRs(field)		((field)>>11)
#define ET_GetSavedVRSAVE(field)	(((field)>>6)&0x0001)
#define ET_HasVectorInfo(field)		(((field)>>2)&0x0001)
#define ET_ClearVectorBit(field)	((field) & ~(1<<2))
#define ET_SetVectorBit(field)		((field) |  (1<<2))
#endif


//	Exception Table Indices (per-function)

typedef struct ExceptionTableIndex {
	unsigned long	functionoffset;		//	code-relative address of function
		/*   0            1                31   */
		/*  +--------------------------------+  */
		/*  | directstore | functionsize     |  */
		/*  +--------------------------------+  */
	unsigned long   eti_field;			//	directstore 		0 (1 -> 4 byte exception table is in 'exceptionoffset')
										//	functionsize 	1:31 (length of function)
	unsigned long	exceptionoffset;	//	data-relative address of exception table
} ExceptionTableIndex;					//	(or table itself if 'directstore' == 1)

#define ETI_MakeField(direct,fsize)	((((long)(direct))<<31)|((fsize)&0x7fffffff))
#define ETI_GetDirectStore(field)	((field)>>31)
#define ETI_GetFunctionSize(field)	((field)&0x7fffffff)

#if __MWERKS__
#pragma options align=reset
#endif

#endif
