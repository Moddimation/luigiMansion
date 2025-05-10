/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 01:02:54 $ 
 *  $Revision: 1.7.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	wchar_io.c
 *	
 *	Routines
 *	--------
 *		fgetwc				
 *		getwc				
 *		getwchar
 *
 *		fgetws				
 *		getws
 *
 *		ungetwc
 *
 *		fputwc				
 *		putwc				
 *		putwchar
 *
 *		fputws				
 *		putws
 *
 *		__get_wchar
 *		put_wchar
 *
 *		__ungotten
 *
 *		fwide
 */

#pragma ANSI_strict off  /*  990729 vss  empty compilation unit illegal in ANSI C */

#ifndef __NO_WIDE_CHAR				/* mm 980204 */
 
#pragma ANSI_strict reset

#include <cstdio>
#include "ansi_files.h"
#include "wstdio.h"
#include "wchar.h"

/*
	This function writes the wide character c 
	to the output stream file,
	and returns (wchar_t) c.
*/

wchar_t	putwc(wchar_t c, FILE * file)		
{
		
	if (fwide(file, 1) <= 0)			/* mm 980206 */
		return(WEOF);					/* mm 980206 */
	if( fwrite( (char*) &c, sizeof(c), 1, file) == 0)          /* mm 980130 */
		return WEOF;

	return(c);
}

/*
	putwchar is identical to putwc with the
	second argument stdout
*/

wchar_t	putwchar(wchar_t c)				
{
		
	if (fwide(stdout, 1) <= 0)			/* mm 980206 */
		return(WEOF);					/* mm 980206 */
	if( fwrite( (char*) &c, sizeof(c), 1, stdout) == WEOF)	/* mm 980130 */
		return WEOF;

	return(c);
}



/*
	This function is the same at putwc.
*/

wchar_t	fputwc(wchar_t c, FILE * file)		
{
	return(putwc(c, file));
}


/*
	This function gets the wide character c 
	from the input stream file,
	and returns (wint_t) c.
*/

/*
	getwc
	
	Description
	
	If the next wide character is present from the input
	stream pointed to by file, then fgetwc function obtains
	that wide character and advances the associated file
	position indicator for the stream (if defined).
	
	
	Returns
	
	The fgetwc function returns the next wide character
	from the input stream pointed to by file. If the stream
	as at end-of-file, then ind-of-file indicator for the stream
	is set and fgetwc returns WEOF.
*/


wchar_t	getwc(FILE * file)					/* jcm 971208 */
{
	wchar_t		c;							/* wide char to hold the final result */

	if (fwide(file, 1) <= 0)			/* mm 980206 */
		return(WEOF);					/* mm 980206 */
	if(fread( (char*) &c, sizeof(c), 1, file) == 0)             /* mm 980130 */
		return WEOF;
	return(c);
}



wchar_t	getwchar(void)					/* jcm 971208 */
{
	wchar_t		c;							/* wide char to hold the final result */

	if (fwide(stdin, 1) <= 0)			/* mm 980206 */
		return(WEOF);					/* mm 980206 */
	if(fread( (char*) &c, sizeof(c), 1, stdin) == 0)        /* mm 980102 */
		return WEOF;

	return(c);
}



/*
	This function is the same at getwc.
*/

wchar_t	fgetwc(FILE * file)					/* jcm 971208 */
{
	wchar_t	c;
	
	c = getwc(file);

	return(c);
}



wchar_t ungetwc(wchar_t c, FILE * file)
{

	int state = file->state.io_state;
	if (fwide(file, 1) <= 0)			/* mm 980206 */
		return(WEOF);					/* mm 980206 */
	
	if (state == __writing || state == __rereading + __ungetc_buffer_size - 1 || c == WEOF)
		return(WEOF);

	if (state < __rereading)
	{	
		file->saved_buffer_len = file->buffer_len;
		file->buffer_len       = 0;
		
		state = file->state.io_state = __rereading;
	}
	else
		state = ++file->state.io_state;
	
	file->ungetwc_buffer[state - __rereading] = c;
	
	file->state.eof = 0;
	return((wchar_t) c);
}

/*
	fputws
	
	Description
	
	The fputws writes the wide string pointed to
	by s to the stream pointed to by file. The terminating
	mull wide character is not written.

	Returns
	
	The fputws function returns EOF if a write or encoding error
	occurs: otherwise, it returns a nonnegative value.
	
Note by mm 980619
It looks to me as though the C9x specification has a typo here and really means that
WEOF is returned if a write or encoding error occurs.  We return WEOF.
*/

int	fputws(const wchar_t * s, FILE * file)
{
    int     i; 
    int		errno;
    wchar_t	c;

	if (fwide(file, 1) <= 0)			/* mm 980206 */
		return(WEOF);					/* mm 980206 */

    for (i = 0; s[i] != L'\0'; i++) 				/* go through the string until reaching NULL */
    { 
       errno = 0; 									/* initialize errno to zero */
       c = s[i];									/* get the current character */
       
       if (WEOF == putwc(c, file) )					/* check for end of file */
       { 
          return (WEOF); 							/* return wide EOF if it is the end of file */
       } 
    } 

	return 1;
}


/*
	fgetws
	
	Description
	
	The fgetws function reads at most one less than the
	number of wide characters specified by n from the stream
	pointed to by file into the array pointed to by s. No
	additionaly wide characters are read after a new-line wide
	character (which is retained) or after end-of-file. A null
	wide character is written immediately after the last wide
	character read into the array.

	Returns
	
	The fgetws function returns s if successful. If end-
	of-file is encountered and no characters have been read into 
	the array, the contents of the array remain unchanged and a 
	null pointer is returned. If a read or encoding error occurs
	during the operatioin, the array contents are indeterminate
	and a null pointer is returned.
*/

wchar_t *fgetws(wchar_t * s, int n, FILE * file)
{
	wchar_t *	p = s;
	wchar_t			c;
	
	if (fwide(file, 1) <= 0)			/* mm 980206 */
		return(NULL);					/* mm 980206 */

	if (--n < 0)
		return(NULL);
	
	if (n)
		do
		{
			c = getwc(file);
			
			if (c == WEOF)
				if (file->state.eof && p != s)
					break;
				else
					return(NULL);
			
			*p++ = c;
		}
		while (c != L'\n' && --n);
	
	*p = L'\0';
	
	return(s);

}

int fwide(FILE* stream, int mode)
{
	int orientation;
	int result;
	
	if (stream->mode.file_kind == __closed_file)
		return 0;   /* there is no associated stream therefore no orientation */
	orientation = stream->mode.file_orientation;
	switch (orientation)
	{
		case __unoriented:
			if (mode > 0)
				stream->mode.file_orientation = __wide_oriented;
			else if (mode < 0)
				stream->mode.file_orientation = __char_oriented;
			result = mode;
			break;
			
		case __wide_oriented:
			result = 1;
			break;
			
		case __char_oriented:
			result = -1;
			break;
	}
	return result;
}
			
#endif /* ifndef __NO_WIDE_CHAR */				/* mm 981020 */
	

/*  Change Record
 *	980121 	JCM  First code release.
 *  mm 980206  Added cide to handle wide char unget properly and implementation of fwide.
 *  mm 981020  Added __NO_WIDE_CHAR wrappers.
 */