/* Values returned by the __builtin_ntype function	*/

#ifndef __bt_h
#define __bt_h

#define __BT_VOID		0x0000
#define __BT_INT		0x0100		//	has BTS_* subtype
#define __BT_FLOAT		0x0200		//	has BTS_* subtype
#define __BT_ENUM		0x0400		//	has BTS_* subtype
#define __BT_POINTER	0x0800
#define __BT_ARRAY		0x1000
#define __BT_STRUCT		0x2000
#define __BT_PTM		0x4000
#define __BT_FUNC		0x8000

#define __BTS_BOOL		0x0001		//	bool
#define __BTS_CHAR		0x0002		//	char
#define __BTS_SCHAR		0x0003		//	signed char
#define __BTS_UCHAR		0x0004		//	unsigned char
#define __BTS_WCHAR		0x0005		//	wchar_t
#define __BTS_SHORT		0x0006		//	short
#define __BTS_USHORT	0x0007		//	unsigned short
#define __BTS_INT		0x0008		//	int
#define __BTS_UINT		0x0009		//	unsigned int
#define __BTS_LONG		0x000a		//	long
#define __BTS_ULONG		0x000b		//	unsigned long
#define __BTS_LONGLONG	0x000c		//	long long
#define __BTS_ULONGLONG	0x000d		//	unsigned long long
#define __BTS_FLOAT		0x000e		//	float
#define __BTS_SDOUBLE	0x000f		//	short float
#define __BTS_DOUBLE	0x0010		//	double
#define __BTS_LDOUBLE	0x0011		//	long double

#endif