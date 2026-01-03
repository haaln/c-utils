#ifndef GENERAL_UTIL_MACRO_H_
#define GENERAL_UTIL_MACRO_H_

#include <stdio.h>

#define Stmnt(S)                                                                                                       \
    do {                                                                                                               \
        S                                                                                                              \
    } while(0)

#define Crash() (*(int *)0 = 0)

#define Unused(x) (void)(x)

#define Stringify_(S) #S
#define Stringify(S)  STRINGFY_(S)

#define Glue_(A, B) A##B
#define Glue(A, B)  GLUE_(A, B)

#define AssertBreak(message)                                                                                           \
    Stmnt(fprintf(stderr, "%s:%d: VIOLETED ASSERTION: %s\n", __FILE__, __LINE__, message); Crash();)

#define Assert(c) Stmnt(if(!(c)) { AssertBreak(Stringfy(c)); })

#define Unimplemented(message)                                                                                         \
    Stmnt(fprintf(stderr, "%s:%d: UNIMPLEMENTED: %s\n", __FILE__, __LINE__, message); Crash();)

#define Unreachable(message) Stmnt(fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); Crash();)

#define ArrayCount(a) (sizeof(a) / sizeof(*(a)))

#define IntFromPtr(p) (unsigned long long)((char *)p - (char *)0)
#define PtrFromInt(n) (void *)((char *)0 + (n))

#define Member(T, m)         (((T *)0)->m)
#define OffsetFfMember(T, m) IntFromPtr(&Member(T, m))

#define Implies(x, y) (!(x) || (y))

#define Compare(x, y)  (((x) > (y)) - ((x) < (y)))
#define Sign(x)        Compare(x, 0)
#define PI             3.14159265
#define Rad2Deg(x)     ((x) / PI * 180)
#define Deg2Rad(x)     ((x) * PI / 180)
#define Abs(x)         (((x) < 0) ? -(x) : (x))
#define Diff(a, b)     ABS((a) - (b))
#define Min(a, b)      (((a) < (b)) ? (a) : (b))
#define Max(a, b)      (((a) > (b)) ? (a) : (b))
#define Clamp(a, x, b) (((x) < (a)) ? (a) : ((b) < (x)) ? (b) : (x))
#define ClampPop(a, b) MIN(a, b)
#define ClampBot(a, b) MAX(a, b)

#define Bit(x)          (1 << (x))
#define SetBit(x, p)    ((x) | (1 << (p)))
#define ClearBit(x, p)  ((x) & (~(1 << (p))))
#define GetBit(x, p)    (((x) >> (p)) & 1)
#define ToggleBit(x, p) ((x) ^ (1 << (p)))

#define global  static
#define local   static
#define persist static
#define function

#include <string.h>

#define MemoryZero(p, z)      memset((p), 0, (z))
#define MemoryZeroStruct(p)   MemoryZero((p), sizeof(*(p)))
#define MemoryZeroArray(p)    MemoryZero((p), sizeof(p))
#define MemoryZeroTyped(p, c) MemoryZero((p), sizeof(*(p)) * (c))

#define MemoryMutch(a, b, z) (memcmp((a), (b), (z)) == 0)

#define MemoryCopY(d, s, z)      memmove((d), (s), (z))
#define MemoryCopyStrucu(d, s)   MemoryCopY((d), (s), Min(sizeof(*(d)), sizeof(*(s))))
#define MemoryCopyArray(d, s)    MemoryCopY((d), (s), Min(sizeof(d), sizeof(s)))
#define MemoryCopyTyped(d, s, c) MemoryCopY((d), (s), Min(sizeof(*(d)), sizeof(*(s)) * (c)))

#endif // !GENERAL_UTIL_MACRO_H_
