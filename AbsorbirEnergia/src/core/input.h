#ifndef INPUT_H
#define INPUT_H

#include "definitions.h"
#include <GLFW/glfw3.h>

static u32 KEY_INPUTS[1024];
static u32 KEY_JUST_PRESSED[1024];

void input_Initialize();

u32 input_IsKeyPressed(u32 key);
u32 input_IsKeyJustPressed(u32 key);
void input_ClearJustPressed();

void _input_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

#endif // !INPUT_H
