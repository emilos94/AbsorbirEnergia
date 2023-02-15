#include "window.h"

Window graphics_WindowCreate(char* title, size_t width, size_t height) 
{
	glfwInit();    
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // mac setting

	GLFWwindow* windowHandle = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!windowHandle)
	{
		glfwTerminate();
		ASSERT(FALSE);
	}
	glfwMakeContextCurrent(windowHandle);

	ASSERT(gladLoadGL());
	
	glViewport(0, 0, width, height);

	glfwSetFramebufferSizeCallback(windowHandle, _graphics_WindowFramebufferSizeCallback);
	glfwSetKeyCallback(windowHandle, _input_KeyCallback);

	Window w;
	w.handle = windowHandle;
	w.width = width;
	w.height = height;
	w.title = title;
	
	return w;
}

U32 graphics_WindowShouldClose(Window* window)
{
	if (glfwWindowShouldClose(window->handle)) return TRUE;

	return FALSE;
}


void graphics_WindowClear()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void graphics_SwapBuffersAndPollEvents(Window* window)
{
	glfwSwapBuffers(window->handle);
	glfwPollEvents();
}

void graphics_WindowTerminate()
{
	glfwTerminate();
}

void _graphics_WindowFramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
