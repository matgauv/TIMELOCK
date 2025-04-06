#include "player_mechanics.h"

#include <iostream>


void player_climb(Entity& entity, Motion& motion, float step_seconds) {
	if (!registry.climbing.has(entity)) return;

	Climbing& climbing = registry.climbing.get(entity);
	std::cout << climbing.is_up << std::endl;
	std::cout << "w held: " << registry.players.components[0].w_is_held << std::endl;

	if (climbing.is_up) {

		if (motion.velocity.y < -PLAYER_CLIMBING_SPEED) {
			motion.velocity.y = -PLAYER_CLIMBING_SPEED;
		} else {
			motion.velocity.y = lerp(motion.velocity.y, -PLAYER_CLIMBING_SPEED, step_seconds * 2.0);
		}

	} else {

		if (motion.velocity.y > PLAYER_CLIMBING_SPEED) {
			motion.velocity.y = PLAYER_CLIMBING_SPEED;
		} else {
			motion.velocity.y = lerp(motion.velocity.y, PLAYER_CLIMBING_SPEED, step_seconds * 2.0);
		}
	}
}

// Moves the player left or right depending on the direction specified in the walking component
// will not move if blocked
void player_walk(Entity& entity, Motion& motion, float step_seconds)
{
	if (registry.walking.has(entity))
	{

		// lower player friction while they walk
		PhysicsObject& phys = registry.physicsObjects.get(entity);
		phys.friction = DYNAMIC_FRICTION;

		float acceleration = PLAYER_WALK_ACCELERATION;
		if (registry.climbing.has(entity)) {
			acceleration = PLAYER_WALK_LADDER_ACCELERATION;
		}

        FlagState& flags = registry.flags.components[0];
        if (flags.fly) acceleration *= 5.0f;


		Walking& walking = registry.walking.get(entity);
		vec2 desired_direction = {walking.is_left ? -1.0f : 1.0f, 0.0f};
		vec2 accel = desired_direction * acceleration;

		vec2 platform_velocity = motion.velocity;
		if (registry.onGrounds.has(entity)) {
			unsigned int ground_id = registry.onGrounds.get(entity).other_id;
			Motion& platform_motion = registry.motions.get(ground_id);
			platform_velocity = get_modified_velocity(platform_motion);
		}

		vec2 rel_velocity = motion.velocity - platform_velocity;

		if (rel_velocity.x * desired_direction.x < 0.0f) acceleration *= 5.0f;

		// accelerate on a curve of x^2 instead of linear
		float normalizedSpeed = fabs(rel_velocity.x) / PLAYER_MAX_WALKING_SPEED;
		float interpFactor = 1.0f - (normalizedSpeed * normalizedSpeed);
		rel_velocity += desired_direction * acceleration * interpFactor * step_seconds;

		// clamp relative velocity instead of world velocity
        if (!flags.fly) rel_velocity.x = clamp(rel_velocity.x, -PLAYER_MAX_WALKING_SPEED, PLAYER_MAX_WALKING_SPEED);

		motion.velocity = rel_velocity + platform_velocity;
	} else {
		PhysicsObject& phys = registry.physicsObjects.get(entity);
		phys.friction = PLAYER_STATIC_FRICTION;
	}
}