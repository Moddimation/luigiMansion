/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:33 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	signal.c
 *	
 *	Routines
 *	--------
 *		signal
 *		raise
 *	
 *
 */

#include "critical_regions.h"
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

static __signal_func_ptr	signal_funcs[__signal_max];

__signal_func_ptr signal(int signal, __signal_func_ptr signal_func)
{
	__signal_func_ptr	old_signal_func;
	
	if (signal < 1 || signal > __signal_max)
	{
		errno = ESIGPARM;
		return(SIG_ERR);
	}
	
	__begin_critical_region(signal_funcs_access);
	
	old_signal_func = signal_funcs[signal-1];
	
	signal_funcs[signal-1] = signal_func;
	
	__end_critical_region(signal_funcs_access);
	
	return(old_signal_func);
}

int raise(int signal)
{
	__signal_func_ptr	signal_func;
	
	if (signal < 1 || signal > __signal_max)
		return(-1);
	
	__begin_critical_region(signal_funcs_access);
	
	signal_func = signal_funcs[signal-1];
	
	if (signal_func != SIG_IGN)
		signal_funcs[signal-1] = SIG_DFL;
	
	__end_critical_region(signal_funcs_access);
	
	if (signal_func == SIG_IGN || (signal_func == SIG_DFL && signal == SIGABRT))
			return(0);
	
	if (signal_func == SIG_DFL)
		exit(0);
	
	(*signal_func)(signal);
	
	return(0);
}

/*  Change Record
 *	14-Sep-95 JFH  First code release.
 *	12-Oct-95 JFH  Moved __myraise back to unix.c
 *	15-Dec-95 JFH  Changed code to make SIGABRT handler of SIG_DFL to return instead of exit().
*/
