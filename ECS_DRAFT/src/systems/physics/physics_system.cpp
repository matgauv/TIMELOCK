// internal
#include "physics_system.hpp"

#include <cfloat>

#include "../world/world_init.hpp"
#include "../player/player_system.hpp"
#include <iostream>
#include "../parser/parsing_system.hpp"

#include <iostream>
void PhysicsSystem::init(GLFWwindow* window) {
	this->window = window;
}

// NOTE: this system expects to be stepped at a FIXED rate, and requires multiple steps per frame for proper behaviour.
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

	drop_bolt_when_player_near(DISTANCE_TO_DROP_BOLT);

	for (uint i = 0; i < registry.pendulums.size(); i++) {
		Entity& entity = registry.pendulums.entities[i];
		update_pendulum(entity, step_seconds);
	}

	update_pendulum_rods();

	for(uint i = 0; i< motion_registry.size(); i++)
	{

		Motion& motion = motion_registry.components[i];
		Entity& entity = motion_registry.entities[i];

		// we handle the pendulum motion separately
	if (registry.pendulums.has(entity)) {
			continue;
		}

		if (registry.physicsObjects.has(entity)) {

			PhysicsObject& phys = registry.physicsObjects.get(entity);
			if (phys.apply_gravity) apply_gravity(entity, motion, step_seconds);

			if (phys.apply_rotation) {
                // Update angle
				float modified_angular_velocity = get_modified_angular_velocity(motion, phys);

                motion.angle += degrees(modified_angular_velocity * step_seconds);
                motion.cache_invalidated = true;

				float angle_rad = radians(motion.angle);

				vec2 tangent = { -sin(angle_rad), cos(angle_rad) };
				float rotationFrictionFactor = 0.5f;
				vec2 angular_push = tangent * fabs(modified_angular_velocity) * rotationFrictionFactor;
				if (phys.mass >0.0f) motion.position += angular_push * step_seconds;
				if (phys.angular_damping > 0.0f) phys.angular_velocity *= (1.0f - phys.angular_damping * step_seconds); // Damping factor

				if (registry.rotatingGears.has(entity)) {
					phys.angular_velocity = registry.rotatingGears.get(entity).angular_velocity; // no need to modify here, should store normal time ang vel
				}
			}

		}

		if (registry.players.has(entity)	) {
			player_walk(entity, motion, step_seconds);
			player_climb(entity, motion, step_seconds);
		}

		if (registry.movementPaths.has(entity)) {
			move_object_along_path(entity, motion, step_seconds);
		}

		vec2 oldMotion = motion.position;

		// TODO: handle boss and wall collisions
		if (registry.bosses.has(entity)) {
			vec2 next_pos = motion.position + (motion.velocity * motion.velocityModifier) * step_seconds;
			if (next_pos.x < 383.f) {
				motion.position.x = 383.f;
				motion.position.y += (motion.velocity.y * motion.velocityModifier) * step_seconds;
				motion.velocity *= -1.0f;
			} else if (next_pos.x > 1050.f) {
				motion.position.x = 1050.f;
				motion.position.y += (motion.velocity.y * motion.velocityModifier) * step_seconds;
				motion.velocity *= -1.0f;
			} else {
				motion.position = next_pos;
			}

		} else {
			motion.position += (motion.velocity * motion.velocityModifier) * step_seconds;
		}

		// invalidate the cached vertices of the motion moved
		if (oldMotion != motion.position) {
			motion.cache_invalidated = true;
		}
	}

	for (Entity entity : registry.motions.entities) {
		Motion& motion = registry.motions.get(entity);
		if (motion.cache_invalidated) {
			compute_vertices(motion, entity);
			compute_axes(motion, motion.cached_vertices);
			motion.cache_invalidated = false; // Reset flag
		}
	}

	detect_collisions();
	handle_collisions(elapsed_ms);
}

void PhysicsSystem::late_step(float elapsed_ms) {

}

void PhysicsSystem::detect_collisions() {
	auto& physics_objects = registry.physicsObjects;
	auto& platform_container = registry.platforms;
	auto& colliders = registry.nonPhysicsColliders;

	for (uint i = 0; i < physics_objects.size(); ++i) {
		Entity& entity_i = physics_objects.entities[i];

		// only check collsions between objects and platforms (no need for platform-platform collisions)
		if (platform_container.has(entity_i)) continue;

		Motion& motion_i = registry.motions.get(entity_i);

		// check between each object and each platform
		for (uint j = 0; j < platform_container.size(); ++j) {
			Entity& entity_j = platform_container.entities[j];
			collision_check(entity_i, motion_i, entity_j);
		}

		// check between each physics object and each other
		for (uint j = i+1; j < physics_objects.size(); ++j) {
			Entity& entity_j = physics_objects.entities[j];
			collision_check(entity_i, motion_i, entity_j);
		}

		// check between physics objects and any other colliders
		for (uint j = 0; j < colliders.size(); ++j) {
			Entity& entity_j = colliders.entities[j];
			collision_check(entity_i, motion_i, entity_j);
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
	float step_seconds = elapsed_ms / 1000.0f;

	bool player_ladder_collision = false;


	for (uint i = 0; i < collision_container.components.size(); i++) {
		Entity& one = collision_container.entities[i];
		Collision& collision = collision_container.components[i];
		Entity other = Entity(collision.other_id);

		// do not collide with anything if no clip is on
		bool no_clip = registry.flags.components[0].no_clip;
		if (no_clip && (registry.players.has(one) || registry.players.has(other))) {
			continue;
		}

		// if player hits a breakable platform
		if (registry.players.has(one) && registry.breakables.has(other)) {
			handle_player_breakable_collision(other, elapsed_ms);
		} else if (registry.players.has(other) && registry.breakables.has(one)) {
			handle_player_breakable_collision(one, elapsed_ms);
		}

		if (registry.players.has(one) && registry.doors.has(other)) {
			handle_player_door_collision();
		} else if (registry.players.has(other) && registry.doors.has(other)) {
			handle_player_door_collision();
		}

		// handle player and boss projectile collision
		if (registry.players.has(one) && registry.projectiles.has(other)) {
			// TODO: should handle_player_projectile_collision() be handle_player_attack_collision() ?
			// TODO: should leave all events that kill player to collision with harmful entities
			handle_player_attack_collision(one, other, collision);
		} else if (registry.players.has(other) && registry.projectiles.has(one)) {
			handle_player_attack_collision(other, one, collision);
		}

		// TODO: handle player and boss collision (temporarily moving this into the boss_one_utils.cpp)
		// if (registry.players.has(one) && registry.bosses.has(other)) {
			
		// 	// kill the player if the boss is harmful (during dash attack)
		// 	Entity& boss_entity = registry.bosses.entities[0];
		// 	if (registry.harmfuls.has(boss_entity)) {
		// 		PlayerSystem::kill();
		// 	}
		// } else if (registry.players.has(other) && registry.bosses.has(one)) {
			
		// 	// kill the player if the boss is harmful (during dash attack)
		// 	Entity& boss_entity = registry.bosses.entities[0];
		// 	if (registry.harmfuls.has(boss_entity)) {
		// 		PlayerSystem::kill();
		// 	}
		// }

		// TODO: handle player and snooze button collision
		if (registry.players.has(one) && registry.snoozeButtons.has(other)) {

			FirstBoss& firstBoss = registry.firstBosses.components[0];
			firstBoss.player_collided_with_snooze_button = true;
			// registry.remove_all_components_of(other); // remove snooze button -> maybe this should be the job of a particular boss state

		} else if (registry.players.has(other) && registry.snoozeButtons.has(one)) {

			FirstBoss& firstBoss = registry.firstBosses.components[0];
			firstBoss.player_collided_with_snooze_button = true;
			// registry.remove_all_components_of(one); // remove snooze button -> maybe this should be the job of a particular boss state
		}

		GameState& gameState = registry.gameStates.components[0];
		// Very coarse method of eliminating projectiles;
		// Should consider:
		// - Bouncing projectiles (if exists);
		// - Projectile dying effect (need particle system);
		if (registry.projectiles.has(one)) {
			handle_projectile_collision(one, other);
		}
		else if (registry.projectiles.has(other)) {
			handle_projectile_collision(other, one);
		}

		// if (registry.players.has(one) && registry.bosses.has(other)) {
		// 	handle_player_boss_collision(one, other, collision);
		// } else if (registry.players.has(other) && registry.bosses.has(one)) {
		// 	handle_player_boss_collision(other, one, collision);
		// }

		// if player touches boundary or spike, reset the game
		if (is_collision_between_player_and_boundary(one, other) || is_collision_between_player_and_spike(one, other) || player_harmful_collision(one, other)) {
			PlayerSystem::kill();
		}

	//		bolts break on spikes
		if (registry.bolts.has(one) && registry.spikes.has(other)) {
			registry.remove_all_components_of(one);
		}
		if (registry.bolts.has(other) && registry.spikes.has(one)) {
			registry.remove_all_components_of(other);
		}

		if (registry.players.has(one) && registry.ladders.has(other)) {
			handle_player_ladder_collision(one, other, step_seconds);
			player_ladder_collision = true;
		} else if (registry.players.has(other) && registry.ladders.has(one)) {
			handle_player_ladder_collision(other, one, step_seconds);
			player_ladder_collision = true;
		}

		if (registry.players.has(one) && registry.clockHoles.has(other)) {
			handle_player_clock_hole_collision();
		} else if (registry.players.has(other) && registry.clockHoles.has(one)) {
			handle_player_clock_hole_collision();
		}

		if (registry.physicsObjects.has(one) && registry.physicsObjects.has(other)) {
			handle_physics_collision(step_seconds, one, other, collision, groundedEntities);
		}
	}

	for (int i = 0; i < registry.physicsObjects.entities.size(); i++){
		Entity& entity = registry.physicsObjects.entities[i];

		if(!in(groundedEntities, entity.id())) {
			registry.onGrounds.remove(entity);
			Motion& motion = registry.motions.get(entity);
			apply_air_resistance(entity, motion, step_seconds);
		}
	}

	if (!player_ladder_collision) {
		registry.climbing.remove(registry.players.entities[0]);
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}
