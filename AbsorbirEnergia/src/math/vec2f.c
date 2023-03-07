#include "vec2f.h"

Vec2f math_Vec2f(float x, float y)
{
	Vec2f v;
	v.x = x;
	v.y = y;

	return v;
}


Vec2f math_Vec2fMul(Vec2f either, Vec2f other)
{
	Vec2f result;
	result.x = either.x * other.x;
	result.y = either.y * other.y;
	
	return result;
}


Vec2f math_Vec2fSub(Vec2f either, Vec2f other)
{
	Vec2f result;
	result.x = either.x - other.x;
	result.y = either.y - other.y;

	return result;
}

Vec2f math_Vec2fSubScalar(Vec2f vec, float scalar)
{
	Vec2f result;
	result.x = vec.x - scalar;
	result.y = vec.y - scalar;

	return result;
}

Vec2f math_Vec2fAdd(Vec2f either, Vec2f other)
{
	Vec2f result;
	result.x = either.x + other.x;
	result.y = either.y + other.y;

	return result;
}


Vec2f math_Vec2fMulScalar(Vec2f vec, float scalar)
{
	Vec2f result;
	result.x = vec.x * scalar;
	result.y = vec.y * scalar;

	return result;
}


Vec2f math_Vec2fDivScalar(Vec2f vec, float scalar)
{
	ASSERT(scalar != 0.0f);

	Vec2f result;
	result.x = vec.x / scalar;
	result.y = vec.y / scalar;

	return result;
}

Vec2f math_Vec2fNeg(Vec2f vec)
{
	Vec2f result;
	result.x = -vec.x;
	result.y = -vec.y;

	return result;
}


void math_vec2f_negate(Vec2f* vec)
{
	ASSERT(vec);

	vec->x *= -1.0f;
	vec->y *= -1.0f;
}

void math_vec2f_zero(Vec2f* vec)
{
	ASSERT(vec);

	vec->x = 0.0f;
	vec->y = 0.0f;
}


void math_vec2f_mul_scalar(Vec2f* vec, float scalar)
{
	ASSERT(vec);

	vec->x *= scalar;
	vec->y *= scalar;
}
