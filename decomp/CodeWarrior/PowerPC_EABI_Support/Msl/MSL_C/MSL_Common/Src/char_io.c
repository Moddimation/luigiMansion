/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:32 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	char_io.c
 *
 *	Routines
 *	--------
 *		fgetc
 *		getc
 *		getchar
 *
 *		fgets
 *		gets
 *
 *		ungetc
 *
 *		fputc
 *		putc
 *		putchar
 *
 *		fputs
 *		puts
 *
 *		__get_char
 *		__put_char
 *
 *		__ungotten
 *
 *
 */

#include <stdio.h>
#include "ansi_files.h"
#include "buffer_io.h"
#include "char_io.h"
#include "misc_io.h"

#ifndef __m56800__						/* mm 981029 */
int __get_char(FILE * file)
{
	int	state, ioresult;
	
	file->buffer_len = 0;
	
	if (file->state.error || file->mode.file_kind == __closed_file)
		return(EOF);
	
	state = file->state.io_state;
	
	if (state == __writing || !(file->mode.io_mode & __read))
	{
		set_error(file);
		return(EOF);
	}
	
	if (state >= __rereading)
	{
		file->state.io_state--;
		
		if (state == __rereading)
			file->buffer_len = file->saved_buffer_len;
		
		return(file->ungetc_buffer[state - __rereading]);
	}
	else
		file->state.io_state = __reading;
	
	ioresult = __load_buffer(file, NULL, __align_buffer);
	
	if (ioresult || !file->buffer_len)
	{
		if (ioresult == __io_error)
			set_error(file);
		else
			set_eof(file);
		
		return(EOF);
	}
	
	file->buffer_len--;
	
	return(*file->buffer_ptr++);
}
#endif  /* #ifndef __m56800__ */           /* mm 981029 */

int fgetc(FILE * file)
{
	return(getc(file));
}

int (getc)(FILE * file)
{
	return(getc(file));
}

#ifndef __m56800__						/* mm 981029 */
int (getchar)(void)
{
	return(getchar());
}
#endif  /* #ifndef __m56800__ */           /* mm 981029 */

char * fgets(char * s, int n, FILE * file)
{
	char *	p = s;
	int			c;
	
	if (--n < 0)
		return(NULL);
	
	if (n)
		do
		{
			c = getc(file);
			
			if (c == EOF)
				if (file->state.eof && p != s)
					break;
				else
					return(NULL);
			
			*p++ = c;
		}
		while (c != '\n' && --n);
	
	*p = 0;
	
	return(s);
}

char * gets(char * s)
{
	char *	p = s;
	int			c;
	FILE *	file = stdin;
	
	for (;;)
	{
		c = getc(file);
		
		if (c == EOF)
			if (file->state.eof && p != s)
				break;
			else
				return(NULL);
		
		if (c == '\n')
			break;
		
		*p++ = c;
	}
	
	*p = 0;
	
	return(s);
}

#ifndef __m56800__						/* mm 981029 */
int ungetc(int c, FILE * file)
{
	int state = file->state.io_state;
	
#ifndef __NO_WIDE_CHAR										/* mm 980205 */
	if (fwide(file, -1) >= 0)
		return(EOF);
#endif /* __NO_WIDE_CHAR */                                 /* mm 980205 */

	if (state == __writing || state == __rereading + __ungetc_buffer_size - 1 || c == EOF)
		return(EOF);

	if (state < __rereading)
	{	
		file->saved_buffer_len = file->buffer_len;
		file->buffer_len       = 0;
		
		state = file->state.io_state = __rereading;
	}
	else
		state = ++file->state.io_state;
	
	file->ungetc_buffer[state - __rereading] = c;
	
	file->state.eof = 0;
	
	return((unsigned char) c);
}
#endif  /* #ifndef __m56800__ */           /* mm 981029 */

int __ungotten(FILE * file)
{
	if (file->state.io_state < __rereading)
		return(EOF);
	
	return(file->ungetc_buffer[file->state.io_state - __rereading]);
}

#ifndef __m56800__						/* mm 981029 */
int __put_char(int c, FILE * file)
{
	int	kind = file->mode.file_kind;
	
	file->buffer_len = 0;
	
	if (file->state.error || kind == __closed_file)
		return(EOF);
	
	if (kind == __console_file)
		__stdio_atexit();
	
	if (file->state.io_state == __neutral)
		if (file->mode.io_mode & __write)
		{
#ifndef _No_Disk_File_OS_Support
			if (file->mode.io_mode & __append)
				if (fseek(file, 0, SEEK_END))
					return(0);
#endif
			
			file->state.io_state = __writing;
			
			__prep_buffer(file);
		}
	
	if (file->state.io_state != __writing)
	{
		set_error(file);
		return(EOF);
	}
	
	if (file->mode.buffer_mode == _IOFBF || file->buffer_ptr - file->buffer == file->buffer_size)
		if (__flush_buffer(file, NULL))
		{
			set_error(file);
			return(EOF);
		}
	
	 file->buffer_len--;
	*file->buffer_ptr++ = c;
	
	if (file->mode.buffer_mode != _IOFBF)
	{
		if ((file->mode.buffer_mode == _IONBF || c == '\n'))
			if (__flush_buffer(file, NULL))
			{
				set_error(file);
				return(EOF);
			}
		
		file->buffer_len = 0;
	}
	
	return((unsigned char) c);
}

#endif  /* #ifndef __m56800__ */           /* mm 981029 */

int fputc(int c, FILE * file)
{
	return(putc(c, file));
}

int (putc)(int c, FILE * file)
{
	return(putc(c, file));
}

#ifndef __m56800__						/* mm 981029 */
int (putchar)(int c)
{
	return(putchar(c));
}
#endif  /* #ifndef __m56800__ */           /* mm 981029 */

int fputs(const char * s, FILE * file)
{
	int	c;
	
	while ((c = *s++) != 0)
		if (putc(c, file) == EOF)
			return(EOF);
	
	return(0);
}

int puts(const char * s)
{
	int			c;
	FILE *	file = stdout;
	
	while ((c = *s++) != 0)
		if (putc(c, file) == EOF)
			return(EOF);
	
	c = '\n';
	
	if (putc(c, file) == EOF)
		return(EOF);
	
	return(0);
}

/*  Change Record
 *	08-Sep-95 JFH  First code release.
 *	16-Oct-95 JFH  Added __ungotten for SIOUX PascalHook
 *	08-Jan-96 JFH  At the behest of Be, I put in real output line buffering.
 *  22-Jan-96 JFH  Bracketed getc(), getchar(), putc(), and putchar() by #ifndef __cplusplus
 *								 (they are inlined in <stdio.h>).
 *	12-Feb-96 JFH  Added call to __stdio_atexit in __put_char for console files (so they will
 *								 get flushed and closed properly).
 *  19-Feb-96 JFH  Added check for closed files to __get_char and __put_char.
 *   4-Mar-96 JFH  Allowed ungetc to work when a file is in the __neutral io_state.
 *	25-Apr-96 JFH  Modified __get_char, ungetc, and __ungotten to account for multi-level 'ungetc'.
 *  12-Jun-97 FS   Added #if __dest_os == __ppc_eabi_bare for Motorola 821
 *  10-Jul-97 SCM  Removed __ppc_eabi_bare conditions after implementing
 *					serial console IO for non-os systems.
 *	20-Jul-97 MEA  Changed __no_os to _No_Disk_File_OS_Support.
 *  mm 980204      Changes to file structure to support wide characters
 *  mm 981029     Excluded certain functions for __m56800__
*/