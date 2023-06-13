#ifndef VEC2F_H
#define VEC2F_H

#include "../core/definitions.h"
#include "math_util.h"

struct Vec2f
{
	float x, y;
};
typedef struct Vec2f Vec2f;

Vec2f math_vec2f(float x, float y);
Vec2f math_Vec2fMul(Vec2f either, Vec2f other);
Vec2f math_Vec2fSub(Vec2f either, Vec2f other);
Vec2f math_Vec2fSubScalar(Vec2f vec, float scalar);
Vec2f math_Vec2fAdd(Vec2f either, Vec2f other);
Vec2f math_Vec2fMulScalar(Vec2f vec, float scalar);
Vec2f math_Vec2fDivScalar(Vec2f vec, float scalar);
Vec2f math_Vec2fNeg(Vec2f vec);
b8 math_vec2f_equals(Vec2f either, Vec2f other);
f32 math_vec2f_distance(Vec2f either, Vec2f other);

void math_vec2f_negate(Vec2f* vec);
void math_vec2f_zero(Vec2f* vec);
void math_vec2f_set(Vec2f* vec, float value);
void math_vec2f_mul_scalar(Vec2f* vec, float scalar);

// utility functions
Vec2f math_vec2f_random_withinrange(Vec2f origin, f32 minimum_distance, f32 maximum_distance);

#endif // !VEC2F_H
