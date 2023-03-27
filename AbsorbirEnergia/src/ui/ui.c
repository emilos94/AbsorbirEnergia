#include "ui.h"

static b8 initialized = FALSE;
static MemoryArena* arena_ui = 0;
static UI_Render_State* render_state = 0;
static u32 arena_start_offset = 0;
static ShaderProgram shader_program_ui;
static VertexArrayObject vao_quad;

void ui_initialize(Mat4f* projection_matrix)
{
	if (initialized) return;

	initialized = TRUE;
	arena_ui = memory_MemoryArenaCreate(memory_Megabytes(1));
	shader_program_ui = graphics_ShaderLoad(arena_ui, "res/shaders/ui.vert", "res/shaders/ui.frag");
	graphics_ShaderBind(shader_program_ui);
	graphics_ShaderSetUniformMat4(shader_program_ui, "projection_matrix", projection_matrix);
	graphics_ShaderUnbind();
	memory_MemoryArenaReset(arena_ui);

	render_state = memory_struct_zero_allocate(arena_ui, UI_Render_State);
	render_state->widget_first = 0;
	arena_start_offset = arena_ui->offset;
	vao_quad = graphics_vao_quad_create();
}

UI_Info* ui_button(f32 x, f32 y, f32 width, f32 height)
{
	UI_Widget* widget = memory_struct_zero_allocate(arena_ui, UI_Widget);
	widget->flags = UI_WidgetFlag_Clickable | UI_WidgetFlag_DrawBackground;
	widget->position.x = x;
	widget->position.y = y;
	widget->size.x = width;
	widget->size.y = height;
	math_vec3f_set_vec3f(&render_state->theme.color_main, &widget->color);

	UI_Info* info = memory_struct_zero_allocate(arena_ui, UI_Info);
	info->widget = widget;
	Vec2f mouse_position = input_mouse_render_coords();

	b8 x_hover = mouse_position.x >= x && mouse_position.x <= x + width;
	b8 y_hover = mouse_position.y >= y && mouse_position.y <= y + height;
	if (x_hover && y_hover)
	{
		info->hot = TRUE;
		// todo: perhaps be smart about setting active on release if user clicked on this as well
		info->active = input_mouse_left_down();
	}
	else
	{
		info->hot = FALSE;
		info->active = FALSE;
	}

	if (render_state->widget_first)
	{
		render_state->widget_last->next = widget;
		widget->prev = render_state->widget_last;
		render_state->widget_last = widget;
	}
	else 
	{
		render_state->widget_first = widget;
		render_state->widget_last = widget;
	}
	return info;
}

void ui_render_flush()
{
	UI_Widget* widget = render_state->widget_first;

	graphics_ShaderBind(shader_program_ui);
	Mat4f model;
	while (widget)
	{
		math_mat4_model(widget->position, widget->size, 0.0f, &model);
		graphics_ShaderSetUniformMat4(shader_program_ui, "model", &model);
		graphics_shader_uniform_vec3f(shader_program_ui, "color", &widget->color);
		graphics_VaoRender(&vao_quad);

		widget = widget->next;
	}
	graphics_ShaderUnbind();

	render_state->widget_first = 0;
	render_state->widget_last = 0;
	arena_ui->offset = arena_start_offset;
}

void ui_render_destroy()
{
	graphics_ShaderDestroy(shader_program_ui);
	graphics_VaoDestroy(&vao_quad);
	memory_MemoryArenaFree(arena_ui);
	initialized = FALSE;
}