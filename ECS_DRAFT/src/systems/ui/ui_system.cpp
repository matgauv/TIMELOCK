#include "ui_system.hpp"

void UiSystem::init(GLFWwindow* window) {
	this->window = window;
}


void UiSystem::step(float elapsed_ms) {
	deceleration_bar_step(elapsed_ms);
}


void UiSystem::late_step(float elapsed_ms) {

}

void UiSystem::deceleration_bar_step(float elapsed_ms) {
	// Decel Bar
	if (registry.decelerationBars.size() > 0) {
		const Entity decelbar_entity = registry.decelerationBars.entities[0];
		DecelerationBar& decelbar = registry.decelerationBars.components[0];

		// update motion
		Motion& decelbar_motion = registry.motions.get(decelbar_entity);
		const Motion& player_motion = registry.motions.get(registry.players.entities[0]);
		const float player_speed = glm::length(player_motion.velocity);

		vec2 target_pos = player_motion.position + DECEL_BAR_OFFSET;
		if (player_speed > 1e-4) {
			target_pos -= glm::normalize(player_motion.velocity) * std::clamp(player_speed/ PLAYER_MAX_FALLING_SPEED, 0.0f, 1.0f) * DECEL_BAR_DEVIATION;
		}

		const float distance = glm::length(decelbar_motion.position - target_pos);
		if (distance< 1.0f || distance > 5.0f*DECEL_BAR_DEVIATION) {
			decelbar_motion.position = target_pos;
		}
		else {
			decelbar_motion.position = 0.5f * decelbar_motion.position + 0.5f * target_pos;
		}

		// update animation
		AnimateRequest& decelbar_anim = registry.animateRequests.get(decelbar_entity);
		vec3& decelbar_color = registry.colors.get(decelbar_entity);
		const GameState& gameState = registry.gameStates.components[0];
		
		float expected_width = DECEL_BAR_WIDTH;
		vec3 expected_color = vec3(1.0f);
		if (gameState.decelerate_timer >= 0.0 && gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED) {
			// Deceleration Activated
			decelbar_anim.timer = std::clamp((DECELERATION_DURATION_MS - gameState.decelerate_timer) / DECELERATION_DURATION_MS, 0.0f, 1.0f);
		}
		else if (gameState.decelerate_timer < -1e-8) {
			// Deceleration Cooldown
			decelbar_anim.timer = std::clamp((1.0f - (DECELERATION_COOLDOWN_MS + gameState.decelerate_timer) / DECELERATION_COOLDOWN_MS), 0.0f, 1.0f);
			expected_color = vec3(0.45f);
		} else {
			expected_width = 0.0f;
		}

		// interpolate color
		if (glm::length(decelbar_color - expected_color) < 0.05f) {
			decelbar_color = expected_color;
		}
		else {
			decelbar_color = 0.9f * decelbar_color + 0.1f * expected_color;
		}

		// interpolate width
		if (abs(decelbar_motion.scale.x - expected_width) < 4.0f) {
			decelbar_motion.scale.x = expected_width;
		}
		else {
			decelbar_motion.scale.x = 0.8f * decelbar_motion.scale.x + 0.2f * expected_width;
		}
	}
}