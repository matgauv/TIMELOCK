// internal
#include "physics_system.hpp"
#include "../world/world_init.hpp"
#include <iostream>

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

SIDE get_collision_side(const Motion& a, const Motion& b) {
	vec2 aHalf = get_bounding_box(a) * 0.5f;
	vec2 bHalf = get_bounding_box(b) * 0.5f;

	vec2 delta = a.position - b.position;

	float overlapX = (aHalf.x + bHalf.x) - fabs(delta.x);
	float overlapY = (aHalf.y + bHalf.y) - fabs(delta.y);

	SIDE result = SIDE::NONE;
	if (overlapX > 0 && overlapY > 0) {
		if (overlapX < overlapY) {
			result = (delta.x > 0) ? SIDE::LEFT : SIDE::RIGHT;
		} else {
			result = (delta.y > 0) ? SIDE::TOP : SIDE::BOTTOM;
		}
	}

	return result;
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

void PhysicsSystem::init(GLFWwindow* window) {
	this->window = window;
}

void PhysicsSystem::step(float elapsed_ms) {
	auto& falling_registry = registry.falling;

	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		float step_seconds = elapsed_ms / 1000.f;

		if (falling_registry.has(entity)) {
			apply_gravity(entity, motion, step_seconds);
		}

		if (registry.players.has(entity)	) {
			player_walk(entity, motion, step_seconds);
		}

		motion.position += motion.velocity * step_seconds;
	}

	// clear blocked...
	for (Entity player : registry.players.entities) {
		if (registry.blocked.has(player)) {
			Blocked& blocked = registry.blocked.get(player);
			blocked.left = false;
			blocked.right = false;
		}
	}

	// check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j < motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			SIDE side = get_collision_side(motion_i, motion_j);

			if (side != SIDE::NONE)
			{
				Entity entity_j = motion_container.entities[j];
				registry.collisions.emplace_with_duplicates(entity_i, entity_j, side);
			}
		}
	}
}

void PhysicsSystem::apply_gravity(Entity& entity, Motion& motion, float step_seconds) {
	bool grounded = false;
	if (registry.blocked.has(entity)) {
		Blocked& blocked = registry.blocked.get(entity);
		grounded = blocked.bottom;
	}

	if (!grounded) {
		float max_fall_speed = OBJECT_MAX_FALLING_SPEED;
		if (registry.players.has(entity)) {
			max_fall_speed = PLAYER_MAX_FALLING_SPEED;
		}

		if (motion.velocity.y < max_fall_speed) {
			motion.velocity.y += GRAVITY * step_seconds; // gravity is (m/s^2) so * by
		}
	}
}
void PhysicsSystem::player_walk(Entity& entity, Motion& motion, float step_seconds) {
	if (registry.walking.has(entity)) {
		Walking& walking = registry.walking.get(entity);

		int desired_direction = walking.is_left ? -1 : 1;

		// check if blocked
		bool blockedDirection = false;
		if (registry.blocked.has(entity)) {
			Blocked& blocked = registry.blocked.get(entity);
			if (desired_direction < 0 && blocked.left) {
				blockedDirection = true;
			}
			else if (desired_direction > 0 && blocked.right) {
				blockedDirection = true;
			}
		}

		if (!blockedDirection) {
			// for snappier movement changes (if entity is moving in oposite of desired direction)
			// "braking" force is 2x walking acceleration
			if (motion.velocity.x * desired_direction < 0) {
				motion.velocity.x += desired_direction * PLAYER_WALK_ACCELERATION * 2.0f * step_seconds;

				if (motion.velocity.x * desired_direction > 0)
					motion.velocity.x = 0;
			}

			motion.velocity.x += desired_direction * PLAYER_WALK_ACCELERATION * step_seconds;
			// clamp to max walking speed
			if (fabs(motion.velocity.x) > PLAYER_MAX_WALKING_SPEED)
				motion.velocity.x = desired_direction * PLAYER_MAX_WALKING_SPEED;
		} else {
			motion.velocity.x = 0.0f; // if blocked, don't move
		}

	} else {
		// if not walkin, we're stoppin -- slow down to 0
		// TODO: other sources of motion might break this... (ie. when moving on a platform)
		if (motion.velocity.x > 0) {
			motion.velocity.x -= FRICTION * step_seconds;
			if (motion.velocity.x < 0)
				motion.velocity.x = 0;
		}
		else if (motion.velocity.x < 0) {
			motion.velocity.x += FRICTION * step_seconds;
			if (motion.velocity.x > 0)
				motion.velocity.x = 0;
		}
	}
}


void PhysicsSystem::late_step(float elapsed_ms) {
	// nothing yet
	(void)elapsed_ms;
}
