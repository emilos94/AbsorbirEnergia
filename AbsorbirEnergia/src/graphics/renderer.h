#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>

#include "../core/entity.h"
#include "../core/memory_arena.h"
#include "shader_program.h"

struct VertexArrayObject
{
	GLuint vertexArrayId;
	GLuint vertexBufferId[2];
	U32 verticesCount;
};
typedef struct VertexArrayObject VertexArrayObject;

// Vertex array object
VertexArrayObject graphics_VaoCreate();
void graphics_VaoAddFloatBuffer(VertexArrayObject* vao, U32 index, U32 elementsPerEntry, float* values, U32 valueCount, U32 isPositions);
void graphics_VaoRender(VertexArrayObject* vao);
void graphics_VaoDestroy(VertexArrayObject* vao);


// rendering
void graphics_RendererInit(MemoryArena* arena);
void graphics_RenderEntity(MemoryArena* arena, ShaderProgram shader, Entity* entity);
void graphics_RendererCleanup();

#endif

