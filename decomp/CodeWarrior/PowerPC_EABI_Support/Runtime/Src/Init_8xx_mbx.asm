##############################################################################
#
# MBX8xx Initialization Routine 
#
#
##############################################################################

# Define board type and processor speed

TARGET_SYSTEM_FREQUENCY	.equ	40			# Target System Frequency
__MOT_MBX_A__			.equ	0
__MOT_MBX__				.equ	1
__MOT_ADS__				.equ	0

.include "init8xx.asm"						# main init code

