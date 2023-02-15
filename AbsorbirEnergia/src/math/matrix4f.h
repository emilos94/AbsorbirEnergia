#ifndef MATRIX_FOUR_F_H
#define MATRIX_FOUR_F_H

#include "../core/definitions.h"
#include "../core/entity.h"
#include "vec2f.h"

#include "math_util.h"

#include <stdio.h>

struct Mat4f
{
	float values[16];
};
typedef struct Mat4f Mat4f;

Mat4f math_Mat4Identity();
void math_Mat4SetIdentity(Mat4f* mat4);
void math_Mat4SetZero(Mat4f* mat4);
Mat4f math_Mat4Orthographic(float left, float right, float bottom, float top, float near, float far);
Mat4f math_Mat4ModelMatrix(Transform* transform);

void math_Mat4Mul(Mat4f* left, Mat4f* right, Mat4f* destination);

void math_Mat4Copy(Mat4f* source, Mat4f* destination);

void math_Mat4Translate(Mat4f* mat4, Vec2f vec2);
void math_Mat4RotateZ(Mat4f* mat4, float angle);
void math_Mat4Scale(Mat4f* mat4, Vec2f vec2);

void math_Mat4PrettyPrint(Mat4f* mat4);

#endif // !MATRIX4f_H
