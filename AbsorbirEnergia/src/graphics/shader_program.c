#include "shader_program.h"

ShaderProgram graphics_ShaderLoad(MemoryArena* arena, char* vertexPath, char* fragmentPath)
{
	U32 vertexShaderId = _shader_LoadShaderSource(arena, vertexPath, GL_VERTEX_SHADER);
	U32 fragmentShaderId = _shader_LoadShaderSource(arena, fragmentPath, GL_FRAGMENT_SHADER);

	GLuint shaderProgramId = glCreateProgram();
	GLCall(glAttachShader(shaderProgramId, vertexShaderId));
	GLCall(glAttachShader(shaderProgramId, fragmentShaderId));
	GLCall(glLinkProgram(shaderProgramId));

	int success;
	char infoLog[512];
	GLCall(glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &success));
	if (!success) 
	{
		GLCall(glGetProgramInfoLog(shaderProgramId, 512, NULL, infoLog));
		printf("ERROR: Program linking failed\n%s\n", infoLog);
		ASSERT(FALSE);
	}

	GLCall(glUseProgram(shaderProgramId));

	GLCall(glDeleteShader(vertexShaderId));
	GLCall(glDeleteShader(fragmentShaderId));

	return shaderProgramId;
}

void graphics_ShaderBind(ShaderProgram shaderProgram)
{
	GLCall(glUseProgram(shaderProgram));
}

void graphics_ShaderUnbind()
{
	GLCall(glUseProgram(0));
}

U32 _shader_LoadShaderSource(MemoryArena* arena, char* path, U32 shaderType)
{
	FileResult* fileResult = file_ReadFileToCharArray(arena, path);

	U32 shaderId = glCreateShader(shaderType);
	GLCall(glShaderSource(shaderId, 1, &fileResult->fileContents, NULL));
	GLCall(glCompileShader(shaderId));

	int success;
	char infoLog[512];
	GLCall(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success));
	
	if (!success)
	{
		GLCall(glGetShaderInfoLog(shaderId, 512, NULL, infoLog));
		printf("ERROR: Shader compilation failed\n%s\n", infoLog);
		ASSERT(FALSE);
	}

	return shaderId;
}

void graphics_ShaderDestroy(ShaderProgram shaderProgram)
{
	GLCall(glDeleteProgram(shaderProgram));
}


void graphics_ShaderSetUniformF(ShaderProgram shaderProgram, char* name, float value)
{
	U32 location = glGetUniformLocation(shaderProgram, name);
	GLCall(glUniform1f(location, value));
}

void graphics_ShaderSetUniformMat4(ShaderProgram shaderProgram, char* name, Mat4f* mat4)
{
	U32 location = glGetUniformLocation(shaderProgram, name);
	GLCall(glUniformMatrix4fv(location, 1, GL_TRUE, &mat4->values[0]));
}
