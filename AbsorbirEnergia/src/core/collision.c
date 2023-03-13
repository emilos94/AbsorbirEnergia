#include "collision.h"

B32 collision_check_aabb_aabb(Vec2f either_offset, CollisionBox either, Vec2f other_offset, CollisionBox other)
{
	float either_x_min = either_offset.x + either.top_left.x;
	float either_x_max = either_offset.x + either.bottom_right.x;
	float either_y_min = either_offset.y + either.bottom_right.y;
	float either_y_max = either_offset.y + either.top_left.y;

	float other_x_min = other_offset.x + other.top_left.x;
	float other_x_max = other_offset.x + other.bottom_right.x;
	float other_y_min = other_offset.y + other.bottom_right.y;
	float other_y_max = other_offset.y + other.top_left.y;

	B32 result = 
		either_x_min < other_x_max &&
		either_x_max > other_x_min &&
		either_y_min < other_y_max &&
		either_y_max > other_y_min;

	return result;
}
