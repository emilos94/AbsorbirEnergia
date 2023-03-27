#include "vec3f.h"

Vec3f math_vec3f(float x, float y, float z)
{
	Vec3f result;
	result.x = x;
	result.y = y;
	result.z = z;

	return result;
}

void math_vec3f_set(float x, float y, float z, Vec3f* vec)
{
	vec->x = x;
	vec->y = y;
	vec->z = z;
}

void math_vec3f_set_vec3f(Vec3f* source, Vec3f* destination)
{
	destination->x = source->x;
	destination->y = source->y;
	destination->z = source->z;
}