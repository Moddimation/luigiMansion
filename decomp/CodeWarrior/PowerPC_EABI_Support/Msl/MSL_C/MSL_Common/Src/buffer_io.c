/*  Metrowerks Standard Library  */

/*  $Date: 1999/08/03 11:55:09 $ 
 *  $Revision: 1.7.4.3 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	buffer_io.c
 *	
 *	Routines
 *	--------
 *		setvbuf
 *		setbuf
 *
 *		__convert_from_newlines
 *		__convert_to_newlines
 *
 *		__prep_buffer
 *		__load_buffer
 *		__flush_buffer
 *
 *	Implementation Notes
 *	--------------------
 *
 *		Technically, there is no such thing as "unbuffered I/O" (_IONBF) in this
 *		implementation. With two exceptions (noted below) all I/O passes through a
 *		buffer.
 *		
 *		_IONBF forces the use of a single-character internal buffer that is
 *		associated with each file (whether used or not). Both _IOLBF and _IOFBF
 *		indicate the use of either a user-supplied or an internally-allocated
 *		buffer. A newly opened file effectively has already had a
 *		
 *		   setvbuf(f, NULL, _IOFBF, BUFSIZ)
 *		
 *		done for it. Any subsequent setbuf or setvbuf disposes of this buffer.
 *		Console input via SIOUX is line buffered automatically; there is no way to
 *		force it to do anything else.
 *		
 *		fread() and fwrite() will bypass buffers for binary I/O once the buffers
 *		are empty. Thus, if you are *only* doing fread() and/or fwrite() on a file
 *		in binary mode, then perhaps it makes sense to use _IONBF on a file;
 *		otherwise, using _IONBF will result in rather poor performance.
 *		
 *		While reading from/writing to a buffer, buffer_ptr points to the next byte
 *		to be read from/written to. buffer_pos indicated the file_position of the
 *		first byte in the buffer. Thus, the current file position can be inferred
 *		from buffer_pos and buffer_ptr (as well as the base address of the
 *		buffer). buffer_ptr is also used, when writing, to infer the number of
 *		bytes written. buffer_size is the number of bytes available in the buffer.
 *		buffer_len indicates the number of bytes left in the buffer to read or the
 *		number of bytes left in the buffer to be written to.
 *		
 *		The getc/putc macros depend on buffer_len being zero when a file's buffer
 *		is empty/full. This forces the macros to call the internal routines
 *		__get_char/__put_char, which are capable of much more sophisticated
 *		handling.
 *		
 *		buffer_len is also set to zero in cases where it is necessary to bypass
 *		the macros:
 *		
 *		o When calling fseek, fsetpos, rewind, and fflush. All these operations
 *		  force the buffer to flush and go to a "neutral" state where either
 *		  reading or writing is permitted next. The macros are thus disabled until
 *		  after the next byte is read/written. Note that it is safe at this time
 *		  to call setbuf or setvbuf, even though the standard says you can only
 *		  do it before the first I/O operation.
 *		  
 *		o Whenever the file's error flag is set. Thus, the macros are disabled and
 *		  all the I/O routines will punt until explicit action is taken to clear
 *		  the error flag.
 *		
 *		o At all times when writing a file in _IONBF or _IOLBF buffer mode:
 *		
 *		    _IONBF needs to flush the "buffer" after every character.
 *
 *			_IOLBF needs to watch for and flush after every newline character.
 *
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stddef.h>           /* mm 971031 */
#include "buffer_io.h"

#if __dest_os == __undef_os
#error __dest_os undefined
#endif

#if (__dest_os == __mac_os) && \
    !__option(mpwc_newline)  /*  vss 980825  */

void __convert_from_newlines(unsigned char * buf, size_t * n)    /*mm 970210*/
{                                                                /*mm 970210*/
	unsigned char *	p = buf;                                     /*mm 970210*/
	size_t			i = *n;                                      /*mm 970210*/
	while (i--)                                                  /*mm 970210*/
	{                                                            /*mm 970210*/
		if (*p=='\r') *p = '\n';                                 /*mm 970210*/
		else if (*p=='\n') *p = '\r';                            /*mm 970210*/
		p++;                                                     /*mm 970210*/
	}                                                            /*mm 970210*/
}

void __convert_to_newlines(unsigned char * buf, size_t * n)      /*mm 970210*/
{                                                                /*mm 970210*/
	unsigned char *	p = buf;                                     /*mm 970210*/
	size_t			i = *n;                                      /*mm 970210*/
	while (i--)                                                  /*mm 970210*/
	{                                                            /*mm 970210*/
		if (*p=='\n') *p = '\r';                                 /*mm 970210*/
		else if (*p=='\r') *p = '\n';                            /*mm 970210*/
		p++;                                                     /*mm 970210*/
	}                                                            /*mm 970210*/
}

#else

void __convert_from_newlines(unsigned char * p, size_t * n)
{
#pragma unused(p,n)
}

void __convert_to_newlines(unsigned char * p, size_t * n)
{
#pragma unused(p,n)
}

#endif

void __prep_buffer(FILE * file)
{
	file->buffer_ptr    = file->buffer;
	file->buffer_len    = file->buffer_size;	
	file->buffer_len   -= file->position & file->buffer_alignment;
	file->buffer_pos    = file->position;
}

int __load_buffer(FILE * file, size_t * bytes_loaded, int alignment)
{
	int	ioresult;
	
	__prep_buffer(file);
	
	if (alignment == __dont_align_buffer)
		file->buffer_len = file->buffer_size;	
	
	ioresult = (*file->read_proc)(file->handle, file->buffer, (size_t *) &file->buffer_len, file->idle_proc);
	
	if (ioresult == __io_EOF)     /* mm 961031 */
		file->buffer_len = 0;
	
	if (bytes_loaded)
		*bytes_loaded = file->buffer_len;
	
	if (ioresult)
		return(ioresult);
	
	file->position += file->buffer_len;
#if	(__dest_os == __win32_os  || __dest_os == __wince_os)
   /* beginning mm 971031 */
	/* In Windows, during input, CRLF is converted to \n, i.e. one
	   character less, and buffer_len is adjusted correspondingly.
	   However, the value of position must be adjusted so that it is
	   a disk file position and not a buffer position.  The following
	   code does this.*/
	if (!file->mode.binary_io)
	{
		ptrdiff_t		n = file->buffer_len;
		unsigned char *	p = file->buffer;
		
		while (n--)                    /* jz 971105 */
			if (*p++ == '\n')
				file->position++;
	}
	
#endif                       /* Ending mm 971031 */

	
	if (!file->mode.binary_io)
		__convert_to_newlines(file->buffer, (size_t *) &file->buffer_len);
	
	return(__no_io_error);
}

int __flush_buffer(FILE * file, size_t * bytes_flushed)
{
	size_t	buffer_len;
	int			ioresult;
	
	buffer_len = file->buffer_ptr - file->buffer;
	
	if (buffer_len)
	{
		file->buffer_len = buffer_len;
		
		if (!file->mode.binary_io)
			__convert_from_newlines(file->buffer, (size_t *) &file->buffer_len);
		
		ioresult = (*file->write_proc)(file->handle, file->buffer, (size_t *) &file->buffer_len, file->idle_proc);
		
		if (bytes_flushed)
			*bytes_flushed = file->buffer_len;
		
		if (ioresult)
			return(ioresult);
		
		file->position += file->buffer_len;
	}
	
	__prep_buffer(file);
	
	return(__no_io_error);
}

int setvbuf(FILE * file, char * buff, int mode, size_t size)
{
	int kind = file->mode.file_kind;
	
	if (mode == _IONBF)           /* 970101	mani@be */  /* mm 970708 */
		fflush(file);             /* 970101	mani@be */  /* mm 970708 */

	if (file->state.io_state != __neutral || kind == __closed_file ) /*970318 bkoz*/
		return(-1);
	
	if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF)
		return(-1);
	
	if (file->buffer && file->state.free_buffer)
#ifndef _No_Disk_File_OS_Support
		free(file->buffer);
#else
		return(-1);
#endif
	
	file->mode.buffer_mode  = mode;
	file->state.free_buffer = 0;
	file->buffer            = &file->char_buffer;
	file->buffer_ptr        = &file->char_buffer;
	file->buffer_size       = 1;
	file->buffer_len        = 0;
	file->buffer_alignment  = 0;
	
	if (mode == _IONBF || size < 1)
	{
		*(file->buffer_ptr) = '\0';           /*mm 970306*/
		return(0);
	}
	
	if (!buff)
	{
#ifndef _No_Disk_File_OS_Support
		if (!(buff = (char *) malloc(size)))
			return(-1);
			
		file->state.free_buffer = 1;
#else
		return(-1);
#endif
	}
	
	file->buffer      = (unsigned char *) buff;
	file->buffer_ptr  = file->buffer;
	file->buffer_size = size;
	
#if __dest_os == __mac_os || __dest_os == __be_os

#define alignment_mask	(512L - 1L)

	file->buffer_alignment = 0;
	
	if (file->mode.file_kind == __disk_file && !(size & alignment_mask))
		file->buffer_alignment = alignment_mask;		/* if buffer is a multiple of 512, take steps */
														/* to align buffers on block bounderies       */
														/*		(see __prep_buffer)                     */
#else

	file->buffer_alignment = 0;
	
#endif
	
	return(0);
}

void setbuf(FILE * file, char * buff)
{
	if (buff)
		setvbuf(file, buff, _IOFBF, BUFSIZ);
	else
		setvbuf(file, 0, _IONBF, 0);
}

/*  Change Record
 *	24-Aug-95 JFH  First code release.
 *	08-Nov-95 JFH  Fixed a small problem in setvbuf where passing a size of 1 always
 *								 caused _IONBF behavior, which was a problem for sscanf of a single-char
 *								 string.
 *	08-Jan-96 JFH  At the behest of Be, I put in real output line buffering.
 *	19-Jan-96 JFH  Removed paranoid (*pos_proc)() calls in __load_buffer and __flush_buffer.
 *  22-Jan-96 JFH  Added casts from (void *) for C++ compatibility.
 *	 4-Mar-96 JFH  Added checking to the 'mode' parameter of setvbuf.
 *	26-Apr-96 JFH  Merged Win32 changes in.
 *						CTV
 *  mm 961031  Changes for Pascal
 *  961223 bkoz	 line 89 changed from if dest os == mac os
 *	970101	mani@be	Make unbuffered output really unbuffered.
 *  mm 970210  Straighten out the \n \r schism
 *  mm 970306  When setting a v buffer, if size == 0, then set value of 1 char buffer to '\0'
 *  bkoz 970318 line 199 allow setvbuf to work on console files
 *  FS 970612	Wrapped _flush_buffer() routine with #if __dest_os == __ppc_eabi_bare
 			    since this routine is os dependent.
 *	10-Jul-97 SCM	Unwrapped _flush_buffer() to enable serial console IO.
 *	20-Jul-97 MEA   Changed __no_os to _No_Disk_File_OS_Support.
 * mm 970708  Inserted Be changes
 *  mm 971031  Adjustment to preserve file->position as a disk position in Windows.
 *  jz 971105  Fixes one-off bug, from Jay Zipnick
 * vss 980825  Seemed to have lost the logic when merging these sources - this caused a
 *             problem with MPW's newlines
*/
