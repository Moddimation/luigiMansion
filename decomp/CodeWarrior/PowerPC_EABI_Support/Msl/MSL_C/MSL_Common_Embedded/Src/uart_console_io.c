/*  Metrowerks Standard Library  Version 2.1.2  1997 July  */

/*
 *	uart_console_io.c
 *	
 *		Copyright 1997 Metrowerks, Inc.
 *		All rights reserved.
 *	
 *	Routines
 *	--------
 *		__read_console
 *		__write_console
 *		__close_console
 *
 *	Implementation
 *	--------------
 *
 *		These routines implement low-level console IO routines using
 *		any UART library which conforms to the MetroTRK UART library
 *		interface.  They are intended to provide basic console IO
 *		services in an embedded environment which lacks OS console
 *		support.
 *
 *
 */

#ifndef _No_Console

#include "UART.h"
#include "console_io.h"
#include "uart_console_config.h"
#include "stdio.h"


static UARTError __init_uart_console(void);


/************************************************************************/
int		__read_console	  (__file_handle handle, unsigned char * buffer, 
							size_t * count, __idle_proc idle_proc)
{
	#pragma unused( handle, idle_proc )

	size_t limit;
	UARTError err;

	if (__init_uart_console() != kUARTNoError)
		return( __io_error );

	/*
	** A console read should return when it encounters a line-ending
	** or after it has read the requested number of characters.
	*/

	limit = *count;
	*count = 0;
	err = kUARTNoError;

	while( *count <= limit && err == kUARTNoError )
	{
#if  __PPCGEKKO__
		/* Changed this to ReadUARTN and set the count to 1
		 * since it is the same functionality as ReadUART1,
		 * but ReadUART1 broke end user's implementations
		 */
		err = ReadUARTN( (char *)buffer, 1 );
#else		
		err = ReadUART1( (char *)buffer );
#endif		
		++(*count);
		if (*buffer == '\r')
			break;
		++buffer;
	}

	return( (err == kUARTNoError) ? __no_io_error : __io_error );
}



/************************************************************************/
int		__write_console	  (__file_handle handle, unsigned char * buffer, 
							size_t * count, __idle_proc idle_proc)
{
	#pragma unused( handle, idle_proc )

	if (__init_uart_console() != kUARTNoError)
		return( __io_error );

	if (WriteUARTN( buffer, *count ) != kUARTNoError)
	{
		*count = 0;
		return( __io_error );
	}
	
	return( __no_io_error );
	
}



/************************************************************************/
int		__close_console	  (__file_handle handle)
{
	#pragma unused( handle )

	return( __no_io_error );
}



/************************************************************************/
UARTError __init_uart_console()
{
	UARTError err = kUARTNoError;
	static int initialized = 0;
	
	if (initialized == 0)
	{
		err = InitializeUART( UART_CONSOLE_BAUD_RATE );
		
		if (err == kUARTNoError)
			initialized = 1;
	}
	
	return( err );
}

#endif /* _No_Console */

/*	Change Record
 *	09-Jul-97 SCM	Created
 *	20-Aug-99 SCM	Enhanced __read_console() to return when it receives an EOL.
*/

