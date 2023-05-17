#include "window.h"

static f32 WINDOW_SIZE_X = 0.0f;
static f32 WINDOW_SIZE_Y = 0.0f;
static f32 WINDOW_SIZE_RENDER_SCALE_X = 0.0f;
static f32 WINDOW_SIZE_RENDER_SCALE_Y = 0.0f;
static f32 WINDOW_RENDER_WIDTH = 320.0f;
static f32 WINDOW_RENDER_HEIGHT = 180.0f;

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

	WINDOW_SIZE_X = width;
	WINDOW_SIZE_Y = height;
	f32 render_width = WINDOW_RENDER_WIDTH;
	f32 render_height = WINDOW_RENDER_HEIGHT;

	WINDOW_SIZE_RENDER_SCALE_X = render_width / WINDOW_SIZE_X;
	WINDOW_SIZE_RENDER_SCALE_Y = render_height / WINDOW_SIZE_Y;

	glfwMakeContextCurrent(windowHandle);

	ASSERT(gladLoadGL());
	
	glViewport(0, 0, width, height);

	glfwSetFramebufferSizeCallback(windowHandle, _graphics_WindowFramebufferSizeCallback);
	glfwSetKeyCallback(windowHandle, _input_KeyCallback);
	glfwSetCursorPosCallback(windowHandle, _input_mouse_callback);
	glfwSetMouseButtonCallback(windowHandle, _input_mouse_button_callback);

	Window w;
	w.handle = windowHandle;
	w.width = width;
	w.height = height;
	w.title = title;
	
	return w;
}

u32 graphics_WindowShouldClose(Window* window)
{
	if (glfwWindowShouldClose(window->handle)) return TRUE;

	return FALSE;
}


void graphics_WindowClear()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); ;
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
	WINDOW_SIZE_X = width;
	WINDOW_SIZE_Y = height;

	f32 render_width = WINDOW_RENDER_WIDTH;
	f32 render_height = WINDOW_RENDER_HEIGHT;

	WINDOW_SIZE_RENDER_SCALE_X = render_width / WINDOW_SIZE_X;
	WINDOW_SIZE_RENDER_SCALE_Y = render_height / WINDOW_SIZE_Y;
}


f32 graphics_window_width()
{
	return WINDOW_SIZE_X;
}

f32 graphics_window_height()
{
	return WINDOW_SIZE_Y;
}

f32 graphics_window_render_scale_x()
{
	return WINDOW_SIZE_RENDER_SCALE_X;
}

f32 graphics_window_render_scale_y()
{
	return WINDOW_SIZE_RENDER_SCALE_Y;
}

f32 graphics_window_render_width()
{
	return WINDOW_RENDER_WIDTH;
}

f32 graphics_window_render_height()
{
	return WINDOW_RENDER_HEIGHT;
}
