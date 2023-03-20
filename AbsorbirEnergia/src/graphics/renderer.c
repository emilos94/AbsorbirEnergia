#include "renderer.h"

static VertexArrayObject quadVao;

void graphics_RendererInit(MemoryArena* arena)
{
	quadVao = graphics_VaoCreate();
	float* vertices = memory_AllocateArray(arena, float, 12);
	vertices[0] = 0.0f;
	vertices[1] = 0.0f;
	vertices[2] = 1.0f;
	vertices[3] = 0.0f;
	vertices[4] = 1.0f;
	vertices[5] = 1.0f;
	vertices[6] = 1.0f;
	vertices[7] = 1.0f;
	vertices[8] = 0.0f;
	vertices[9] = 1.0f;
	vertices[10] = 0.0f;
	vertices[11] = 0.0f;
	graphics_VaoAddFloatBuffer(&quadVao, 0, 2, vertices, 12, TRUE);

	float* uvCoordinates = memory_AllocateArray(arena, float, 12);
	uvCoordinates[0] = 0.0f;
	uvCoordinates[1] = 0.0f;
	uvCoordinates[2] = 1.0f;
	uvCoordinates[3] = 0.0f;
	uvCoordinates[4] = 1.0f;
	uvCoordinates[5] = 1.0f;
	uvCoordinates[6] = 1.0f;
	uvCoordinates[7] = 1.0f;
	uvCoordinates[8] = 0.0f;
	uvCoordinates[9] = 1.0f;
	uvCoordinates[10] = 0.0f;
	uvCoordinates[11] = 0.0f;
	graphics_VaoAddFloatBuffer(&quadVao, 1, 2, uvCoordinates, 12, FALSE);
}

VertexArrayObject graphics_VaoCreate()
{
	VertexArrayObject vao;
	GLCall(glGenVertexArrays(1, &vao.vertexArrayId));
	return vao;
}

void graphics_VaoAddFloatBuffer(VertexArrayObject* vao, u32 index, u32 elementsPerEntry, float* values, u32 valueCount, u32 isPositions)
{
	GLClearErrors();

	glBindVertexArray(vao->vertexArrayId);
	glGenBuffers(1, &vao->vertexBufferId[index]);
	glBindBuffer(GL_ARRAY_BUFFER, vao->vertexBufferId[index]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * valueCount, values, GL_STATIC_DRAW);

	if (isPositions)
	{
		vao->verticesCount = valueCount / elementsPerEntry;
	}

	glVertexAttribPointer(index, elementsPerEntry, GL_FLOAT, GL_FALSE, elementsPerEntry * sizeof(float), (void*)0);
	glEnableVertexAttribArray(index);

	GLAssertNoErrors("Vao add float buffer");
}

void graphics_VaoRender(VertexArrayObject* vao)
{
	GLCall(glBindVertexArray(vao->vertexArrayId));
	GLCall(glDrawArrays(GL_TRIANGLES, 0, vao->verticesCount));
}


void graphics_render_quad_color(ShaderProgram shader, Vec2f bottom_left, Vec2f top_right,
	float r, float g, float b)
{
	graphics_ShaderBind(shader);

	Vec2f scale = math_vec2f(top_right.x - bottom_left.x, top_right.y - bottom_left.y);
	Mat4f model = math_Mat4ModelMatrix(bottom_left, scale, 0.0f);
	graphics_ShaderSetUniformMat4(shader, "model", &model);
	graphics_shader_uniform_3f(shader, "color", r, g, b);
	graphics_VaoRender(&quadVao);

	graphics_ShaderUnbind();
}

void graphics_VaoDestroy(VertexArrayObject* vao)
{
	GLCall(glDeleteBuffers(1, &vao->vertexBufferId[0]));
	GLCall(glDeleteBuffers(1, &vao->vertexBufferId[1]));
	GLCall(glDeleteVertexArrays(1, &vao->vertexArrayId));
}

void graphics_entity_render(ShaderProgram shader, Entity* entity)
{
	if (!entity->isVisible) return;

	graphics_ShaderBind(shader);

	Mat4f model = math_Mat4ModelMatrixTransform(&entity->transform);
	graphics_ShaderSetUniformMat4(shader, "model", &model);

	if (entity->entityFlags & EntityFlag_HasAnimations)
	{
		Animation* animation = entity->animations[entity->currentAnimation];
		graphics_TextureBind(&entity->spriteSheet.texture);
		graphics_ShaderSetUniformF(shader, "spriteCount", (float)entity->spriteSheet.spriteCount);
		graphics_ShaderSetUniformF(shader, "spriteIndex", (float)animation->spriteIndex);
	}
	else
	{
		graphics_TextureBind(&entity->texture);
		graphics_ShaderSetUniformF(shader, "spriteCount", 1.0f);
		graphics_ShaderSetUniformF(shader, "spriteIndex", 0.0f);
	}

	graphics_VaoRender(&quadVao);
}

void graphics_RendererCleanup()
{
	graphics_VaoDestroy(&quadVao);
}