#ifndef UTIL_MACRO_H
#define UTIL_MACRO_H

#define Bit(x) (1 << x)

#if ASSERT_DEBUGBREAK
#include <intrin.h>

#if defined(_WIN32)
#define assert(expr)                                                                                                   \
    assert(expr); :__debugbreak()
#elif defined(__linux)
#define assert(expr)                                                                                                   \
    assert(expr);                                                                                                      \
    __builtin_trap()
#endif
#endif // !ASSERT_DEBUGBREAK

#endif // UTIL_MACRO_H
