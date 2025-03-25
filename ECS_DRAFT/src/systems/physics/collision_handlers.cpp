//
// Created by d on 2025-03-23.
//

#include "collision_handlers.h"
#include <iostream>

void handle_player_ladder_collision(Entity& player_entity, Entity& ladder_entity, int step_seconds) {
    Motion& player_motion = registry.motions.get(player_entity);
    Motion& ladder_motion = registry.motions.get(ladder_entity);

    float ladder_top = ladder_motion.position.y - ladder_motion.scale.y/2;
    float player_bottom = player_motion.position.y + player_motion.scale.y/2;

    if (player_bottom > ladder_top) {
        if (!registry.climbing.has(player_entity)) {
            registry.climbing.emplace(player_entity);
        }

        player_motion.velocity.x *= 0.999f;

        if (player_bottom < ladder_top + LADDER_TOP_OUT_THRESH) {
            player_motion.velocity.y = std::max(player_motion.velocity.y, 0.0f);
        }

    } else {
        if (registry.climbing.has(player_entity)) {
            if (player_motion.velocity.y < 0) {
                player_motion.velocity.y = 0;
            }
            registry.climbing.remove(player_entity);
        }
    }
}

// apply gravity to any bolt that is within dist_threshold
void drop_bolt_when_player_near(float dist_threshold) {
	Entity& player = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player);

	for (Entity& bolt : registry.bolts.entities)
	{
		Motion& bolt_motion = registry.motions.get(bolt);
		vec2 dist_to_bolt = abs(bolt_motion.position - player_motion.position);

		if (length(dist_to_bolt) < dist_threshold) {
			PhysicsObject& bolt_physics_object = registry.physicsObjects.get(bolt);
			bolt_physics_object.apply_gravity = true;
		}
	}
}


void handle_player_boss_collision(Entity& player_entity, Entity& boss_entity, Collision& collision) {
	Boss& boss = registry.bosses.get(boss_entity);

	boss.health -= PLAYER_ATTACK_DAMAGE;

	if (boss.health <= 0.f) {
		assert(registry.gameStates.components.size() <= 1);
		GameState& gameState = registry.gameStates.components[0];
		gameState.is_in_boss_fight = 0;
		registry.bosses.remove(boss_entity);
	}
}

void handle_projectile_collision(Entity& proj_entity, Entity& other_entity) {
	// If not harmful, don't remove projectile upon player collision
	if (!registry.harmfuls.has(proj_entity) /* && registry.players.has(other_entity)*/) {
		return;
	}

	// Upon colliding with physicsObjects (other projectiles, player) / platforms
	if (registry.physicsObjects.has(other_entity) || (registry.platforms.has(other_entity))) {
		// TODO: add more effects to killing projectiles, likely based on types
		registry.remove_all_components_of(proj_entity);
	}
}

void handle_player_attack_collision(Entity& player_entity, Entity& attack_entity, Collision& collision) {
	GameState& gameState = registry.gameStates.components[0];

	// TODO: make this part of logic consistent with WorldSystem::control_time
	if ((registry.harmfuls.has(attack_entity))) {
		assert(registry.gameStates.components.size() <= 1);
		GameState& gameState = registry.gameStates.components[0];
		gameState.game_running_state = GAME_RUNNING_STATE::OVER; // might not be necessary

		PlayerSystem::kill();
	}
}

void handle_player_breakable_collision(Entity& breakable_entity, float elapsed_ms) {
	Breakable& breakable = registry.breakables.get(breakable_entity);
	float health_decrease_factor = 1.0;
	if (registry.timeControllables.has(breakable_entity)) {
		TimeControllable& tc = registry.timeControllables.get(breakable_entity);
		health_decrease_factor = tc.target_time_control_factor;
	}

	if (breakable.should_break_instantly) {
		assert(registry.gameStates.components.size() <= 1);
		GameState& gameState = registry.gameStates.components[0];

		if (gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED) {
			WorldSystem::destroy_breakable_platform(breakable_entity);
			return;
		}
	}

	breakable.health -= health_decrease_factor * breakable.degrade_speed_per_ms * elapsed_ms;

	if (breakable.health <= 0) {
		WorldSystem::destroy_breakable_platform(breakable_entity);
	}
}

void handle_player_door_collision() {
  	// std::cout << "DOOR COLLISION" << std::endl;
	LevelState& ls = registry.levelStates.components[0];

	if (ls.curr_level_folder_name == ls.next_level_folder_name) return;

	ls.curr_level_folder_name = ls.next_level_folder_name;
	ls.shouldLoad = true;
}