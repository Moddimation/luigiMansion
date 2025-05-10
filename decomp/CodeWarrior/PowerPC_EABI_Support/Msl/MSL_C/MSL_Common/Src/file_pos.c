/*  Metrowerks Standard Library  */

/*  $Date: 1999/01/22 23:40:32 $ 
 *  $Revision: 1.7 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	file_pos.c
 *	
 *	Routines
 *	--------
 *		ftell
 *		fseek
 *
 *		fgetpos
 *		fsetpos
 *
 *		rewind
 *
 *
 */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include "ansi_files.h"
#include "buffer_io.h"

     /* mm 970708 begin */
/*
	 XXXdbg - declare _ftell() and _fseek() the way they should have been
	          in the first place and map ftell() and fseek() to use the
						underscore versions.  This is necessary for systems (like
						the BeOS) where fpos_t is really a 64 bit quantity.
*/
						
fpos_t _ftell(FILE * file)
{
	int charsInUndoBuffer = 0;			/* jz/ma 971105 */
	fpos_t	position;
	
	/*970324 bkoz*/
	unsigned char tmp_kind = file->mode.file_kind;
	if ( !(tmp_kind == __disk_file || tmp_kind == __console_file) || file->state.error)
	{
		errno = EFPOS;
		return(-1L);
	}
	
	if (file->state.io_state == __neutral)
		return(file->position);
	
	position = file->buffer_pos + (file->buffer_ptr - file->buffer);

	if (file->state.io_state >= __rereading)
	{
		charsInUndoBuffer = file->state.io_state - __rereading + 1;	/* jz/ma 971105 */
		position -= charsInUndoBuffer;								/* jz/ma 971105 */
	}


#if	(__dest_os == __win32_os  || __dest_os == __wince_os)
	
	if (!file->mode.binary_io)
	{
	/* begin jz/ma 971105 */
	/*	n is the number of characters before the current location.
		We can calculate that by buffer_ptr - buffer. However if
		there are characters in the undo buffer then we need to not
		consider those, so we subtract charsInUndoBuffe from n. */
	ptrdiff_t		n = file->buffer_ptr - file->buffer - charsInUndoBuffer;
	unsigned char *	p = file->buffer;
	
	while (n--)
		if (*p++ == '\n')
			position++;
	/* end jz/ma 971105 */
	}
	
#endif

	return(position);
}

long ftell(FILE * file)
{
	return (long)_ftell(file);
}

int fgetpos(FILE * file, fpos_t * pos)
{
	*pos = _ftell(file);
	
	return(*pos == -1);
}

int _fseek(FILE * file, fpos_t offset, int mode)
{
	fpos_t			position;
	__pos_proc	pos_proc;
	
	/*970324 bkoz*/
	unsigned char tmp_kind = file->mode.file_kind;
	if ( !(tmp_kind == __disk_file ) || file->state.error)
	{
		errno = EFPOS;
		return(-1);
	}
	
	if (file->state.io_state == __writing)
	{
		if (__flush_buffer(file, NULL) != __no_io_error)
		{
			set_error(file);
			errno = EFPOS;
			return(-1);
		}
	}
	
	if (mode == SEEK_CUR)
	{

		mode = SEEK_SET;
	
		if ((position = _ftell(file)) < 0)
			position = 0;
		
		offset += position;
	}
	
/* this block commented back in by BK 970530 after adding the line changing
   state to __reading.  this fixes the problems seen with ifstream */
   
#if	!(__dest_os	== __win32_os || __dest_os	== __wince_os) /* this optimization breaks C++ ifstream */ /* bk 970527 */
	/* 970507  mm begin */
	if ((mode != SEEK_END) && (file->mode.io_mode != __read_write) &&        /*mm 970623 */
			((file->state.io_state == __reading) || (file->state.io_state == __rereading)))
	{	/* When in read mode, check for a position that is within the bounds of the current buffer */
		if ((offset >= file->position) || offset < file->buffer_pos)
		{	/* not within buffer */
			file->state.io_state = __neutral;
		}
		else
		{	/* within the buffer, reset the buffer_ptr and buffer_len */
			file->buffer_ptr = file->buffer + (offset - file->buffer_pos);	
			file->buffer_len = file->position - offset;
			file->state.io_state = __reading;
		}
	}
	else
	{	/* not reading */
		file->state.io_state = __neutral;
	}

	if (file->state.io_state == __neutral)
	{
		if ((pos_proc = file->position_proc) != 0 && (*pos_proc)(file->handle, &offset, mode, file->idle_proc))
		{
			set_error(file);
			errno = EFPOS;
			return(-1);
		}
		
		file->state.eof      = 0;
		file->position       = offset;
		file->buffer_len     = 0;
	}
	/* 970507  mm end */
#else
	if ((pos_proc = file->position_proc) != 0 && (*pos_proc)(file->handle, &offset, mode, file->idle_proc))
	{
		set_error(file);
		errno = EFPOS;
		return(-1);
	}
		
	file->state.io_state = __neutral;
	file->state.eof      = 0;
	file->position       = offset;
	file->buffer_len     = 0;
#endif

	return(0);
}

int fseek(FILE * file, long offset, int mode)
{
		fpos_t real_offset = (fpos_t)offset;

		return _fseek(file, real_offset, mode);
}

int fsetpos(FILE * file, const fpos_t * pos)
{
	return(_fseek(file, *pos, SEEK_SET));
}

     /* mm 970708 end */

void rewind(FILE * file)
{
	file->state.error = 0;				/* give the seek some hope of success */
	
	fseek(file, 0, SEEK_SET);
	
	file->state.error = 0;				/* the standard says we have to do this
										 * even if the seek fails (though it's
										 * admittedly not likely to
										 */
}

/*  Change Record
 *	28-Aug-95 JFH  First code release.
 *	25-Apr-96 JFH  Modified ftell to account for multi-level 'ungetc'.
 *	29-Apr-96 JFH  Merged Win32 changes in.
 *						CTV
 *	970320	bkoz	line 32 changed so that console files don't bail out of ftell
 *	970324 	bkoz	line 78 changed so that console files don't bail out of fseek
 *	970507  mm      Change so that a seek that stays within the buffer doesn't
 *					require the buffer to be reloaded. (change put on hold)
 *  970623  mm      Change to prevent in buffer seeking with update files since these files can change from reading
 *                  to writing and vice-versa through use of seek---see ANSI C 7.9.5.3
 * mm 970708  Inserted Be changes
 * mm 971103  Change to Windows code to do nothing if fseek stays where it is.  i.e.
              it is certain that the target of the seek is in the buffer.
 * jz/ma 971105  Changes to calculate the number of new lines in buffer correctly.  Changes from Mitch Adler and Jay Zipnick
 * mf 980227   undid change mm 971103
 */
