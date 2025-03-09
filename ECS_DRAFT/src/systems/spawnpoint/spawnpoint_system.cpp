#include "spawnpoint_system.hpp"
#include <iostream>

void SpawnPointSystem::init(GLFWwindow* window) {
	this->window = window;
}

void SpawnPointSystem::step(float elapsed_ms) {
	Entity player_entity = registry.players.entities[0];
	const vec2& player_pos = registry.motions.get(player_entity).position;

	for (Entity e : registry.spawnPoints.entities) {
		const vec2& spawnpoint_pos = registry.motions.get(e).position;

		// Iterate until first successful update
		if (glm::length(spawnpoint_pos - player_pos) <= SPAWNPOINT_DETECTION_RANGE) {
			if (update_spawnpoints(e)) {
				return;
			}
		}
	}
}


void SpawnPointSystem::late_step(float elapsed_ms) {
}


// Update spawn point information;
bool SpawnPointSystem::update_spawnpoints(Entity active_spawnpoint_entity) {
	SpawnPoint& active_spawnpoint = registry.spawnPoints.get(active_spawnpoint_entity);

	// Already activated; no actions needed
	if (active_spawnpoint.state == SPAWN_POINT_STATE::ACTIVE) {
		return false;
	}

	if (active_spawnpoint.state == SPAWN_POINT_STATE::UNVISITED) {
		activate_spawnpoint(active_spawnpoint_entity);
	} else {
		reactivate_spawnpoint(active_spawnpoint_entity);
	}

	for (int i = 0; i < registry.spawnPoints.size(); i++) {
		Entity e = registry.spawnPoints.entities[i];
		if (e == active_spawnpoint_entity) {
			continue;
		}

		SpawnPoint &spawnPoint = registry.spawnPoints.components[i];

		// deactivate spawn points that are already active
		if (spawnPoint.state == SPAWN_POINT_STATE::ACTIVE) {
			deactivate_spawnpoint(e);
		}
	}

	return true;
}

// Activate unvisited spawn point
void SpawnPointSystem::activate_spawnpoint(Entity entity) {
	registry.spawnPoints.get(entity).state = SPAWN_POINT_STATE::ACTIVE;

	AnimateRequest& animRequest = registry.animateRequests.emplace(entity);
	animRequest.used_animation = ANIMATION_ID::SPAWNPOINT_ACTIVATE;

	// Set spawn point
	set_player_spawnpoint(entity);
}

// Deactivate an activated spawn point if other spawn points are active
void SpawnPointSystem::deactivate_spawnpoint(Entity entity) {
	registry.spawnPoints.get(entity).state = SPAWN_POINT_STATE::VISITED;

	// Should either have activate or reactivate animation
	if (!registry.animateRequests.has(entity)) {
		return;
	}

	AnimateRequest& animRequest = registry.animateRequests.get(entity);
	animRequest.timer = max(0.0f, SPAWNPOINT_CHARGE_TIME_MS - animRequest.timer);
	animRequest.used_animation = ANIMATION_ID::SPAWNPOINT_DEACTIVATE;
}

// Reactivate a deactivated spawn point
void SpawnPointSystem::reactivate_spawnpoint(Entity entity) {
	registry.spawnPoints.get(entity).state = SPAWN_POINT_STATE::ACTIVE;

	// Should have deactivate animation
	if (!registry.animateRequests.has(entity)) {
		return;
	}

	AnimateRequest& animRequest = registry.animateRequests.get(entity);
	animRequest.timer = max(SPAWNPOINT_CHARGE_TIME_MS - animRequest.timer, 0.0f);
	animRequest.used_animation = ANIMATION_ID::SPAWNPOINT_REACTIVATE;

	set_player_spawnpoint(entity);
}

void SpawnPointSystem::set_player_spawnpoint(Entity spawnpoint_entity) {
	registry.players.components[0].spawn_point =
		registry.motions.get(spawnpoint_entity).position +
		vec2{ 0.0f, SPAWNPOINT_SCALE[1] - PLAYER_SCALE[1] - 0.1f };
}