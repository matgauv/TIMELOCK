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

SIDE get_collision_side(Motion& a, Motion& b) {
	vec2 overlap = PhysicsSystem::get_collision_overlap(a, b);
	vec2 delta = a.position - b.position;
	SIDE result = SIDE::NONE;
	if (overlap.x >= 0 && overlap.y >= 0) {
		if (overlap.x < overlap.y) {
			result = (delta.x > 0) ? SIDE::LEFT : SIDE::RIGHT;
		} else {
			result = (delta.y > 0) ? SIDE::TOP : SIDE::BOTTOM;
		}
	}
	return result;
}

void PhysicsSystem::init(GLFWwindow* window) {
	this->window = window;
}

void PhysicsSystem::step(float elapsed_ms) {

	auto& motion_registry = registry.motions;
	float step_seconds = elapsed_ms / 1000.f;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];


		if (registry.falling.has(entity)) {
			apply_gravity(entity, motion, step_seconds);
		}

		if (registry.players.has(entity)	) {
			player_walk(entity, motion, step_seconds);
		}

		if (registry.movementPaths.has(entity)) {
			move_object(entity, motion, step_seconds);
		}

		motion.position += (motion.baseVelocity + motion.velocity) * step_seconds;
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
			Entity entity_j = motion_container.entities[j];
			Motion& motion_j = motion_container.components[j];
			SIDE side = get_collision_side(motion_i, motion_j);
			if (side != SIDE::NONE)
			{
				registry.collisions.emplace_with_duplicates(entity_i, entity_j, side);
			}
		}
	}
}

void PhysicsSystem::move_object(Entity& entity, Motion& motion, float step_seconds) {
	MovementPath& movement_path = registry.movementPaths.get(entity);

	Path currentPath = movement_path.paths[movement_path.currentPathIndex];

	if (abs(motion.position.x - currentPath.end.x) <= abs(currentPath.velocity.x * step_seconds) &&
		abs(motion.position.y - currentPath.end.y) <= abs(currentPath.velocity.y * step_seconds)) {
		if (movement_path.currentPathIndex == movement_path.paths.size() - 1) {
			movement_path.currentPathIndex = 0;
		} else {
			movement_path.currentPathIndex++;
		}
		currentPath = movement_path.paths[movement_path.currentPathIndex];
		motion.position = currentPath.start;
		// motion.position = currentPath.start; // TODO: don't need this? safeguard in case path start/ends dont line up
	}
	motion.velocity = currentPath.velocity;
}

void PhysicsSystem::apply_gravity(Entity& entity, Motion& motion, float step_seconds) {
	float max_fall_speed = OBJECT_MAX_FALLING_SPEED;
	if (registry.players.has(entity)) {
		max_fall_speed = PLAYER_MAX_FALLING_SPEED;
	}

	if (motion.velocity.y < max_fall_speed) {
		motion.velocity.y += GRAVITY * step_seconds; // gravity is (m/s^2) so * by
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
			motion.velocity.x -= STATIC_FRICTION * step_seconds;
			if (motion.velocity.x < 0)
				motion.velocity.x = 0;
		}
		else if (motion.velocity.x < 0) {
			motion.velocity.x += STATIC_FRICTION * step_seconds;
			if (motion.velocity.x > 0)
				motion.velocity.x = 0;
		}
	}
}


void PhysicsSystem::late_step(float elapsed_ms) {
	// nothing yet
	(void)elapsed_ms;
}
