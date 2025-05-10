/*  Metrowerks Standard Library  */

/*  $Date: 1999/07/30 01:02:53 $ 
 *  $Revision: 1.8.4.1 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */
 
/*
 *	wtime.c
 *
 *	Routines
 *	--------
 *		wasctime	 
 *		wctime		 
 *		wcsftime	
 *
 *
 */

#pragma ANSI_strict off  /*  990729 vss  empty compilation unit illegal in ANSI C */

#ifndef __NO_WIDE_CHAR				/* mm 980204 */
 
#pragma ANSI_strict reset

#include <arith.h>
#include <climits>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>

#include <wchar.h>


#if (__dest_os	== __win32_os || __dest_os	== __wince_os)
	#include <ThreadLocalData.h>
#endif

#define seconds_per_minute	(60L)
#define seconds_per_hour		(60L * seconds_per_minute)
#define seconds_per_day			(24L * seconds_per_hour)

static const short month_to_days[2][13] =
	{
		{   0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304,
334, 365 },
		{   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305,
335, 366 }
	};

static const wchar_t *day_name[]
	= {L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday",
		L"Friday", L"Saturday"};

static const wchar_t *month_name[]
	= {L"January", L"February", L"March",     L"April",   L"May",      L"June",
		 L"July",     L"August",  L"September", L"October", L"November",
L"December"};

/*
 *	leap_year - return nonzero if year is a leap year, zero otherwise (year 0 = 1900)
 */

static int leap_year(int year)
{
	return(__mod(year, 4) == 0 && (__mod(year, 100) != 0 || __mod(year, 400) == 100));
}

/*
 *	leap_days - return the number of leap days between 1900 (year 0)
 *							and the given year and month. year can be negative,
 *							month cannot.
 */

static int leap_days(int year, int mon)
{
	int		n;
	div_t	q;

	q = div(year, 4);
	/* 1 leap day every four years */

	n = q.quot;

	q = div(year, 100);						/* except at the turn of the century */

	n -= q.quot;

	if (year < 100)
	/* not counting the turn of the millenium */
	{
		q = __div(year+899, 1000);

		n += q.quot;
	}
	else
	{
		q = __div(year-100, 1000);

		n += q.quot + 1;
	}

	if (leap_year(year))
		if (year < 0)
		{
			if (mon > 1)
				++n;
		}
		else
			if (mon <= 1)
				--n;

	return(n);
}

/*
 *	adjust - force x to be a modulo y number, add overflow to z
 */


static int adjust(int * x, int y, int * z)
{
	div_t	q;

	q = __div(*x, y);

	*x = q.rem;

	return(__add(z, q.quot));
}

/*
 *	__time2tm - convert seconds since midnight, 1/1/1900 (or 1970 on
Win32),
 *	to broken-down time
 */

#if (__dest_os == __win32_os || __dest_os == __wince_os)
static void __time2tm(time_t inTime, struct tm * tm)
#else
static void __time2tm(time_t time, struct tm * tm)
#endif
{
	unsigned long	years, months, days, seconds;
	int						is_leap_year;

	#if (__dest_os == __win32_os  || __dest_os == __wince_os)
		/* Since Win32 time_t is relative to 1970 rather than 1900.
		 * This must be of type unsigned long rather than a signed
		 * time_t to prevent overflow */
		unsigned long time = inTime + ((365 * 70UL) + 17) * 24 * 60 * 60;
	#endif

	if (!tm)
		return;

	tm->tm_isdst = -1;

	days    = time / seconds_per_day;
	seconds = time % seconds_per_day;

	tm->tm_wday = (days + 1) % 7;
			/* January 1, 1900 was a Monday */

	years = 0;

	for (;;)
	{
		unsigned long	days_this_year = leap_year(years) ? 366 : 365;

		if (days < days_this_year)
			break;

		days  -= days_this_year;
		years += 1;
	}

	tm->tm_year = years;
	tm->tm_yday = days;

	months = 0;

	is_leap_year = leap_year(years);

	for (;;)
	{
		unsigned long days_thru_this_month = month_to_days[is_leap_year][months+1];

		if (days < days_thru_this_month)
		{
			days -= month_to_days[is_leap_year][months];
			break;
		}

		++months;
	}

	tm->tm_mon  = months;
	tm->tm_mday = days + 1;

	tm->tm_hour = seconds / seconds_per_hour;

	seconds %= seconds_per_hour;

	tm->tm_min = seconds / seconds_per_minute;
	tm->tm_sec = seconds % seconds_per_minute;
}

/*
 *	__tm2time - convert broken-down time to seconds since midnight,
 *	1/1/1900 (or 1970 on Win32). return zero if broken-down time can't be
 *	represented; otherwise, convert seconds back into broken-down time and
 *	return nonzero.
 *
 *	Note:	Each field in broken-down time is forced into the normal range
 *	for that field, with overflow being added to next field up
 *	through mday (day of month). mday is allowed to remain out of
 *	range. mon is forced into its normal range with overflow being
 *	added to year. year and mon are converted to days since 1/1/1900
 *	with mday and leap days between 1/1/1900 and year and mon added.
 *	If the result is negative, zero is returned. Otherwise, it is
 *	converted to seconds and added to hour, min and sec converted to
 *	seconds to yield the final result. Overflow checking is performed
 *	where needed and if overflow occurs, zero is returned; nonzero
 *	otherwise.
 */

static int __tm2time(struct tm * tm, time_t * time)
{
	long		days;
	time_t	seconds, day_secs;

	if (!tm || !time)
		return(0);


	/* I don't know why it isn't     */
	--tm->tm_mday;
	/* zero-based in the first place */

	if (!adjust(&tm->tm_sec,  60, &tm->tm_min))	goto no_exit;
			/* put sec  in range 0-59 */
	if (!adjust(&tm->tm_min,  60, &tm->tm_hour))	goto no_exit;
		/* put min  in range 0-59 */
	if (!adjust(&tm->tm_hour, 24, &tm->tm_mday))	goto no_exit;
		/* put hour in range 0-23 */

	if (!adjust(&tm->tm_mon,  12, &tm->tm_year))	goto no_exit;
		/* put mon  in range 0-11 */

	days = tm->tm_year;

	if (!__lmul(&days, 365))

		goto no_exit;		/* convert year to days */
	if (!__ladd(&days, leap_days(tm->tm_year, tm->tm_mon)))	goto
no_exit;		/* add leap days */
	if (!__ladd(&days, month_to_days[0][tm->tm_mon]))
		goto no_exit;		/* add days to month */
	if (!__ladd(&days, tm->tm_mday))
								goto
no_exit;		/* add days in month */

	if (days < 0 || days > (ULONG_MAX / seconds_per_day))
				/* make sure we're in range */
		goto no_exit;

	day_secs = days * seconds_per_day;

		/* convert days to seconds */

	seconds = (tm->tm_hour * seconds_per_hour) 	 +
							/* convert HMS to
seconds */
						(tm->tm_min  *
seconds_per_minute) +
						 tm->tm_sec;

	if (seconds > ULONG_MAX - day_secs)

		/* make sure there's room */
		goto no_exit;

	seconds += day_secs;

	#if (__dest_os == __win32_os || __dest_os == __wince_os)
		/* Make this number relative to 1970 rather than 1900 */
		seconds -= ((365 * 70UL) + 17) * 24 * 60 * 60;
	#endif

	*time = seconds;

	__time2tm(seconds, tm);

	return(1);

no_exit:

	return(0);
}

/*
 *	wasciitime - similar to wasctime, but requires a pointer to result
string as input
 */

static void wasciitime(struct tm tm, wchar_t * str)
{
	size_t n;
	
	if (mktime(&tm) == (time_t) -1)
	{
		n = wcslen(L"xxx xxx xx xx:xx:xx xxxx\n");
		swprintf(str, n, L"xxx xxx xx xx:xx:xx xxxx\n");
		return;
	}

		n = wcslen( L"%.3s %.3s%3d %.2d:%.2d:%.2d %d\n");
	swprintf(str, n, L"%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",

	day_name[tm.tm_wday],

	month_name[tm.tm_mon],

	tm.tm_mday,

	tm.tm_hour,

	tm.tm_min,

	tm.tm_sec,

	tm.tm_year + 1900);
}

/*
 *	clear_tm - sets a broken-down time to the equivalent of 1900/1/1 00:00:00
 */

static void clear_tm(struct tm * tm)
{
	tm->tm_sec   =  0;
	tm->tm_min   =  0;
	tm->tm_hour  =  0;
	tm->tm_mday  =  1;
	tm->tm_mon   =  0;
	tm->tm_year  =  0;
	tm->tm_wday  =  1;
	tm->tm_yday  =  0;
	tm->tm_isdst = -1;
}

/*
 *	ANSI Routines
 */

#ifndef _No_Time_OS_Support              /* mea 970720 */


/*

clock_t clock(void)
{
	return(__get_clock());
}

*/


#endif /* ndef _No_Time_OS_Support */   /* mea 970720 */

#if __dest_os != __be_os                /* mm 970708 */

#ifndef _No_Floating_Point              /* scm 970715 */


/*

double difftime(time_t time1, time_t time0)
{
	time_t diff;

	if (time1 >= time0)
		return(time1 - time0);

	diff = time0 - time1;

	return(- (double) diff);
}

*/


#endif /* ndef _No_Floating_Point */    /* scm 970715 */


/*

time_t mktime(struct tm * timeptr)
{
	struct tm	tm = *timeptr;
	time_t 		time;

	if (!__tm2time(&tm, &time))
		return((time_t) -1);

	*timeptr = tm;

	return(time);
}

*/

#endif /* __dest_os != __be_os */      /* mm 970708 */

#ifndef _No_Time_OS_Support            /* mea 970720 */


/* 

time_t time(time_t * timer)
{
	time_t	time = __get_time();

	if (timer)
		*timer = time;

	return(time);
}

*/

#endif /* ndef _No_Time_OS_Support */  /* mea 970720 */

#if __dest_os != __be_os               /* mm 970708 */

wchar_t * wasctime(const struct tm * tm)
{
	static const struct tm	err_tm = {0, 0, 0, 1, 0, -1, 1, 0, -1};

#if (__dest_os	== __win32_os || __dest_os	== __wince_os)
	if (tm)
		wasciitime(*tm, (wchar_t*)_GetThreadLocalData()->asctime_result);
	else
		wasciitime(err_tm, (wchar_t*)_GetThreadLocalData()->asctime_result);

	return((wchar_t*)_GetThreadLocalData()->asctime_result);
#else
	__tls static wchar_t result[26];

	if (tm)
		wasciitime(*tm, result);
	else
		wasciitime(err_tm, result);

	return(result);
#endif
}

wchar_t * wctime(const time_t * timer)
{
	return(wasctime(localtime(timer)));
}

#ifndef _No_Time_OS_Support                /* mea 970720 */


/*

struct tm * gmtime(const time_t * timer)
{
	time_t
	time;

#if (__dest_os	!= __win32_os)
	__tls static struct tm	tm;
#endif

	if (!timer)
	{
#if (__dest_os	== __win32_os)
		clear_tm(&_GetThreadLocalData()->gmtime_tm);
		return(&_GetThreadLocalData()->gmtime_tm);
#else
		clear_tm(&tm);
		return(&tm);
#endif
	}

	time = *timer;

	if (!__to_gm_time(&time))
		return(NULL);


#if (__dest_os	== __win32_os)
	__time2tm(time, &_GetThreadLocalData()->gmtime_tm);

	return(&_GetThreadLocalData()->gmtime_tm);
#else
	__time2tm(time, &tm);

	return(&tm);
#endif
}

*/


#endif /* ndef _No_Time_OS_Support */      /* mea 970720 */


/*

struct tm * localtime(const time_t * timer)
{
	static struct tm	tm;

	if (!timer)
		clear_tm(&tm);
	else
		__time2tm(*timer, &tm);

	return(&tm);
}

*/


#endif /* __dest_os != __be_os */            /* mm 970708 */

static int wemit(wchar_t * str, size_t size, size_t * max_size, const wchar_t * format_str, ...)
{
#if __PPC_EABI__ || __MIPS__  /* __dest_os == __mips_bare // scm 970709 */
	va_list args;                            /* scm 970709 */	
	va_start( args, format_str );            /* scm 970709 */
#endif                                       /* scm 970709 */

	if (size > *max_size)
		return(0);

	*max_size -= size;

#if __PPC_EABI__ || __MIPS__  /* __dest_os == __mips_bare // scm 970709 */
	return(vswprintf(str, size+1,  format_str, args)); /* scm 970709 */		/* mm 990322 */
#else                                        /* scm 970709 */
	return(vswprintf(str, size+1, format_str, __va_start(format_str)));		/* mm 990322 */
#endif                                       /* scm 970709 */
}

static int week_num(const struct tm * tm, int starting_day)
{
	int	days = tm->tm_yday;

	days -= __mod(tm->tm_wday - starting_day, 7);

	if (days < 0)
		return(0);

	return((days / 7) + 1);
}


#if __dest_os != __be_os          /* mm 970708 */

size_t wcsftime(wchar_t * str, size_t max_size, const wchar_t * format_str, const struct tm * timeptr)
{
	struct tm
	tm;
	static const struct tm	default_tm = {0, 0, 0, 1, 0, 0, 1, 0, -1};
	size_t
	num_chars, chars_written;
	const wchar_t *						format_ptr;
	const wchar_t *						curr_format;
	int
			n;

	if (--max_size <= 0)
		return(0);

	tm = default_tm;

	if (timeptr)
	{
		tm = *timeptr;

		if (mktime(&tm) == (time_t) -1)
			tm = default_tm;
	}

	format_ptr    = format_str;
	chars_written = 0;

	while (*format_ptr)
	{
		if (!(curr_format = wcschr(format_ptr, '%')))
		{
			if ((num_chars = wcslen(format_ptr)) != 0)
			{
				wmemcpy(str, format_ptr, num_chars);

				chars_written += num_chars;
				str           += num_chars;
			}

			break;
		}

		if ((num_chars = curr_format - format_ptr) != 0)
		{
			wmemcpy(str, format_ptr, num_chars);

			chars_written += num_chars;
			str           += num_chars;
		}

		format_ptr = curr_format;

		switch (*++format_ptr)
		{
			case L'a':

				num_chars = wemit(str, 3, &max_size, L"%.3s", day_name[tm.tm_wday]);
				break;

			case L'A':

				num_chars = wemit(str, wcslen(day_name[tm.tm_wday]), &max_size, L"%s", day_name[tm.tm_wday]);
				break;

			case L'b':

				num_chars = wemit(str, 3, &max_size, L"%.3s", month_name[tm.tm_mon]);
				break;

			case L'B':

				num_chars = wemit(str, wcslen(month_name[tm.tm_mon]), &max_size, L"%s", month_name[tm.tm_mon]);
				break;

			case L'c':

				num_chars = wcsftime(str, max_size, L"%A, %d %B, %Y  %I:%M:%S %p", &tm);
				max_size -= num_chars;
				break;

			case L'd':

				num_chars = wemit(str, 2, &max_size, L"%.2d", tm.tm_mday);
				break;

			case L'H':

				num_chars = wemit(str, 2, &max_size, L"%.2d", tm.tm_hour);
				break;

			case L'I':
				num_chars = wemit(str, 2, &max_size, L"%.2d", (n = tm.tm_hour % 12) ? n : 12);
				break;

			case L'j':

				num_chars = wemit(str, 3, &max_size, L"%.3d", tm.tm_yday + 1);
				break;

			case L'm':

				num_chars = wemit(str, 2, &max_size, L"%.2d", tm.tm_mon + 1);
				break;

			case L'M':

				num_chars = wemit(str, 2, &max_size, L"%.2d", tm.tm_min);
				break;

			case L'p':

				num_chars = wemit(str, 2, &max_size, L"%.2s", tm.tm_hour < 12 ? L"AM" : L"PM");/* mm 990322a */
				break;

			case L'S':

				num_chars = wemit(str, 2, &max_size, L"%.2d", tm.tm_sec);
				break;

			case L'U':

				num_chars = wemit(str, 2, &max_size, L"%.2d", week_num(&tm, 0));
				break;

			case L'w':

				num_chars = wemit(str, 1, &max_size, L"%.1d", tm.tm_wday);
				break;

			case L'W':

				num_chars = wemit(str, 2, &max_size, L"%.2d", week_num(&tm, 1));
				break;

			case L'x':

				num_chars = wcsftime(str, max_size, L"%d %B, %Y", &tm);
				max_size -= num_chars;
				break;

			case L'X':

				num_chars = wcsftime(str, max_size, L"%H:%M:%S", &tm);
				max_size -= num_chars;
				break;

			case L'y':

				num_chars = wemit(str, 2, &max_size, L"%.2d", tm.tm_year % 100);   /* mm 970728 */
				break;

			case L'Y':

				num_chars = wemit(str, 4, &max_size, L"%.4d", tm.tm_year + 1900);
				break;

			case L'Z':

				++format_ptr;

				continue;
			case L'%':                    /* mf 092497 */

				num_chars = wemit(str,2, &max_size, L"%%", *format_ptr);
				break;

			default:

				num_chars = wemit(str, 2, &max_size, L"%%%c", *format_ptr);
				break;
		}

		if (!num_chars)
			return(0);

		chars_written += num_chars;
		str           += num_chars;

		++format_ptr;
	}

	*str = 0;
if(max_size  < chars_written) return (0);
else return(chars_written);


}

#endif /* __dest_os != __be_os */          /* mm 970708 */
#endif /* #ifndef __NO_WIDE_CHAR*/				/* mm 981020 */

/*  Change Record
 *	980128 JCM  First code release.
 *  mf 051498 wince changes
 * mm 981020  Added #ifndef __NO_WIDE_CHAR wrappers
 *  mm 990322	Allow for the fact that the count wsprintf includes the terminating null char.  See C9x
 *  mm 990322a  Make two char string constants into wchar_t constants.	
 */

