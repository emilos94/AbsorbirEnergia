#include <stdio.h>

#include "graphics/shader_program.h"
#include "graphics/input.h"
#include "graphics/texture.h"
#include "graphics/renderer.h"
#include "graphics/window.h"

int main(void) 
{	
	Window window = graphics_WindowCreate("Hello window", 1280, 720);

	MemoryArena* arena = memory_MemoryArenaCreate(memory_Megabytes(1));

	ShaderProgram shaderProgram = graphics_ShaderLoad(arena, "res/shaders/default.vert", "res/shaders/default.frag");
	memory_MemoryArenaReset(arena);
	Texture texture = graphics_TextureLoad(arena, "res/images/container.jpg");
	memory_MemoryArenaReset(arena);

	graphics_RendererInit(arena);

	Entity entity;
	entity.texture = graphics_TextureLoad(arena, "res/images/container.jpg");
	entity.transform.position.x = 32.0f;
	entity.transform.position.y = 0.0f;
	entity.transform.scale.x = 32.0f;
	entity.transform.scale.y = 32.0f;
	entity.transform.rotation = 0.0f;

	Mat4f projectionMatrix = math_Mat4Orthographic(0.0f, 320.0f, 0.0f, 180.0f, -1.0f, 1.0f);
	graphics_ShaderBind(shaderProgram);
	graphics_ShaderSetUniformMat4(arena, shaderProgram, "projectionMatrix", &projectionMatrix);
	graphics_ShaderUnbind(shaderProgram);

	float secondsPerUpdate = 1.0f / 60.0f;
	float previous = glfwGetTime();
	float lag = 0.0f;
	
	U32 updateCounter = 0;
	float secondCounter = 0.0f;

	while (!graphics_WindowShouldClose(&window) && !input_IsKeyPressed(GLFW_KEY_ESCAPE))
	{
		float current = glfwGetTime();
		float elapsed = current - previous;
		previous = current;

		lag += elapsed;
		secondCounter += elapsed;

		if (secondCounter >= 1.0f)
		{
			printf("UPS: %d\n", updateCounter);
			secondCounter = 0.0f;
			updateCounter = 0;
		}

		while (lag >= secondsPerUpdate)
		{
			lag -= secondsPerUpdate;
			updateCounter++;

			graphics_WindowClear();
			graphics_RenderEntity(arena, shaderProgram, &entity);

			input_ClearJustPressed();
			graphics_SwapBuffersAndPollEvents(&window);
		}
	}

	graphics_WindowTerminate();
	graphics_ShaderDestroy(shaderProgram); 
	graphics_RendererCleanup();
	graphics_TextureDestroy(&texture);
	memory_MemoryArenaFree(arena);

	return 0;
}