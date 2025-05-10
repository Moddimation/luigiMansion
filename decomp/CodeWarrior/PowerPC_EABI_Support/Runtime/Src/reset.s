#// This is an example of how to make your code boot from reset.
#// __start is the entry point, and we want to put a branch to it
#// at the reset vector.

.extern __start  	#// entry point
.globl	reset

.section 	.reset, 8, x

.org 	0x100

reset:
b	__start