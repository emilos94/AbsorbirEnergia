#include "renderer.h"

static VertexArrayObject quadVao;

void graphics_RendererInit(MemoryArena* arena)
{
	quadVao = graphics_vao_create();
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
	graphics_vao_floatbuffer_add(&quadVao, 0, 2, vertices, 12, VAO_Options_IsPositions);

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
	graphics_vao_floatbuffer_add(&quadVao, 1, 2, uvCoordinates, 12, VAO_Options_Empty);
}

VertexArrayObject graphics_vao_create()
{
	VertexArrayObject vao;
	GLCall(glGenVertexArrays(1, &vao.vertexArrayId));
	vao.has_indices = FALSE;
	return vao;
}


VertexArrayObject graphics_vao_quad_create()
{
	VertexArrayObject vao = graphics_vao_create();
	float vertices[12];
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
	graphics_vao_floatbuffer_add(&vao, 0, 2, &vertices[0], 12, VAO_Options_IsPositions);
	return vao;
}

void graphics_vao_floatbuffer_add(VertexArrayObject* vao, u32 index, u32 elementsPerEntry, float* values, u32 valueCount, VAO_Options options)
{
	GLClearErrors();

	glBindVertexArray(vao->vertexArrayId);
	glGenBuffers(1, &vao->vertexBufferId[index]);
	glBindBuffer(GL_ARRAY_BUFFER, vao->vertexBufferId[index]);

	if (options & VAO_Options_IsDynamic)
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * valueCount, values, GL_DYNAMIC_DRAW);
	}
	else
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * valueCount, values, GL_STATIC_DRAW);
	}

	if (options & VAO_Options_IsPositions)
	{
		vao->verticesCount = valueCount / elementsPerEntry;
	}

	glVertexAttribPointer(index, elementsPerEntry, GL_FLOAT, GL_FALSE, elementsPerEntry * sizeof(float), (void*)0);
	glEnableVertexAttribArray(index);

	GLAssertNoErrors("Vao add float buffer");
}

void graphics_vao_elementindices_add(VertexArrayObject* vao, u32* values, u32 indices_count, VAO_Options options)
{
	GLClearErrors();

	glBindVertexArray(vao->vertexArrayId);
	glGenBuffers(1, &vao->element_buffer_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->element_buffer_id);

	if (options & VAO_Options_IsDynamic)
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indices_count, values, GL_DYNAMIC_DRAW);
	}
	else
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indices_count, values, GL_STATIC_DRAW);
	}

	vao->verticesCount = indices_count;

	GLAssertNoErrors("Vao add element indicies buffer");
}

void graphics_vao_buffer_subdata_floats(VertexArrayObject* vao, u32 index, f32* values, u32 count)
{
	GLClearErrors();

	glBindVertexArray(vao->vertexArrayId);
	glBindBuffer(GL_ARRAY_BUFFER, vao->vertexBufferId[index]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(f32), values);

	GLAssertNoErrors("Vao buffer sub data floats");
}

void graphics_VaoRender(VertexArrayObject* vao)
{
	GLCall(glBindVertexArray(vao->vertexArrayId));
	if (vao->has_indices)
	{
		GLCall(glDrawElements(GL_TRIANGLES, vao->verticesCount, GL_UNSIGNED_INT, 0));
	}
	else
	{
		GLCall(glDrawArrays(GL_TRIANGLES, 0, vao->verticesCount));
	}
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

	if (entity->entityFlags & EntityFlag_HasAnimations) {
		Animation* animation = entity->animations[entity->currentAnimation];
		graphics_TextureBind(&entity->spriteSheet.texture);
		graphics_ShaderSetUniformF(shader, "spriteCount", (float)entity->spriteSheet.spriteCount);
		graphics_ShaderSetUniformF(shader, "spriteIndex", (float)animation->spriteIndex);
	}
	else {
		graphics_TextureBind(&entity->texture);
		graphics_ShaderSetUniformF(shader, "spriteCount", 1.0f);
		graphics_ShaderSetUniformF(shader, "spriteIndex", 0.0f);
	}

	if (entity->tint_active) {
		graphics_ShaderSetUniformF(shader, "tintStrength", entity->tint_strength);
		Vec3f tint_color = entity->tint_color;
		graphics_shader_uniform_3f(shader, "tintColor", tint_color.x, tint_color.y, tint_color.z);
	}
	else {
		graphics_ShaderSetUniformF(shader, "tintStrength", 0.0f);
	}

	graphics_VaoRender(&quadVao);
}

void graphics_RendererCleanup()
{
	graphics_VaoDestroy(&quadVao);
}