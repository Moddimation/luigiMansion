	.globl		usr_init  

	.extern		init_board

MMCR0			.equ		952
MMCR1			.equ		956
ICTC			.equ		1019
THRM1			.equ		1020
THRM2			.equ		1021
THRM3			.equ		1022
L2CR			.equ		1017

_7XX_VERSION	.equ		0x0008		;; bits 0..15 of PVR on 7xx


	.section   .init,4,1,6
	.align     2            	# align on word boundary

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;	usr_init
;;
;;	Initializes the 7xx or 603e processor to provide a reasonable
;;	initial state.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

usr_init:	
	;;
	;; Initialize MSR:
	;;	Reserved[0..12]	= 0
	;;	POW[13]			= 0 (power mgmt disabled)
	;;	TGPR[14]		= 0 (no temporary TGPR remapping (603e))
	;;	ILE[15]			= 0 (big-endian exception context)
	;;	EE[16]			= 0 (external interrupts disabled)
	;;	PR[17]			= 0 (supervisor level)
	;;	FP[18]			= 1 (floating point available)
	;;	ME[19]			= 1 (machine check exceptions enabled)
	;;	FE0[20]			= 0 (w/FE1 -> floating pt exceptions disabled)
	;;	SE[21]			= 0 (single-step trace disabled)
	;;	BE[22]			= 0 (branch trace disabled)
	;;	FE1[23]			= 0 (w/FE0 -> floating pt exceptions disabled)
	;;	Reserved[24]	= 0
	;;	IP[25]			= 1 (exception handler table at 0xfff00000)
	;;	IR[26]			= 0 (instruction address xlation disabled)
	;;	DR[27]			= 0 (data address xlation disabled)
	;;	Reserved[28]	= 0
	;;	PM[29]			= 0 (not marked for performance monitor)
	;;	RI[30]			= 0 (exception state is not recoverable)
	;;	LE[31]			= 0 (big-endian)
	;;

	addi	r3, r0, 0x3040
	mtmsr	r3
	sync
	
	;;
	;; Initialize HID0:
	;;	EMCP[0]			= 1 (allow MCP' to cause checkstop/machine check)
	;;	DBP[1]			= 0 (disable bus parity generation (7xx))
	;;	EBA[2]			= 0 (prevent address parity checking)
	;;	EBD[3]			= 0 (prevent data parity checking)
	;;	BCLK[4]			= 1 (w/ECLK=1 -> CLK_OUT = Bus clock)
	;;	EICE[5]			= 0 (disable ICE outputs (603e))
	;;	ECLK[6]			= 1 (w/BCLK=1 -> CLK_OUT = Bus clock)
	;;	PAR[7]			= 0 (enable precharge of ARTRY')
	;;	DOZE[8]			= 0 (doze mode disabled)
	;;	NAP[9]			= 0 (nap mode disabled)
	;;	SLEEP[10]		= 0 (sleep mode disabled)
	;;	DPM[11]			= 1 (dynamic power mgmt enabled)
	;;	RISEG[12]		= 0 (<reserved for test>)
	;;	Not_used[13..14]= 0
	;;	NHR[15]			= 1 (not hard reset - should init to 1 (7xx))
	;;	ICE[16]			= 0 (disable instruction cache)
	;;	DCE[17]			= 0 (disable data cache)
	;;	ILOCK[18]		= 0 (instruction cache not locked)
	;;	DLOCK[19]		= 0 (data cache not locked)
	;;	ICFI[20]		= 1 (instruction cache flush invalidate)
	;;	DCFI[21]		= 1 (data cache flush invalidate)
	;;	SPD[22]			= 0 (enable speculative cache access (7xx))
	;;	IFEM[23]		= 1 (enable M bit for instruction fetches (7xx))
	;;	SGE[24]			= 1 (enable store gathering (7xx - on 603e bit 24 is IFEM))
	;;	DCFA[25]		= 0 (disable data cache flush assist (7xx))
	;;	BTIC[26]		= 0 (enable branch instruction cache (7xx))
	;;	FBIOB[27]		= 0 (don't force branch indirect on bus (603e))
	;;	ABE[28]			= 0 (don't broadcase address-only operations)
	;;	BHT[29]			= 1 (enable branch history table (7xx))
	;;	Not_used[30]	= 0
	;;	NOOPTI[31]		= 0 (enable dcbt and dcbtst instructions)
	;;
	
	addis	r3, r0, 0x8a11
	ori		r3, r3, 0x0d84
	mtspr	HID0, r3
	sync
	andi.	r3, r3, 0xf3ff	;; clear ICFI and DCFI bits (for 603e)
	mtspr	HID0, r3
	sync


	;;
	;; Clear IABR (disable instruction address breakpoint)
	;;
	
	addis	r3, r0, 0
	mtspr	IABR, r3

	;;
	;; Check the PVR and perform initializations which are
	;; specific to either the 603e or the 7xx processors.
	;;
	
	mfspr	r4, PVR
	addis	r3, r0, 0xffff
	and		r4, r4, r3		;; get just the version number
	addis	r3, r0, _7XX_VERSION
	cmp		cr0, 0, r3, r4
	bne		cr0, __603e_specific_init	;; if not 7xx, assume 603e
	
__7xx_specific_init:

	;;
	;; Initialize MMCR0:
	;;	DIS[0]				= 1 (PMCn counters do not change)
	;;	DP[1]				= 0 (counters can operate in supervisor mode)
	;;	DU[2]				= 0 (counters can operate in user mode)
	;;	DMS[3]				= 0 (counters can operate when MSR[PM]=1)
	;;	DMR[4]				= 0 (counters can operate when MSR[PM]=0)
	;;	ENINT[5]			= 0 (perf monitor int signalling disabled)
	;;	DISCOUNT[6]			= 0 (perf mon int does not affect counting)
	;;	RTCSELECT[7..8]		= 00 (count time base bit 63)
	;;	INTONBITTRANS[9]	= 0 (don't interrupt on bit transition)
	;;	THRESHOLD[10..15]	= 000000 (0 threshold value)
	;;	PMC1INTCONTROL[16]	= 0 (don't interrupt on PMC1 overflow)
	;;	PMCINTCONTROL[17]	= 0 (don't interrupt on PMC2..4 overflow)
	;;	PMCTRIGGER[18]		= 0 (enable PMC2..4 counting after PMC1 overflow)
	;;	PMC1SELECT[19..25]	= 0000000 (event = reg holds current val)
	;;	PMC2SELECT[26..31]	= 000000 (event = reg holds current val)
	;;
	;; Initialize MMCR1:
	;;	PMC3SELECT[0..4]	= 00000 (event = reg holds current val)
	;;	PMC4SELECT[5..9]	= 00000 (event = reg holds current val)
	;;	Reserved[10..31]	= 0
	;;

	addis	r3, r0, 0x8000
	mtspr	MMCR0, r3
	addis	r3, r0, 0
	mtspr	MMCR1, r3
	sync
	
	;;
	;; Clear ICTC (full speed)
	;; Clear IABR (disable instruction address breakpoint)
	;; Clear DABR (disable data address breakpoint)
	;; Clear THRM1, THRM2, THRM3 (disable thermal management)
	;; Clear L2CR (disable L2 cache)
	;;
	
	addis	r3, r0, 0
	mtspr	ICTC, r3
	mtspr	DABR, r3
	mtspr	THRM1, r3
	mtspr	THRM2, r3
	mtspr	THRM3, r3
	mtspr	L2CR, r3
	sync
	
	b		__end_specific_init

__603e_specific_init:

	;; Nothing specific is needed for the 603e.  The common init
	;; takes care of everything.
	
__end_specific_init:
	
	;;
	;; Invalidate the TLB
	;;	

	addis	r3, r0, 0	;; set up counter at 0x00000000 ;;
	addis	r5, r0, 0x4	;; set up high bound of 0x00040000 for 603e/7xx ;;

tlblp:	
	tlbie	r3
	sync
	addi	r3,r3,0x1000
	cmp		cr0,0,r3,r5	;; check if all 32(603) or 64(740) TLB's invalidated ;;
	blt	tlblp
	
	;;
	;; Call any board-specific initialization required.
	;;
	
	mflr	r30				;; save link register in a NV register ;;
	bl		init_board
	mtlr	r30				;; restore saved link register ;;
	
	blr
