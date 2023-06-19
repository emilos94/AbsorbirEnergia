#include "math_util.h"

float math_ToRadians(float angle)
{
	return angle * (PI / 180.0f);
}


float math_maxf(float a, float b)
{
	if (a >= b) return a;

	return b;
}

float math_minf(float a, float b)
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

float math_clamp(float min, float value, float max) {
	if (value <= min) {
		return min;
	}
	if (value >= max) {
		return max;
	}
	return value;
}

float math_lerp(float a, float b, float rate) {
	return a + (b - a) * rate;
}

float math_abs(float a) {
	if (a < 0.0f) return a * -1.0f;
	return a;
}

f32 math_rand() {
	f32 result = (f32)rand() / (f32)RAND_MAX;
	return result;
}

f32 math_rand_range(f32 min, f32 max) {
	f32 result = (f32)rand() / (f32)RAND_MAX;
	f32 range = math_abs(max - min);

	return min + result * range;
}


f32 math_ease_out_cubic(f32 value) {
	f32 result = 1 - powf(1 - value, 3);
}