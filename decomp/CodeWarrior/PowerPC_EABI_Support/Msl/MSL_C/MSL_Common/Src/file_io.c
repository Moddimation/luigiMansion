/*  Metrowerks Standard Library  */

/*  $Date: 1999/03/09 17:16:22 $ 
 *  $Revision: 1.10 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	file_io.c
 *	
 *	Routines
 *	--------
 *		remove
 *		rename
 *		tmpnam
 *		tmpfile
 *		fclose
 *		fflush
 *		fopen
 *		freopen
 *
 *		__handle_reopen
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "ansi_files.h"
#include "buffer_io.h"
#include "critical_regions.h"
#include "file_io.h"
#include "misc_io.h"

#if (__dest_os	== __win32_os  || __dest_os	== __wince_os)
	#include <ThreadLocalData.h>
#endif

#ifndef _No_Disk_File_OS_Support

/*
hh  971206  Moved __temp_file_mode to get it into ::
*/
__file_modes	__temp_file_mode = 
#ifndef __NO_WIDE_CHAR						/* mm 980707 */
			{__create_or_truncate, __read_write, _IOFBF, __disk_file, __unoriented, 1};
#else
			{__create_or_truncate, __read_write, _IOFBF, __disk_file, 1};
#endif /* not __NO_WIDE_CHAR */				/* mm 980707 */

int remove(const char * name)
{
	return((__delete_file(name) == __no_io_error) ? 0 : -1);
}

#if __dest_os != __be_os  /*  bds 990121 */
/* beos has its own version of this funtion  bds */
int rename(const char * old_name, const char * new_name)
{
	return((__rename_file(old_name, new_name) == __no_io_error) ? 0 : -1);
}
#endif

char * tmpnam(char * name)
{
	__tls static char temp_name[L_tmpnam];
	
	if (name)
	{
		__temp_file_name(name, NULL);
		return(name);
	}
	else
	{
	/* 961219 KO */
#if _MWMT && (__dest_os	== __win32_os || __dest_os	== __wince_os)
		__temp_file_name(_GetThreadLocalData()->tmpnam_temp_name, NULL);
		return(_GetThreadLocalData()->tmpnam_temp_name);
#else
		__temp_file_name(temp_name, NULL);
		return(temp_name);
#endif
	}
}

FILE * tmpfile(void)
{
	FILE *	file;
	
	__stdio_atexit();
	
	__begin_critical_region(files_access);
	
	if ((file = __find_unopened_file()) != 0)
	{	
		__init_file(file, __temp_file_mode, 0, BUFSIZ);
		
		if (__open_temp_file(&file->handle))
		{
			file->mode.file_kind = __closed_file;
			file = NULL;		
		}
	}

	__end_critical_region(files_access);
	
	return(file);
}

#endif /* ndef _No_Disk_File_OS_Support */

int fclose(FILE * file)
{
	int	flush_result, close_result;
	
	if (file == NULL)               /* MM-960625 */
		return(EOF);                /* MM-960625 */
	if (file->mode.file_kind == __closed_file)
		return(0);
	
	flush_result = fflush(file);
	
	close_result = (*file->close_proc)(file->handle);
	
	file->mode.file_kind = __closed_file;
	file->handle         = 0;
	
	if (file->state.free_buffer)
#ifndef _No_Disk_File_OS_Support
		free(file->buffer);
#else
		return(-1);
#endif
	
	return((flush_result || close_result) ? EOF : 0);
}

#ifndef __m56800__		/* mm 981029 */
int fflush(FILE * file)
{
	fpos_t	position;                    /* mm 970708 */
	
	if (!file)
		return(__flush_all());
	
	if (file->state.error || file->mode.file_kind == __closed_file)
		return(EOF);
	
	if (file->mode.io_mode == __read)		/* mm 980430 */
		return 0;							/* mm 980430 */
	
	if (file->state.io_state >= __rereading)
		file->state.io_state = __reading;
	
	if (file->state.io_state == __reading)
		file->buffer_len = 0;
	
	if (file->state.io_state != __writing)
	{
		file->state.io_state = __neutral;  /* mm 970905 */
		return(0);
	}
	
#ifndef _No_Disk_File_OS_Support
	if (file->mode.file_kind != __disk_file || (position = ftell(file)) < 0)
		position = 0;
#else
	position = 0;
#endif
	
	if (__flush_buffer(file, NULL))
	{
		set_error(file);
		return(EOF);
	}
	
	file->state.io_state = __neutral;
	file->position       = position;
	file->buffer_len     = 0;
	
	return(0);
}

#endif /* #ifndef __m56800__ */        /* mm 981029 */

#ifndef _No_Disk_File_OS_Support

FILE * fopen(const char * name, const char * mode)
{
	FILE * file;
	
	__begin_critical_region(files_access);
	
	file = freopen(name, mode, __find_unopened_file());
	
	__end_critical_region(files_access);
	
	return(file);
}

FILE * freopen(const char * name, const char * mode, FILE * file)
{
	__file_modes	modes;
	
	__stdio_atexit();
	
	if (!file)
		return(NULL);
	
	fclose(file);
	clearerr(file);
	
	if (!__get_file_modes(mode, &modes))
		return(NULL);
	
	__init_file(file, modes, 0, BUFSIZ);
	
	if (__open_file(name, modes, &file->handle))
	{
		file->mode.file_kind = __closed_file;
	    if (file->state.free_buffer)       /* mm-960719 */
	    	free(file->buffer);            /* mm-960719 */
		return(NULL);
	}
	if (modes.io_mode & __append)			/* mm 990119 */
		fseek(file, 0, SEEK_END);			/* mm 990119 */
	
	return(file);
}

FILE * __handle_open(__file_handle handle, const char * mode)
{
	FILE * file;
	
	__begin_critical_region(files_access);
	
	file = __handle_reopen(handle, mode, __find_unopened_file());
	
	__end_critical_region(files_access);
	
	return(file);
}

FILE * __handle_reopen(__file_handle handle, const char * mode, FILE * file)
{
	__file_modes	modes;
	
	__stdio_atexit();
	
	if (!file)
		return(NULL);
	
	fclose(file);
	clearerr(file);
	
	if (!__get_file_modes(mode, &modes))
		return(NULL);
	
	__init_file(file, modes, 0, BUFSIZ);
	
	file->handle = handle;
	
	return(file);
}

int __get_file_modes(const char * mode, __file_modes * modes)
{
	const char *	mode_ptr = mode;
	unsigned long	mode_str;
	unsigned char	open_mode, io_mode;
	
	modes->file_kind = __disk_file;
#ifndef __NO_WIDE_CHAR						/* mm 980204 */
	modes->file_orientation = __unoriented;
#endif /* not __NO_WIDE_CHAR */				/* mm 980204 */
	modes->binary_io = 0;
	
	mode_str = *mode_ptr++;
	
	switch (mode_str)
	{
		case 'r':
			open_mode = __must_exist;
			break;
		
		case 'w':
			open_mode = __create_or_truncate;
			break;
		
		case 'a':
			open_mode = __create_if_necessary;
			break;
		
		default:
			return(0);
	}
	
	modes->open_mode = open_mode;
	
	switch (*mode_ptr++)
	{
		case 'b':
			modes->binary_io = 1;
			
			if (*mode_ptr == '+')
				mode_str = (mode_str << 8) | '+';
			
			break;
			
		case '+':
			mode_str = (mode_str << 8) | '+';
			
			if (*mode_ptr == 'b')
				modes->binary_io = 1;
			
			break;
	}
	
	switch (mode_str)
	{
		case 'r':
			io_mode = __read;
			break;
						
		case 'w':
			io_mode = __write;
			break;
			
		case 'a':
			io_mode = __write | __append;
			break;
			
		case 'r+':
			io_mode = __read_write;
			break;
			
		case 'w+':
			io_mode = __read_write;
			break;
			
		case 'a+':
			io_mode = __read_write | __append;
			break;
	}
	
	modes->io_mode = io_mode;
	
	return(1);
}

#endif /* ndef _No_Disk_File_OS_Support */

void __set_idle_proc(FILE * file, __idle_proc idle_proc)
{
	file->idle_proc = idle_proc;
}

/*  Change Record
 *	14-Aug-95 JFH  First code release.
 *	16-Oct-95 JFH  Modified fflush to flush read buffers for SIOUX
 *	13-Dec-95 JFH  Added idle_proc to FILE and __set_idle_proc(). Intended primarily for Mac I/O
 *								 (see file_io.mac.c).
 *	19-Feb-96 JFH  Added closed-file check to fflush.
 *	25-Apr-96 JFH  Modified fflush to account for multi-level 'ungetc'.
 *	29-Apr-96 JFH  Merged Win32 changes in.
 *						CTV
 *  96-Jun-25 MM   Added code to avoid bus error in fclose with null file pointer
 *  96-Jul-19 MM   Added code to prevent memory leak in freopen
 *	19-Dec-96 KO	Added some Win32 ifdefs to use my thread local data structure rather
 *              	than static local variables.
 *	961228	  bobk line 135 added for clean exits on win32
 *  11-Jul-97 SCM  Wrapped os-dependent functions in #ifndef __no_os.
 *	20-Jul-97 MEA  Changed __no_os to _No_Disk_File_OS_Support.
 * mm 970708  Inserted Be changes
 * mm 970905  Correction to mark file as _neutral when flushed in other than _writing mode.
 * mm 980204  Changes to support wide characters
 * mm 980430  The Standard does not define any action for fflush on a file opened in read only mode so do
 *			  nothing.  MW02733
 * mf 980512  wince changes
 * mm 980707  Corrected initialization of __temp_file_mode to allow for wide characters
 * mm 981029  Excluded fflush() for __m56800__
 * mm 990119  When a file is opened in append mode, make sure that it is positioned to eof IL9901-1347 
 * bds 990121 Added BEOS wrapper
 * mf  990301 removed calls to _GetThreadLocalData for single threaded lib ansix86st.lib
 */
