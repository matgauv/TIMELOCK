#include "animation_system.hpp"
#include <cmath>
#include "../world/world_init.hpp"
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




		/*
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
		}*/
	}
	update_rolling_things(elapsed_ms);

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
		if (animateRequest.timer + elapsed_ms <= (animationConfig.duration_ms + animationConfig.ms_per_frame)) {
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
	auto& animateRequest_registry = registry.animateRequests;
	auto& renderRequest_registry = registry.renderRequests;

	for (int i = 0; i < animateRequest_registry.size(); i++) {
		Entity entity = animateRequest_registry.entities[i];
		AnimateRequest& animateRequest = animateRequest_registry.components[i];

		const AnimationConfig& animationConfig = this->animation_collections.at(animateRequest.used_animation);



		// Update Render Request
		if (renderRequest_registry.has(entity)) {
			RenderRequest& renderRequest = renderRequest_registry.get(entity);
			if (renderRequest.used_texture != animationConfig.sprite_texture) {
				renderRequest.used_texture = animationConfig.sprite_texture;
			}

			// Calculate Frame
			int frame = min((int)(animateRequest.timer / animationConfig.ms_per_frame), animationConfig.frame_count - 1);
			float start_u_coord = (float)frame / (float)animationConfig.frame_count;
			animateRequest.tex_u_range = { start_u_coord, start_u_coord + 1. / animationConfig.frame_count };

			if (registry.cutScenes.has(entity))
			{
				CutScene& cutscene = registry.cutScenes.get(entity);

				if (animateRequest.used_animation == ANIMATION_ID::OUTRO_1 && frame == animationConfig.frame_count - 1) {
					animateRequest.timer = 0.0f;
					animateRequest.used_animation = ANIMATION_ID::OUTRO_2;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::OUTRO_2 && frame == animationConfig.frame_count - 1) {
					animateRequest.timer = 0.0f;
					animateRequest.used_animation = ANIMATION_ID::OUTRO_3;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::OUTRO_3 && frame == animationConfig.frame_count - 1) {
					animateRequest.timer = 0.0f;
					animateRequest.used_animation = ANIMATION_ID::OUTRO_4;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::OUTRO_4 && frame == animationConfig.frame_count - 1) {
					cutscene.state = 0;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::INTRO_1 && frame == animationConfig.frame_count - 1) {
					animateRequest.timer = 0.0f;
					animateRequest.used_animation = ANIMATION_ID::INTRO_2;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::INTRO_2 && frame == animationConfig.frame_count - 1) {
					animateRequest.timer = 0.0f;
					animateRequest.used_animation = ANIMATION_ID::INTRO_3;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::INTRO_3 && frame == animationConfig.frame_count - 1) {
					animateRequest.timer = 0.0f;
					animateRequest.used_animation = ANIMATION_ID::INTRO_4;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::INTRO_4 && frame == animationConfig.frame_count - 1) {
					animateRequest.timer = 0.0f;
					animateRequest.used_animation = ANIMATION_ID::INTRO_5;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::INTRO_5 && frame == animationConfig.frame_count - 1) {
					animateRequest.timer = 0.0f;
					animateRequest.used_animation = ANIMATION_ID::INTRO_6;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::INTRO_6 && frame == animationConfig.frame_count - 1) {
					animateRequest.timer = 0.0f;
					animateRequest.used_animation = ANIMATION_ID::INTRO_7;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::INTRO_7 && frame == animationConfig.frame_count - 1) {
					animateRequest.timer = 0.0f;
					animateRequest.used_animation = ANIMATION_ID::INTRO_8;
				}
				else if (animateRequest.used_animation == ANIMATION_ID::INTRO_8 && frame == animationConfig.frame_count - 1) {
					cutscene.state = 0;
				}
			}
		}
	}
}

// TODO: sorry for cluttering the animation system with this, I just needed the animation config info...
void AnimationSystem::update_rolling_things(float elapsed_ms)
{
	// 66 frames total, check if frame has elapsed (reference animation data)
	// for each rolling thing:
	//		check if any platforms need to be spawned
	//		   if yes, spawn platform at x offset stored in the json data from the left of the image (relative to motion.position, which stores the center)
	//		   track spawned platforms
	//     for spawned platforms:
	//         increment "frames alive"
	//         if frames alive > frames to last, destroy platform

	for (Entity& rt : registry.rollingThings.entities) {
		Motion& rtMotion = registry.motions.get(rt);
		AnimateRequest& animateRequest = registry.animateRequests.get(rt);
		AnimationConfig animationConfig = this->animation_collections.at(animateRequest.used_animation);
		RollingThing& rThing = registry.rollingThings.get(rt);

		int frame = min((int)(animateRequest.timer / animationConfig.ms_per_frame), animationConfig.frame_count);

		if (frame != rThing.current_frame)
		{
			rThing.current_frame = frame;

			// first, check if we need to swap between the two animations (spritesheet is larger than max texture size)
			if (animateRequest.used_animation == ANIMATION_ID::ROLLING_THING_1 && rThing.current_frame == animationConfig.frame_count) {
				animateRequest.used_animation = ANIMATION_ID::ROLLING_THING_2;
				animateRequest.timer = 0.f;
			} else if (animateRequest.used_animation == ANIMATION_ID::ROLLING_THING_2 && rThing.current_frame == animationConfig.frame_count) {
				animateRequest.used_animation = ANIMATION_ID::ROLLING_THING_3;
				animateRequest.timer = 0.f;
			} else if (animateRequest.used_animation == ANIMATION_ID::ROLLING_THING_3 && rThing.current_frame == animationConfig.frame_count) {
				animateRequest.used_animation = ANIMATION_ID::ROLLING_THING_4;
				animateRequest.timer = 0.f;
			} else if (animateRequest.used_animation == ANIMATION_ID::ROLLING_THING_4 && rThing.current_frame == animationConfig.frame_count){
				animateRequest.used_animation = ANIMATION_ID::ROLLING_THING_1;
				animateRequest.timer = 0.f;
			}

			// TOOO more elegant way of getting this to work...
			if (animateRequest.used_animation == ANIMATION_ID::ROLLING_THING_2) {
				frame += animationConfig.frame_count;
			} else if (animateRequest.used_animation == ANIMATION_ID::ROLLING_THING_3)
			{
				frame += (animationConfig.frame_count*2);
			} else if (animateRequest.used_animation == ANIMATION_ID::ROLLING_THING_4)
			{
				frame += (17 * 3);
			}


			// for all the current rolling platforms, check if need to delete, if not move down
			for (unsigned int platform_id : rThing.platforms) {
				Entity platform_entity = Entity(platform_id);
				RollingPlatform& platform = registry.rollingPlatforms.get(platform_entity);
				platform.frames_left--;
				if (platform.frames_left <= 0) {
					rThing.platforms.erase(std::remove(rThing.platforms.begin(), rThing.platforms.end(), platform_id), rThing.platforms.end());
					registry.remove_all_components_of(platform_entity);
				}
			}

			// check json if we need to create new platforms this frame
			std::string frame_num_str = std::to_string(frame);
			std::vector<int>& to_spawn = registry.rolling_thing_data[frame_num_str];

			float y_vel = (ROLLING_PLATFORM_SPEED / animationConfig.ms_per_frame) * 1000.f;

			for (auto& spawn_x : to_spawn) {
				float x_pos = spawn_x + (rtMotion.position.x - rtMotion.scale.x / 2);
				float y_pos = (rtMotion.position.y - rtMotion.scale.y / 2) + 62.0f;
				Entity spawned_platform = create_rolling_platform(vec2{x_pos, y_pos}, ROLLING_PLATFORM_SIZE, y_vel);
				RollingPlatform& rp = registry.rollingPlatforms.emplace(spawned_platform);
				rp.frames_left = ROLLING_PLATFORM_FRAMES_ALIVE;
				rThing.platforms.push_back(spawned_platform.id());
			}


		}
	}
}