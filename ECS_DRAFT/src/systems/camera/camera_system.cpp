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
	const GameState& gameState = registry.gameStates.components[0];
	Camera& cam = registry.cameras.components[0];

	if (gameState.is_in_boss_fight) {
		const LevelState& levelState = registry.levelStates.components[0];
		follow(camera_motion, levelState.dimensions * 0.5f, 
			vec2{WINDOW_WIDTH_PX / levelState.dimensions.x, WINDOW_HEIGHT_PX/levelState.dimensions.y });
	}
	else {
		// assert(registry.players.entities.size() == 1);
		Entity player_entity = registry.players.entities[0];
		if (!registry.motions.has(player_entity)) {
			return;
		}

		const Player& player = registry.players.components[0];
		const Motion& player_motion = registry.motions.get(player_entity);
		if (player.state == PLAYER_STATE::RESPAWNED) {
			// Force focus
			reset(camera_motion, player.spawn_point);
		}
		else {
			// Create horizontal spacing along motion distance

			// Check player facing direction
			float facing_dir = (registry.renderRequests.get(player_entity).flipped ? -1.0f : 1.0f);
			float offset =
				std::clamp((facing_dir * player_motion.velocity[0]) / PLAYER_MAX_WALKING_SPEED, 0.0f, 1.0f) *
				CAMERA_SCREEN_SPACING_FOR_MOTION_RATIO * WINDOW_WIDTH_PX *
				facing_dir;

			// If in the same direction & smaller offset, do not force-set and wait for natural decay
			if ((offset * cam.horizontal_offset < 0) || (abs(offset) > abs(cam.horizontal_offset))) {
				cam.horizontal_offset = (1.0f - (CAMERA_VEL_LERP_FACTOR * 0.5f)) * cam.horizontal_offset + CAMERA_VEL_LERP_FACTOR * 0.5f * offset;
			}

			follow(camera_motion, player_motion.position + vec2{ cam.horizontal_offset, 0.0f });
		}
	}

	// Update offset
	if (abs(cam.horizontal_offset) < 0.5f) {
		cam.horizontal_offset = 0.0f;
	}
	else {
		cam.horizontal_offset *= (1.0f - CAMERA_VEL_LERP_FACTOR * 0.1f);
	}
}


void CameraSystem::late_step(float elapsed_ms) {
	(void)elapsed_ms;
}

void CameraSystem::follow(Motion& cam_motion, vec2 target, vec2 scale) {
	target = restricted_boundary_position(target, cam_motion.scale);
	vec2 displacement = target - cam_motion.position;
	const float dist = glm::length(displacement);

	if (dist < 0.5f) {
		// Snap camera to ideal location if within small range
		cam_motion.position = target;
		cam_motion.velocity = { 0.f, 0.f };
	}
	else
	{
		// Gradual trace
		vec2 direction = displacement / dist;
		float speed = CAMERA_MAX_SPEED * std::clamp(dist / CAMERA_TRACE_RANGE, 0.0f, 1.0f);
		vec2 expected_vel = speed * direction;


		// Reduces overshoot
		if (glm::length(expected_vel - cam_motion.velocity) <= CAMERA_VELOCITY_CLAMP_THRESHOLD) {
			cam_motion.velocity = expected_vel;
		}
		else {
			cam_motion.velocity =
				cam_motion.velocity * (1.0f - CAMERA_VEL_LERP_FACTOR) +
				expected_vel * CAMERA_VEL_LERP_FACTOR;
		}
	}
}

void CameraSystem::reset(Motion& cam_motion, vec2 target, vec2 scale) {
	cam_motion.position = restricted_boundary_position(target, cam_motion.scale);
	cam_motion.velocity = { 0.f, 0.f };
	cam_motion.scale = scale;

	registry.cameras.components[0].horizontal_offset = 0.0f;
}

vec2 CameraSystem::get_camera_offsets(vec2 camera_scale) {
	float x_scale = std::clamp(camera_scale[0], CAMERA_MIN_SCALING, CAMERA_MAX_SCALING);
	float y_scale = std::clamp(camera_scale[1], CAMERA_MIN_SCALING, CAMERA_MAX_SCALING);

	// Note that this might not work for the case when scene is smaller than camera front screen
	const float cam_x_offset = 0.5f * CAMERA_DEFAULT_SCALING * WINDOW_WIDTH_PX / x_scale;
	const float cam_y_offset = 0.5f * CAMERA_DEFAULT_SCALING * WINDOW_HEIGHT_PX / y_scale;

	return vec2{ cam_x_offset, cam_y_offset };
}

vec2 CameraSystem::restricted_boundary_position(vec2 raw_target, vec2 camera_scale = { 1.0, 1.0 }) {
	const LevelState& level_state = registry.levelStates.components[0];

	vec2 camera_offsets = get_camera_offsets(camera_scale);
	camera_offsets *= CAMERA_BOUNDARY_PADDING;

	float refined_x = (
		(level_state.dimensions[0] - camera_offsets[0] > camera_offsets[0]) ?
		std::clamp(raw_target[0], camera_offsets[0], level_state.dimensions[0] - camera_offsets[0]) :
		level_state.dimensions[0] * 0.5f);

	float refined_y = (
		(level_state.dimensions[1] - camera_offsets[1] > camera_offsets[1]) ?
		std::clamp(raw_target[1], camera_offsets[1], level_state.dimensions[1] - camera_offsets[1]) :
		level_state.dimensions[1] * 0.5f);

	return vec2{ refined_x, refined_y };
}