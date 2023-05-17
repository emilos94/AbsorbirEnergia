#ifndef VEC3F_H
#define VEC3F_H

#include "../core/definitions.h"

struct Vec3f
{
	float x, y, z;
};
typedef struct Vec3f Vec3f;

Vec3f math_vec3f(float x, float y, float z);
void math_vec3f_set(float x, float y, float z, Vec3f* vec);
void math_vec3f_set_vec3f(Vec3f* source, Vec3f* destination);
#endif