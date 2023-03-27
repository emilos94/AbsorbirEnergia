#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

#define u32 unsigned int
#define f32 float
#define f64 double
#define b32 unsigned int
#define b8 uint8_t

#define TRUE 1
#define FALSE 0

#define ASSERT(x) if(!(x)) __debugbreak();
#define assert_msg(expr, ...) if ((!expr)) { fprintf(__VA_ARGS__); __debugbreak(); }

#endif // !DEFINITIONS_H
