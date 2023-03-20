#include "input.h"

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

void input_ClearJustPressed()
{
	for (int i = 0; i < 1024; i++)
	{
		KEY_JUST_PRESSED[i] = FALSE;
	}
}
