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
	u32 verticesCount;
};
typedef struct VertexArrayObject VertexArrayObject;

// Vertex array object
VertexArrayObject graphics_VaoCreate();
VertexArrayObject graphics_vao_quad_create();
void graphics_VaoAddFloatBuffer(VertexArrayObject* vao, u32 index, u32 elementsPerEntry, float* values, u32 valueCount, u32 isPositions);
void graphics_VaoRender(VertexArrayObject* vao);
void graphics_VaoDestroy(VertexArrayObject* vao);

// rendering
void graphics_RendererInit(MemoryArena* arena);
void graphics_entity_render(ShaderProgram shader, Entity* entity);
void graphics_render_quad_color(ShaderProgram shader, Vec2f bottom_left, Vec2f top_right,
	float r, float g, float b);
void graphics_RendererCleanup();

#endif

