#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "../math/matrix4f.h"
#include "../core/file_loader.h"
#include "opengl_utils.h"

typedef GLuint ShaderProgram;

ShaderProgram graphics_ShaderLoad(MemoryArena* arena, char* vertexSource, char* fragmentSource);
void graphics_ShaderBind(ShaderProgram shaderProgram);
void graphics_ShaderUnbind();
void graphics_ShaderDestroy(ShaderProgram shaderProgram);

void graphics_ShaderSetUniformMat4(MemoryArena* arena, ShaderProgram shaderProgram, char* name, Mat4f* mat4);
U32 _shader_LoadShaderSource(MemoryArena* arena, char* path, U32 shaderType);

#endif // !SHADER_PROGRAM_H
