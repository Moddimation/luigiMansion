;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Phytec PPC 505 initialization code. 
;
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	.globl		usr_init
	
	.section   .init,4,1,6

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;	usr_init
;;
;;	Performs board-specific initializations.
;;
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

usr_init:
; Disable watchdog
	lis		r3, 0x0
	ori		r3, r3, 0x0
	lis		r4, 0x8007
	ori		r4, r4, 0xEFC4
	stw		r3, 0(r4)
	
; UART (CS3)
	lis		r3, 0x0010
	ori		r3, r3, 0x0
	lis		r4, 0x8007
	ori		r4, r4, 0xFDD0
	stw		r3, 0(r4)
	
	lis		r3, 0x9084
	ori		r3, r3, 0xC001
	lis		r4, 0x8007
	ori		r4, r4, 0xFDD4
	stw		r3, 0(r4)
	
; second SRAM (CS2)
	lis		r3, 0x0004
	ori		r3, r3, 0x0
	lis		r4, 0x8007
	ori		r4, r4, 0xFDD8
	stw		r3, 0(r4)
	
	lis		r3, 0x7003
	ori		r3, r3, 0xC005
	lis		r4, 0x8007
	ori		r4, r4, 0xFDDC
	stw		r3, 0(r4)
	
; first SRAM (CS1)
	lis		r3, 0x0
	ori		r3, r3, 0x0
	lis		r4, 0x8007
	ori		r4, r4, 0xFDE0
	stw		r3, 0(r4)
	
	lis		r3, 0x7003
	ori		r3, r3, 0xC005
	lis		r4, 0x8007
	ori		r4, r4, 0xFDE4
	stw		r3, 0(r4)

	
; SCCR setup
	lis		r3, 0x0
	ori		r3, r3, 0x0
	lis		r4, 0x8007
	ori		r4, r4, 0xFC50
	stw		r3, 0(r4)
		
;  MSR setup
	lis		r3, 0x0
	ori		r3, r3, 0x3002
	mtmsr	r3

	blr