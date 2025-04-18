/* Metrowerks Standard Library
 * Copyright C 1995-2001 Metrowerks Corporation.  All rights reserved.
 *
 * $Date: 2001/07/21 18:43:30 $
 * $Revision: 1.12.4.1 $
 */

/*
 *  Based on the C99 Standard for stdint.h
 */

#ifndef _MSL_STDINT_H
#define _MSL_STDINT_H

#if __MACH__
	#error You must have the /usr/include access path before the MSL access path
#else

#include <cstdint>

#if defined(__cplusplus) && defined(_MSL_USING_NAMESPACE)
	using std::int8_t;
	using std::int16_t;
	using std::int32_t;
	using std::uint8_t;
	using std::uint16_t;
	using std::uint32_t;
	using std::int_least8_t;
	using std::int_least16_t;
	using std::int_least32_t;
	using std::uint_least8_t;
	using std::uint_least16_t;
	using std::uint_least32_t;
	using std::int_fast8_t;
	using std::int_fast16_t;
	using std::int_fast32_t;
	using std::uint_fast8_t;
	using std::uint_fast16_t;
	using std::uint_fast32_t;
#ifdef __MSL_LONGLONG_SUPPORT__
	using std::int64_t;
	using std::uint64_t;
	using std::int_least64_t;
	using std::uint_least64_t;
	using std::int_fast64_t;
	using std::uint_fast64_t;
#endif
	using std::intptr_t;
	using std::uintptr_t;
	using std::intmax_t;
	using std::uintmax_t;
#endif

#endif /* __MACH__ */

#endif /* _MSL_STDINT_H */


/* Change record:
 * blc 990323 Created.
 * hh  991113 Fixed using bug.
 * mm  000512 Updated to C99 Standard
 * JWW 001208 Added case for targeting Mach-O
 */