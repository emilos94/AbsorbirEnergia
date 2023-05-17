#ifndef COLLISION_H
#define COLLISION_H

#include "definitions.h"
#include "../math/vec2f.h"

struct CollisionBox
{
	Vec2f bottom_left, top_right;
};
typedef struct CollisionBox CollisionBox;

b32 collision_check_aabb_aabb(Vec2f either_offset, CollisionBox either, Vec2f other_offset, CollisionBox other);
b32 collision_position_in_rect(Vec2f position, Vec2f rect_bottom_left, Vec2f rect_size);

#endif // !COLLISION_H

