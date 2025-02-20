#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "systems/ISystem.hpp"
#include <map>

struct AnimationConfig {
	const TEXTURE_ASSET_ID sprite_texture;
	const ANIMATION_TYPE_ID animation_type;
	const int frame_count;
	const float duration_ms;
	const float ms_per_frame;

	AnimationConfig(TEXTURE_ASSET_ID sprite_texture, ANIMATION_TYPE_ID animation_type, int frame_count, float duration_ms) :
		sprite_texture(sprite_texture), animation_type(animation_type), 
		frame_count(frame_count), duration_ms(duration_ms), ms_per_frame(duration_ms/frame_count)
	{}
};

// Animation System that updates uv of texture to render
class AnimationSystem : public ISystem
{
public:
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;

	AnimationSystem()
	{
	}
private:
	GLFWwindow* window = nullptr;

	/* Animation Clip Collections :
	* Key:		Animation ID
	* Value:	Spritesheet texture id, type (cycle vs freeze on last), frame count, one cycle duration
	*/ 
	const std::unordered_map<ANIMATION_ID, AnimationConfig> animation_collections = {
		{ANIMATION_ID::PLAYER_WALKING, AnimationConfig(TEXTURE_ASSET_ID::SAMPLE_PLAYER_WALKING, ANIMATION_TYPE_ID::CYCLE , 4, 300.0)},
		{ANIMATION_ID::PLAYER_STANDING, AnimationConfig(TEXTURE_ASSET_ID::SAMPLE_PLAYER_STANDING, ANIMATION_TYPE_ID::CYCLE , 2, 450.0)}
	};

	void updateTimer(AnimateRequest &animateRequest, const AnimationConfig& animationConfig, float elapsed_ms);
};