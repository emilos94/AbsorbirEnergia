#ifndef INPUT_H
#define INPUT_H

#include "definitions.h"
#include "../graphics/window.h"
#include "../math/vec2f.h"
#include <GLFW/glfw3.h>

static u32 KEY_INPUTS[1024];
static u32 KEY_JUST_PRESSED[1024];

void input_Initialize();

u32 input_IsKeyPressed(u32 key);
u32 input_IsKeyJustPressed(u32 key);
Vec2f input_mouse_render_coords();
b8 input_mouse_left_down();
b8 input_mouse_right_down();

void _input_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void _input_mouse_callback(GLFWwindow* window, f64 mouse_x, f64 mouse_y);
void _input_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

#endif // !INPUT_H
