#ifndef _H_MACROS_
#define _H_MACROS_

#define ARRAY_COUNT(arr)     (int)(sizeof (arr) / sizeof (arr[0]))
#define FLAG_ON(V, F)        (((V) & (F)) == 0)
#define FLAG_OFF(V, F)       (((V) & (F)) != 0)

#define ALIGN_PREV(u, align) (u & (~(align - 1)))
#define ALIGN_NEXT(u, align) ((u + (align - 1)) & (~(align - 1)))
#define IS_ALIGNED(X, N)     (((X) & ((N) - 1)) == 0)
#define IS_NOT_ALIGNED(X, N) (((X) & ((N) - 1)) != 0)

#define READU32_BE(ptr, offset)                                                                \
    (((u32)ptr[offset] << 24) | ((u32)ptr[offset + 1] << 16) | ((u32)ptr[offset + 2] << 8) |   \
     (u32)ptr[offset + 3]);

#ifdef DEBUG
#define ASSERTLINE(cond, line)                                                                 \
    ((cond) || (OSPanic (__FILE__, line, "Failed assertion " #cond), 0))

#define ASSERTMSGLINE(cond, msg, line) ((cond) || (OSPanic (__FILE__, line, msg), 0))

// This is dumb but we dont have a Metrowerks way to do variadic macros in the
// macro to make this done in a not scrubby way.
#define ASSERTMSG1LINE(cond, msg, arg1, line)                                                  \
    ((cond) || (OSPanic (__FILE__, line, msg, arg1), 0))

#define ASSERTMSG2LINE(cond, msg, arg1, arg2, line)                                            \
    ((cond) || (OSPanic (__FILE__, line, msg, arg1, arg2), 0))

#define ASSERTMSGVLINE(cond, line, ...) ((cond) || (OSPanic (__FILE__, line, __VA_ARGS__), 0))

#else
#define ASSERTLINE(cond, line)                      (void)0
#define ASSERTMSGLINE(cond, msg, line)              (void)0
#define ASSERTMSG1LINE(cond, msg, arg1, line)       (void)0
#define ASSERTMSG2LINE(cond, msg, arg1, arg2, line) (void)0
#define ASSERTMSGLINEV(cond, line, ...)             (void)0
#endif

// clang-format off
#define FORCE_DONT_INLINE \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; \
	(void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0; (void*)0;
// clang-format on

#define TRUNC(n, a)                     (((u32)(n)) & ~((a) - 1))

#define OFFSET(addr, align)             (((u32)(addr) & ((align) - 1)))

#define PATH_MAX                        (256)

// Sets specific flag to 1
#define SET_FLAG(x, val)                (x |= (val))

// Resets specific flag from (val) back to 0
#define RESET_FLAG(x, val)              (x &= ~(val))

// Return 1 if flag is set, 0 if flag is not set
#define IS_FLAG(x, val)                 (x & val)

// Array size define
#define ARRAY_SIZE(o)                   (sizeof ((o)) / sizeof (*(o)))

// Align object to num bytes (num should be power of two)
#define ATTRIBUTE_ALIGN(num)            __attribute__ ((aligned (num)))

// Checks if a flag is set in a bitfield
#define IS_FLAG_SET(flags, bitsFromLSB) (((flags) >> (bitsFromLSB) & 1))

#define ASSERT_HANG(cond)                                                                      \
    if (!(cond))                                                                               \
    {                                                                                          \
        while (true) {}                                                                        \
    }

// Get the maximum of two values
#define MAX(a, b)          (((a) > (b)) ? (a) : (b))

// Get the minimum of two values
#define MIN(a, b)          (((a) < (b)) ? (a) : (b))

// Rounds a float to a u8
#define ROUND_F32_TO_U8(a) a >= 0.0f ? a + 0.5f : a - 0.5f

// Number of bytes in a kilobyte
#define KILOBYTE_BYTECOUNT 1024

#define BUMP_REGISTER(reg)                                                                     \
    {                                                                                          \
        asm { mr reg, reg }                                                                       \
    }

#ifdef __MWERKS__
#define WEAKFUNC        __declspec (weak)
#define DECL_SECT(name) __declspec (section name)
#define ASM             asm
#else
#define WEAKFUNC
#define DECL_SECT(name)
#define ASM
#endif

// Disable some clangd warnings
#ifdef __clang__
// Allow string literals to be converted to char*
#pragma clang diagnostic ignored "-Wc++11-compat-deprecated-writable-strings"
#endif

#define __MSTRING(x)           #x
#define MSTRING(x)             __MSTRING (x)

#define __MSTRING_CONCAT(a, b) a##b
#define MSTRING_CONCAT(a, b)   __MSTRING_CONCAT (a, b)

// TODO: make more unique
#define SASSERT(expr)                                                                          \
    enum                                                                                       \
    {                                                                                          \
        MSTRING_CONCAT (assert_fail_at_, __LINE__) = 1 / (expr)                                \
    }
#define SASSERT_SIZE(type, size) SASSERT (sizeof (type) == size)

#ifndef __MWERKS__

#ifdef __POWERPC__
#undef __POWERPC__
#endif

#ifndef __PPC_EABI__
#define __PPC_EABI__
#endif

#define __builtin_va_info
#define __option(x)   0
#define __declspec(x) 0
#define __frsqrte(x)  0
#define __fabsf(x)    0
#define __sync()      0
#define __cntlzw(x)   0
#define __cdecl       0
#define asm

#endif

#endif // _H_MACROS_
