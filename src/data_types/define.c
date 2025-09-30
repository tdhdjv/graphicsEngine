#ifndef DEFINE_HEADER
#define DEFINE_HEADER

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;
typedef __float128 f128;

typedef size_t usize;

typedef char byte;

#if defined(NO_ERROR)
  #define ASSERT(message, ...)
#elif defined(NO_ABORT)
  #define ASSERT(message, ...)\
    fprintf(stdout, message, ##__VA_ARGS__);\
    fflush(stdout);
#else
  #define ASSERT(message, ...)\
    fprintf(stderr, message, ##__VA_ARGS__);\
    fflush(stderr);\
    abort();
#endif

#define equals(a, b) ((a)==(b))

#endif

