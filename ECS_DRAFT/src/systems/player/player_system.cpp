#include "player_system.hpp"

void PlayerSystem::init(GLFWwindow* window) {
	this->window = window;
}


void PlayerSystem::step(float elapsed_ms) {
	Player& player = registry.players.components[0];

	if (player.timer > 0) {
		// player timer never influenced by acceleration/deceleration
		player.timer -= elapsed_ms;
	}

	if (player.timer <= 0) {
		if (player.state == PLAYER_STATE::DEAD) {
			player_respawn();
		}
		else {
			player.timer = 0;
		}
	}
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
	motion.selfVelocity *= 0;
}

void PlayerSystem::player_respawn() {
	Player& player = registry.players.components[0];
	player.timer = DEAD_REVIVE_TIME_MS;
	player.state = PLAYER_STATE::ALIVE;

	Entity e = registry.players.entities[0];
	Motion& motion = registry.motions.get(e);
	motion.position = player.spawn_point;
}