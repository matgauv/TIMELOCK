#include "player_system.hpp"
#include "../particle/particle_system.hpp"
#include <iostream>

void PlayerSystem::init(GLFWwindow* window) {
	this->window = window;
}


void PlayerSystem::step(float elapsed_ms) {
	Player& player = registry.players.components[0];


	if (player.timer > 0) {
		// player timer never influenced by acceleration/deceleration
		player.timer -= elapsed_ms;
	}

	switch (player.state) {
		case PLAYER_STATE::DEAD:
			if (player.timer <= 0) {
				player_respawn();
			}
			break;
		case PLAYER_STATE::RESPAWNED:
			if (player.timer <= 0) {
				player.state = PLAYER_STATE::STANDING;
				player.timer = 0;
			}
			break;
		/*
		case PLAYER_STATE::COYOTE:
			if (player.timer <= 0) {
				player.state = PLAYER_STATE::STANDING;
				player.timer = 0;
			}
			break;
		*/
		default:
			//player.timer = 0;
			handle_player_motion();
			break;
	}

	if (player.jumping_valid_time >= 0) {
		player.jumping_valid_time -= elapsed_ms;

		if (player.jumping_valid_time <= 0) {
			player.jumping_valid_time = -1.0f;
		}
	}

	// Generate running particles
	const Entity player_entity = registry.players.entities[0];
	const vec2 player_velocity = registry.motions.get(player_entity).velocity;

	// TODO: check relative velocity
	if (JUMPING_VALID_TIME_MS - registry.players.components[0].jumping_valid_time < 25.0f
		&& abs(player_velocity.x) > DUST_SUMMONING_SPEED) {
		float speed_factor = min(1.0f, (abs(player_velocity.x) - DUST_SUMMONING_SPEED) / (PLAYER_MAX_WALKING_SPEED - DUST_SUMMONING_SPEED));
		float rand_threshold = lerpToTarget(speed_factor, 0.8f, 0.2f);

		float rand_factor = rand_float();
		if (rand_factor > rand_threshold) {
			ParticleSystem::spawn_particle(vec3{ 0.35f, 0.35f, 0.35f },
				random_sample_rectangle(registry.motions.get(player_entity).position + vec2{0.0f, PLAYER_SCALE.y * 0.35f}, { PLAYER_SCALE.x * 0.65f, 2.0f }),
				0.0f, vec2{ 1.5f, 1.5f } * (1.0f + 0.25f * rand_factor), rand_direction() * 20.0f, 1000.0, 0.8f, {50.0f, 200.0f});
		}
	}


	//std::cout << registry.onGrounds.has(registry.players.entities[0]);

	//Motion& motion = registry.motions.get(registry.players.entities[0]);

	//std::cout << "(" << motion.selfVelocity[0] << "," << motion.selfVelocity[1] << ") : (" << motion.appliedVelocity[0] << "," << motion.appliedVelocity[1] << ")" << std::endl;
}

void PlayerSystem::handle_player_motion() {
	const Entity entity = registry.players.entities[0];

	// Climbing overweights walking
	if (registry.climbing.has(entity)) {
		set_climbing();
	}
	else if (registry.walking.has(entity)) {
		set_walking();
	}
	else {
		set_standing();
	}
}

void PlayerSystem::set_jumping_validity(bool can_jump) {
	Player& player = registry.players.components[0];

	if (can_jump) {
		player.jumping_valid_time = JUMPING_VALID_TIME_MS;
	}
	else {
		// Currently setting to false implies a jump

		// Check for Coyote jump
		// Generate Coyote Jump particles
		const Entity player_entity = registry.players.entities[0];
		const vec2 player_velocity = registry.motions.get(player_entity).velocity;

		if (JUMPING_VALID_TIME_MS - player.jumping_valid_time > 25.0f) {
			const int particles_count = 3 + (rand() % 3);
			for (int i = 0; i < particles_count; i++) {
				ParticleSystem::spawn_particle(PARTICLE_ID::COYOTE_PARTICLES,
					random_sample_rectangle(registry.motions.get(player_entity).position + vec2{ 0.0f, PLAYER_SCALE.y * 0.35f }, { PLAYER_SCALE.x * 0.65f, 2.0f }),
					0.0f, vec2(10.0f) * rand_float(1.0f, 1.25f), 
					-0.5f * player_velocity, COYOTE_PARTICLES_DURATION, 1.0f, {10.0f, 0.0f});
			}
		}

		player.jumping_valid_time = -1.0f;
	}
}

bool PlayerSystem::can_jump() {
	return registry.players.components[0].jumping_valid_time > 0.0f;
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

void PlayerSystem::set_standing() {
	Entity& player_entity = registry.players.entities[0];

	if (registry.animateRequests.has(player_entity)) {
		registry.animateRequests.get(player_entity).used_animation = ANIMATION_ID::PLAYER_STANDING;
	}

	Player& player = registry.players.components[0];
	player.state = PLAYER_STATE::STANDING;
}

void PlayerSystem::set_walking() {
	Entity& player_entity = registry.players.entities[0];

	if (registry.animateRequests.has(player_entity)) {
		registry.animateRequests.get(player_entity).used_animation = ANIMATION_ID::PLAYER_WALKING;
	}

	Player& player = registry.players.components[0];
	player.state = PLAYER_STATE::WALKING;
}

void PlayerSystem::set_climbing() {
	Entity& player_entity = registry.players.entities[0];
	
	if (registry.animateRequests.has(player_entity)) {
		const Climbing& climbing = registry.climbing.get(player_entity);
		const Motion& motion = registry.motions.get(player_entity);
		registry.animateRequests.get(player_entity).used_animation = 
			(glm::length(motion.velocity) > 2.0f) ? ANIMATION_ID::PLAYER_CLIMB : ANIMATION_ID::PLAYER_CLIMB_FREEZE;
	}

	Player& player = registry.players.components[0];
	player.state = PLAYER_STATE::CLIMB;
}

void PlayerSystem::set_coyote() {
	Entity& player_entity = registry.players.entities[0];

	if (registry.animateRequests.has(player_entity)) {
		registry.animateRequests.get(player_entity).used_animation = ANIMATION_ID::PLAYER_COYOTE;
	}

	Player& player = registry.players.components[0];
	player.state = PLAYER_STATE::COYOTE;
	player.timer = COYOTE_JUMP_DURATION;
}

void PlayerSystem::set_direction(bool is_left) {
	Entity& player_entity = registry.players.entities[0];

	if (registry.renderRequests.has(player_entity)) {
		registry.renderRequests.get(player_entity).flipped = is_left;
	}
}