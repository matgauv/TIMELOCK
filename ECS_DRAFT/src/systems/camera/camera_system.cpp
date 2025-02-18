#include "camera_system.hpp"

void CameraSystem::init(GLFWwindow* window) {
	this->window = window;
}


void CameraSystem::step(float elapsed_ms) {
	// Let Camera follow targets; can be Player/Boss
	if (registry.cameras.entities.size() < 1) {
		return;
	}

	Entity camera_entity = registry.cameras.entities[0];
	if (!registry.motions.has(camera_entity)) {
		return;
	}

	Motion& camera_motion = registry.motions.get(camera_entity);

	// TODO: if target is player
	// assert(registry.players.entities.size() == 1);
	Entity player_entity = registry.players.entities[0];
	if (!registry.motions.has(player_entity)) {
		return;
	}

	const Motion& player_motion = registry.motions.get(player_entity);

	follow(camera_motion, player_motion.position, elapsed_ms);
}


void CameraSystem::late_step(float elapsed_ms) {
	(void)elapsed_ms;
}

void CameraSystem::follow(Motion& cam_motion, vec2 target, float elapsed_ms) {
	cam_motion.position = target;
}