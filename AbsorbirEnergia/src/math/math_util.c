#include "math_util.h"

float math_ToRadians(float angle)
{
	return angle * (PI / 180.0f);
}


float math_MaxF(float a, float b)
{
	if (a >= b) return a;

	return b;
}

float math_MinF(float a, float b)
{
	if (a <= b) return a;

	return b;
}


b32 math_float_equals(float a, float b)
{
	return math_Abs(a - b) <= FLOAT_EPSILON;
}

float math_Abs(float a)
{
	if (a < 0.0f)
	{
		return -1.0f * a;
	}

	return a;
}