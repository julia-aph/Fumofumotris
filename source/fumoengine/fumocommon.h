#pragma once
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>

#define nullptr ((void *)0)

#define ONE_E_9 1000000000


typedef size_t          usize;


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

typedef int16_t         i16;
typedef int_fast16_t    i16f;

typedef int32_t         i32;
typedef int_fast32_t    i32f;

typedef int64_t         i64;
typedef int_fast64_t    i64f;


typedef u64 nsec;


size_t min_usize(usize a, usize b);

nsec TimeNow();
double TimeNowD();

u32 Hash(void *item, usize size);