/*  Metrowerks Standard Library  */

/*  $Date: 2000/10/30 22:24:16 $ 
 *  $Revision: 1.11.4.2.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	abort_exit.c
 *	
 *	Routines
 *	--------
 *		abort
 *
 *		atexit
 *		exit
 *
 *		__atexit
 *		__exit
 *
 *		__setup_exit
 *	
 *
 */

#include "abort_exit.h"
#include "critical_regions.h"
#include "misc_io.h"
#include <signal.h>
#include <setjmp.h>
#include <cstdlib>
#include <stdio.h>
#include <pool_alloc.h>			/*  mf 980825  */

#if macintosh && !defined(__dest_os)               /*MW-mm 960927a*/
  #define __dest_os __mac_os                       /*MW-mm 960927a*/
#endif                                             /*MW-mm 960927a*/

#if __dest_os == __undef_os
#error __dest_os undefined
#endif

#if __dest_os == __mac_os
#include <console.h>
#include <SegLoad.h>
#include <Processes.h>
#endif

#if __dest_os == __win32_os || __dest_os == __wince_os

/* #include <Windows.h> hh 980122 commented out.  A real problem child.
                                  The next two includes do the job right.
*/
#include <WINDEF.H>
#include <WINBASE.H>
#endif

#define max_funcs	64

#if defined(__CFM68K__)
	#pragma import on
#endif

extern void __destroy_global_chain(void);

#if defined(__CFM68K__)
	#pragma import reset
#endif

int	__aborting = 0;       /* hh 971206  just moved from below */

static void (*atexit_funcs[max_funcs])(void);
static long	atexit_curr_func = 0;

static void (*__atexit_funcs[max_funcs])(void);
static long	__atexit_curr_func = 0;

void (* __stdio_exit)  (void) = 0;
void (* __console_exit)(void) = 0;

#if (__dest_os == __win32_os) && (STOP_PROGRAM_BEFORE_EXIT==1)	/* mm 981227 */
static void __StopProgramBeforeExit();							/* mm 981227 */
#endif															/* mm 981227 */

#if __dest_os == __be_os							/* ELR */

extern void (*__atexit_hook)(void);
extern void (*___atexit_hook)(void);

extern void ___teardown_be(int);

static short exitSetup = 0;

static void ANSI_Exit(void)
{
	exit(0);
}

static void ANSI__Exit(void)
{
	__exit(0);
}

void __setup_exit(void)
{
	if (!exitSetup)
	{
		__atexit_hook	 = ANSI_Exit;
		___atexit_hook = ANSI__Exit;

		exitSetup++;
	}
}

#endif

void	abort(void)
{
#if (__dest_os == __win32_os) && (_WINSIOUX==1)						/* mm 981227 */
	WinSIOUXAbort();												/* mm 981227 */
#elif (__dest_os == __win32_os) && (STOP_PROGRAM_BEFORE_EXIT==1)	/* mm 981227 */
	__StopProgramBeforeExit();										/* mm 981227 */
#endif																/* mm 981227 */
	raise(SIGABRT);
	
	__aborting = 1;
	
	exit(EXIT_FAILURE);
}

int atexit(void (*func)(void))
{	

	if (atexit_curr_func == max_funcs)
		return(-1);

	__begin_critical_region(atexit_funcs_access);	/* 961218 KO */
	
	__setup_exit();
	
	atexit_funcs[atexit_curr_func++] = func;
	
	__end_critical_region(atexit_funcs_access);

	return(0);
}

int __atexit(void (*func)(void))
{	

	if (__atexit_curr_func == max_funcs)
		return(-1);

	__begin_critical_region(atexit_funcs_access);	/* 961218 KO */
	
	__setup_exit();
	
	__atexit_funcs[__atexit_curr_func++] = func;
	
	__end_critical_region(atexit_funcs_access);
	
	return(0);
}

/*  mdf  971021  */
#if (__dest_os == __win32_os) && (STOP_PROGRAM_BEFORE_EXIT==1)
static void __StopProgramBeforeExit()
{
	DWORD read;
	HANDLE h;
	SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};
	char* buf="\n";

	if(GetFileType(GetStdHandle(STD_OUTPUT_HANDLE))==FILE_TYPE_CHAR)
	{
		
		printf("\n \n Press Enter to continue \n");
		/*  fflush(stdin);                   remove line  mdf  971119  */
		/*  971019  mdf  */
		h=CreateFile("CONIN$",
				       GENERIC_READ | GENERIC_WRITE,
					   FILE_SHARE_READ | FILE_SHARE_WRITE,
					   &sa,
				       OPEN_EXISTING,NULL,NULL);
		
				       
		if(!ReadFile(h, &buf, 1, &read,0))
		{
			read=GetLastError();
			printf("exit routine error GetLastError=%i \n",read);
		}
	}
	return;
}
#endif
		/*  971019  end insert code  mdf  */	

#ifndef __m56800__									/* mm 981015 */ /* mm 981029 */

#if __dest_os == __be_os							/* ELR */
void _libc_exit_(int status)
#else
void exit(int status)
#endif
{

		if (!__aborting)
	{
		__begin_critical_region(atexit_funcs_access);
		
		while (atexit_curr_func > 0)
			(*atexit_funcs[--atexit_curr_func])();
#if (__dest_os == __win32_os) && (_WINSIOUX==1)					/* mm 990122 */
		WinSIOUXAbort();										/* mm 990122 */
#elif (__dest_os == __win32_os) && (STOP_PROGRAM_BEFORE_EXIT==1)/* mm 990122 */
		__StopProgramBeforeExit();								/* mm 981227 */
#endif															/* mm 981227 */
		__end_critical_region(atexit_funcs_access);

	 /*
	 970218 bkoz
	 		need to move destroy global chain above __stdio_exit as
		 	some static objects may have destructors that flush streams
	 */
	 #if !__INTEL__
	 #if  __MIPS__ || __POWERPC__ || __CFM68K__ || (__MC68K__ && __A5__) || (__dest_os == __be_os)
		__destroy_global_chain();
	 #endif
	 #if  __PPC_EABI__
		{
			typedef void (*voidfunctionptr) (void);	/* ptr to function returning void */
			extern voidfunctionptr _dtors[];
			voidfunctionptr *destructor;
			/*
			 *	call other destructors
			 */
			 for (destructor = _dtors; *destructor; destructor++) {
			 	(*destructor)();
			 }
		}
	 #endif
	 #endif
		if (__stdio_exit)
		{
			(*__stdio_exit)();
			__stdio_exit = 0;
		}
	}

	__exit(status);
}
#else /* __m56800__ defined */                        /* mm 981015 */  /* mm 981029 */

void exit_dsp568();
void exit(int status)
{
	exit_dsp568();
}
#endif /* __m56800__ not defined */                   /* mm 981015 */ /* mm 981029 */


void __exit(int status)
{
#if __dest_os != __be_os
	#pragma unused(status)
#endif
	
	__begin_critical_region(atexit_funcs_access);

	while (__atexit_curr_func > 0)
		(*__atexit_funcs[--__atexit_curr_func])();
	
	__end_critical_region(atexit_funcs_access);
	
#if !__MIPS__
	__kill_critical_regions();
#endif
	
	if (__console_exit)
	{
		(*__console_exit)();
		__console_exit = 0;
	}

#if __dest_os == __mac_os

	ExitToShell();

#elif __dest_os == __be_os															/* ELR */

	___teardown_be(status);

#elif __dest_os == __win32_os

	ExitProcess(status);
	
#elif __dest_os == __wince_os	

TerminateProcess(GetCurrentProcess(),status);	
/*CE doesn't have ExitProcess */	
  
#elif __dest_os == __ppc_eabi || __dest_os == __nec_eabi || __dest_os == __mips_bare || __dest_os == __dolphin_os

	_ExitProcess();													

#endif
}

/* hh 980122 #include "critical_regions.h" doesn't belong here

#include "critical_regions.h"

*/
#if (__dest_os == __win32_os  || __dest_os == __wince_os)

/* This function should be equivalent to the ANSI "exit" without the ExitProcess
 * call. This function is needed so that a DLL can clean up itself and return
 * to Windows which finishes cleaning up the process (other DLLs and the main
 * application might not have destructed yet).
 * I would have just separated this code out of exit but that code is already too
 * separated...
 */

void _CleanUpMSL()
{
	extern int *__get_MSL_init_count(void);

	if (--(*__get_MSL_init_count()) > 0) return;		/* blc 980812 */
	
	__begin_critical_region(atexit_funcs_access);
	
	while (atexit_curr_func > 0)
		(*atexit_funcs[--atexit_curr_func])();
	
	__end_critical_region(atexit_funcs_access);
	
	if (__stdio_exit)
	{
		(*__stdio_exit)();
		__stdio_exit = 0;
	}

	__destroy_global_chain();
	
	__kill_critical_regions();
	
	if (__console_exit)
	{
		(*__console_exit)();
		__console_exit = 0;
	}
	
#ifndef _MSL_PRO4_MALLOC
	__pool_free_all();	/*  mf  980825, hh 990227  */
#else
	__pool_free_all(&__malloc_pool);	/*  mf  980825  */
#endif

}

#endif	/* __dest_os == win32 */

/*     Change record
 *	14-Sep-95 JFH  First code release.
 *	12-Oct-95 JFH  Added #include of <SegLoad.h> for ExitToShell() (in case
 *								 MacHeaders not included).
 *	31-Oct-95 JFH  Fixed exit() to longjmp(__program_exit,1) instead of call _exit on PPC
 *	15-Dec-95 JFH  Reworked abort/exit handling to conform to new runtime architecture.
 *	20-Dec-95 JFH  Renamed _atexit/_exit to __atexit/__exit for ANSI naming conformance
 *	27-Dec-95 JFH  Pulled guts out of __setup_exit for new runtime. Tossed __program_exit
 *								 and added __aborting for PPC and CFM68K projects.
 *	12-Feb-96 JFH  Tossed __setup_exit, which had become a NOP.
 *	 1-Mar-96 JFH  Merged Be code into source. For the moment that means: __setup_exit(),
 *								 it's baaack!
 *	26-Apr-96 JFH  Merged Win32 changes in.
 *						CTV
 * MW-mm 960927a   Made sure dest_os set for Macintosh
 *  18-Dec-96 KO   Moved the begin_critical_region call after the error check. Before, if
 *                 there was an error, the critical section would be entered and never left.
 *  19-Dec-96 KO   Added CleanUpMSL.
 *  18-feb-97 bkoz line 154 moved call of __destroy_global_chain() up to 
 *			       exit() before stdio closed
 *	20-Jul-97 MEA  Changed __ppc_eabi_bare to __ppc_eabi.
 *  21-Aug-97 mdf  added printf/scanf to exit for win32 to allow control of
 *                 console apps  	
 *  30-Sept-97 mdf wrapped immediately above change in GetFileType to prevent app from
 *				   stopping when output is redirected to a file.
 *                 Also add fflush so getc will have an empty stream
                   when called in the exit routine		   
 *  21-Oct-97	mdf fixed exiting for windows console apps in cases where
                    the app explicitly closes stdin before exit is reached.
                    this can be done with fclose or freopen and then close on
                    the file associated with the stdin stream.       
 *  19-Nov-97  mdf  Removed fflush on stdin                
 * hh 980122      #include "critical_regions.h" doesn't belong here
 * #include <Windows.h> hh 980122 commented out.  A real problem child.
                                  The next two includes do the job right.
 * mf 980512        changes for wince  
 * mf/blc 98081098  fix to x86 runtime dll crash(when > 1 dll is attached to MSL) 
 * mf  980825  		Deallocate memory pools allocated by system - this fixes a
 *					problem with memory leaking when multiple dll's are loaded and
 *					unloaded.                               
 * mm 981015        Added version of exit for __DSP568
 * mea 981022       added calls to destructors  
 * mm 981029        Changed __DSP568 to __m56800__
 * mm 981227        Avoided closing console windows during abort. WB1-1897
 * mm 990122        Avoided closing console windows during exit.
 * hh 990227        Interface to __pool_free_all changed
 */
