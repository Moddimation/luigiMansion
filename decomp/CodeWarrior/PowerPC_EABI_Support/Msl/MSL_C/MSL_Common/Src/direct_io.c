/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:32 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	direct_io.c
 *	
 *	Routines
 *	--------
 *		fread
 *		fwrite
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include "ansi_files.h"
#include "file_io.h"              /* mm 970708 */
#include "buffer_io.h"
#include "misc_io.h"

size_t fread(void * ptr, size_t memb_size, size_t num_memb, FILE * file)
{
	unsigned char *	read_ptr;
	size_t					num_bytes, bytes_to_go, bytes_read;
	int							ioresult, always_buffer;
	
#ifndef __NO_WIDE_CHAR										/* mm 980205 */
	if (fwide(file, 0) == 0)
		fwide(file, -1);
#endif /* __NO_WIDE_CHAR */                                 /* mm 980205 */

	bytes_to_go = memb_size * num_memb;
	
	if (!bytes_to_go || file->state.error || file->mode.file_kind == __closed_file)
		return(0);
	
	always_buffer = !file->mode.binary_io || file->mode.file_kind == __string_file ||
	                    file->mode.buffer_mode == _IOFBF;  /*MW-mm 961107*/
	
	if (file->state.io_state == __neutral)
		if (file->mode.io_mode & __read)
		{
			file->state.io_state = __reading;
			file->buffer_len     = 0;
		}
	
	if (file->state.io_state < __reading)
	{
		set_error(file);
		return(0);
	}

     /* mm 970708 begin */
	if (file->mode.buffer_mode & (_IOLBF | _IONBF)) {
		if (__flush_line_buffered_output_files()) {
			set_error(file);
			return 0;
		}
	}
     /* mm 970708 end */
	
	read_ptr    = (unsigned char *) ptr;
	bytes_read  = 0;
	
	if (bytes_to_go && file->state.io_state >= __rereading)
	{
		do
		{
	
#ifndef __NO_WIDE_CHAR										/* mm 980205 */
			if (fwide(file, 0) == 1)
			{
				*(wchar_t*)read_ptr = file->ungetwc_buffer[file->state.io_state - __rereading];
				read_ptr   += sizeof(wchar_t);
				bytes_read += sizeof(wchar_t);
				bytes_to_go-= sizeof(wchar_t);
			}
			else
			{
				*read_ptr++ = file->ungetc_buffer[file->state.io_state - __rereading];
				bytes_read++;
				bytes_to_go--;
			}
#else
			*read_ptr++ = file->ungetc_buffer[file->state.io_state - __rereading];
			bytes_read++;
			bytes_to_go--;
#endif /* __NO_WIDE_CHAR */                                 /* mm 980205 */
			
			
			file->state.io_state--;
		}
		while (bytes_to_go && file->state.io_state >= __rereading);
		
		if (file->state.io_state == __reading)
			file->buffer_len = file->saved_buffer_len;
	}
	
	if (bytes_to_go && (file->buffer_len || always_buffer))
		do
		{
			if (!file->buffer_len)
			{
				ioresult = __load_buffer(file, NULL, __align_buffer);
				
				if (ioresult)
				{
					if (ioresult == __io_error)
						set_error(file);
					else
						set_eof(file);
					bytes_to_go = 0;
					break;
				}
			}
			
			num_bytes = file->buffer_len;
			
			if (num_bytes > bytes_to_go)
				num_bytes = bytes_to_go;
			
			memcpy(read_ptr, file->buffer_ptr, num_bytes);
			
			read_ptr    += num_bytes;
			bytes_read  += num_bytes;
			bytes_to_go -= num_bytes;
			
			file->buffer_ptr += num_bytes;
			file->buffer_len -= num_bytes;
		}
		while (bytes_to_go && always_buffer);
	
	if (bytes_to_go && !always_buffer)
	{
		unsigned char *	save_buffer = file->buffer;
		size_t					save_size   = file->buffer_size;
		
		file->buffer      = read_ptr;
		file->buffer_size = bytes_to_go;
		
		ioresult = __load_buffer(file, &num_bytes, __dont_align_buffer);
		
		if (ioresult)
			if (ioresult == __io_error)
				set_error(file);
			else
				set_eof(file);
		
		bytes_read += num_bytes;
		
		file->buffer        = save_buffer;
		file->buffer_size   = save_size;
		
		__prep_buffer(file);
		
		file->buffer_len = 0;
	}
	
	return(bytes_read / memb_size);                         /* mm 980203 */
}

size_t fwrite(const void * ptr, size_t memb_size, size_t num_memb, FILE * file)
{
	unsigned char *	write_ptr;
	size_t					num_bytes, bytes_to_go, bytes_written;
	int							ioresult, always_buffer;
	
	
#ifndef __NO_WIDE_CHAR										/* mm 980205 */
	if (fwide(file, 0) == 0)
		fwide(file, -1);
#endif /* __NO_WIDE_CHAR */                                 /* mm 980205 */

	bytes_to_go = memb_size * num_memb;
	
	if (!bytes_to_go || file->state.error || file->mode.file_kind == __closed_file)
		return(0);

	if (file->mode.file_kind == __console_file)
		__stdio_atexit();
	
	always_buffer = !file->mode.binary_io ||
					 file->mode.file_kind   == __string_file	||
					 file->mode.buffer_mode == _IOFBF           ||  /*MW-mm 961107*/
					 file->mode.buffer_mode == _IOLBF;
	
	if (file->state.io_state == __neutral)
		if (file->mode.io_mode & __write)
		{
#if !defined( __BEOS__) && !defined(_No_Disk_File_OS_Support)   /* we do O_APPEND on open */ /* mm 970708 */
			if (file->mode.io_mode & __append)
				if (fseek(file, 0, SEEK_END))
					return(0);
#endif                                          /* mm 970708 */
						
			file->state.io_state = __writing;
			
			__prep_buffer(file);
		}
	
	if (file->state.io_state != __writing)
	{
		set_error(file);
		return(0);
	}

	write_ptr     = (unsigned char *) ptr;
	bytes_written = 0;
	
	if (bytes_to_go && (file->buffer_ptr != file->buffer || always_buffer))
	{
		file->buffer_len = file->buffer_size - (file->buffer_ptr - file->buffer);
		
		do
		{
			unsigned char *	newline = NULL;
			
			num_bytes = file->buffer_len;
			
			if (num_bytes > bytes_to_go)
				num_bytes = bytes_to_go;
			
#ifndef _Unbuffered_IO

			if (file->mode.buffer_mode == _IOLBF && num_bytes)
				if((newline = (unsigned char *) __memrchr(write_ptr, '\n', num_bytes)) != NULL)
					num_bytes = newline + 1 - write_ptr;
			
#endif
			
			if (num_bytes)
			{
				memcpy(file->buffer_ptr, write_ptr, num_bytes);
				
				write_ptr     += num_bytes;
				bytes_written += num_bytes;
				bytes_to_go   -= num_bytes;
				
				file->buffer_ptr += num_bytes;
				file->buffer_len -= num_bytes;
			}
			
			if (!file->buffer_len && (file->mode.file_kind == __string_file))	/* mm 980715 */
			{																	/* mm 980715 */
				bytes_written += bytes_to_go;									/* mm 980715 */
				break;															/* mm 980715 */
			}																	/* mm 980715 */
			
			if (!file->buffer_len || newline != NULL || (file->mode.buffer_mode == _IONBF))  /* mm 970716 */
			{
				ioresult = __flush_buffer(file, NULL);
				
				if (ioresult)
				{
					set_error(file);
					bytes_to_go = 0;
					break;
				}
			}
		}
		while (bytes_to_go && always_buffer);
	}
	
	if (bytes_to_go && !always_buffer)
	{
		unsigned char *	save_buffer = file->buffer;
		size_t					save_size   = file->buffer_size;
		
		file->buffer      = write_ptr;
		file->buffer_size = bytes_to_go;
		file->buffer_ptr  = write_ptr + bytes_to_go;
		
		if (__flush_buffer(file, &num_bytes) != __no_io_error)
			set_error(file);
		
		bytes_written += num_bytes;
		
		file->buffer      = save_buffer;
		file->buffer_size = save_size;
		
		__prep_buffer(file);
		
		file->buffer_len = 0;
	}
	
	if (file->mode.buffer_mode != _IOFBF)
		file->buffer_len = 0;
		
	
	return((bytes_written + memb_size - 1) / memb_size);
}

/*  Change Record
 *	25-Aug-95 JFH  First code release.
 *	09-Jan-96 JFH  At the behest of Be, I put in real output line buffering.
 *  22-Jan-96 JFH  Added casts from (void *) for C++ compatibility.
 *	12-Feb-96 JFH  Added call to __stdio_atexit in fwrite for console files (so they will
 *								 get flushed and closed properly).
 *  19-Feb-96 JFH  Added check for closed files to fread and fwrite.
 *	25-Apr-96 JFH  Modified fread to account for multi-level 'ungetc'.
 *  mm 961107      Make sure buffering occurs when _IOFBF is set.
 *	01-Jan-97 Mani@be	Make unbuffered stdio really work.
 *	09-Jul-97 SCM  Disabled output line buffering support when _Unbuffered_IO is set.
 *				   Modified to support unbuffered, non-binary files.
 *	20-Jul-97 MEA  Changed __no_os to _No_Disk_File_OS_Support.
 *  mm 970708  Inserted Be changes
 *  mm 970716  Flush unbuffered files after writing.
 *  mm 980203  Corrected return value to fread to round down to the number of complete elements
              successfully read.  MW03272
 *  mm 980205  Support for wide-characters
 *  mm 980715  Made exit from fwrite with string file when buffer (i.e. string) full MW07942    
 */
