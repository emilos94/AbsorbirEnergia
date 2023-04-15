#ifndef UI_H
#define UI_H

#include "../core/memory_arena.h"
#include "../core/definitions.h"
#include "../core/input.h"
#include "../core/mystr.h"
#include "../core/file_loader.h"
#include "../graphics/shader_program.h"
#include "../graphics/renderer.h"
#include "../math/vec2f.h"
#include "../math/vec3f.h"
#include "../math/matrix4f.h"

typedef u32 UI_WidgetFlags;
enum 
{
	UI_WidgetFlag_Clickable = (1 << 0),
	UI_WidgetFlag_DrawBorder = (1 << 1),
	UI_WidgetFlag_DrawBackground = (1 << 2),
	UI_WidgetFlag_DrawText = (1 << 3),
	UI_WidgetFlag_DrawTexture = (1 << 4)
};

struct UI_Widget
{
	struct UI_Widget* next;
	struct UI_Widget* prev;

	UI_WidgetFlags flags;

	Vec2f position;
	Vec2f size;

	Vec3f color;

	char text[50];
};
typedef struct UI_Widget UI_Widget;

struct UI_Info
{
	UI_Widget* widget;

	b8 active;
	b8 hot;
};
typedef struct UI_Info UI_Info;

struct UI_Theme
{
	Vec3f color_main;
	Vec3f color_secondary;
	Vec3f color_text;
};
typedef struct UI_Theme UI_Theme;

struct UI_Render_State
{
	UI_Theme theme;

	UI_Widget* widget_first;
	UI_Widget* widget_last;
};
typedef struct UI_Render_State UI_Render_State;

//UI_Widget* ui_widget_str(UI_WidgetFlags flags, char* text, u32 text_length);
//UI_Info* ui_info_from_widget(UI_Widget* widget);

/* 
*	if (game_state->main_menu_on)
* {
*	
*	f32 button_width = 64.0f;
*	f32 center x = window_width / 2.0f - button_width / 2.0f;
*	f32 start_y = 140.0f;
	UI_Info* play_button = ui_button_text_create(center_x, start_y, button_width, 32.0f, "Play", 4);
	if (player_button->pressed)
	{
		game_state->main_menu_on = false;
		game_state->start_game = true;
	}


*/

/*** TEXT ***/
struct UI_Characterinfo
{
	char c;

	u32 x;
	u32 y;
	u32 width;
	s32 height;
	s32 x_offset;
	s32 y_offset;
	u32 x_advance;

	f32 uv_x_min;
	f32 uv_y_min;
	f32 uv_x_max;
	f32 uv_y_max;
};
typedef struct UI_Characterinfo UI_Characterinfo;

struct UI_Font
{
	Mystr* font_name;

	u32 character_count;
	UI_Characterinfo* character_infos;
	u32* character_info_id_to_index;

	u32 padding_left;
	u32 padding_right;
	u32 padding_top;
	u32 padding_bottom;
	u32 line_height;
	s32 base;

	u32 image_width;
	u32 image_height;
	Texture texture;
};
typedef struct UI_Font UI_Font;

UI_Font* ui_text_font_load(MemoryArena* arena_temp, MemoryArena* arena_permanent, char* path_specification, char* path_texture);
UI_Characterinfo* ui_text_font_get_info(UI_Font* font, char c);

struct UI_Text_State
{
	UI_Font* font_active;
};
typedef struct UI_Text_State UI_Text_State;

struct UI_Text
{
	Mystr* text;
	f32 font_size;
	f32 width;
};
typedef struct UI_Text UI_Text;

void ui_initialize(Mat4f* projection_matrix, UI_Font* font_default);
UI_Info* ui_button(f32 x, f32 y, f32 width, f32 height);
void ui_render_flush(void);
void ui_render_destroy();

UI_Text* ui_text_create(Mystr* text, UI_Font* font, f32 x, f32 y, f32 font_size, f32 line_width);
void ui_text_flush(void);

#endif