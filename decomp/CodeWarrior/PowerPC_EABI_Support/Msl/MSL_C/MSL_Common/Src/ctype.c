/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/29 17:31:44 $ 
 *  $Revision: 1.7.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */
 
/*
 *	ctype.c
 *	
 *	Routines
 *	--------
 *		isalnum
 *		isalpha
 *		iscntrl
 *		isdigit
 *		isgraph
 *		islower
 *		isprint
 *		ispunct
 *		isspace
 *		isupper
 *		isxdigit
 *		tolower
 *		toupper
 *
 *
 */

#include <ctype.h>
#include <stdio.h>

#define ctrl	__control_char
#define motn	__motion_char
#define spac	__space_char
#define punc	__punctuation
#define digi	__digit
#define hexd	__hex_digit
#define lowc	__lower_case
#define uppc	__upper_case
#define dhex	(hexd | digi)
#define uhex	(hexd | uppc)
#define lhex	(hexd | lowc)

const unsigned char	__ctype_map[256] = {
/*   -0    -1    -2    -3    -4    -5    -6    -7    -8    -9    -A    -B    -C    -D    -E    -F       */
	ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, motn, motn, motn, motn, motn, ctrl, ctrl, /* 0- */
	ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, /* 1- */
	spac, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, /* 2- */
	dhex, dhex, dhex, dhex, dhex, dhex, dhex, dhex, dhex, dhex, punc, punc, punc, punc, punc, punc, /* 3- */
	punc, uhex, uhex, uhex, uhex, uhex, uhex, uppc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, /* 4- */
	uppc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, punc, punc, punc, punc, punc, /* 5- */
	punc, lhex, lhex, lhex, lhex, lhex, lhex, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, /* 6- */
	lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, punc, punc, punc, punc, ctrl  /* 7- */
#if __dest_os == __mac_os
	,
	uppc, uppc, uppc, uppc, uppc, uppc, uppc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, /* 8- */
	lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, lowc, /* 9- */
	punc, punc, punc, punc, punc, punc, punc, lowc, punc, punc, punc, punc, punc, punc, uppc, uppc, /* A- */
	punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, lowc, lowc, /* B- */
	punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, spac, uppc, uppc, uppc, uppc, lowc, /* C- */ /*mm 980603*/
	punc, punc, punc, punc, punc, punc, punc, punc, lowc, uppc, punc, punc, punc, punc, lowc, lowc, /* D- */
	punc, punc, punc, punc, punc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, uppc, /* E- */
	punc, uppc, uppc, uppc, uppc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, 000   /* F- */
#elif __dest_os == __be_os       /* mm 970708  begin */
    ,
	punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, /* 8- */
	punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, /* 9- */
	punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, /* A- */
	punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, /* B- */
	punc, punc, punc, punc, punc, punc, punc, punc, punc, punc, spac, punc, punc, punc, punc, punc, /* C- */
	punc, punc, punc, punc, punc, punc, punc, punc, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, /* D- */
	ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, /* E- */
	ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, ctrl, /* F- */
#endif                           /* mm 970708 end */
};

const unsigned char __lower_map[256] = {
/*   -0    -1    -2    -3    -4    -5    -6    -7    -8    -9    -A    -B    -C    -D    -E    -F       */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,/* 0- */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,/* 1- */
	' ',  '!',  '"',  '#',  '$',  '%',  '&',  '\'', '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/', /* 2- */
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?', /* 3- */
	'@',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o', /* 4- */
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '[',  '\\', ']',  '^',  '_', /* 5- */
	'`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o', /* 6- */
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '{',  '|',  '}',  '~',  0x7F,/* 7- */
#if __dest_os == __mac_os
	',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ', /* 8- */
	',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ',  ', /* 9- */
	',  '。',  '「',  '」',  '、',  '・',  'ヲ',  'ァ',  'ィ',  'ゥ',  'ェ',  'ォ',  'ャ',  'ュ',  'セ',  'ソ', /* A- */
	'ー',  'ア',  'イ',  'ウ',  'エ',  'オ',  'カ',  'キ',  'ク',  'ケ',  'コ',  'サ',  'シ',  'ス',  'セ',  'ソ', /* B- */
	'タ',  'チ',  'ツ',  'テ',  'ト',  'ナ',  'ニ',  'ヌ',  'ネ',  'ノ',  'ハ',  ',  ',  ',  'マ',  'マ', /* C- */
	'ミ',  'ム',  'メ',  'モ',  'ヤ',  'ユ',  'ヨ',  'ラ',  'リ',  'リ',  'レ',  'ロ',  'ワ',  'ン',  '゛',  '゜', /* D- */
	'爭,  '瓰,  '皈,  '礑,  '筵,  ',  ',  ',  ',  '  ',  ',  ',  ',  ',  ', /* E- */
	'隘,  ',  ',  ',  ',  ',  ',  ',  ',  '炻,  '愠,  '珖,  '鈆,  ',  ',  EOF  /* F- */
#else
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,/* 8- */
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,/* 9- */
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,/* A- */
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,/* B- */
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,/* C- */
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,/* D- */
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,/* E- */
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF /* F- */
#endif
};

const unsigned char __upper_map[256] = {
/*   -0    -1    -2    -3    -4    -5    -6    -7    -8    -9    -A    -B    -C    -D    -E    -F       */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,/* 0- */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,/* 1- */
	' ',  '!',  '"',  '#',  '$',  '%',  '&',  '\'', '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/', /* 2- */
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?', /* 3- */
	'@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O', /* 4- */
	'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '[',  '\\', ']',  '^',  '_', /* 5- */
	'`',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O', /* 6- */
	'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '{',  '|',  '}',  '~',  0x7F,/* 7- */
#if __dest_os == __mac_os
	',  ',  ',  ',  ',  ',  ',  ',  'ヒ',  ',  ',  'フ',  ',  ',  ',  ', /* 8- */
	',  ',  ',  ',  ',  ',  ',  '淸,  ',  ',  ',  'ヘ',  ',  ',  ',  ', /* 9- */  /*mm 970922*/
	',  '。',  '「',  '」',  '、',  '・',  'ヲ',  'ァ',  'ィ',  'ゥ',  'ェ',  'ォ',  'ャ',  'ュ',  'ョ',  'ッ', /* A- */
	'ー',  'ア',  'イ',  'ウ',  'エ',  'オ',  'カ',  'キ',  'ク',  'ケ',  'コ',  'サ',  'シ',  'ス',  'ョ',  'ッ', /* B- */
	'タ',  'チ',  'ツ',  'テ',  'ト',  'ナ',  'ニ',  'ヌ',  'ネ',  'ノ',  'ハ',  'ヒ',  'フ',  'ヘ',  'ホ',  'ホ', /* C- */
	'ミ',  'ム',  'メ',  'モ',  'ヤ',  'ユ',  'ヨ',  'ラ',  'ル',  'ル',  'レ',  'ロ',  'ワ',  'ン',  '゛',  '゜', /* D- */
	'爭,  '瓰,  '皈,  '礑,  '筵,  '紮,  '罕,  '腱,  '茹,  '薈,  '螂,  '襷,  '讌,  '蹤,  '逾,  '錻, /* E- */
	'隘,  '颶,  '鬣,  '鵲,  ',  ',  ',  ',  ',  '炻,  '愠,  '珖,  '鈆,  ',  ',  EOF  /* F- */
#else
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,/* 8- */
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,/* 9- */
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,/* A- */
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,/* B- */
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,/* C- */
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,/* D- */
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,/* E- */
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF /* F- */
#endif
};

/* #ifndef __cplusplus  hh 980205 C++ is never defined here */

int (isalnum) (int c) { return(isalnum(c));  }
int (isalpha) (int c) { return(isalpha(c));  }
int (iscntrl) (int c) { return(iscntrl(c));  }
int (isdigit) (int c) { return(isdigit(c));  }
int (isgraph) (int c) { return(isgraph(c));  }
int (islower) (int c) { return(islower(c));  }
int (isprint) (int c) { return(isprint(c));  }
int (ispunct) (int c) { return(ispunct(c));  }
int (isspace) (int c) { return(isspace(c));  }
int (isupper) (int c) { return(isupper(c));  }
int (isxdigit)(int c) { return(isxdigit(c)); }
int tolower(int c)		{ return ((c == EOF) ? EOF:((int)  __lower_map[__zero_fill(c)])); } /*mm 970925*/
int toupper(int c)		{ return ((c == EOF) ? EOF:((int)  __upper_map[__zero_fill(c)])); } /*mm 970925*/

/* #endif */

/*  Change Record
 *	07-Jun-95 JFH  First code release.
 *	15-Dec-95 JFH  Added definitions for Mac's upper-128 characters - for compatibility
 *								 with old libs.
 *  22-Jan-96 JFH  Bracketed isXXX(), toupper(), and tolower() by #ifndef __cplusplus
 *								 (they are inlined in <ctype.h>).
 *	13-Feb-96 JFH  Changed the __ctype_map entries for ' (ligature fi) and '゜' (ligature fl) (0xDE and 0xDF) to lowc.
 *	21-Feb-96 JFH  Changed the __ctype_map entry for 'ァ' (0xA7) to lowc also.
 *	01-Jan-97 mani@be	Make isprint() do the right thing for characters greater
 *						than 0x7f.
 * mm 970708  Inserted Be changes
 * mm 970922  Corrected lowercase to uppercase mapping.
 * mm 970925  Added tolower and toupper to allow for detection of EOF character
 * hh 980205  removed __cplusplus
 * MM 980603  Changed __ctype_map so that non-line-breaking space 0xCA is space and not punct.  MW07352
 */