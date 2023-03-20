#include "entity.h"

void entity_set_zero(Entity* entity)
{
	entity->next = 0;

	entity->entityFlags = 0;
	entity->entityTags = 0;
	math_vec2f_zero(&entity->transform.position);
	math_vec2f_zero(&entity->transform.scale);
	math_vec2f_set(&entity->transform.scale, 1.0f);
	
	math_vec2f_zero(&entity->motion.acceleration);
	math_vec2f_zero(&entity->motion.direction);
	math_vec2f_zero(&entity->motion.velocity);
	entity->motion.friction = 0;

	entity->animationCount = 0;
	entity->currentAnimation = 0;
	entity->isAnimationPlaying = FALSE;
	
	entity->isVisible = FALSE;

	entity->createdTime = 0;
	entity->timeToLive = 0;

	entity->move_last_switch = 0;
	entity->move_switch_cooldown = 0;

	math_vec2f_zero(&entity->collision_box.top_right);
	math_vec2f_zero(&entity->collision_box.bottom_left);
}
