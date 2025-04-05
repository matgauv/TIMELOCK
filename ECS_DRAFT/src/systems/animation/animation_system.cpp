#include "animation_system.hpp"
#include <cmath>
#include <iostream>

void AnimationSystem::init(GLFWwindow* window) {
	this->window = window;
}


void AnimationSystem::step(float elapsed_ms) {
	auto& animateRequest_registry = registry.animateRequests;
	auto& renderRequest_registry = registry.renderRequests;
    auto& time_control_registry = registry.timeControllables;

	TIME_CONTROL_STATE current_time_state = registry.gameStates.components[0].game_time_control_state;

	for (int i = 0; i < animateRequest_registry.size(); i++) {
		Entity entity = animateRequest_registry.entities[i];
		AnimateRequest &animateRequest = animateRequest_registry.components[i];

		// TODO: ensure animaiton collection contains animation id
		const AnimationConfig& animationConfig = this->animation_collections.at(animateRequest.used_animation);

		// Calculate time factor upon acceleration/deceleration
		float timeFactor = 1.0f;

        if (time_control_registry.has(entity)) {
          	timeFactor = time_control_registry.get(entity).target_time_control_factor;
        }

		updateTimer(animateRequest, animationConfig, timeFactor * elapsed_ms);

		// Update Render Request
		if (renderRequest_registry.has(entity)) {
			RenderRequest& renderRequest = renderRequest_registry.get(entity);
			if (renderRequest.used_texture != animationConfig.sprite_texture) {
				renderRequest.used_texture = animationConfig.sprite_texture;
			}

			// Calculate Frame
			int frame = min((int)(animateRequest.timer / animationConfig.ms_per_frame), animationConfig.frame_count - 1);
			float start_u_coord = (float)frame / (float)animationConfig.frame_count;
			animateRequest.tex_u_range = {start_u_coord, start_u_coord + 1./animationConfig.frame_count};
		}
	}
}

void AnimationSystem::updateTimer(AnimateRequest& animateRequest, const AnimationConfig& animationConfig, float elapsed_ms) {
	// TODO: INCORPORATE ACCELERATION & DECELERATION LOGIC

	if (animationConfig.animation_type == ANIMATION_TYPE_ID::CYCLE) {
		animateRequest.timer += elapsed_ms;
		if (animateRequest.timer >= animationConfig.duration_ms) {
			animateRequest.timer = fmod(animateRequest.timer, animationConfig.duration_ms);
		}
	}
	else if (animationConfig.animation_type == ANIMATION_TYPE_ID::FREEZE_ON_LAST) {
		if (animateRequest.timer + elapsed_ms <= animationConfig.duration_ms) {
			animateRequest.timer += elapsed_ms;
		}
	}
	else if (animationConfig.animation_type == ANIMATION_TYPE_ID::FREEZE_ON_RANDOM) {
		// Uninitialized
		if (animateRequest.timer <= 0.0) {
			animateRequest.timer = rand_float(1e-4f, animationConfig.duration_ms);
		}
	}
}

void AnimationSystem::late_step(float elapsed_ms) {
	(void)elapsed_ms;
}