#ifndef OPENGL_UTILS_H
#define OPENGL_UTILS_H

#include <glad/glad.h>
#include <stdio.h>
#include "../core/definitions.h"

#define GLAssertNoErrors(blockName) ASSERT(GLLogCall(blockName, __FILE__, __LINE__))

#define GLCall(x) GLClearErrors();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__));

static void GLClearErrors()
{
	while (glGetError() != GL_NO_ERROR);
}

static U32 GLLogCall(const char* functionName, const char* fileName, int line)
{
	U32 noErrors = TRUE;
	GLenum error = glGetError();
	while (error)
	{
		printf("[OpenGL ERROR]: (%d) %s %s %d\n", error, functionName, fileName, line);
		noErrors = FALSE;
		GLenum error = glGetError();
	}

	return noErrors;
}



#endif // !OPENGL_UTILS_H
