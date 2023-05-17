#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>

#include "../core/entity.h"
#include "../core/memory_arena.h"
#include "../ui/ui.h"
#include "shader_program.h"

struct VertexArrayObject
{
	GLuint vertexArrayId;
	GLuint vertexBufferId[2];
	GLuint element_buffer_id;
	b8 has_indices;
	u32 verticesCount;
};
typedef struct VertexArrayObject VertexArrayObject;

typedef u32 VAO_Options;
enum VAO_Options
{
	VAO_Options_Empty = 0,
	VAO_Options_IsPositions = (1 << 0),
	VAO_Options_IsDynamic = (1 << 1),
};

// Vertex array object
VertexArrayObject graphics_vao_create();
VertexArrayObject graphics_vao_quad_create();
void graphics_vao_floatbuffer_add(VertexArrayObject* vao, u32 index, u32 elementsPerEntry, float* values, u32 valueCount, VAO_Options options);
void graphics_vao_elementindices_add(VertexArrayObject* vao, u32* values, u32 indices_count, VAO_Options options);
void graphics_vao_buffer_subdata_floats(VertexArrayObject* vao, u32 index, f32* values, u32 count);
void graphics_VaoRender(VertexArrayObject* vao);
void graphics_VaoDestroy(VertexArrayObject* vao);

// rendering
void graphics_RendererInit(MemoryArena* arena);
void graphics_entity_render(ShaderProgram shader, Entity* entity);
void graphics_render_quad_color(ShaderProgram shader, Vec2f bottom_left, Vec2f top_right,
	float r, float g, float b);
void graphics_RendererCleanup();

#endif

