#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "../math/matrix4f.h"
#include "../math/vec3f.h"
#include "../core/file_loader.h"
#include "opengl_utils.h"

typedef GLuint ShaderProgram;

ShaderProgram graphics_ShaderLoad(MemoryArena* arena, char* vertexPath, char* fragmentPath);
void graphics_ShaderBind(ShaderProgram shaderProgram);
void graphics_ShaderUnbind();
void graphics_ShaderDestroy(ShaderProgram shaderProgram);

void graphics_ShaderSetUniformF(ShaderProgram shaderProgram, char* name, float value);
void graphics_shader_uniform_vec3f(ShaderProgram shaderProgram, char* name, Vec3f* vec);
void graphics_shader_uniform_3f(ShaderProgram shaderProgram, char* name, float x, float y, float z);
void graphics_ShaderSetUniformMat4(ShaderProgram shaderProgram, char* name, Mat4f* mat4);
u32 _shader_LoadShaderSource(MemoryArena* arena, char* path, u32 shaderType);

#endif // !SHADER_PROGRAM_H
