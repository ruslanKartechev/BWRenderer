#pragma once
#include <cstdint>
#include <cglm/cglm.h>

#define f32  float
#define f64  double


#define i8  int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define quaternion versor

#define SET_VEC4(vector, x, y, z, w) vector[0] = x; vector[1] = y; vector[2] = z; vector[3] = w;
#define SET_VEC3(vector, x, y, z) vector[0] = x; vector[1] = y; vector[2] = z;
#define SET_VEC2(vector, x, y) vector[0] = x; vector[1] = y;

