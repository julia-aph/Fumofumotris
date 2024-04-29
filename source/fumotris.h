#pragma once
#include <stddef.h>
#include <stdint.h>

#define nullptr ((void *)0)


typedef uint8_t         u8;
typedef uint_fast8_t    u8f;

typedef uint16_t        u16;
typedef uint_fast16_t   u16f;

typedef uint32_t        u32;
typedef uint_fast32_t   u32f;

typedef uint64_t        u64;
typedef uint_fast64_t   u64f;


typedef int8_t          i8;
typedef int_fast8_t     i8f;

typedef int16_t        i16;
typedef int_fast16_t   i16f;

typedef int32_t        i32;
typedef int_fast32_t   i32f;

typedef int64_t        i64;
typedef int_fast64_t   i64f;


size_t min_size(size_t a, size_t b);


/*const u8 I[16] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    1, 1, 1, 1,
    0, 0, 0, 0
};

const u8 O[4] = {
    1, 1,
    1, 1
};

const u8 T[9] = {
    0, 1, 0,
    1, 1, 1,
    0, 0, 0
};

const u8 S[9] = {
    0, 1, 1,
    1, 1, 0,
    0, 0, 0
};

const u8 Z[9] = {
    1, 1, 0,
    0, 1, 1,
    0, 0, 0
};

const u8 J[9] = {
    1, 0, 0,
    1, 1, 1,
    0, 0, 0
};

const u8 L[9] = {
    0, 0, 1,
    1, 1, 1,
    0, 0, 0
};*/