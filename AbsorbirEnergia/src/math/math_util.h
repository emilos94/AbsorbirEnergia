#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <math.h>
#include <stdlib.h>
#include "../core/definitions.h"

#define PI 3.14159265359f
#define FLOAT_EPSILON 0.0001f

float math_ToRadians(float angle);
float math_MaxF(float a, float b);
float math_MinF(float a, float b);
b32 math_float_equals(float a, float b);
float math_clamp(float min, float value, float max);
float math_lerp(float a, float b, float rate);
float math_abs(float a);
float math_rand();
f32 math_rand_range(f32 min, f32 max);

#endif // !MATH_UTIL_H
