#include "collision.h"

b32 collision_check_aabb_aabb(Vec2f either_offset, CollisionBox either, Vec2f other_offset, CollisionBox other)
{
	float either_x_min = either_offset.x + either.bottom_left.x;
	float either_x_max = either_offset.x + either.top_right.x;
	float either_y_min = either_offset.y + either.bottom_left.y;
	float either_y_max = either_offset.y + either.top_right.y;

	float other_x_min = other_offset.x + other.bottom_left.x;
	float other_x_max = other_offset.x + other.top_right.x;
	float other_y_min = other_offset.y + other.bottom_left.y;
	float other_y_max = other_offset.y + other.top_right.y;

	b32 result = 
		!(either_x_max < other_x_min ||
		either_x_min > other_x_max ||
		either_y_max < other_y_min ||
		either_y_min > other_y_max);

	return result;
}


b32 collision_position_in_rect(Vec2f position, Vec2f rect_bottom_left, Vec2f rect_size)
{
	b8 x_overlap = position.x >= rect_bottom_left.x && position.x <= rect_bottom_left.x + rect_size.x;
	b8 y_overlap = position.y >= rect_bottom_left.y && position.y <= rect_bottom_left.y + rect_size.y;

	return x_overlap && y_overlap;
}