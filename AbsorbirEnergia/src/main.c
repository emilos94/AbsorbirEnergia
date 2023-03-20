#include <stdio.h>

#include "graphics/shader_program.h"
#include "graphics/texture.h"
#include "graphics/renderer.h"
#include "graphics/window.h"
#include "core/input.h"
#include "core/assets.h"
#include "game.h"

#define DEBUG_RENDER_COLLISION_QUADS 1

int main(void) 
{	
	Window window = graphics_WindowCreate("Absorbir energia", 1280, 720);

	// configurations
	stbi_set_flip_vertically_on_load(TRUE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);

	MemoryArena* arena = memory_MemoryArenaCreate(memory_Megabytes(1));

	ShaderProgram shaderProgram = graphics_ShaderLoad(arena, "res/shaders/default.vert", "res/shaders/default.frag");
	ShaderProgram shader_quad_color = graphics_ShaderLoad(arena, "res/shaders/color.vert", "res/shaders/color.frag");
	memory_MemoryArenaReset(arena);
	graphics_RendererInit(arena);

	Assets* assets = assets_load(arena);
	GameState* gameState = game_Init(arena, assets);

	Mat4f projectionMatrix = math_Mat4Orthographic(0.0f, 320.0f, 0.0f, 180.0f, -1.0f, 1.0f);
	graphics_ShaderBind(shaderProgram);
	graphics_ShaderSetUniformMat4(shaderProgram, "projectionMatrix", &projectionMatrix);
	graphics_ShaderBind(shader_quad_color);
	graphics_ShaderSetUniformMat4(shader_quad_color, "projectionMatrix", &projectionMatrix);
	graphics_ShaderUnbind();

	float secondsPerUpdate = 1.0f / 60.0f;
	float previous = glfwGetTime();
	float lag = 0.0f;
	
	u32 updateCounter = 0;
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

		gameState->secondsSinceStart += elapsed;
		
		if (lag >= secondsPerUpdate)
		{
			lag -= secondsPerUpdate;
			updateCounter++;

			game_Input(gameState, arena, assets);
			game_Update(gameState, arena, assets, secondsPerUpdate);

			graphics_WindowClear();
			game_render(gameState, shaderProgram, shader_quad_color);

			input_ClearJustPressed();
			graphics_SwapBuffersAndPollEvents(&window);
		}
	}

	graphics_ShaderDestroy(shaderProgram); 
	graphics_RendererCleanup();
	graphics_WindowTerminate();
	assets_cleanup(assets);
	memory_MemoryArenaFree(arena);

	return 0;
}