#include "matrix4f.h"

Mat4f math_Mat4Identity()
{
	Mat4f mat4;
	for (U32 i = 0; i < 16; i++)
	{
		mat4.values[i] = 0;
	}

	mat4.values[0] = 1.0f;
	mat4.values[5] = 1.0f;
	mat4.values[10] = 1.0f;
	mat4.values[15] = 1.0f;

	return mat4;
}


void math_Mat4SetIdentity(Mat4f* mat4)
{
	for (U32 i = 0; i < 16; i++)
	{
		mat4->values[i] = 0;
	}

	mat4->values[0] = 1.0f;
	mat4->values[5] = 1.0f;
	mat4->values[10] = 1.0f;
	mat4->values[15] = 1.0f;
}

void math_Mat4SetZero(Mat4f* mat4)
{
	for (U32 i = 0; i < 16; i++)
	{
		mat4->values[i] = 0;
	}
}

Mat4f math_Mat4Orthographic(float left, float right, float bottom, float top, float near, float far)
{
	Mat4f mat4 = math_Mat4Identity();

	mat4.values[0] = 2.0f / (right - left);
	mat4.values[5] = 2.0f / (top - bottom);
	mat4.values[10] = -2.0f / (far - near);

	mat4.values[3] = -(right + left) / (right - left);
	mat4.values[7] = -(top + bottom) / (top - bottom);
	mat4.values[11] = -(far + near) / (far - near);
	mat4.values[15] = 1.0f;

	return mat4;
}

Mat4f math_Mat4ModelMatrix(Transform* transform)
{
	Mat4f model = math_Mat4Identity();

	//printf("\nTranslated matrix:\n");
	math_Mat4Translate(&model, transform->position);
	math_Mat4Scale(&model, transform->scale);
	math_Mat4RotateZ(&model, transform->rotation);
	//math_Mat4PrettyPrint(&model);
/*
	printf("\nRotated matrix:\n");
	math_Mat4PrettyPrint(&model);*/

	//printf("\nScaled matrix:\n");
	//math_Mat4PrettyPrint(&model);


	return model;
}

void math_Mat4Mul(Mat4f* left, Mat4f* right, Mat4f* destination)
{
	Mat4f temp;
	math_Mat4SetZero(&temp);

	for (U32 x = 0; x < 4; x++)
	{
		for (U32 y = 0; y < 4; y++)
		{
			float result = 0.0f;
			for (U32 k = 0; k < 4; k++)
			{
				U32 indexLeft = k + y * 4;
				U32 indexRight = x + k * 4;

				temp.values[x + y * 4] += left->values[indexLeft] * right->values[indexRight];
			}

		}
	}
	math_Mat4Copy(&temp, destination);
}


void math_Mat4Copy(Mat4f* source, Mat4f* destination)
{
	for (U32 i = 0; i < 16; i++)
	{
		destination->values[i] = source->values[i];
	}
}

void math_Mat4Translate(Mat4f* mat4, Vec2f vec2)
{
	/*
	R[3][0] = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0]
	R[3][1] = m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1]
	R[3][2] = m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2]
	R[3][3] = m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3]
	*/
	Mat4f temp;
	math_Mat4SetIdentity(&temp);

	temp.values[3] = vec2.x;
	temp.values[7] = vec2.y;

	math_Mat4Mul(mat4, &temp, mat4);
}

void math_Mat4RotateZ(Mat4f* mat4, float angle)
{
	float radianAngle = math_ToRadians(angle);

	Mat4f temp;
	math_Mat4SetIdentity(&temp);

	temp.values[0] = cos(radianAngle);
	temp.values[1] = -sin(radianAngle);
	temp.values[4] = sin(radianAngle);
	temp.values[5] = cos(radianAngle);

	math_Mat4Mul(mat4, &temp, mat4);
}

void math_Mat4Scale(Mat4f* mat4, Vec2f vec2)
{
	Mat4f temp;
	math_Mat4SetIdentity(&temp);

	temp.values[0] = vec2.x;
	temp.values[5] = vec2.y;

	math_Mat4Mul(mat4, &temp, mat4);
}

void math_Mat4PrettyPrint(Mat4f* mat4)
{
	printf("%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n",
		mat4->values[0], mat4->values[1], mat4->values[2], mat4->values[3],
		mat4->values[4], mat4->values[5], mat4->values[6], mat4->values[7],
		mat4->values[8], mat4->values[9], mat4->values[10], mat4->values[11],
		mat4->values[12], mat4->values[13], mat4->values[14], mat4->values[15]
	);
}
