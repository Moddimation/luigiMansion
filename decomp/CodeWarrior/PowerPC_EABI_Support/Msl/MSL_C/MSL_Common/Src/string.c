/*  Metrowerks Standard Library  */

/*  $Date: 1999/12/03 23:59:00 $ 
 *  $Revision: 1.8.4.3.2.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */

/*
 *	string.c
 *	
 *	
 *	Routines
 *	--------
 *		strlen
 *
 *		strcpy
 *		strcat
 *		strcmp
 *
 *		strncpy
 *		strncat
 *		strncmp
 *
 *		strcoll
 *		strxfrm
 *
 *		strchr
 *		strrchr
 *		strstr
 *
 *		strspn
 *		strcspn
 *		strpbrk
 *		strtok
 *
 *		strerror
 *	
 *
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#if (__dest_os	== __win32_os  || __dest_os	== __wince_os)
	#include <ThreadLocalData.h>
#endif

#pragma warn_possunwant off

#if __INTEL__

#include <string.x86.h>  /* 012099 m.f. changed header file name */

#elif !__MC68K__ || _No_String_Inlines || !defined(__cplusplus)

#if __dest_os != __n64_os			/* ad 1.28.99 */

size_t (strlen)(const char * str)
{
	size_t	len = -1;
	
#if !__POWERPC__
	
	do
		len++;
	while (*str++);
	
#else
	
	unsigned char * p = (unsigned char *) str - 1;
	
	do
		len++;
	while (*++p);
	
#endif
	
	return(len);
}
#endif /* __dest_os != __n64_os */

#if __MIPS__

	/* 
	 *	There is really no need to waste 8 byte of memory, spend 2
	 * 	loads, to access those two constants.
	 *
	 */
	#define K1 0x80808080
	#define K2 0xfefefeff
	
#else

	static int K1 = 0x80808080;
	static int K2 = 0xfefefeff;

#endif

#pragma ANSI_strict off
char * (strcpy)(char * dst, const char * src)
{
#if !__POWERPC__ && !__MIPS__
	
	const	char * p = src;
			char * q = dst;
	
	while (*q++ = *p++);
	
#elif __PPC_EABI__ || __MIPS__
	
	/*
	 *	strcpy routine designed to minimized the number of
	 *	loads and stores.  We make sure that all the loads and
	 *  stores are properly aligned.
	 *
	 */
	register unsigned char *destb, *fromb;
	register unsigned int w, t, align;
	register unsigned int k1;
	register unsigned int k2;
	
	fromb = (unsigned char *)src;
	destb = (unsigned char *)dst;

	/*
	 *	If either the destination or the source are not 
	 *	aligned on the same boundary, we do a byte copy.  
	 *	Otherwise we align them to a word boundary.
	 */
	if ((align = ((int)fromb & 3)) != ((int)destb & 3)) {
		goto bytecopy;
	}

	if (align) {
		if ((*destb = *fromb) == 0) 
			return(dst);
		for (align = 3 - align; align; align--) {
			if ((*(++destb) = *(++fromb)) == 0) 
				return(dst);
		}
		++destb;
		++fromb;
	}

	/*	Source and destination are word aligned */
	
	k1 = K1;
	k2 = K2;

	/*	Load a word and test for a null byte. */
	w = *((int*)(fromb));
	/*
	 *	Here we need to test if one of the bytes in w is 0, in
	 *	which case we have reached the end of the string.  If we
	 *	reach the end of the string we go to the one-byte-copy 
	 *	block.  This code is endian-neutral.
	 *	The naive way would be to test each byte of the word, but
	 *	that would require at least four comparisons, few shifts,
	 *	and few branches.  This would be extremely costly especially
	 *	on machines where the conditional branch destroys the cache.
	 *		
	 *	Assertion:
	 *	----------
	 *	Let B be a byte.  The most significant bit of 
	 *	(B-1) & ~B will be set, if and only if B==0. 
	 *
	 *	Proof:
	 *	------
	 * 		B			| 	00	| 01..7F|	80	| 81..FF|
	 * 		B-1			|	FF	| 00..7E|	7F	| 80..FE|
	 * 		~B			|	FF	| FE..80|	7F	| 7E..00|
	 *	MSB((B-1)&~B)	|	1	| 0		|	0	| 0		|
	 *
	 *	Adding 0xFF to a byte will always generate a carry
	 *	except when the byte is 0.  So when working with 4
	 *	bytes at a time, we will add FF to the first byte and
	 *	FE to the other bytes because the carry will come 
	 *	from the preceding byte.
	 */
	t = w + k2;
	/*
	 *	Extract the most significant bit of each byte.  If one
	 *	bit is left it means we have seen a 0.
	 */
	t &= k1;
	if (t) goto bytecopy;
	--((int*)(destb));
	
	do {
		/*	Put the tested word into the incremented destination. */
		*(++((int*)(destb))) = w;
		/*	Load a word and increment source for testing. */
		w = *(++((int*)(fromb)));
		
		t = w + k2;
		t &= k1;
		if (t) goto adjust;
	} while(1);

adjust:
	++((int*)(destb));
bytecopy:
	if ((*destb = *fromb) == 0) 
		return(dst);
	do {
		if ((*(++destb) = *(++fromb)) == 0) 
			return(dst);
	} while(1);
#else
	
	const	unsigned char * p = (unsigned char *) src - 1;
			unsigned char * q = (unsigned char *) dst - 1;
	
	while (*++q = *++p);
#endif
	
	return(dst);
}
#pragma ANSI_strict reset
#endif /* !__MC68K__ || _No_String_Inlines || !defined(__cplusplus) */

#if !defined(__INTEL__)

char * strncpy(char * dst, const char * src, size_t n)
{
#if !__POWERPC__
	
	const	char * p = src;
				char * q = dst;
	
	n++;
	
	while (--n)
		if (!(*q++ = *p++))
		{
			while (--n)
				*q++ = 0;
			break;
		}
	
#else
	
	const	unsigned char * p		= (const unsigned char *) src - 1;
				unsigned char * q		= (unsigned char *) dst - 1;
				unsigned char zero	= 0;
	
	n++;
	
	while (--n)
		if (!(*++q = *++p))
		{
			while (--n)
				*++q = 0;
			break;
		}

#endif
	
	return(dst);
}

char * strcat(char * dst, const char * src)
{
#if !__POWERPC__
	
	const	char * p = src;
				char * q = dst;
	
	while (*q++);
	
	q--;
	
	while (*q++ = *p++);
	
#else
	
	const	unsigned char * p = (unsigned char *) src - 1;
				unsigned char * q = (unsigned char *) dst - 1;
	
	while (*++q);
	
	q--;
	
	while (*++q = *++p);

#endif
	
	return(dst);
}

char * strncat(char * dst, const char * src, size_t n)
{
#if !__POWERPC__
	
	const	char * p = src;
				char * q = dst;
	
	while (*q++);
	
	q--; n++;
	
	while (--n)
		if (!(*q++ = *p++))
		{
			q--;
			break;
		}
	
	*q = 0;
	
#else
	
	const	unsigned char * p = (unsigned char *) src - 1;
				unsigned char * q = (unsigned char *) dst - 1;
	
	while (*++q);
	
	q--; n++;
	
	while (--n)
		if (!(*++q = *++p))
		{
			q--;
			break;
		}
	
	*++q = 0;

#endif
	
	return(dst);
}

#pragma ANSI_strict off
int strcmp(const char * str1, const char * str2)
{
#if !__POWERPC__ && !__MIPS__
	
	const	unsigned char * p1 = (unsigned char *) str1;
	const	unsigned char * p2 = (unsigned char *) str2;
				unsigned char		c1, c2;
	
	while ((c1 = *p1++) == (c2 = *p2++))
		if (!c1)
			return(0);

	return(c1 - c2);

#elif __PPC_EABI__ || __MIPS__

	/*
	 *	strcmp routine designed to minimize the number of
	 *	loads and stores.  We make sure that all the loads and
	 *  stores are properly aligned.
	 *
	 */
	register unsigned char *left = (unsigned char *)str1;
	register unsigned char *right = (unsigned char *)str2;
	unsigned int k1, k2, align, l1, r1, x;

	/*	Check the simple case of the first byte being different. */
	l1 = *left;
	r1 = *right;
	if (l1 - r1) {
		return (l1 - r1);
	}
	
	/*
	 *	If either the destination or the source are not 
	 *	aligned on the same boundary, we do a byte copy.  
	 *	Otherwise we align them to a word boundary. 
	 */ 
	if ((align = ((int)left & 3)) != ((int)right & 3)) {
		goto bytecopy;
	}
	if (align) {
		/*	Continuation of test of first byte. */
		if (l1 == 0) {
			return (0);
		}
		for (align = 3 - align; align; align--) {
			l1 = *(++left);
			r1 = *(++right);
			if (l1 - r1) {
				return (l1 - r1);
			}
			if (l1 == 0) {
				return (0);
			}
		}
		left++;
		right++;
	}

	/*	Strings are word aligned */
	
	k1 = K1;
	k2 = K2;

	/*	Load a word from each string. */
	l1 = *(int*)left;
	r1 = *(int*)right;
	/*	See comments in strcpy function. */
	x = l1 + k2;
	if (x & k1) {
		goto adjust;
	}
	while(l1 == r1) {
		/*	Load a word and increment strings. */
		l1 = *(++((int*)(left)));
		r1 = *(++((int*)(right)));
		x = l1 + k2;
		if (x & k1) {
			goto adjust;
		}
	}
	/*	l1 and r1 are not equal.  */
	if (l1 > r1)
		return (1);
	return (-1);	

adjust:
	l1 = *left;
	r1 = *right;
	if (l1 - r1) {
		return (l1 - r1);
	}
bytecopy:
	if (l1 == 0) {
		return (0);
	}
	do {
		l1 = *(++left);
		r1 = *(++right);
		if (l1 - r1) {
			return (l1 - r1);
		}
		if (l1 == 0) {
			return (0);
		}
	} while (1);

#else
	
	const	unsigned char * p1 = (unsigned char *) str1 - 1;
	const	unsigned char * p2 = (unsigned char *) str2 - 1;
			unsigned long		c1, c2;
		
	while ((c1 = *++p1) == (c2 = *++p2))
		if (!c1)
			return(0);

	return(c1 - c2);
#endif	
}
#pragma ANSI_strict reset
int strncmp(const char * str1, const char * str2, size_t n)
{
#if !__POWERPC__
	
	const	unsigned char * p1 = (unsigned char *) str1;
	const	unsigned char * p2 = (unsigned char *) str2;
				unsigned char		c1, c2;
	
	n++;
	
	while (--n)
		if ((c1 = *p1++) != (c2 = *p2++))
			return(c1 - c2);
		else if (!c1)
			break;
	
#else
	
	const	unsigned char * p1 = (unsigned char *) str1 - 1;
	const	unsigned char * p2 = (unsigned char *) str2 - 1;
				unsigned long		c1, c2;
	
	n++;
	
	while (--n)
		if ((c1 = *++p1) != (c2 = *++p2))
			return(c1 - c2);
		else if (!c1)
			break;

#endif
	
	return(0);
}

#if __dest_os != __n64_os 			/* ad 1.28.99 */

char * strchr(const char * str, int chr)
{
#if !__POWERPC__

	const char * p = str;
	      char   c = chr;
	      char   ch;
	
	while(ch = *p++)
		if (ch == c)
			return((char *) (p - 1));
	
	return(c ? 0 : (char *) (p - 1));

#else

	const unsigned char * p = (unsigned char *) str - 1;
	      unsigned long   c = (chr & 0xff);               /*mm 970327*/
	      unsigned long   ch;
	
	while(ch = *++p)                             /*mm 970327*/ /*mm 980604 */
		if (ch == c)
			return((char *) p);
	
	return(c ? 0 : (char *) p);

#endif
}

#endif /* __dest_os != __n64_os */
#endif /* __dest_os != __win32_os */

int strcoll(const char *str1, const char * str2)
{
	return(strcmp(str1, str2));
}

size_t strxfrm(char * str1, const char * str2, size_t n)
{
	strncpy(str1, str2, n);
	return(strlen(str2));
}

char * strrchr(const char * str, int chr)
{
#if !__POWERPC__

	const char * p = str;
	const char * q = 0;
	      char   c = chr;
	      char   ch;
	
	while(ch = *p++)
		if (ch == c)
			q = p - 1;
	
	if (q)
		return((char *) q);
	
	return(c ? 0 : (char *) (p - 1));

#else

	const unsigned char * p = (unsigned char *) str - 1;
	const unsigned char * q = 0;
	      unsigned long   c = (chr & 0xff);				/*bb 970530*/
	      unsigned long   ch;
	
	while(ch = *++p)								/*bb 970530*/ /* mm 980604 */
		if (ch == c)
			q = p;
	
	if (q)
		return((char *) q);
	
	return(c ? 0 : (char *) p);

#endif
}

typedef unsigned char char_map[32];

#define set_char_map(map, ch)	 map[ch>>3] |= (1 << (ch&7))
#define tst_char_map(map, ch) (map[ch>>3] &  (1 << (ch&7)))

char * strpbrk(const char * str, const char * set)
{
	const unsigned char *	p;
	      int							c;
				char_map				map = {0};
	
#if !__POWERPC__
	
	p = (unsigned char *) set;

	while (c = *p++)
		set_char_map(map, c);
	
	p = (unsigned char *) str;
	
	while (c = *p++)
		if (tst_char_map(map, c))
			return((char *) (p - 1));
			
	return(NULL);

#else
	
	p = (unsigned char *) set - 1;

	while (c = *++p)
		set_char_map(map, c);
	
	p = (unsigned char *) str - 1;
	
	while (c = *++p)
		if (tst_char_map(map, c))
			return((char *) p);
			
	return(NULL);

#endif
}

size_t strspn(const char * str, const char * set)
{
	const unsigned char *	p;
	      int							c;
				char_map				map = {0};
	
#if !__POWERPC__
	
	p = (unsigned char *) set;

	while (c = *p++)
		set_char_map(map, c);
	
	p = (unsigned char *) str;
	
	while (c = *p++)
		if (!tst_char_map(map, c))
			break;
			
	return(p - (unsigned char *) str - 1);

#else
	
	p = (unsigned char *) set - 1;

	while (c = *++p)
		set_char_map(map, c);
	
	p = (unsigned char *) str - 1;
	
	while (c = *++p)
		if (!tst_char_map(map, c))
			break;
			
	return(p - (unsigned char *) str);

#endif
}

size_t strcspn(const char * str, const char * set)
{
	const unsigned char *	p;
	      int							c;
				char_map				map = {0};
	
#if !__POWERPC__
	
	p = (unsigned char *) set;

	while (c = *p++)
		set_char_map(map, c);
	
	p = (unsigned char *) str;
	
	while (c = *p++)
		if (tst_char_map(map, c))
			break;
			
	return(p - (unsigned char *) str - 1);

#else
	
	p = (unsigned char *) set - 1;

	while (c = *++p)
		set_char_map(map, c);
	
	p = (unsigned char *) str - 1;
	
	while (c = *++p)
		if (tst_char_map(map, c))
			break;
			
	return(p - (unsigned char *) str);

#endif
}

#if (__dest_os	== __win32_os || __dest_os	== __wince_os)

char * strtok(char * str, const char * set)
{
	unsigned char *	p, * q;
	int							c;
	char_map				map	= {0};

	if (str)
		_GetThreadLocalData()->strtok_s = (unsigned char *) str;
		
	p = (unsigned char *) set;

	while (c = *p++)
		set_char_map(map, c);
	
	p = _GetThreadLocalData()->strtok_s;
	
	while (c = *p++)
		if (!tst_char_map(map, c))
			break;
	
	if (!c)
	{
		_GetThreadLocalData()->strtok_s = _GetThreadLocalData()->strtok_n;
		return(NULL);
	}
	
	q = p - 1;
	
	while (c = *p++)
		if (tst_char_map(map, c))
			break;
	
	if (!c)
		_GetThreadLocalData()->strtok_s = _GetThreadLocalData()->strtok_n;
	else
	{
		_GetThreadLocalData()->strtok_s    = p;
		*--p = 0;
	}
	
	return((char *) q);
}

#else /* if __dest_os != win32 */

char * strtok(char * str, const char * set)
{
								unsigned char *	p, * q;
	__tls static	unsigned char *	n		= (unsigned char *) "";
	__tls static	unsigned char *	s		= (unsigned char *) "";
							  int							c;
								char_map				map	= {0};
	
	if (str)
		s = (unsigned char *) str;
	
#if !__POWERPC__
	
	p = (unsigned char *) set;

	while (c = *p++)
		set_char_map(map, c);
	
	p = s;
	
	while (c = *p++)
		if (!tst_char_map(map, c))
			break;
	
	if (!c)
	{
		s = n;
		return(NULL);
	}
	
	q = p - 1;
	
	while (c = *p++)
		if (tst_char_map(map, c))
			break;
	
	if (!c)
		s = n;
	else
	{
		s    = p;
		*--p = 0;
	}
	
	return((char *) q);

#else
	
	p = (unsigned char *) set - 1;

	while (c = *++p)
		set_char_map(map, c);
	
	p = s - 1;
	
	while (c = *++p)
		if (!tst_char_map(map, c))
			break;
	
	if (!c)
	{
		s = n;
		return(NULL);
	}
	
	q = p;
	
	while (c = *++p)
		if (tst_char_map(map, c))
			break;
	
	if (!c)
		s = n;
	else
	{
		s  = p + 1;
		*p = 0;
	}
	
	return((char *) q);

#endif
}
#endif /* __dest_os != win32 */

char * strstr(const char * str, const char * pat)
{
#if !__POWERPC__

	unsigned char * s1 = (unsigned char *) str;
	unsigned char * p1 = (unsigned char *) pat;
	unsigned char firstc, c1, c2;
	
	if ((pat == NULL) || (!(firstc = *p1++)))
										/* 980424  mm   if pat is a NULL pointer, we return str */
										/* 971017  beb  if pat is an empty string we return str */
		return((char *) str);

	while(c1 = *s1++)
		if (c1 == firstc)
		{
			const unsigned char * s2 = s1;
			const unsigned char * p2 = p1;
			
			while ((c1 = *s2++) == (c2 = *p2++) && c1);
			
			if (!c2)
				return((char *) s1 - 1);
		}
	
	return(NULL);

#else

	unsigned char * s1 = (unsigned char *) str-1;
	unsigned char * p1 = (unsigned char *) pat-1;
	unsigned long firstc, c1, c2;
	
	if ((pat == NULL) || (!(firstc = *++p1)))    /* 980807  vss  PPC must be pre-increment */
										/* 980424  mm   if pat is a NULL pointer, we return str */
										/* 971017  beb  if pat is an empty string we return str */
		return((char *) str);

	while(c1 = *++s1)
		if (c1 == firstc)
		{
			const unsigned char * s2 = s1-1;
			const unsigned char * p2 = p1-1;
			
			while ((c1 = *++s2) == (c2 = *++p2) && c1);
			
			if (!c2)
				return((char *) s1);
		}
	
	return(NULL);

#endif
}

char * strerror(int errnum)
{
	static char errstr[__max_errstr];
	
	return(__strerror(errnum, errstr));
}

char * __strerror(int errnum, char * str)
{
	switch (errnum)
	{
		case ENOERR:				strcpy(str,  "No Error");                  					break;
		case EDOM:					strcpy(str,  "Domain Error");              					break;
		case ERANGE:				strcpy(str,  "Range Error");               					break;
		case EFPOS:					strcpy(str,  "File Position Error");       					break;
		case ESIGPARM:			strcpy(str,  "Signal Error");              					break;
#if __dest_os == __be_os
		case E2BIG:         strcpy(str, "Argument too big");           					break;
		case EACCES:        strcpy(str, "Permission denied");          					break;
		case EAGAIN:        strcpy(str, "Try again");                  					break;
		case EBADF:         strcpy(str, "Bad file descriptor");        					break;
		case EBUSY:         strcpy(str, "Device/File/Resource busy");  					break;
		case ECHILD:        strcpy(str, "No child process");           					break;
		case EDEADLK:       strcpy(str, "Resource deadlock");          					break;
		case EFAULT:        strcpy(str, "Bad address");                					break;
		case EFBIG:         strcpy(str, "File too large");             					break;
		case EINTR:         strcpy(str, "Interrupted system call");    					break;
		case EINVAL:        strcpy(str, "Invalid argument");           					break;
		case EIO:           strcpy(str, "I/O Error");                  					break;
		case EISDIR:        strcpy(str, "Is a directory");             					break;
		case EMFILE:        strcpy(str, "Too many open files");        					break;
		case EMLINK:        strcpy(str, "Too many links");             					break;
		case ENAMETOOLONG:  strcpy(str, "File name too long");         					break;
		case ENFILE:        strcpy(str, "File table overflow");        					break;
		case ENODEV:        strcpy(str, "No such device");             					break;
		case ENOENT:        strcpy(str, "No such file or directory");  					break;
		case ENOEXEC:       strcpy(str, "Not an executable");          					break;
		case ENOLCK:        strcpy(str, "No record locks available");  					break;
		case ENOMEM:        strcpy(str, "No memory");                  					break;
		case ENOSPC:        strcpy(str, "No space left on device");    					break;
		case ENOSYS:        strcpy(str, "Function not implemented");   					break;
		case ENOTDIR:       strcpy(str, "Not a directory");            					break;
		case ENOTEMPTY:     strcpy(str, "Directory not empty");        					break;
		case ENOTTY:        strcpy(str, "Not a tty");                  					break;
		case ENXIO:         strcpy(str, "No such device");             					break;
		case EPERM:         strcpy(str, "Operation not allowed");      					break;
		case EPIPE:         strcpy(str, "Broken pipe");                					break;
		case EROFS:         strcpy(str, "Read-only file system");      					break;
		case ESRCH:         strcpy(str, "No such process");            					break;
		case EXDEV:         strcpy(str, "Cross-device link");          					break;
		case EEXIST:        strcpy(str, "File or Directory already exists");    break;
		case ESPIPE:        strcpy(str, "Seek not allowed on file descriptor"); break;
#endif
		default:						sprintf(str, "Unknown Error (%d)", errnum); 				break;
	}
	
	return(str);
}

#pragma warn_possunwant reset
/*  Change Record
 *	24-May-95 JFH  First code release.
 *	23-Jun-95 JFH  Fixed strstr's tendency to not find patterns that aren't
 *								 suffixes of the target string. Dumb mistake.
 *  29-Nov-95 JFH  Tweaked strncpy, strcmp, and strncmp for efficiency.
 *	08-Dec-95 JFH  Fixed bug in PPC version of strpbrk (returned p-1 instead of p).
 *	08-Dec-95 JFH  Fixed bug in PPC version of strrchr (was copy of PPC version of strchr).
 *  22-Jan-96 JFH  Bracketed strlen() and strcpy() by #if <condition too complex to put here>
 *								 (they are inlined in <string.h> if this condition fails).
 *	29-Apr-96 JFH  Merged Win32 changes in.
 *						CTV
 *	19-Dec-96 KO   Added some Win32 ifdefs to use my thread local data structure rather
 *                 than static local variables.
 *  mm970327       Change to strchr to allow chars beyond 128 to work with signed chars
 *  bb970530       Applied mm970327 change to the strrchr routine.
 *  971017 beb     Fix string return value, should be str, not s1
 *  980424 mm      Modify strstr so that if the pattern pointer is NULL, a pointer to str is 
 *                 returned.  The Standard leaves this undefined so defined a consistent result.
 *  980429 mf      changed macro __dest_os_== win32_os to __INTEL__ so all intel oses pick
 *                 up optimized string
 *                 functions (eg. wince and beos)
 *  980512 mf      wince changes
 *  980604 mm      Removed part of mm970327 from strchr and strrchr as not needed to simplify code.
 *                 MW06890
 *  980804 vss  PPC must be pre-increment
 *  980807 vss  Fix inadvertant mistype of increment operator on pointer to pattern in strstr
 *  981116 mf   mips optimizations for strcmp etc...
 *  990120 mf   changed intel header file name from string.win32.h to string.x86.h 
 * 1.28.99 ad  guarded strlen() and strchr() with  if __dest_os != __n64_os
 */