#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <math.h>
#include "../core/definitions.h"

#define PI 3.14159265359f
#define FLOAT_EPSILON 0.0001f

float math_ToRadians(float angle);
float math_MaxF(float a, float b);
float math_MinF(float a, float b);
B32 math_FloatEquals(float a, float b);

#endif // !MATH_UTIL_H
