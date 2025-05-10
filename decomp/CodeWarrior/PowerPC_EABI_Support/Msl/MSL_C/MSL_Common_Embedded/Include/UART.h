/****************************************************************************/
/*

FILE

	UART.h
	
	$Header: /u/msl/MSL_C/MSL_Common_Embedded/Include/UART.h,v 1.1.2.1 1999/12/07 20:30:53 fassiott Exp $

DESCRIPTION

	Abstract interface to serial device (UART) for MetroTRK.
	For maximum portability, do not change this file.

COPYRIGHT

	(C) 1996-8 Metrowerks Corporation
	All rights reserved.

HISTORY

	96 DEC 08 LLY	Created.
	
	$History: UART.h $
//	
//	*****************  Version 5  *****************
//	User: Smoore       Date: 8/09/99    Time: 5:31p
//	Updated in $/Embedded/MetroTRK/Export
//	Added kDataError definition.
//	
//	*****************  Version 4  *****************
//	User: Smoore       Date: 6/25/98    Time: 1:59p
//	Updated in $/Embedded/MetroTRK/Import
//	Merging in latest changes from VR TRK:
//	   - Added support for FlushCache and Stop commands.
//	   - Added support for interrupt-driven I/O.
//	   - Added several extensions to the MetroTRK API,
//	     including Level 3 (OS-level) extensions.
//	   - Added support for the Midas RTE-VR5000-PC board.
//	   
//	
//	*****************  Version 5  *****************
//	User: Smoore       Date: 6/24/98    Time: 2:39p
//	Updated in $/Embedded/TRK_In_Progress/NEC_VR_TRK/MetroTRK/Import
//	Added support for interrupt-driven I/O.
//	Added support for the FlushCache command.
//	Made several extensions to the MetroTRK API, including all of the
//	level 3 (OS-level) extensions.  None are currently implemented.
//	
//	*****************  Version 3  *****************
//	User: Smoore       Date: 11/07/97   Time: 5:16p
//	Updated in $/Embedded/MetroTRK/Import
//	Replace '#pragma once' with #ifdef's.  Added hardware-set baud
//	rate option.
*/
/****************************************************************************/

/* #pragma once */
#ifndef uart_h_included
#define uart_h_included


/****************************************************************************/

typedef int UARTError;

enum {
	kUARTDataError = -1,
	kUARTNoError = 0,
	kUARTUnknownBaudRate,
	kUARTConfigurationError,
	kUARTBufferOverflow,				/* specified buffer was too small */
	kUARTNoData							/* no data available from polling */
};

/****************************************************************************/

typedef enum {
	kBaudHWSet = -1,					/* use HW settings such as DIP switches */
	kBaud300 = 300,						/* valid baud rates */
	kBaud600 = 600,
	kBaud1200 = 1200,
	kBaud1800 = 1800,
	kBaud2000 = 2000,
	kBaud2400 = 2400,
	kBaud3600 = 3600,
	kBaud4800 = 4800,
	kBaud7200 = 7200,
	kBaud9600 = 9600,
	kBaud19200 = 19200,
	kBaud38400 = 38400,
	kBaud57600 = 57600,
	kBaud115200 = 115200,
	kBaud230400 = 230400
} UARTBaudRate;



/****************************************************************************/

UARTError InitializeUART(UARTBaudRate baudRate);
UARTError InitializeIntDrivenUART( UARTBaudRate baudRate, 
								   unsigned char intDrivenInput,
								   unsigned char intDrivenOutput,
								   volatile unsigned char **inputPendingPtrRef );
UARTError TerminateUART(void);

UARTError ReadUARTPoll(char* c);

UARTError ReadUART1(char* c);
UARTError ReadUARTN(void* bytes, unsigned long length);
UARTError ReadUARTString(char* string, unsigned long limit, char terminateChar);

UARTError WriteUART1(char c);
UARTError WriteUARTN(const void* bytes, unsigned long length);
UARTError WriteUARTString(const char* string);

void UARTInterruptHandler( void );

#endif /* uart_h_included */
