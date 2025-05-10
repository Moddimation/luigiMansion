### This is the board initialization file. It initializes all the registers and sets up the controllers
### 971215 - Khurram Qureshi
### R5 and R4 are used as scratch registers. 
###  



	.section   .init,4,1,6
	.globl	usr_init
 	.align 4
 	
 usr_init:
;----------------------------------------------------------------------
; Core Register Initialization
;----------------------------------------------------------------------
		
;----------------------------------------------------------------------
; SIU, MEMMAP and PCUMCR  are at fixed addresses for the 505evb
;----------------------------------------------------------------------
		lis			r4,0x8007
		ori			r4,r4,0xfc00
        lis		   	r5,0x9200
        stw			r5,0(r4)
        lis			r5,0x80c0
        ori			r5,r5,0x4080
        stw			r5,0x20(r4)
        
        lis			r4,0x8007
        ori			r4,r4,0xef80
        
        lis			r5,0x6080
        stw			r5,0(r4)			
;----------------------------------------------------------------------
; PCU Watchdog timer (Disable Watchdog timer)  SWR is RO
;----------------------------------------------------------------------
		lis			r4,0x8007
		ori			r4,r4,0xfc00
		lis			r5,0x0
		ori			r5,r5,0xffff
		stw			r5,0x40(r4)
		
		lis			r5,0x0400
		stw			r5,0x48(r4)
		
		lis			r5,0x01ff
		ori			r5,r5,0xffff
		lis			r4,0x8007
		ori			r4,r4,0xefc4
		stw			r5,0(r4)
	
;----------------------------------------------------------------------
; MSR	(Machine Check Enable and Can recover from Reset and Machine Check)
;----------------------------------------------------------------------
		lis			r5,0
		ori			r5,r5,0x3002
		mtmsr		r5			; RI=1, IP=0, ME=1, FP=1
		mtspr		27,r5		; SRR1

;----------------------------------------------------------------------
; DER - Debug Enable Register: for the time being, BDM catches bkpoint and
; trace exceptions
;----------------------------------------------------------------------
;		lis			r5,0x0000
;		ori			r5,r5,0x0000
;		mtspr		149,r5			; clear the DER

;----------------------------------------------------------------------
; ICTRL (Instruction Support Control Register)
; 0x00000003 = no show cycles will be performed for fetched instructions, serialized
; 0x00000007 = no show cycles performed and not serialized...error in tech. notes bit 29 not reserved 
;              bug in chip if !serialized and icache enabled then whacky, due to cache bug...
;----------------------------------------------------------------------
		li			r5,3
		mtspr		158,r5		
;----------------------------------------------------------------------
; Chip select initialization
;----------------------------------------------------------------------
		lis			r4,0x8007
		ori			r4,r4,0xfd00
		li			r5,0x1000
		stw			r5,0xec(r4)		; CSOR0 PCON=10(~OE)
		
		li			r5,0
		stw			r5,0xf0(r4)		; CSBTSBBAR
		
		stw			r5,0xf4(r4)		; CSBTSBOR
		
		lis			r5,0x0100
		stw			r5,0xf8(r4)		; CSBTBAR FLASH Base Address = 0x01000000-0x0107FFFF
		
		lis			r5,0x8007
		ori			r5,r5,0xc000
		stw			r5,0xfc(r4)		; CSBTOR  BSIZE=1000(512kb), ACKEN=1, PS=10
		
		li			r5,0
		stw			r5,0xe0(r4)		; CSBAR1  SRAM Base Address = 0x00000000-0x0003FFFF
		lis			r5,0x7007
		ori			r5,r5,0xC005
		stw			r5,0xe4(r4)		; CSOR1   BSIZE=0111 (256kbytes), ACKEN=1, TADLY=000, PS=10, PCON=00(~CE) ITYPE=0101(Type1)
		
		lis			r5,0x0004
		stw			r5,0xd8(r4)		; CSBAR2  BA=00040	
		lis			r5,0x7007
		ori			r5,r5,0xc005
		stw			r5,0xdc(r4)		; CSOR2   BSIZE=0 (256kb), ACKEN=1, PS=10, ITYPE=0101

		lis			r5,0x0008
		stw			r5,0xd0(r4)		; CSBAR3  BA=00080	
		lis			r5,0x7007
		ori			r5,r5,0xc005
		stw			r5,0xd4(r4)  	; CSOR3   BSIZE=0 (256kb), ACKEN=1, PS=10, ITYPE=0101
		
		lis			r5,0x000C
		stw			r5,0xc8(r4)		; CSBAR4  BA=000C0	
		lis			r5,0x6007
		ori			r5,r5,0xc005
		stw			r5,0xcc(r4)		; CSOR4   BSIZE=0 (128kb), ACKEN=1, PS=10, ITYPE=0101
		

		lis			r5,0x0200
		stw			r5,0xc0(r4)		; CSBAR5  BA=02000
		
		lis			r5,0xB280
		ori			r5,r5,0x4000
		stw			r5,0xc4(r4)		; CSOR5   BSIZE=0 (2Mb I/O space), DSP=1, CI=1, PS=10
		
		li			r5,0x1800
		stw			r5,0xbc(r4)		; CSOR6   PCON=11(Alternate function)
		stw			r5,0xb4(r4)		; CSOR7   PCON=11
		stw			r5,0xac(r4)		; CSOR8   PCON=11
		stw			r5,0xa4(r4)		; CSOR9   PCON=11
		stw			r5,0x9c(r4)		; CSOR10  PCON=11
		stw			r5,0x94(r4)		; CSOR11  PCON=11
		
		lis			r4,0x8007
		ori			r4,r4,0xfc50
		
		lis			r5,0x0418
		ori			r5,r5,0x0000	; put contents of SCCR in r5
		stw			r5,0(r4)
		
		blr

