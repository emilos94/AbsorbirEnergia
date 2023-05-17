#include "ui.h"

static b8 initialized = FALSE;
static MemoryArena* arena_ui = 0;
static UI_Render_State* render_state = 0;
static u32 arena_start_offset = 0;
static ShaderProgram shader_program_ui;
static VertexArrayObject vao_quad;


// Text render state
static ShaderProgram shader_program_text;
static f32* buffer_text_positions;
static f32* buffer_text_uvs;
static u32* buffer_text_element_indicies;
static u32 buffer_size;
static u32 buffers_current_quad_count;
VertexArrayObject vao_text;

static UI_Text_State ui_text_state;

// forward declarations
UI_Info* ui_info_from_widget(UI_Widget* widget);

void ui_initialize(Mat4f* projection_matrix, UI_Font* font_default)
{
	if (initialized) return;

	initialized = TRUE;
	arena_ui = memory_MemoryArenaCreate(memory_Megabytes(1));
	shader_program_ui = graphics_ShaderLoad(arena_ui, "res/shaders/ui.vert", "res/shaders/ui.frag");
	graphics_ShaderBind(shader_program_ui);
	graphics_ShaderSetUniformMat4(shader_program_ui, "projection_matrix", projection_matrix);
	graphics_ShaderUnbind();

	shader_program_text = graphics_ShaderLoad(arena_ui, "res/shaders/text.vert", "res/shaders/text.frag");
	graphics_ShaderBind(shader_program_text);
	graphics_ShaderSetUniformMat4(shader_program_text, "projection_matrix", projection_matrix);
	graphics_ShaderUnbind();

	memory_MemoryArenaReset(arena_ui);
	ui_text_state.font_active = font_default;

	render_state = memory_struct_zero_allocate(arena_ui, UI_Render_State);
	render_state->widget_first = 0;
	render_state->theme.color_main = math_vec3f(0.5f, 0.0f, 0.5f);
	render_state->theme.color_secondary = math_vec3f(1.0f, 1.0f, 1.0f);
	
	vao_quad = graphics_vao_quad_create();

	u32 floats_per_position = 2;
	u32 floats_per_quad = 8;
	u32 positions_per_quad = 4;
	u32 buffer_quad_capacity = 500;
	buffer_size = buffer_quad_capacity * floats_per_quad;
	buffer_text_positions = memory_AllocateArray(arena_ui, f32, buffer_size);
	buffer_text_uvs = memory_AllocateArray(arena_ui, f32, buffer_size);
	buffers_current_quad_count = 0;

	u32 indices_per_quad = 6;
	u32 indices_count = indices_per_quad * buffer_quad_capacity;
	buffer_text_element_indicies = memory_AllocateArray(arena_ui, u32, indices_count);

	arena_start_offset = arena_ui->offset;

	// setup layout for quad buffer indices
	u32 vertex_index = 0;
	for (u32 i = 0; i < indices_count; i += indices_per_quad)
	{
		buffer_text_element_indicies[i] = vertex_index;			// 0
		buffer_text_element_indicies[i + 1] = vertex_index + 1; // 1
		buffer_text_element_indicies[i + 2] = vertex_index + 2; // 2
		buffer_text_element_indicies[i + 3] = vertex_index + 2;	// 2
		buffer_text_element_indicies[i + 4] = vertex_index + 3;	// 3
		buffer_text_element_indicies[i + 5] = vertex_index;		// 0

		vertex_index += positions_per_quad;
	}

	vao_text = graphics_vao_create();
	graphics_vao_floatbuffer_add(&vao_text, 0, 2, buffer_text_positions, buffer_size, VAO_Options_IsDynamic | VAO_Options_IsPositions);
	graphics_vao_floatbuffer_add(&vao_text, 1, 2, buffer_text_uvs, buffer_size, VAO_Options_IsDynamic);
	graphics_vao_elementindices_add(&vao_text, buffer_text_element_indicies, indices_count, VAO_Options_Empty);
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

	UI_Info* info = ui_info_from_widget(widget);
	return info;
}


UiInfoSlider* ui_slider(f32 x, f32 y, f32 width, f32 height, f32 bar_thickness, f32 handle_thickness, f32 initial_value, f32* result)
{
	UiInfoSlider* slider_info = memory_struct_zero_allocate(arena_ui, UiInfoSlider);
	UI_Widget* bar = memory_struct_zero_allocate(arena_ui, UI_Widget);
	bar->flags = UI_WidgetFlag_Clickable | UI_WidgetFlag_DrawBackground;
	bar->position.x = x;
	bar->position.y = y + (height / 2.0f + bar_thickness / 2.0f);
	bar->size.x = width;
	bar->size.y = bar_thickness;
	math_vec3f_set_vec3f(&render_state->theme.color_main, &bar->color);
	slider_info->bar = bar;

	UI_Info* info_bar = ui_info_from_widget(bar);
	Vec2f mouse_coords = input_mouse_render_coords();
	Vec2f handle_pos = math_vec2f(x, y);
	if (info_bar->active)
	{
		handle_pos.x = mouse_coords.x;
	}

	UI_Widget* handle = memory_struct_zero_allocate(arena_ui, UI_Widget);
	handle->flags = UI_WidgetFlag_Clickable | UI_WidgetFlag_DrawBackground;
	handle->position.x = handle_pos.x;
	handle->position.y = handle_pos.y;
	handle->size.x = handle_thickness;
	handle->size.y = height;
	math_vec3f_set_vec3f(&render_state->theme.color_secondary, &handle->color);
	slider_info->handle = handle;

	UI_Info* info_handle = ui_info_from_widget(handle);
	if (info_handle->active)
	{
		handle->position.x = math_clamp(x, mouse_coords.x, x + width);
	}

	f32 value = (handle->position.x - x) / (x + width);
	result[0] = value;
	// todo: return both or 'wrap' in 'parent' ?
	return slider_info;
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
	new_offset += property_name_length + file_offset;
	ASSERT(mystr_u32_parse(file_result->text + new_offset, destination));
	return new_offset;
}

u32 ui_font_parse_property_s32(FileResult* file_result, u32 file_offset, char* property_name, u32 property_name_length, s32* destination)
{
	u32 new_offset = mystr_char_array_find_indexof(file_result->text + file_offset, file_result->length - file_offset, property_name, property_name_length);
	new_offset += property_name_length + file_offset;
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
	text_index = ui_font_parse_property_s32(result, text_index, "base=", 5, &font->base);
	text_index = ui_font_parse_property_u32(result, text_index, "scaleW=", 7, &font->image_width);
	text_index = ui_font_parse_property_u32(result, text_index, "scaleH=", 7, &font->image_height);
	text_index = ui_font_parse_property_u32(result, text_index, "chars count=", 12, &font->character_count);

	font->character_infos = memory_AllocateArray(arena_permanent, UI_Characterinfo, font->character_count);
	font->character_info_id_to_index = memory_AllocateArray(arena_permanent, u32, font->character_count);

	for (u32 i = 0; i < font->character_count; i++)
	{
		// parse character line
		u32 character_id = 0;
		text_index = ui_font_parse_property_u32(result, text_index, "char id=", 8, &character_id);

		// todo: figure out lookup. create u32-u32 map? need to go from 'a' -> char info
		UI_Characterinfo* info = font->character_infos + i;// *sizeof(UI_Characterinfo);
		info->c = (char)character_id;

		text_index = ui_font_parse_property_u32(result, text_index, "x=", 2, &info->x);
		text_index = ui_font_parse_property_u32(result, text_index, "y=", 2, &info->y);
		text_index = ui_font_parse_property_u32(result, text_index, "width=", 6, &info->width);
		text_index = ui_font_parse_property_s32(result, text_index, "height=", 7, &info->height);

		text_index = ui_font_parse_property_s32(result, text_index, "xoffset=", 8, &info->x_offset);
		text_index = ui_font_parse_property_s32(result, text_index, "yoffset=", 8, &info->y_offset);

		text_index = ui_font_parse_property_u32(result, text_index, "xadvance=", 9, &info->x_advance);
		
		info->uv_x_min = (f32)info->x / (f32)font->image_width;
		info->uv_y_min = 1.0f - ((f32)info->y / (f32)font->image_height);
		info->uv_x_max = (f32)(info->x + info->width) / (f32)font->image_width;
		info->uv_y_max = 1.0f - ((f32)(info->y + info->height) / (f32)font->image_height);
	}

	font->texture = graphics_TextureLoad(arena_temp, path_texture);

	return font;
}

// note: centering will not work if line exceeds space left in render buffer
UI_Text* ui_text_create(Mystr* text, UI_Font* font, f32 x, f32 y, f32 font_size, f32 line_width, b32 center_text)
{
	u32 floats_per_quad = 8;
	u32 uv_position_offset = buffers_current_quad_count * floats_per_quad;
	u32 quads_added = 0;

	f32* positions = buffer_text_positions + uv_position_offset;
	f32* uv_coordinates = buffer_text_uvs + uv_position_offset;

	f32 text_x = x;
	f32 text_height = font->line_height * font_size;

	f32 cursor_x = x;
	f32 cursor_y = y + font_size * (f32)font->base;
	f32 word_width = 0.0f;
	u32 word_start_index = 0;
	u32 line_start_index = 0;
	f32 line_current_width = 0.0f;
	f32 text_widest_line_width = 0.0f;
	u32 positions_index_local = 0;
	u32 uvs_index_local = 0;
	for (u32 i = 0; i < text->length; i++)
	{
		// Flush buffer if it is full
		if (i * floats_per_quad + uv_position_offset > buffer_size)
		{
			ui_text_flush();
			positions = buffer_text_positions;
			uv_coordinates = buffer_text_uvs;
			uv_position_offset = 0;
			quads_added = 0;
		}

		char c = text->text[i];
		UI_Characterinfo* character_info = ui_text_font_get_info(font, c);

		f32 x_left = cursor_x + character_info->x_offset * font_size;
		f32 x_right = x_left + character_info->width * font_size;
		f32 y_top = cursor_y - font_size * (f32)(character_info->y_offset);
		f32 y_bottom = y_top - font_size * (f32)(character_info->height);

		positions[positions_index_local++] = x_left;
		positions[positions_index_local++] = y_bottom;
		uv_coordinates[uvs_index_local++] = character_info->uv_x_min;
		uv_coordinates[uvs_index_local++] = character_info->uv_y_max;

		positions[positions_index_local++] = x_left;
		positions[positions_index_local++] = y_top;
		uv_coordinates[uvs_index_local++] = character_info->uv_x_min;
		uv_coordinates[uvs_index_local++] = character_info->uv_y_min;

		positions[positions_index_local++] = x_right;
		positions[positions_index_local++] = y_top;
		uv_coordinates[uvs_index_local++] = character_info->uv_x_max;
		uv_coordinates[uvs_index_local++] = character_info->uv_y_min;

		positions[positions_index_local++] = x_right;
		positions[positions_index_local++] = y_bottom;
		uv_coordinates[uvs_index_local++] = character_info->uv_x_max;
		uv_coordinates[uvs_index_local++] = character_info->uv_y_max;

		quads_added++;

		f32 x_advance = character_info->x_advance * font_size;
		cursor_x += x_advance;
		word_width += x_advance;
		line_current_width += x_advance;

		b8 word_end = c == ' ';
		if (word_end)
		{
			if (cursor_x - x > line_width)
			{
				// Move word down if it exceeds line width
				// todo: fix this!
				f32 line_height = font->line_height * font_size;
				for (u32 word_index = word_start_index * floats_per_quad; word_index < i * floats_per_quad; word_index += floats_per_quad)
				{
					// Align to the left when moving down
					positions[word_index + 0] = cursor_x;
					positions[word_index + 2] = cursor_x;
					positions[word_index + 4] = cursor_x;
					positions[word_index + 6] = cursor_x;

					positions[word_index + 1] -= line_height;
					positions[word_index + 3] -= line_height;
					positions[word_index + 5] -= line_height;
					positions[word_index + 7] -= line_height;

				}
				line_start_index = word_start_index;
				line_current_width = word_width;
				cursor_y -= line_height;
				cursor_x = x;
				text_height += line_height;

				// center previous line
				// we know it's total width now that the current word doesn't fit on the line
				if (center_text) {
					line_current_width -= word_width;
					f32 x_offset = (line_width - x) / 2.0f - line_current_width / 2.0f;
					text_x = x_offset;
					for (u32 index = line_start_index * floats_per_quad; index < (word_start_index-1) * floats_per_quad; index += floats_per_quad) {
						positions[index + 0] += x_offset;
						positions[index + 2] += x_offset;
						positions[index + 4] += x_offset;
						positions[index + 6] += x_offset;
					}
				}

				if (line_current_width > text_widest_line_width) {
					text_widest_line_width = line_current_width;
				}
			}
			else
			{
				text_widest_line_width += word_width;
			}

			word_width = 0.0f;
			word_start_index = i + 1;
		}
	}

	// center last line
	if (center_text) {
		f32 x_offset = (line_width - x) / 2.0f - line_current_width / 2.0f;
		for (u32 index = line_start_index * floats_per_quad; index < text->length * floats_per_quad; index += floats_per_quad) {
			positions[index + 0] += x_offset;
			positions[index + 2] += x_offset;
			positions[index + 4] += x_offset;
			positions[index + 6] += x_offset;
		}
	}

	if (text_widest_line_width == 0.0f) {
		text_widest_line_width = cursor_x - x;
	}

	buffers_current_quad_count += quads_added;
	UI_Text* uitext = memory_struct_zero_allocate(arena_ui, UI_Text);
	uitext->width = text_widest_line_width;
	uitext->text = text;
	uitext->height = text_height;
	uitext->position.y = cursor_y;
	uitext->position.x = text_x;

	return uitext;
}


UI_Info* ui_block(f32 x, f32 y, f32 width, f32 height, Vec3f color) {
	UI_Widget* widget = memory_struct_zero_allocate(arena_ui, UI_Widget);
	widget->flags = UI_WidgetFlag_Clickable | UI_WidgetFlag_DrawBackground;
	widget->position.x = x;
	widget->position.y = y;
	widget->size.x = width;
	widget->size.y = height;
	math_vec3f_set(color.x, color.y, color.z, &widget->color);

	UI_Info* info = ui_info_from_widget(widget);
	return info;
}

void ui_text_flush(void)
{
	graphics_ShaderBind(shader_program_text);
	graphics_TextureBind(&ui_text_state.font_active->texture);
	graphics_vao_buffer_subdata_floats(&vao_text, 0, buffer_text_positions, buffers_current_quad_count * 8);
	graphics_vao_buffer_subdata_floats(&vao_text, 1, buffer_text_uvs, buffers_current_quad_count * 8);
	GLCall(glBindVertexArray(vao_text.vertexArrayId));
	GLCall(glDrawElements(GL_TRIANGLES, buffers_current_quad_count * 6, GL_UNSIGNED_INT, 0));
	buffers_current_quad_count = 0;
}

UI_Characterinfo* ui_text_font_get_info(UI_Font* font, char c)
{
	UI_Characterinfo* info_default = 0;
	for (u32 i = 0; i < font->character_count; i++)
	{
		UI_Characterinfo* info = font->character_infos + i;
		if (info->c == c)
		{
			return info;
		}
		else if (info->c == '?')
		{
			info_default = info;
		}
	}

	return info_default;
}

UI_Info* ui_info_from_widget(UI_Widget* widget)
{
	UI_Info* info = memory_struct_zero_allocate(arena_ui, UI_Info);
	info->widget = widget;
	Vec2f mouse_position = input_mouse_render_coords();

	b8 hovered = collision_position_in_rect(mouse_position, widget->position, widget->size);
	if (hovered)
	{
		info->hot = TRUE;
		// todo: perhaps be smart about setting active on release if user clicked on this as well
		info->active = input_mouse_left_down();

		// Set cursor icons
		//if (widget->flags & UI_WidgetFlag_Clickable)
		//{

		//}
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