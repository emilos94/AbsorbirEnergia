#ifndef WINDOW_H
#define WINDOW_H

#include "opengl_utils.h"
#include <GLFW/glfw3.h>
#include "../core/definitions.h"
#include "../core/input.h"

struct Window {

	GLFWwindow* handle;
	size_t width, height;
	char* title;
};
typedef struct Window Window;

Window graphics_WindowCreate(char* title, size_t width, size_t height);
U32 graphics_WindowShouldClose(Window* window);
void graphics_WindowClear(); 
void graphics_SwapBuffersAndPollEvents(Window* window);
void graphics_WindowTerminate();

void _graphics_WindowFramebufferSizeCallback(GLFWwindow* window, int width, int height);

#endif // !WINDOW_H
