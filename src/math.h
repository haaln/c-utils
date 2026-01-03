#ifndef MATH_MACRO_H__
#define MATH_MACRO_H__

/*
 * NOTE: It is probably not a good idea to include these inside a header;
 * define them inside a .cpp file to avoid leaking macro definitions.
 */

#define RandBool() (rand() % 2)

#define Lerp(a, b, t)       (a + (b - a) * t)
#define FLerp(a, b, t)      (((a) * (1.0f - (t))) + ((b) * (t)))
#define LerpSmooth(a, b, t) LERP((a), (b), (t) * (t) * (3.0f - 2.0f * (t)))
#define LerpAcc(a, b, t)    LERP((a), (b), (t) * (t))
#define LerpDec(a, b, t)    LERP((a), (b), 1.0f - (1.0f - (t)) * (1.0f - (t)))

#define RandInt(min, max) (rand() % (max - min + 1) + min)

#define Clamp(x, minl, maxl) (x < minl ? minl : (x > maxl ? maxl : x))

#ifndef Max
#define Max(a, b)                                                                                                      \
    ({                                                                                                                 \
        __typeof__(a) _a = (a);                                                                                        \
        __typeof__(b) _b = (b);                                                                                        \
        _a > _b ? _a : _b;                                                                                             \
    })
#endif

#ifndef Min
#define Min(a, b)                                                                                                      \
    ({                                                                                                                 \
        __typeof__(a) _a = (a);                                                                                        \
        __typeof__(b) _b = (b);                                                                                        \
        _a < _b ? _a : _b;                                                                                             \
    })
#endif

#endif // MATH_MACRO_H__
