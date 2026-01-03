#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

typedef double f64;
typedef float f32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t b8;
typedef uint16_t b16;
typedef uint32_t b32;
typedef uint64_t b64;

#define enum8(type)  u8
#define enum16(type) u16
#define enum32(type) u32
#define enum64(type) u64

typedef struct string_t {
    char *data;
    i32 length;
} string_t;

#endif // TYPES_H_
