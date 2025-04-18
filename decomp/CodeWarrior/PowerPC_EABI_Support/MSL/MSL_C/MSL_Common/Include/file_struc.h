/* Metrowerks Standard Library
 * Copyright C 1995-2001 Metrowerks Corporation.  All rights reserved.
 *
 * $Date: 2001/04/20 19:34:14 $
 * $Revision: 1.17 $
 */

/* This header file contains the definitions connected with file structures
 * to separate their definition from the stdio prototypes
 */
 
#ifndef _MSL_FILE_STRUC_H
#define _MSL_FILE_STRUC_H

#include <size_t.h>

#ifndef __NO_WIDE_CHAR				/*- mm 980204 -*/
	#include <wchar_t.h>				/*- mm 990217 -*/
#endif	/* __NO_WIDE_CHAR */        /*- mm 980204 -*/

#ifndef RC_INVOKED

#pragma options align=native

_MSL_BEGIN_NAMESPACE_STD	/*- cc 010409 -*/
_MSL_BEGIN_EXTERN_C			/*- cc 010409 -*/

enum __file_kinds 
{
	__closed_file,
	__disk_file,
	__console_file,
	__unavailable_file
};

enum __open_modes 
{
	__must_exist,
	__create_if_necessary,
	__create_or_truncate
};

#ifndef __NO_WIDE_CHAR				/*- mm 980204 -*/
	enum __file_orientation				/*- mm 980204 -*/
	{
		__unoriented,
		__char_oriented,
		__wide_oriented
	};
#endif /* not __NO_WIDE_CHAR */		/*- mm 980204 -*/

enum __io_modes 
{
	__read				= 1,
	__write				= 2,
	__read_write		= 3,
	__append			= 4
};

typedef struct 
{
	unsigned int	open_mode		: 2;
	unsigned int	io_mode			: 3;
	unsigned int	buffer_mode		: 2;
	unsigned int	file_kind		: 3;	/*- mm 980708 -*/
	
	#ifndef __NO_WIDE_CHAR						/*- mm 980204 -*/
		unsigned int	file_orientation: 2;
	#endif /* not __NO_WIDE_CHAR */				/*- mm 980204 -*/
	
	unsigned int	binary_io		: 1;
} __file_modes;

enum __io_states 
{
	__neutral,
	__writing,
	__reading,
	__rereading
};

typedef struct 
{
	unsigned int	io_state	: 3;
	unsigned int	free_buffer	: 1;
	unsigned char	eof;
	unsigned char	error;
} __file_state;

typedef unsigned long	__file_handle;

typedef unsigned long	fpos_t;

typedef struct _FILE FILE;

enum __io_results 
{
     __no_io_error,
     __io_error,
     __io_EOF         /*- mm 961031 -*/
};

typedef void (* __idle_proc)  (void);
typedef int  (* __pos_proc)   (__file_handle file, fpos_t * position, int mode, __idle_proc idle_proc);     /*- mm 970708 -*/
typedef int  (* __io_proc)    (__file_handle file, unsigned char * buff, size_t * count, __idle_proc idle_proc);
typedef int	 (* __close_proc) (__file_handle file);

#define __ungetc_buffer_size	2

struct _FILE 
{
	__file_handle		handle;
	__file_modes		mode;
	__file_state		state;
	
	#ifndef _No_Disk_File_OS_Support							/*- mm 981007 -*/
		unsigned char       is_dynamically_allocated;			/*- mm 981007 -*/
	#endif  /* not _No_Disk_File_OS_Support */					/*- mm 981007 -*/
	
	unsigned char		char_buffer;
	unsigned char		char_buffer_overflow;
	unsigned char		ungetc_buffer[__ungetc_buffer_size];
	
	#ifndef __NO_WIDE_CHAR										/*- mm 980204 -*/
		wchar_t				ungetwc_buffer[__ungetc_buffer_size];
	#endif /* not __NO_WIDE_CHAR */								/*- mm 980204 -*/
	
	unsigned long		position;
	unsigned char *		buffer;
	unsigned long		buffer_size;
	unsigned char *		buffer_ptr;
	unsigned long		buffer_len;
	unsigned long		buffer_alignment;
	unsigned long		saved_buffer_len;
	unsigned long		buffer_pos;
	__pos_proc			position_proc;
	__io_proc			read_proc;
	__io_proc			write_proc;
	__close_proc		close_proc;
	__idle_proc			idle_proc;
	
	#ifndef _No_Disk_File_OS_Support							/*- mm 981007 -*/
		struct _FILE *      next_file_struct;					/*- mm 981007 -*/
	#endif  /* not _No_Disk_File_OS_Support */					/*- mm 981007 -*/
};

#if	defined(_No_Disk_File_OS_Support) && (defined(_No_Console) || defined(_Unbuffered_Console))
	#define _Unbuffered_IO
#endif	/* defined(_No_Disk_File_OS_Support) && (defined(_No_Console) || defined(_Unbuffered_Console)) */

#define _IONBF	0
#define _IOLBF	1
#define _IOFBF	2

#if __dest_os == __m56800_os || \
    __dest_os == __m56800E_os
      #define BUFSIZ 100
#else
    #define BUFSIZ 4096
#endif


#ifdef _No_Disk_File_OS_Support
	#define _STATIC_FILES        3			/*- mm 981007 -*/
	#ifdef _No_Console
	#define FOPEN_MAX			 0
	#else
	#define FOPEN_MAX			 3
	#endif
#else
	#define FOPEN_MAX			35
	#define _STATIC_FILES        4			/*- mm 981007 -*/
#endif

#define FILENAME_MAX	256

#if (__dest_os == __mac_os)
	#define L_tmpnam		 32
#else
	#define	L_tmpnam		 512
#endif	/* __dest_os == __mac_os */

#define TMP_MAX		32767

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define stdin  	(&__std(__files[0]))   /*- mm 961031 -*/    /*- mm 961203 -*/ /*- mm 000201 -*/
#define stdout	(&__std(__files[1]))   /*- mm 961031 -*/    /*- mm 961203 -*/ /*- mm 000201 -*/
#define stderr	(&__std(__files[2]))   /*- mm 961031 -*/    /*- mm 961203 -*/ /*- mm 000201 -*/

extern _MSL_IMP_EXP_C FILE __files[];      /*- mm 961031 -*/    /*- mm 961203 -*/

_MSL_END_EXTERN_C	/*- cc 010409 -*/
_MSL_END_NAMESPACE_STD	/*- cc 010409 -*/

#pragma options align=reset

#endif /* RC_INVOKED */

#endif	/* _MSL_FILE_STRUC_H */

/* Change record:
 * mm  961031 Changes for Pascal
 * mm  961203 Changed files to __files
 * mm  970708 Inserted Be changes
 * hh  971206 Changed file name from stdio.h to cstdio and added namespace support
 * hh  971230 added RC_INVOKED wrapper
 * mm  980204 added fields in file structure for wide characters.
 * rjk 980313 add DLLDATA for exporting data in dll
 * mm  980708 Corrected space allotment for the values of __file_kinds in __file_modes
 * vss 980817 Define EOF in its own file and include it here to avoid circular dep
 * mm  981007 Changes for dynamic allocation of file structures.
 * mm  990217 Deleted #include cstddef as not needed and replaced by #include <wchar_t.h>
 * mm  990806 Header file constructed by moving contents from cstdio.
 * mm  990816 Removed reference to __string_file
 * blc 990825 Fixed missing brace for extern "C" part of header
 * mm  000201 Added __std wrappers
 * cc  000326 removed dest_os to be_os 
 * cc  010409 updated defines to JWW new namespace macros 	 
 */

