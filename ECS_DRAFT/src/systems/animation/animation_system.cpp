#include "animation_system.hpp"
#include <cmath>
#include <iostream>

void AnimationSystem::init(GLFWwindow* window) {
	this->window = window;
}


void AnimationSystem::step(float elapsed_ms) {
	auto& animateRequest_registry = registry.animateRequests;
	auto& renderRequest_registry = registry.renderRequests;

	for (int i = 0; i < animateRequest_registry.size(); i++) {
		Entity entity = animateRequest_registry.entities[i];
		AnimateRequest &animateRequest = animateRequest_registry.components[i];

		// TODO: ensure animaiton collection contains animation id
		const AnimationConfig& animationConfig = this->animation_collections.at(animateRequest.used_animation);

		updateTimer(animateRequest, animationConfig, elapsed_ms);

		if (renderRequest_registry.has(entity)) {
			RenderRequest& renderRequest = renderRequest_registry.get(entity);
			renderRequest.used_texture = animationConfig.sprite_texture;

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
}

void AnimationSystem::late_step(float elapsed_ms) {
	(void)elapsed_ms;
}