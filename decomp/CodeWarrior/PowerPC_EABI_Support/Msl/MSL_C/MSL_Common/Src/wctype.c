/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 01:02:54 $ 
 *  $Revision: 1.7.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */
 
/*
 *	wctype.c
 *	
 *	
 *	Routines
 *	--------
 *		iswalnum		
 *		iswalpha		
 *		iswcntrl		
 *		iswdigit		
 *		iswgraph		
 *		iswlower		
 *		iswprint		
 *		iswpunct		
 *		iswspace		
 *		iswupper		
 *		iswxdigit		
 *		towlower
 *		towupper
 *      wctob        mm 980625
 *
 */

#pragma ANSI_strict off  /*  990729 vss  empty compilation unit illegal in ANSI C */

#ifndef __NO_WIDE_CHAR				/* mm 980204 */
 
#pragma ANSI_strict reset

#include <cstdio>
#include <wctype.h>
#include <string.h> /* mm 980629 */

#define wctrl	__wcontrol_char
#define wmotn	__wmotion_char
#define wspac	__wspace_char
#define wpunc	__wpunctuation
#define wdigi	__wdigit
#define whexd	__whex_digit
#define wlowc	__wlower_case
#define wuppc	__wupper_case
#define wdhex	(whexd | wdigi)
#define wuhex	(whexd | wuppc)
#define wlhex	(whexd | wlowc)

wchar_t	__wctype_map[__WCMAP_SIZE] = {
	wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wmotn, wmotn, wmotn, wmotn, wmotn, wctrl, wctrl,
	wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl,
	wspac, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc,
	wdhex, wdhex, wdhex, wdhex, wdhex, wdhex, wdhex, wdhex, wdhex, wdhex, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc,
	wpunc, wuhex, wuhex, wuhex, wuhex, wuhex, wuhex, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc,
	wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wpunc, wpunc, wpunc, wpunc, wpunc,
	wpunc, wlhex, wlhex, wlhex, wlhex, wlhex, wlhex, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc,
	wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wpunc, wpunc, wpunc, wpunc, wctrl
#if __dest_os == __mac_os
	,
	wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc,
	wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc, wlowc,
	wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wlowc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wuppc, wuppc,
	wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wlowc, wlowc,
	wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wuppc, wuppc, wuppc, wuppc, wlowc,
	wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wlowc, wuppc, wpunc, wpunc, wpunc, wpunc, wlowc, wlowc,
	wpunc, wpunc, wpunc, wpunc, wpunc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc, wuppc,
	wpunc, wuppc, wuppc, wuppc, wuppc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, 000
#elif __dest_os == __be_os       /* mm 970708  begin */
    ,
	wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc,
	wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc,
	wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc,
	wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc,
	wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wspac, wpunc, wpunc, wpunc, wpunc, wpunc,
	wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wpunc, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl,
	wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl,
	wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl, wctrl,
#endif                           /* mm 970708 end */
};

wchar_t __wlower_map[__WCMAP_SIZE] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
	L' ',  L'!',  L'"',  L'#',  L'$',  L'%',  L'&',  L'\'', L'(',  L')',  L'*',  L'+',  L',',  L'-',  L'.',  L'/',
	L'0',  L'1',  L'2',  L'3',  L'4',  L'5',  L'6',  L'7',  L'8',  L'9',  L':',  L';',  L'<',  L'=',  L'>',  L'?',
	L'@',  L'a',  L'b',  L'c',  L'd',  L'e',  L'f',  L'g',  L'h',  L'i',  L'j',  L'k',  L'l',  L'm',  L'n',  L'o',
	L'p',  L'q',  L'r',  L's',  L't',  L'u',  L'v',  L'w',  L'x',  L'y',  L'z',  L'[',  L'\\', L']',  L'^',  L'_',
	L'`',  L'a',  L'b',  L'c',  L'd',  L'e',  L'f',  L'g',  L'h',  L'i',  L'j',  L'k',  L'l',  L'm',  L'n',  L'o',
	L'p',  L'q',  L'r',  L's',  L't',  L'u',  L'v',  L'w',  L'x',  L'y',  L'z',  L'{',  L'|',  L'}',  L'~',  0x007F,
#if __dest_os == __mac_os
	L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',
	L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',  L',
	L',  L'。',  L'「',  L'」',  L'、',  L'・',  L'ヲ',  L'ァ',  L'ィ',  L'ゥ',  L'ェ',  L'ォ',  L'ャ',  L'ュ',  L'セ',  L'ソ',
	L'ー',  L'ア',  L'イ',  L'ウ',  L'エ',  L'オ',  L'カ',  L'キ',  L'ク',  L'ケ',  L'コ',  L'サ',  L'シ',  L'ス',  L'セ',  L'ソ',
	L'タ',  L'チ',  L'ツ',  L'テ',  L'ト',  L'ナ',  L'ニ',  L'ヌ',  L'ネ',  L'ノ',  L'ハ',  L',  L',  L',  L'マ',  L'マ',
	L'ミ',  L'ム',  L'メ',  L'モ',  L'ヤ',  L'ユ',  L'ヨ',  L'ラ',  L'リ',  L'リ',  L'レ',  L'ロ',  L'ワ',  L'ン',  L'゛',  L'゜',
	L'爭,  L'瓰,  L'皈,  L'礑,  L'筵,  L',  L',  L',  L',  L'  L',  L',  L',  L',  L',  L',
	L'隘,  L',  L',  L',  L',  L',  L',  L',  L',  L'炻,  L'愠,  L'珖,  L'鈆,  L',  L',  0x00FF /*mm 980629b */
#else
	0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
	0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
	0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
	0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
	0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
#endif
};

wchar_t __wupper_map[__WCMAP_SIZE] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
	L' ',  L'!',  L'"',  L'#',  L'$',  L'%',  L'&',  L'\'', L'(',  L')',  L'*',  L'+',  L',',  L'-',  L'.',  L'/',
	L'0',  L'1',  L'2',  L'3',  L'4',  L'5',  L'6',  L'7',  L'8',  L'9',  L':',  L';',  L'<',  L'=',  L'>',  L'?',
	L'@',  L'A',  L'B',  L'C',  L'D',  L'E',  L'F',  L'G',  L'H',  L'I',  L'J',  L'K',  L'L',  L'M',  L'N',  L'O',
	L'P',  L'Q',  L'R',  L'S',  L'T',  L'U',  L'V',  L'W',  L'X',  L'Y',  L'Z',  L'[',  L'\\', L']',  L'^',  L'_',
	L'`',  L'A',  L'B',  L'C',  L'D',  L'E',  L'F',  L'G',  L'H',  L'I',  L'J',  L'K',  L'L',  L'M',  L'N',  L'O',
	L'P',  L'Q',  L'R',  L'S',  L'T',  L'U',  L'V',  L'W',  L'X',  L'Y',  L'Z',  L'{',  L'|',  L'}',  L'~',  0x007F,
#if __dest_os == __mac_os
	L',  L',  L',  L',  L',  L',  L',  L',  L'ヒ',  L',  L',  L'フ',  L',  L',  L',  L',
	L',  L',  L',  L',  L',  L',  L',  L'淸,  L',  L',  L',  L'ヘ',  L',  L',  L',  L',  /*mm 970922*/
	L',  L'。',  L'「',  L'」',  L'、',  L'・',  L'ヲ',  L'ァ',  L'ィ',  L'ゥ',  L'ェ',  L'ォ',  L'ャ',  L'ュ',  L'ョ',  L'ッ',
	L'ー',  L'ア',  L'イ',  L'ウ',  L'エ',  L'オ',  L'カ',  L'キ',  L'ク',  L'ケ',  L'コ',  L'サ',  L'シ',  L'ス',  L'ョ',  L'ッ',
	L'タ',  L'チ',  L'ツ',  L'テ',  L'ト',  L'ナ',  L'ニ',  L'ヌ',  L'ネ',  L'ノ',  L'ハ',  L'ヒ',  L'フ',  L'ヘ',  L'ホ',  L'ホ',
	L'ミ',  L'ム',  L'メ',  L'モ',  L'ヤ',  L'ユ',  L'ヨ',  L'ラ',  L'ル',  L'ル',  L'レ',  L'ロ',  L'ワ',  L'ン',  L'゛',  L'゜',
	L'爭,  L'瓰,  L'皈,  L'礑,  L'筵,  L'紮,  L'罕,  L'腱,  L'茹,  L'薈,  L'螂,  L'襷,  L'讌,  L'蹤,  L'逾,  L'錻,
	L'隘,  L'颶,  L'鬣,  L'鵲,  L',  L',  L',  L',  L',  L'炻,  L'愠,  L'珖,  L'鈆,  L',  L',  0x00FF /* mm 980629b */   
#else
	0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
	0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
	0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
	0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
	0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
	0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
	0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
#endif
};

#ifndef __cplusplus

/* begin mm 980626 */
int (iswalnum) (wint_t c) { return(iswalnum(c));  }
int (iswalpha) (wint_t c) { return(iswalpha(c));  }
int (iswcntrl) (wint_t c) { return(iswcntrl(c));  }
int (iswdigit) (wint_t c) { return(iswdigit(c));  }
int (iswgraph) (wint_t c) { return(iswgraph(c));  }
int (iswlower) (wint_t c) { return(iswlower(c));  }
int (iswprint) (wint_t c) { return(iswprint(c));  }
int (iswpunct) (wint_t c) { return(iswpunct(c));  }
int (iswspace) (wint_t c) { return(iswspace(c));  }
int (iswupper) (wint_t c) { return(iswupper(c));  }
int (iswxdigit)(wint_t c) { return(iswxdigit(c)); }

wint_t towlower(wint_t c){ return ((c >= __WCMAP_SIZE) ? c:((wint_t)  __wlower_map[__wz_fill(c)])); } /*mm 970925*/
wint_t towupper(wint_t c){ return ((c >= __WCMAP_SIZE) ? c:((wint_t)  __wupper_map[__wz_fill(c)])); } /*mm 970925*/
/* end mm 980626 */
int wctob(wint_t c) { return((c & 0xFF00) ? EOF : (int)c);}  /* mm 980625 */
#endif

/* begin mm 980629a */
wctype_t wctype(const char *property)
{
	if (strcmp(property, "alnum") == 0)
		return(__walphanumeric);
	if (strcmp(property, "alpha") == 0)
		return(__wletter);
	if (strcmp(property, "cntrl") == 0)
		return(__wcontrol);
	if (strcmp(property, "digit") == 0)
		return(__wdigit);
	if (strcmp(property, "graph") == 0)
		return(__wgraphic);
	if (strcmp(property, "lower") == 0)
		return(__wlower_case);
	if (strcmp(property, "print") == 0)
		return(__wprintable);
	if (strcmp(property, "punct") == 0)
		return(__wpunctuation);
	if (strcmp(property, "space") == 0)
		return(__wwhitespace);
	if (strcmp(property, "upper") == 0)
		return(__wupper_case);
	if (strcmp(property, "xdigit") == 0)
		return(__whex_digit);
	return(0);   /* if all else fails */
}

int iswctype(wint_t wc, wctype_t desc)
{
	if (wc < __WCMAP_SIZE)
		return(__wctype_map[__wz_fill(wc)] & desc);
	else
		return 0;
}
/* end mm 980629a */
#endif /* #ifndef __NO_WIDE_CHAR */				/* mm981030 */

/*  Change Record
 *	980121 	JCM  First code release
 *  mm 980625  Defined wctob.
 *  mm 980626  Considerable rewrite of the wc classification functions, towlower and towupper to 
 *             match C9x
 *  mm 980629a  Implemented wctype() and iswctype()
 *  mm 980629b  Correction to __wupper_map and __wlower_map
 *  mm 981030   Added #ifndef __NO_WIDE_CHAR wrappers
 */
