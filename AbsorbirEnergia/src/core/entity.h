#ifndef ENTITY_H
#define ENTITY_H

#include "definitions.h"
#include "../math/vec2f.h"
#include "../graphics/texture.h"

struct Transform
{
	Vec2f position;
	Vec2f scale;
	float rotation;
};
typedef struct Transform Transform;

struct Entity
{
	U32 entityId;
	U32 entityFlags;

	Transform transform;
	Texture texture;
};
typedef struct Entity Entity;

#endif // !ENTITY_H
