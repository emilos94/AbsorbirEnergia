#ifndef COLLISION_H
#define COLLISION_H

#include "definitions.h"
#include "../math/vec2f.h"

struct CollisionBox
{
	Vec2f top_left, bottom_right;
};
typedef struct CollisionBox CollisionBox;

B32 collision_check_aabb_aabb(Vec2f either_offset, CollisionBox either, Vec2f other_offset, CollisionBox other);

#endif // !COLLISION_H

