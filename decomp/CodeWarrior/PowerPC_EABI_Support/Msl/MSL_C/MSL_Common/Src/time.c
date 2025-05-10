/*  Metrowerks Standard Library  */

/*  $Date: 1999/02/03 19:56:32 $ 
 *  $Revision: 1.9 $ 
 *  $NoKeywords: $ 
 *
 *		Copyright 1995-1999 Metrowerks, Inc.
 *		All rights reserved.
 */
 
/*
 *	time.c
 *
 *	Routines
 *	--------
 *		clock
 *		difftime 	(non-BeOS)      mm 970708 
 *		mktime		(non-BeOS)      mm 970708 
 *		time
 *		asctime		(non-BeOS)      mm 970708 
 *		ctime		(non-BeOS)      mm 970708 
 *		gmtime		(non-BeOS)      mm 970708
 *		localtime	(non-BeOS)      mm 970708
 *		strftime	(non-BeOS)      mm 970708
 *
 *
 */

#include "arith.h"
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if (__dest_os	== __win32_os  || __dest_os	== __wince_os)
	#include <ThreadLocalData.h>
#endif

#define seconds_per_minute	(60L)
#define seconds_per_hour		(60L * seconds_per_minute)
#define seconds_per_day			(24L * seconds_per_hour)

 const short __month_to_days[2][13] =
	{
	{0,31,59,90, 120, 151, 181, 212, 243, 273, 304,334, 365 },
	{0,31,60,91, 121, 152, 182, 213, 244, 274, 305,335, 366 }
	};

static const char *day_name[]
	= {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

static const char *month_name[]
	= {"January", "February", "March","April","May","June",
	   "July", "August",  "September", "October", "November","December"};

/*
 *	leap_year - return nonzero if year is a leap year, zero otherwise (year 0 = 1900)
 */

int __leap_year(int year);  /* hh 980205 added prototype */
int __leap_year(int year)
{
	return(__mod(year, 4) == 0 && (__mod(year, 100) != 0 || __mod(year, 400) == 100));
}

/*
 *	leap_days - return the number of leap days between 1900 (year 0)
 *	and the given year and month. year can be negative,
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

	if (__leap_year(year))
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
 *	__time2tm - convert seconds since midnight, 1/1/1900 (or 1970 on Win32),
 *	to broken-down time
 */

#if (__dest_os == __win32_os  || __dest_os == __wince_os)
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
		unsigned long	days_this_year = __leap_year(years) ? 366 : 365;

		if (days < days_this_year)
			break;

		days  -= days_this_year;
		years += 1;
	}

	tm->tm_year = years;
	tm->tm_yday = days;

	months = 0;

	is_leap_year = __leap_year(years);

	for (;;)
	{
		unsigned long days_thru_this_month = __month_to_days[is_leap_year][months+1];

		if (days < days_thru_this_month)
		{
			days -= __month_to_days[is_leap_year][months];
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
	if (!__ladd(&days, leap_days(tm->tm_year, tm->tm_mon)))	
		goto no_exit;		/* add leap days */
	if (!__ladd(&days, __month_to_days[0][tm->tm_mon]))
		goto no_exit;		/* add days to month */
	if (!__ladd(&days, tm->tm_mday))
		goto no_exit;		/* add days in month */
	if (days < 0 || days > (ULONG_MAX / seconds_per_day))
				/* make sure we're in range */
		goto no_exit;

	day_secs = days * seconds_per_day;

		/* convert days to seconds */

	seconds = (tm->tm_hour * seconds_per_hour) 	 +
							/* convert HMS to seconds */
						(tm->tm_min  * seconds_per_minute) + tm->tm_sec;

	if (seconds > ULONG_MAX - day_secs)

		/* make sure there's room */
		goto no_exit;

	seconds += day_secs;

	#if (__dest_os == __win32_os  || __dest_os == __wince_os)
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
 *	asciitime - similar to asctime, but requires a pointer to result string as input
 */

static void asciitime(struct tm tm, char * str)
{
	if (mktime(&tm) == (time_t) -1)
	{
		sprintf(str, "xxx xxx xx xx:xx:xx xxxx\n");
		return;
	}

	sprintf(str, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",

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

clock_t clock(void)
{
	return(__get_clock());
}

#endif /* ndef _No_Time_OS_Support */   /* mea 970720 */

#if __dest_os != __be_os                /* mm 970708 */

#ifndef _No_Floating_Point              /* scm 970715 */

double difftime(time_t time1, time_t time0)
{
	time_t diff;

	if (time1 >= time0)
		return(time1 - time0);

	diff = time0 - time1;

	return(- (double) diff);
}

#endif /* ndef _No_Floating_Point */    /* scm 970715 */

time_t mktime(struct tm * timeptr)
{
	struct tm	tm = *timeptr;
	time_t 		time;

	if (!__tm2time(&tm, &time))
		return((time_t) -1);

	*timeptr = tm;

	return(time);
}

#endif /* __dest_os != __be_os */      /* mm 970708 */

#ifndef _No_Time_OS_Support            /* mea 970720 */

time_t time(time_t * timer)
{
	time_t	time = __get_time();

	if (timer)
		*timer = time;

	return(time);
}

#endif /* ndef _No_Time_OS_Support */  /* mea 970720 */

#if __dest_os != __be_os               /* mm 970708 */

char * asctime(const struct tm * tm)
{
	static const struct tm	err_tm = {0, 0, 0, 1, 0, -1, 1, 0, -1};

#if (__dest_os	== __win32_os  || __dest_os	== __wince_os)
	if (tm)
		asciitime(*tm, _GetThreadLocalData()->asctime_result);
	else
		asciitime(err_tm, _GetThreadLocalData()->asctime_result);

	return(_GetThreadLocalData()->asctime_result);
#else
	__tls static char result[26];

	if (tm)
		asciitime(*tm, result);
	else
		asciitime(err_tm, result);

	return(result);
#endif
}

char * ctime(const time_t * timer)
{
	return(asctime(localtime(timer)));
}

#ifndef _No_Time_OS_Support                /* mea 970720 */

struct tm * gmtime(const time_t * timer)
{
	time_t	time;

#if !(__dest_os	== __win32_os  || __dest_os	== __wince_os)
	__tls static struct tm	tm;
#endif

	if (!timer)
	{
#if (__dest_os	== __win32_os  || __dest_os	== __wince_os)
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


#if (__dest_os	== __win32_os  || __dest_os	== __wince_os)
	__time2tm(time, &_GetThreadLocalData()->gmtime_tm);

	return(&_GetThreadLocalData()->gmtime_tm);
#else
	__time2tm(time, &tm);

	return(&tm);
#endif
}

#endif /* ndef _No_Time_OS_Support */      /* mea 970720 */

struct tm * localtime(const time_t * timer)
{
	static struct tm	tm;

	if (!timer)
		clear_tm(&tm);
	else
		__time2tm(*timer, &tm);

	return(&tm);
}

#endif /* __dest_os != __be_os */            /* mm 970708 */

static int emit(char * str, size_t size, size_t * max_size, const char * format_str, ...)
{
#if __PPC_EABI__ || __MIPS__ /* // scm 970709 */
	va_list args;                            /* scm 970709 */
	va_start( args, format_str );            /* scm 970709 */
#endif                                       /* scm 970709 */

	if (size > *max_size)
		return(0);

	*max_size -= size;

#if __PPC_EABI__ || __MIPS__ /*__dest_os == __mips_bare // scm 970709 */
	return(vsprintf(str, format_str, args)); /* scm 970709 */
#else                                        /* scm 970709 */
	return(vsprintf(str, format_str, __va_start(format_str)));
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

size_t strftime(char * str, size_t max_size, const char * format_str, const struct tm * timeptr)
{
	struct 				tm tm;
	static const struct tm	default_tm = {0, 0, 0, 1, 0, 0, 1, 0, -1};
	size_t 				num_chars, chars_written, space_remaining;      /* mm 980501 */
	const char *		format_ptr;
	const char *		curr_format;
	int					n;

	if ((space_remaining = --max_size) <= 0)							/* mm 980501 */
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
		if (!(curr_format = strchr(format_ptr, '%')))
		{
			if ((num_chars = strlen(format_ptr)) != 0)
			{
				if (num_chars <= space_remaining)						/* mm 980501 */
				{
					memcpy(str, format_ptr, num_chars);

					chars_written   += num_chars;
					str             += num_chars;
					space_remaining -= num_chars;						/* mm 980501 */
				}
				else													/* mm 980501 */
					return (0);											/* mm 980501 */
			}

			break;
		}

		if ((num_chars = curr_format - format_ptr) != 0)
		{
			if (num_chars <= space_remaining)							/* mm 980501 */
			{
				memcpy(str, format_ptr, num_chars);

				chars_written   += num_chars;
				str             += num_chars;
				space_remaining -= num_chars;							/* mm 980501 */
			}
			else														/* mm 980501 */
				return (0);												/* mm 980501 */
		}

		format_ptr = curr_format;

		switch (*++format_ptr)
		{
			case 'a':

				num_chars = emit(str, 3, &space_remaining, "%.3s", day_name[tm.tm_wday]);	/* mm 980501 */
				break;

			case 'A':

				num_chars = emit(str, strlen(day_name[tm.tm_wday]), &space_remaining, "%s", 
																	day_name[tm.tm_wday]);	/* mm 980501 */
				break;

			case 'b':

				num_chars = emit(str, 3, &space_remaining, "%.3s", month_name[tm.tm_mon]);	/* mm 980501 */
				break;

			case 'B':

				num_chars = emit(str, strlen(month_name[tm.tm_mon]), &space_remaining, "%s", 
																	month_name[tm.tm_mon]); /* mm 980501 */
				break;

			case 'c':

				num_chars = strftime(str, space_remaining+1, "%A, %d %B, %Y  %I:%M:%S %p", &tm);/* mm 980501 */
				space_remaining -= num_chars;												/* mm 980501 */
				break;

			case 'd':

				num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_mday);				/* mm 980501 */
				break;

			case 'H':

				num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_hour);				/* mm 980501 */
				break;

			case 'I':
				num_chars = emit(str, 2, &space_remaining, "%.2d", (n = tm.tm_hour % 12) ? n : 12);/* mm 980501 */
				break;

			case 'j':

				num_chars = emit(str, 3, &space_remaining, "%.3d", tm.tm_yday + 1);			/* mm 980501 */
				break;

			case 'm':

				num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_mon + 1);			/* mm 980501 */
				break;

			case 'M':

				num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_min);				/* mm 980501 */
				break;

			case 'p':

				num_chars = emit(str, 2, &space_remaining, "%.2s", tm.tm_hour < 12 ? "AM" : "PM");/* mm 980501 */
				break;

			case 'S':

				num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_sec);				/* mm 980501 */
				break;

			case 'U':

				num_chars = emit(str, 2, &space_remaining, "%.2d", week_num(&tm, 0));		/* mm 980501 */
				break;

			case 'w':

				num_chars = emit(str, 1, &space_remaining, "%.1d", tm.tm_wday);				/* mm 980501 */
				break;

			case 'W':

				num_chars = emit(str, 2, &space_remaining, "%.2d", week_num(&tm, 1));		/* mm 980501 */
				break;

			case 'x':

				num_chars = strftime(str, space_remaining+1, "%d %B, %Y", &tm);				/* mm 980501 */
				space_remaining -= num_chars;												/* mm 980501 */
				break;

			case 'X':

				num_chars = strftime(str, space_remaining+1, "%H:%M:%S", &tm);				/* mm 980501 */
				space_remaining -= num_chars;												/* mm 980501 */
				break;

			case 'y':

				num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_year % 100);   /* mm 970728 */ /* mm 980501 */
				break;

			case 'Y':

				num_chars = emit(str, 4, &space_remaining, "%.4d", tm.tm_year + 1900);		/* mm 980501 */
				break;

			case 'Z':

				++format_ptr;
				continue;
				
			case '%':                    /* mf 092497 */

				num_chars = emit(str,2, &space_remaining, "%%", *format_ptr);				/* mm 980501 */
				break;

			default:

				num_chars = emit(str, 2, &space_remaining, "%%%c", *format_ptr);			/* mm 980501 */
				break;
		}

		if (!num_chars)
			return(0);

		chars_written += num_chars;
		str           += num_chars;

		++format_ptr;
	}

	*str = 0;
	if (max_size  < chars_written) 
		return (0);
	else 
		return(chars_written);


}

#endif /* __dest_os != __be_os */          /* mm 970708 */


/*  Change Record
 *	03-Oct-95 JFH  First code release.
 *	18-Dec-95 JFH  Fixed bug in strftime where '%j' was returning 0-365 instead of 1-366.
 *	 1-Mar-96 JFH  Merged Be source in.
 *	29-Apr-96 JFH  Merged Win32 changes in.
 *						CTV
 *	14-Oct-96 KO	Changed this so when compiling for Win32 time_t is recognized
 *	            	as representing the number of seconds since 1900 rather than 1970.
 *	            	This change is needed for more compatibility with Microsoft.
 *	19-Dec-96 KO	Added some Win32 ifdefs to use my thread local data structure rather
 *              	than static local variables. Removed pointless __tls from a const
 *					static object.
 * mm 970708  Inserted Be changes
 *	09-Jul-97 SCM	Converted __va_start() call to va_start() call for PPC_EABI.
 *	15-Jul-97 SCM   Disabled difftime when _No_Floating_Point defined.
 *	20-Jul-97 MEA   Changed __no_os to _No_Time_OS_Support.
 *  mm 970728       Corrected strftime with specification %y to produce year number in range 00-99
                    BW01283
 *  mf 970924       strftime needs to return 0 when max_size < chars_written. 
 *                  Also, if the string "%%" is passed strftime should return "%".
 *  mm 980501       Corrected strftime to return correct number of characters written and make test for overrun
 *					MW06496
 *  mf 980512       wince changes
 */
