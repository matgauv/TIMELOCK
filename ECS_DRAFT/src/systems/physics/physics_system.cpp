// internal
#include "physics_system.hpp"
#include "../world/world_init.hpp"
#include <iostream>

void PhysicsSystem::init(GLFWwindow* window) {
	this->window = window;
}

/*
 *	PhysicsSystem::step
 *		- Loops over every motion entity
 *		- Applies gravity to entities that are mid-air (falling)
 *		- Moves player entity if it's walking (i.e. the player is holding the walk key)
 *		- Updates positions of entities with movement paths (i.e. moving platforms).
 *		- Detects collisions between all entities.
 */
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
			move_object_along_path(entity, motion, step_seconds);
		}

		motion.position += ((motion.appliedVelocity + motion.selfVelocity) * motion.velocityModifier) * step_seconds;
	}

	// clear blocked...
	for (Entity entity : registry.physicsObjects.entities) {
		if (registry.blocked.has(entity)) {
			Blocked& blocked = registry.blocked.get(entity);
			blocked.left = false;
			blocked.right = false;
			blocked.top = false;
			blocked.bottom = false;
		}
	}

	detect_collisions();
}

// Handles all collisions detected in PhysicsSystem::step
void PhysicsSystem::late_step(float elapsed_ms) {
	handle_collisions(elapsed_ms);
}


// Returns the overlap in the x and y direction of motions a and b
vec2 PhysicsSystem::get_collision_overlap(Motion& a, Motion& b) {
	vec2 playerBB = {abs(a.scale.x), abs(a.scale.y)};
	vec2 objBB = {abs(b.scale.x), abs(b.scale.y)};

	vec2 playerHalf = playerBB * 0.5f;
	vec2 objHalf = objBB * 0.5f;

	vec2 delta = a.position - b.position;

	float overlapX = (playerHalf.x + objHalf.x) - fabs(delta.x);
	float overlapY = (playerHalf.y + objHalf.y) - fabs(delta.y);
	return {overlapX, overlapY};
}

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 PhysicsSystem::get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// returns NONE if no collision, otherwise returns the side from the perspective of motion a
SIDE PhysicsSystem::get_collision_side(Motion& a, Motion& b) {
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



// detect collisions between all moving entities.
void PhysicsSystem::detect_collisions() {

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

// will move an object along it's movement path, rotating between the different paths specified in the component
void PhysicsSystem::move_object_along_path(Entity& entity, Motion& motion, float step_seconds) {
	MovementPath& movement_path = registry.movementPaths.get(entity);

	Path currentPath = movement_path.paths[movement_path.currentPathIndex];

	if (abs(motion.position.x - currentPath.end.x) <= abs(currentPath.velocity.x * motion.velocityModifier.x * step_seconds) &&
		abs(motion.position.y - currentPath.end.y) <= abs(currentPath.velocity.y * motion.velocityModifier.y * step_seconds)) {
		if (movement_path.currentPathIndex == movement_path.paths.size() - 1) {
			movement_path.currentPathIndex = 0;
		} else {
			movement_path.currentPathIndex++;
		}
			currentPath = movement_path.paths[movement_path.currentPathIndex];
			motion.position = currentPath.start;
		}
	motion.selfVelocity = currentPath.velocity;
}

// accelerates the entity by GRAVITY until it reaches the max_speed. currently only one max speed.
// TODO: should this be applied velocity?
void PhysicsSystem::apply_gravity(Entity& entity, Motion& motion, float step_seconds) {
	float max_fall_speed = OBJECT_MAX_FALLING_SPEED;
	if (registry.players.has(entity)) {
		max_fall_speed = PLAYER_MAX_FALLING_SPEED;
	}

	if (motion.selfVelocity.y < max_fall_speed) {
		motion.selfVelocity.y += GRAVITY * step_seconds;
	}
}

// Moves the player left or right depending on the direction specified in the walking component
// will not move if blocked
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
			// for snappier movement changes (if entity is moving in opposite of desired direction)
			// "braking" force is 2x walking acceleration
			if (motion.selfVelocity.x * desired_direction < 0) {
				motion.selfVelocity.x += desired_direction * PLAYER_WALK_ACCELERATION * 2.0f * step_seconds;

				if (motion.selfVelocity.x * desired_direction > 0)
					motion.selfVelocity.x = 0;
			}

			motion.selfVelocity.x += desired_direction * PLAYER_WALK_ACCELERATION * step_seconds;
			// clamp to max walking speed
			if (fabs(motion.selfVelocity.x) > PLAYER_MAX_WALKING_SPEED)
				motion.selfVelocity.x = desired_direction * PLAYER_MAX_WALKING_SPEED;
		} else {
			motion.selfVelocity.x = 0.0f; // if blocked, don't move
		}

	} else {
		// if not walking, we're stopping -- slow down to 0.
		// Any other source of "selfVelocity" in the x direction will prob break this with this.
		if (motion.selfVelocity.x > 0) {
			motion.selfVelocity.x -= STATIC_FRICTION * step_seconds;
			if (motion.selfVelocity.x < 0)
				motion.selfVelocity.x = 0;
		}
		else if (motion.selfVelocity.x < 0) {
			motion.selfVelocity.x += STATIC_FRICTION * step_seconds;
			if (motion.selfVelocity.x > 0)
				motion.selfVelocity.x = 0;
		}
	}
}

/*
 * Handles collisions between entities, specifically:
 *		- PhysicsObject <-> Platform
 *		- PhysicsObject <-> PhysicsObject
 */
void PhysicsSystem::handle_collisions(float elapsed_ms) {
	ComponentContainer<Collision>& collision_container = registry.collisions;

	std::vector<unsigned int> groundedEntities = {};
	std::vector<unsigned int> onMovingPlatform = {};

	float step_seconds = elapsed_ms / 1000.0f;

	for (uint i = 0; i < collision_container.components.size(); i++) {
		Entity& one = collision_container.entities[i];
		Collision& collision = collision_container.components[i];
		Entity& other = collision.other;

		// order here is important so handle both cases sep
		if (registry.physicsObjects.has(one) && registry.platforms.has(other)) {
			handle_object_platform_collision(one, other, collision, step_seconds, groundedEntities, onMovingPlatform);
		} else if (registry.physicsObjects.has(other) && registry.platforms.has(one)) {
			collision.side = (SIDE)((collision.side + 2) % 4); //swap sides since coll is from perspective of one (left<->right) (top <-> bottom)
			handle_object_platform_collision(other, one, collision, step_seconds, groundedEntities, onMovingPlatform);
		}

		if (registry.players.has(one) && registry.projectiles.has(other)) {
			// TODO: should handle_player_projectile_collision() be handle_player_attack_collision() ?
			handle_player_attack_collision(one, other, collision);
		} else if (registry.players.has(other) && registry.projectiles.has(one)) {
			handle_player_attack_collision(other, one, collision);
		}

		if (registry.players.has(one) && registry.bosses.has(other)) {
			handle_player_boss_collision(one, other, collision);
		} else if (registry.players.has(other) && registry.bosses.has(one)) {
			handle_player_boss_collision(other, one, collision);
		}
	}

	for (uint i = 0; i < collision_container.components.size(); i++) {
		Entity& one = collision_container.entities[i];
		Collision& collision = collision_container.components[i];
		Entity& other = collision.other;

		if (registry.physicsObjects.has(one) && registry.physicsObjects.has(other)) {
        	handle_physics_collision(step_seconds, one, other, collision, groundedEntities);
        }
	}

	for (int i = 0; i < registry.physicsObjects.entities.size(); i++){
		Entity& entity = registry.physicsObjects.entities[i];

		// If object is not grounded, it should be falling.
		if(!in(groundedEntities, entity.id())) {
			if (!registry.falling.has(entity)) {
				registry.falling.emplace(entity);
				registry.blocked.remove(entity);
			}
		}

		// If object is not on moving platform, modify base velocity.
		if (!in(onMovingPlatform, entity.id())) {
			Motion& obj_motion = registry.motions.get(entity);
            float diff = AIR_RESISTANCE * step_seconds;
            obj_motion.appliedVelocity.x = clampToTarget(obj_motion.appliedVelocity.x, diff, 0);
            obj_motion.appliedVelocity.y = clampToTarget(obj_motion.appliedVelocity.y, diff, 0);
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Handles collision between a PhysicsObject entity and a Platform entity.
void PhysicsSystem::handle_object_platform_collision(Entity object_entity, Entity platform_entity, Collision collision, float step_seconds, std::vector<unsigned int>& groundedEntities, std::vector<unsigned int>& onMovingPlatform) {
	Motion& object_motion = registry.motions.get(object_entity);
	Motion& platform_motion = registry.motions.get(platform_entity);
	vec2 overlap = get_collision_overlap(object_motion, platform_motion);

	if (!registry.blocked.has(object_entity)) {
			registry.blocked.emplace(object_entity);
	}
	Blocked& blocked = registry.blocked.get(object_entity);

	if (collision.side == SIDE::LEFT) {
		blocked.left = true;
		if (object_motion.selfVelocity.x < 0)
			object_motion.selfVelocity.x = 0.0f;
		object_motion.position.x += overlap.x;
	}
	else if (collision.side == SIDE::RIGHT) {
		blocked.right = true;
		if (object_motion.selfVelocity.x > 0)
			object_motion.selfVelocity.x = 0.0f;
		object_motion.position.x -= overlap.x;
	} else if (collision.side == SIDE::BOTTOM) {
		object_motion.selfVelocity.y = 0.0f;

		registry.falling.remove(object_entity);
		object_motion.position.y -= overlap.y;
		blocked.bottom = true;

		groundedEntities.push_back(object_entity.id());

		// special case for colliding with moving platforms
		if (registry.movementPaths.has(platform_entity)) {
			MovementPath& movementPath = registry.movementPaths.get(platform_entity);
			Path& currPath = movementPath.paths[movementPath.currentPathIndex];

			// tiny bit of friction (simulated by interpolating to target velocity)
			float diff = DYNAMIC_FRICTION * step_seconds;
			object_motion.appliedVelocity.x = clampToTarget(object_motion.appliedVelocity.x, diff, currPath.velocity.x * platform_motion.velocityModifier.x);
			object_motion.appliedVelocity.y = clampToTarget(object_motion.appliedVelocity.y, diff, currPath.velocity.y * platform_motion.velocityModifier.y);

			onMovingPlatform.push_back(object_entity.id());
		} else {
			object_motion.appliedVelocity.y = 0.0f;
		}
	} else if (collision.side == SIDE::TOP) {
		blocked.top = true;
		// stops the player from "sticking" to the bottom of a platform when they jump up into it
		// if player's y velocity is positive (i.e. player is falling), don't set velocity to 0 to avoid hanging.
		object_motion.selfVelocity.y = max(object_motion.selfVelocity.y, 0.0f);
		object_motion.appliedVelocity.y = max(object_motion.appliedVelocity.y, 0.0f);
		object_motion.position += overlap.y;
	}

}

void PhysicsSystem::handle_player_attack_collision(Entity player_entity, Entity attack_entity, Collision collision) {
	if (registry.harmfuls.has(attack_entity)) {
		assert(registry.gameStates.components.size() <= 1);
		GameState& gameState = registry.gameStates.components[0];
		gameState.game_running_state = GAME_RUNNING_STATE::OVER;
	}
}

void PhysicsSystem::handle_player_boss_collision(Entity player_entity, Entity boss_entity, Collision collision) {
	Boss& boss = registry.bosses.get(boss_entity);

	boss.health -= PLAYER_ATTACK_DAMAGE;

	if (boss.health <= 0.f) {
		assert(registry.gameStates.components.size() <= 1);
		GameState& gameState = registry.gameStates.components[0];
		gameState.is_in_boss_fight = 0;
		registry.bosses.remove(boss_entity);
	}
}

// Handles collision between two PhysicsObject entities.
// TODO can probably replace this with more rigorous physics interpretation?
void PhysicsSystem::handle_physics_collision(float step_seconds, Entity entityA, Entity entityB, Collision collision, std::vector<unsigned int>& grounded) {
	Motion& motionA = registry.motions.get(entityA);
	Motion& motionB = registry.motions.get(entityB);
	vec2 overlap = get_collision_overlap(motionA, motionB);

	PhysicsObject& physicsA = registry.physicsObjects.get(entityA);
	PhysicsObject& physicsB = registry.physicsObjects.get(entityB);


	Blocked& blockedA = registry.blocked.has(entityA) ? registry.blocked.get(entityA) : registry.blocked.emplace(entityA);
	Blocked& blockedB = registry.blocked.has(entityB) ? registry.blocked.get(entityB) : registry.blocked.emplace(entityB);


	// TODO can we abstract this??? pretty ugly rn
	switch (collision.side) {
		case SIDE::LEFT:
			if ((physicsA.weight < physicsB.weight && !blockedA.right) || blockedB.left) {
				if (blockedB.left) blockedA.left = true;
				motionA.position.x += overlap.x;
			} else  {
				if (blockedA.right) blockedB.right = true;
				motionB.position.x -= overlap.x;
			}
		break;
		case SIDE::RIGHT:
			if ((physicsA.weight < physicsB.weight && !blockedA.left) || blockedB.right) {
				motionA.position.x -= overlap.x;
				if (blockedB.right) blockedA.right = true;
			} else {
				motionB.position.x += overlap.x;
				if (blockedA.left) blockedB.left = true;
			}

			break;
		case SIDE::TOP:
			if ((physicsA.weight < physicsB.weight && !blockedA.bottom) || blockedB.top) {
				motionA.position.y += overlap.y;
				if (blockedB.top) blockedA.top = true;
			} else {
				motionB.position.y -= overlap.y;
				if (blockedA.bottom) blockedB.bottom = true;
				if (!registry.falling.has(entityA)) {
					registry.falling.remove(entityB);
					grounded.push_back(entityB.id());
				}

				// update vertical velocity so the top object looses it's acceleration
				motionB.selfVelocity.y = motionA.selfVelocity.y * motionA.velocityModifier.y;

				// update horizontal velocity so you can carry stuff
				float diff = STATIC_FRICTION * step_seconds;
				motionB.appliedVelocity.x = clampToTarget(motionB.appliedVelocity.x, diff, (motionA.selfVelocity.x + motionA.appliedVelocity.x) * motionA.velocityModifier.x);

			}
			break;
		case SIDE::BOTTOM:
			if ((physicsA.weight < physicsB.weight && !blockedA.top) || blockedB.bottom) {
				if (blockedB.bottom) blockedA.bottom = true;
				if (!registry.falling.has(entityB)) {
					registry.falling.remove(entityA);
					grounded.push_back(entityA.id());
				}

				motionA.position.y -= overlap.y;

				// update vertical velocity so the top object looses it's acceleration
				motionA.selfVelocity.y = motionB.selfVelocity.y * motionB.velocityModifier.y;

				// update horizontal velocity so you can carry stuff
				float diff = STATIC_FRICTION * step_seconds;
				motionA.appliedVelocity.x = clampToTarget(motionA.appliedVelocity.x, diff, (motionB.selfVelocity.x + motionB.appliedVelocity.x) * motionB.velocityModifier.x);
			} else {
				if (blockedA.top) blockedB.top = true;
				motionB.position.y += overlap.y;
			}
			break;
		default:
			break;
	}
}

// TODO change this to proper lerp
// function for interpolating object velocity, specifically when an object is on a moving platforms.
float PhysicsSystem::clampToTarget(float value, float change, float target) {
	change = abs(change);
	if (value > target) {
		return std::max(value - change, target);
	} else if (value < target) {
		return std::min(value + change, target);
	}
	return target;
}

// Helper function to check if an entity id is within a vector.
bool PhysicsSystem::in(std::vector<unsigned int> vec, unsigned int id) {
	return std::find(vec.begin(), vec.end(), id) != vec.end();
}
