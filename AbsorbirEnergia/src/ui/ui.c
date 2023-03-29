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

u32 ui_font_parse_property_u32(FileResult* file_result, u32 file_offset, char* property_name, u32 property_name_length, u32* destination)
{
	u32 new_offset = mystr_char_array_find_indexof(file_result->text + file_offset, file_result->length - file_offset, property_name, property_name_length);
	new_offset += property_name_length;
	ASSERT(mystr_u32_parse(file_result->text + new_offset, destination));
	return new_offset;
}

u32 ui_font_parse_property_u32(FileResult* file_result, u32 file_offset, char* property_name, u32 property_name_length, s32* destination)
{
	u32 new_offset = mystr_char_array_find_indexof(file_result->text + file_offset, file_result->length - file_offset, property_name, property_name_length);
	new_offset += property_name_length;
	ASSERT(mystr_s32_parse(file_result->text + new_offset, destination));
	return new_offset;
}

UI_Font* ui_text_font_load(MemoryArena* arena_temp, MemoryArena* arena_permanent, char* path_specification, char* path_texture)
{
	UI_Font* font = memory_struct_zero_allocate(arena_permanent, UI_Font);

	FileResult* result = file_ReadFileToCharArray(arena_temp, path_specification);
	u32 text_index = 0;

	text_index = ui_font_parse_property_u32(result, text_index, "padding=", 8, &font->padding_left);
	text_index = ui_font_parse_property_u32(result, text_index, ",", 1, &font->padding_top);
	text_index = ui_font_parse_property_u32(result, text_index, ",", 1, &font->padding_right);
	text_index = ui_font_parse_property_u32(result, text_index, ",", 1, &font->padding_bottom);
	text_index = ui_font_parse_property_u32(result, text_index, "lineHeight=", 11, &font->line_height);
	text_index = ui_font_parse_property_u32(result, text_index, "lineHeight=", 11, &font->line_height);
	text_index = ui_font_parse_property_u32(result, text_index, "scaleW=", 7, &font->image_width);
	text_index = ui_font_parse_property_u32(result, text_index, "scaleH=", 7, &font->image_height);
	text_index = ui_font_parse_property_u32(result, text_index, "chars count=", 12, &font->character_count);

	font->character_infos = memory_AllocateArray(arena_permanent, UI_Characterinfo, font->character_count);

	for (u32 i = 0; i < font->character_count; i++)
	{
		// parse character line
		u32 character_id = 0;
		text_index = ui_font_parse_property_u32(result, text_index, "char id=", 8, &character_id);

		UI_Characterinfo* info = font->character_infos + (i % font->character_count);
		info->c = (char)character_id;

		text_index = ui_font_parse_property_u32(result, text_index, "x=", 2, &info->x);
		text_index = ui_font_parse_property_u32(result, text_index, "y=", 2, &info->y);
		text_index = ui_font_parse_property_u32(result, text_index, "width=", 6, &info->width);
		text_index = ui_font_parse_property_u32(result, text_index, "height=", 7, &info->height);

		text_index = ui_font_parse_property_s32(result, text_index, "xoffset=", 8, &info->x_offset);
		text_index = ui_font_parse_property_s32(result, text_index, "xoffset=", 8, &info->y_offset);

		text_index = ui_font_parse_property_u32(result, text_index, "xadvance=", 9, &info->x_advance);

		info->uv_x_min = (f32)info->x / (f32)font->image_width;
		info->uv_y_min = (f32)info->y / (f32)font->image_height;
		info->uv_x_max = (f32)(info->x + info->width) / (f32)font->image_width;
		info->uv_x_max = (f32)(info->y + info->height) / (f32)font->image_height;
	}

	return font;
}
