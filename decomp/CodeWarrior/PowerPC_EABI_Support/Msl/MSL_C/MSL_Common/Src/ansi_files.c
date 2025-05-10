/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 00:54:45 $ 
 *  $Revision: 1.10.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	ansi_files.c
 *	
 *	Routines
 *	--------
 *		__find_unopened_file
 *		__init_file
 *		__close_all
 *		__flush_all
 *
 *
 */

#include <stdio.h>
#include "ansi_files.h"
#include "console_io.h"
#include "critical_regions.h"
#include "file_io.h"
#include "string_io.h"
#include <stdlib.h>  /* mf 990111-- new FILE creation scheme requires prototype for malloc */
#include <string.h>   /* mf 990111-- new FILE creation scheme requires prototype for memset */ 
#if macintosh && !defined(__dest_os)               /*MW-mm 960927a*/
  #define __dest_os __mac_os                       /*MW-mm 960927a*/
#endif                                             /*MW-mm 960927a*/

#if __dest_os == __undef_os
#error __dest_os undefined
#endif

#ifdef _Unbuffered_Console

	#define console_buff_mode	_IONBF

	#define console_buff_size	1
	
	#define	stdin_buff			&__files[0].char_buffer
	#define	stdout_buff			&__files[1].char_buffer
	#define	stderr_buff			&__files[2].char_buffer

#else

	#define console_buff_mode	_IOLBF

	#define console_buff_size	256
	
	typedef unsigned char	console_buff[console_buff_size];
	
	static console_buff	stdin_buff;
	static console_buff	stdout_buff;
	static console_buff	stderr_buff;
	
#endif

	FILE	__files[_STATIC_FILES] =   /* mm 961031 */  /*MW-mm 961203 */	/* mm 981007 */
	{
		{
			0,												/* handle */
			{												/* mode   */
				__must_exist,								/* open_mode   */
				__read,										/* io_mode     */
				console_buff_mode,							/* buffer_mode */    /* mm 981015 */
				#if __dest_os==__wince_os
				__disk_file,
				#else
				__console_file,								/* file_kind   */
				#endif
#ifndef __NO_WIDE_CHAR				/* mm 980204 */
				__unoriented,								/* file orientation */
#endif /* not __NO_WIDE_CHAR */		/* mm 980204 */
				0											/* binary_io   */
			},
			{												/* state */
				/*__reading,*/								/* io_state    */
				__neutral, /*970320 bkoz*/
				0,											/* free_buffer */
				0,											/* eof         */
				0											/* error       */
			},
#ifndef _No_Disk_File_OS_Support															/* 981007 */
			0,												/* is_dynamically_allocated */ 	/* 981007 */
#endif /* _No_Disk_File_OS_Support */														/* 981007 */
			0,												/* char_buffer      */
			0,												/* char_buffer_overflow */
			{0, 0},											/* ungetc_buffer    */
#ifndef __NO_WIDE_CHAR				/* mm 980204 */
			{0, 0},											/* ungetwc_buffer   */
#endif /* not __NO_WIDE_CHAR */		/* mm 980204 */
			0,												/* position         */
			stdin_buff,										/* buffer           */
			console_buff_size,								/* buffer_size      */
			stdin_buff,										/* buffer_ptr       */
			0,												/* buffer_len       */
			0,												/* buffer_alignment */
			0,												/* saved_buffer_len */
			0,												/* buffer_pos       */
			0,												/* position_proc    */
#if	(__dest_os == __win32_os || __dest_os == __wince_os)
			__read_file,									/* read_proc        */
			__write_file,									/* write_proc       */
			__close_file,									/* close_proc       */
#elif defined(__m56800__)															 /* mm 981029 */
			0,												/* read_proc        */   /* mm 981015 */
			0,												/* write_proc       */   /* mm 981015 */
			0,												/* close_proc       */   /* mm 981015 */
#else
#ifndef	_No_Console	/* ad 1.28.99  */
			__read_console,									/* read_proc        */
			__write_console,								/* write_proc       */
			__close_console,								/* close_proc       */
#endif
#endif   /* (__dest_os == __win32_os || __dest_os == __wince_os) */
			0,												/* idle_proc		*/
#ifndef _No_Disk_File_OS_Support													/* 981007 */
			&__files[1]										/* next structure   */	/* mm 981007 */
#endif /* _No_Disk_File_OS_Support */												/* 981007 */
		},
		{
	#if (__dest_os == __be_os || __dest_os == __wince_os || __dest_os == __win32_os || defined(__BUILDING_MPW__))
			1,												/* handle */
	#else
			0,												/* handle */
	#endif
			{												/* mode   */
				__must_exist,								/* open_mode   */
				__write,									/* io_mode   */
				console_buff_mode,							/* buffer_mode */        /* mm 981015 */
				#if __dest_os==__wince_os
				__disk_file,
				#else
				__console_file,								/* file_kind   */
				#endif
#ifndef __NO_WIDE_CHAR				/* mm 980204 */
				__unoriented,								/* file orientation */
#endif /* not __NO_WIDE_CHAR */		/* mm 980204 */
				0											/* binary_io   */
			},
			{												/* state */
				/*__writing,	970313 bkoz*/
				__neutral,									/* io_state    */
				0,											/* free_buffer */
				0,											/* eof         */
				0											/* error       */
			},
#ifndef _No_Disk_File_OS_Support															/* 981007 */
			0,												/* is_dynamically_allocated */ 	/* 981007 */
#endif /* _No_Disk_File_OS_Support */														/* 981007 */
			0,												/* char_buffer      */
			0,												/* char_buffer_overflow */
			{0, 0},											/* ungetc_buffer    */
#ifndef __NO_WIDE_CHAR				/* mm 980204 */
			{0, 0},											/* ungetwc_buffer   */
#endif /* not __NO_WIDE_CHAR */		/* mm 980204 */
			0,												/* position         */
			stdout_buff,									/* buffer           */
			console_buff_size,								/* buffer_size      */
			stdout_buff,									/* buffer_ptr       */
			0,												/* buffer_len       */
			0,												/* buffer_alignment */
			0,												/* saved_buffer_len */
			0,												/* buffer_pos       */
			0,												/* position_proc    */
#if	(__dest_os == __win32_os || __dest_os == __wince_os)
			__read_file,									/* read_proc        */
			__write_file,									/* write_proc       */
			__close_file,									/* close_proc       */
#elif defined(__m56800__)															 /* mm 981029 */
			0,												/* read_proc        */   /* mm 981015 */
			0,												/* write_proc       */   /* mm 981015 */
			0,												/* close_proc       */   /* mm 981015 */
#else
#ifndef	_No_Console	/* ad 1.28.99  */
			__read_console,									/* read_proc        */
			__write_console,								/* write_proc       */
			__close_console,								/* close_proc       */
#endif
#endif   /* (__dest_os == __win32_os || __dest_os == __wince_os) */
			0,												/* idle_proc		*/
#ifndef _No_Disk_File_OS_Support															/* 981007 */
			&__files[2]										/* next structure   */	/* mm 981007 */
#endif /* _No_Disk_File_OS_Support */														/* 981007 */
		},
		{
	#if __dest_os == __be_os || __dest_os == __wince_os || __dest_os == __win32_os || defined(__BUILDING_MPW__)
			2,												/* handle */
	#else
			0,												/* handle */
	#endif
			{												/* mode   */
				__must_exist,								/* open_mode   */
				__write,									/* io_mode   */
				_IONBF,										/* buffer_mode   Mani 970101*/   /* mm 970708 */
				#if __dest_os==__wince_os
				__disk_file,
				#else
				__console_file,								/* file_kind   */
				#endif
#ifndef __NO_WIDE_CHAR				/* mm 980204 */
				__unoriented,								/* file orientation */
#endif /* not __NO_WIDE_CHAR */		/* mm 980204 */
				0											/* binary_io   */
			},
			{												/* state */
				/*__writing, 970313 bkoz	*/				/* io_state    */
				__neutral,
				0,											/* free_buffer */
				0,											/* eof         */
				0											/* error       */
			},
#ifndef _No_Disk_File_OS_Support															/* 981007 */
			0,												/* is_dynamically_allocated */ 	/* 981007 */
#endif /* _No_Disk_File_OS_Support */														/* 981007 */
			0,												/* char_buffer      */
			0,												/* char_buffer_overflow */
			{0, 0},											/* ungetc_buffer    */
#ifndef __NO_WIDE_CHAR				/* mm 980204 */
			{0, 0},											/* ungetwc_buffer   */
#endif /* not __NO_WIDE_CHAR */		/* mm 980204 */
			0,												/* position         */
			stderr_buff,									/* buffer           */
			console_buff_size,								/* buffer_size      */
			stderr_buff,									/* buffer_ptr       */
			0,												/* buffer_len       */
			0,												/* buffer_alignment */
			0,												/* saved_buffer_len */
			0,												/* buffer_pos       */
			0,												/* position_proc    */
#if	__dest_os == __win32_os || __dest_os == __wince_os
			__read_file,									/* read_proc        */
			__write_file,									/* write_proc       */
			__close_file,									/* close_proc       */
#elif defined(__m56800__)															 /* mm 981029 */
			0,												/* read_proc        */   /* mm 981015 */
			0,												/* write_proc       */   /* mm 981015 */
			0,												/* close_proc       */   /* mm 981015 */
#else
#ifndef	_No_Console	/* ad 1.28.99  */
			__read_console,									/* read_proc        */
			__write_console,								/* write_proc       */
			__close_console,								/* close_proc       */
#endif
#endif
			0,												/* idle_proc		*/
#ifndef _No_Disk_File_OS_Support															/* 981007 */
			&__files[3]										/* next structure   */	/* mm 981007 */
#endif /* _No_Disk_File_OS_Support */														/* 981007 */
		}
	};


#ifndef _No_Disk_File_OS_Support
FILE * __find_unopened_file(void)
{
	FILE *	p = __files[2].next_file_struct;	/* mm 981007 */
	FILE *  plast;								/* mm 981007 */
	
	while(p != NULL)   /*MW-mm 961203 */ 		/* mm 981007 */
	{
		if (p->mode.file_kind == __closed_file)
			return(p);
		else									/* mm 981007 */
		{										/* mm 981007 */
			plast = p;							/* mm 981007 */
			p = p->next_file_struct;			/* mm 981007 */
		}										/* mm 981007 */
	}
	
	if ((p = malloc(sizeof (FILE))) != NULL)	/* mm 981007 */
	{											/* mm 981007 */
		memset((void *)p, 0, sizeof(FILE));		/* mm 981007 */
		p->is_dynamically_allocated = 1;		/* mm 981007 */
		plast->next_file_struct = p;			/* mm 981007 */
		return(p);								/* mm 981007 */
	}
	return(NULL);
}
#endif /* ndef _No_Disk_File_OS_Support */

void __init_file(FILE * file, __file_modes mode, char * buff, size_t size)
{
	file->handle            = 0;
	file->mode              = mode;
	file->state.io_state    = __neutral;
	file->state.free_buffer = 0;
	file->state.eof         = 0;
	file->state.error       = 0;
	file->position          = 0;
	
	if (size)
		setvbuf(file, buff, _IOFBF, size);
	else
		setvbuf(file, 0,    _IONBF, 0);
	
	file->buffer_ptr = file->buffer;
	file->buffer_len = 0;
	
	switch (file->mode.file_kind)
	{
#ifndef _No_Disk_File_OS_Support
		case __disk_file:
			file->position_proc    = __position_file;
			file->read_proc        = __read_file;
			file->write_proc       = __write_file;
			file->close_proc       = __close_file;
			break;
#endif /* ndef _No_Disk_File_OS_Support */
		
		case __string_file:
			file->position_proc    = 0;
			file->read_proc        = __read_string;
			file->write_proc       = __write_string;
			file->close_proc       = __close_string;
			break;
	}
	
	file->idle_proc = 0;
}

void __close_all(void)
{
	FILE *	p = &__files[0];									/* mm 981007 */
	FILE *  plast;												/* mm 981007 */
#ifdef _No_Disk_File_OS_Support									/* mm 981020 */
	int file_index = 1;											/* mm 981020 */
#endif															/* mm 981020 */
	
	__begin_critical_region(files_access);
	
	while(p != NULL)   											/* mm 981007 */
	{
		if (p->mode.file_kind != __closed_file) 				/* mm 981007 */
			fclose(p);
#ifndef _No_Disk_File_OS_Support								/* mm 981020 */
		plast = p;												/* mm 981007 */
		p     = p->next_file_struct;							/* mm 981007 */
		if (plast->is_dynamically_allocated)					/* mm 981007 */
			free(plast);										/* mm 981007 */
		else													/* mm 981007 */
		{														/* mm 981007 */
			plast->mode.file_kind = __unavailable_file;			/* mm 981007 */
			if ((p != NULL) && p->is_dynamically_allocated)		/* mm 981007 */
				plast->next_file_struct = NULL;					/* mm 981007 */
		}														/* mm 981007 */
#else															/* mm 981020 */
		if (file_index < _STATIC_FILES)							/* mm 981020 */
			p = &__files[file_index++];							/* mm 981020 */
		else													/* mm 981020 */
			p = NULL;											/* mm 981020 */
#endif															/* mm 981020 */
	}
	
	__end_critical_region(files_access);
}


int __flush_all(void)
{
	int		result = 0;
	FILE *	p = &__files[0];									/* mm 981007 */
#ifdef _No_Disk_File_OS_Support									/* mm 981020 */
	int file_index = 1;											/* mm 981020 */
#endif															/* mm 981020 */
	
	while (p != NULL)											/* mm 981007 */
	{
		if (p->mode.file_kind != __closed_file)
			if (fflush(p))
				result = EOF;
#ifndef _No_Disk_File_OS_Support								/* mm 981020 */
		p = p->next_file_struct;								/* mm 981007 */
#else															/* mm 981020 */
		if (file_index < _STATIC_FILES)							/* mm 981020 */
			p = &__files[file_index++];							/* mm 981020 */
		else													/* mm 981020 */
			p = NULL;											/* mm 981020 */
#endif															/* mm 981020 */
	}
	
	return(result);
}


          /* mm 970708  begin insert */
int __flush_line_buffered_output_files(void)
{
	int		result = 0;											/* mm 981007 */
	FILE *	p = &__files[0];									/* mm 981007 */
#ifdef _No_Disk_File_OS_Support									/* mm 981020 */
	int file_index = 1;											/* mm 981020 */
#endif															/* mm 981020 */
	
	while (p != NULL)											/* mm 981007 */
	{
		if (p->mode.file_kind != __closed_file  &&
			(p->mode.buffer_mode & _IOLBF)  &&
			p->state.io_state == __writing) 
		{
			if (fflush(p)) 
			{
				result = EOF;
			}
		}
#ifndef _No_Disk_File_OS_Support								/* mm 981020 */
		p = p->next_file_struct;								/* mm 981007 */
#else															/* mm 981020 */
		if (file_index < _STATIC_FILES)							/* mm 981020 */
			p = &__files[file_index++];							/* mm 981020 */
		else													/* mm 981020 */
			p = NULL;											/* mm 981020 */
#endif															/* mm 981020 */
	}
	
	return(result);
}
          /* mm 970708  end insert */


/*  mm 981015a If these routines are really needed, they need updating to account for dynamically */
/*  allocated file structures */
/*
int __file_to_num(FILE * file)
{
	return(file - __files);
}

FILE * __num_to_file(int file_num)
{
	return(&__files[file_num]);
}
*/                       /* mm 981015a */ 
/*     Change record
 *	05-Sep-95 JFH  First code release.
 *	12-Oct-95 JFH  Made __files static and renamed. Added __file_to_num and
 *								 __num_to_file, intended to be used by unix functions. They
 *								 weren't used in the end, but I left them anyway.
 *	13-Dec-95 JFH  Added inits to zero for idle_proc.
 *	19-Feb-96 JFH  Added a conditional that leaves stdin, stdout, and stderr closed
 *								 so that you can omit SIOUX (and Unix routines used by SIOUX) and
 *								 still use the ANSI library.
 *	25-Apr-96 JFH  Modified 'files' initialization to account for multi-level 'ungetc'.
 *	26-Apr-96 JFH  Merged Win32 changes in.
 *						CTV
 *  mm 960927a Inserted setting of __dest_os to __mac_os when not otherwise set.
 *  mm 961031  Changes for Pascal
 *  mm 961203  Changed files to __files
 *	Mani 970101	Make stderr unbuffered output really work.
 *	bkoz 970318 lines ? 146, 103 changed stdin, stdout, stderr to __neutral
 *  12-Jun-97 FS	Add #if __dest_os == __ppc_eabi_bare for PPC 821
 *  09-Jul-97 SCM	Converted "#if __dest_os == __ppc_eabi_bare" to
 *					"#ifndef __no_os" for more general-purpose embedded
 *					use.  Also added support for unbuffered console IO.
 *	20-Jul-97 MEA  Changed __no_os to _No_Disk_File_OS_Support.
 * mm 970708  Inserted Be changes
 * mm 980204  Changes to file structure to support wide characters
 * mf 980512  wince changes
 * mm 980509  Removed static attribute so that it can be linked without error when _No_Console 
 *				is defined.  MW07076
 * mm 981007  Changes to make allocation of file structures dynamic
 * mm 981015  Added wrappers and things to support DSP56800
 * mm 981015a Commented out __file_to_num and __num_to_file while investigating whether they are really needed.
 * mm 981020  Corrections to dynamic file structures changes for _No_Disk_File_OS_Support
 * mea 981023  re-added the console_buff_mode.  Please see ansi_prefix.PPCEABI.bare.h
 * mm 981029  Changed __DSP568 to __m56800__ 
 * mf 990117  added include of stdlib.h/string.h for malloc,memset prototypes
 * ad 990128  Add _No_Console wrappers
 */
