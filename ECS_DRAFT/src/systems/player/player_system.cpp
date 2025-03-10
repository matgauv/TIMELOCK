#include "player_system.hpp"
#include <iostream>

void PlayerSystem::init(GLFWwindow* window) {
	this->window = window;
}


void PlayerSystem::step(float elapsed_ms) {
	Player& player = registry.players.components[0];


	if (player.timer > 0) {
		// player timer never influenced by acceleration/deceleration
		player.timer -= elapsed_ms;

		if (player.timer <= 0) {
			if (player.state == PLAYER_STATE::DEAD) {
				player_respawn();
			}
			else if (player.state == PLAYER_STATE::RESPAWNED) {
				// TODO: potentially expand this to some function for setting the player ready
				player.state = PLAYER_STATE::ALIVE;
				player.timer = 0;
			}
			else {
				player.timer = 0;
			}
		}
	}

	Motion& motion = registry.motions.get(registry.players.entities[0]);

	//std::cout << "(" << motion.selfVelocity[0] << "," << motion.selfVelocity[1] << ") : (" << motion.appliedVelocity[0] << "," << motion.appliedVelocity[1] << ")" << std::endl;
}

void PlayerSystem::late_step(float elapsed_ms) {
}

void PlayerSystem::set_spawnpoint(vec2 spawnpoint) {
	registry.players.components[0].spawn_point = spawnpoint;
}

void PlayerSystem::kill() {
	Player& player = registry.players.components[0];

	if (player.state == PLAYER_STATE::DEAD) {
		return;
	}

	player.state = PLAYER_STATE::DEAD;
	player.timer = DEAD_REVIVE_TIME_MS;

	Entity e = registry.players.entities[0];
	
	Motion& motion = registry.motions.get(e);
	motion.velocity = vec2{ 0.f, 0.f };

	// Clear Physics components
	if (registry.walking.has(e)) {
		registry.walking.remove(e);
	}

	if (registry.onGrounds.has(e)) {
		registry.onGrounds.remove(e);
	}

	AnimateRequest& animateRequest = registry.animateRequests.get(e);
	animateRequest.timer = 0.0;
	animateRequest.used_animation = ANIMATION_ID::PLAYER_KILL;
}

void PlayerSystem::player_respawn() {
	Player& player = registry.players.components[0];
	player.timer = DEAD_REVIVE_TIME_MS;
	player.state = PLAYER_STATE::RESPAWNED;

	Entity e = registry.players.entities[0];
	
	Motion& motion = registry.motions.get(e);
	motion.position = player.spawn_point;
	motion.velocity = vec2{0.f, 0.f};
	motion.cache_invalidated = true;

	AnimateRequest& animateRequest = registry.animateRequests.get(e);
	animateRequest.timer = 0.0;
	animateRequest.used_animation = ANIMATION_ID::PLAYER_RESPAWN;
}

void PlayerSystem::set_standing(bool is_left) {
	Entity& player_entity = registry.players.entities[0];

	if (registry.animateRequests.has(player_entity)) {
		registry.animateRequests.get(player_entity).used_animation = ANIMATION_ID::PLAYER_STANDING;
	}
}

void PlayerSystem::set_walking(bool is_left) {
	Entity& player_entity = registry.players.entities[0];
	// TODO: this might not be the best approach to flip Player sprite;
	// Could potentially isolate all Player-related properties into Player component, and update Player system accordingly
	if (registry.renderRequests.has(player_entity)) {
		registry.renderRequests.get(player_entity).flipped = is_left;
	}

	if (registry.animateRequests.has(player_entity)) {
		registry.animateRequests.get(player_entity).used_animation = ANIMATION_ID::PLAYER_WALKING;
	}
}