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

	follow(camera_motion, player_motion.position);
}


void CameraSystem::late_step(float elapsed_ms) {
	(void)elapsed_ms;
}

void CameraSystem::follow(Motion& cam_motion, vec2 target) {
	vec2 displacement = target - cam_motion.position;
	const float dist = glm::length(displacement);

	if (dist < 1.0f) {
		// Snap camera to ideal location if within small range
		cam_motion.position = target;
		cam_motion.selfVelocity = { 0.f, 0.f };
	}
	else
	{
		// Gradual trace
		vec2 direction = displacement / dist;
		float speed = CAMERA_MAX_SPEED * std::clamp(dist / CAMERA_TRACE_RANGE, 0.0f, 1.0f);
		vec2 expected_vel = speed * direction;

		cam_motion.selfVelocity =
			cam_motion.selfVelocity * (1.0f - CAMERA_VEL_LERP_FACTOR) +
			expected_vel * CAMERA_VEL_LERP_FACTOR;
	}
}