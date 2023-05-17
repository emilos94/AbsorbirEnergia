#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

typedef uint32_t u32;
typedef int32_t s32;
typedef float f32;
#define f64 double
#define b32 unsigned int
#define b8 uint8_t

#define TRUE 1
#define FALSE 0

#define ASSERT(x) if(!(x)) __debugbreak();
#define assert_msg(expr, ...) if (!(expr)) { fprintf(__VA_ARGS__); __debugbreak(); }

#endif // !DEFINITIONS_H
