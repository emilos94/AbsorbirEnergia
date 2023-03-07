#ifndef INPUT_H
#define INPUT_H

#include "definitions.h"
#include <GLFW/glfw3.h>

static U32 KEY_INPUTS[1024];
static U32 KEY_JUST_PRESSED[1024];

void input_Initialize();

U32 input_IsKeyPressed(U32 key);
U32 input_IsKeyJustPressed(U32 key);
void input_ClearJustPressed();

void _input_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

#endif // !INPUT_H
