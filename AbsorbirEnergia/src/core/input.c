#include "input.h"

static Vec2f mouse_position;
static b8 mouse_left_down = FALSE;
static b8 mouse_right_down = FALSE;

void input_Initialize()
{
	for (int i = 0; i < 1024; i++)
	{
		KEY_INPUTS[i] = FALSE;
	}
}

u32 input_IsKeyPressed(u32 key)
{
	return KEY_INPUTS[key];
}

u32 input_IsKeyJustPressed(u32 key)
{
	return KEY_JUST_PRESSED[key];
}

void _input_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (!KEY_INPUTS[key])
		{
			KEY_JUST_PRESSED[key] = TRUE;
		}

		KEY_INPUTS[key] = TRUE;
	}
	else if (action == GLFW_RELEASE)
	{
		KEY_INPUTS[key] = FALSE;
	}
}

void _input_mouse_callback(GLFWwindow* window, f64 mouse_x, f64 mouse_y)
{
	mouse_position.x = mouse_x;
	mouse_position.y = mouse_y;
}

void _input_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		mouse_left_down = action == GLFW_PRESS;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		mouse_right_down = action && GLFW_PRESS;
	}
}

void input_ClearJustPressed()
{
	for (int i = 0; i < 1024; i++)
	{
		KEY_JUST_PRESSED[i] = FALSE;
	}
}

Vec2f input_mouse_render_coords()
{
	Vec2f mouse_render_coords;
	mouse_render_coords.x = mouse_position.x * graphics_window_render_scale_x();
	mouse_render_coords.y = graphics_window_render_height() - mouse_position.y * graphics_window_render_scale_y();
	return mouse_render_coords;
}

b8 input_mouse_left_down()
{
	return mouse_left_down;
}

b8 input_mouse_right_down()
{
	return mouse_right_down;
}