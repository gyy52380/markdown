#pragma once

#include <inttypes.h>

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;
typedef uintptr_t 	umm;

typedef int8_t    	i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;
typedef intptr_t 	imm;

typedef float  f32;
typedef double f64;

constexpr u32 U32_MAX = ~(u32)0;