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
		{ANIMATION_ID::PLAYER_WALKING, AnimationConfig(TEXTURE_ASSET_ID::PLAYER_WALKING_V1, ANIMATION_TYPE_ID::CYCLE , 13, 300.0)},
		{ANIMATION_ID::PLAYER_STANDING, AnimationConfig(TEXTURE_ASSET_ID::PLAYER_STANDING_V1, ANIMATION_TYPE_ID::CYCLE , 1, 450.0)},
		{ANIMATION_ID::PLAYER_KILL, AnimationConfig(TEXTURE_ASSET_ID::PLAYER_KILL, ANIMATION_TYPE_ID::FREEZE_ON_LAST , 4, DEAD_REVIVE_TIME_MS)},
		{ANIMATION_ID::PLAYER_RESPAWN, AnimationConfig(TEXTURE_ASSET_ID::PLAYER_RESPAWN, ANIMATION_TYPE_ID::FREEZE_ON_LAST , 4, DEAD_REVIVE_TIME_MS)},
		{ANIMATION_ID::SPAWNPOINT_ACTIVATE, AnimationConfig(TEXTURE_ASSET_ID::SPAWNPOINT_ACTIVATE, ANIMATION_TYPE_ID::FREEZE_ON_LAST , 6, SPAWNPOINT_CHARGE_TIME_MS)},
		{ANIMATION_ID::SPAWNPOINT_DEACTIVATE, AnimationConfig(TEXTURE_ASSET_ID::SPAWNPOINT_DEACTIVATE, ANIMATION_TYPE_ID::FREEZE_ON_LAST , 6, SPAWNPOINT_CHARGE_TIME_MS)},
		{ANIMATION_ID::SPAWNPOINT_REACTIVATE, AnimationConfig(TEXTURE_ASSET_ID::SPAWNPOINT_REACTIVATE, ANIMATION_TYPE_ID::FREEZE_ON_LAST , 6, SPAWNPOINT_CHARGE_TIME_MS)},
		{ANIMATION_ID::BOSS_ONE_IDLE, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_IDLE_LEFT, ANIMATION_TYPE_ID::CYCLE, 1, 450.0)},
		{ANIMATION_ID::BOSS_ONE_WALK, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_WALK, ANIMATION_TYPE_ID::CYCLE, 10, 750.0)},
		{ANIMATION_ID::BOSS_ONE_EXHAUSTED, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_EXHAUSTED, ANIMATION_TYPE_ID::CYCLE, 1, 450.0)},
		{ANIMATION_ID::BOSS_ONE_DAMAGED, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_DAMAGED, ANIMATION_TYPE_ID::CYCLE, 1, 450.0)},
		{ANIMATION_ID::BOSS_ONE_RECOVERED, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_RECOVERED, ANIMATION_TYPE_ID::CYCLE, 1, 450.0)},
		{ANIMATION_ID::BOSS_ONE_PROJECTILE_LEFT, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_PROJECTILE_LEFT, ANIMATION_TYPE_ID::CYCLE, 1, 450.0)},
		{ANIMATION_ID::BOSS_ONE_PROJECTILE_RIGHT, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_PROJECTILE_RIGHT, ANIMATION_TYPE_ID::CYCLE, 1, 450.0)},
		{ANIMATION_ID::BOSS_ONE_DELAYED_PROJECTILE, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_DELAYED_PROJECTILE, ANIMATION_TYPE_ID::CYCLE, 1, 450.0)},
		{ANIMATION_ID::BOSS_ONE_DASH, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_DASH_LEFT_RIGHT, ANIMATION_TYPE_ID::CYCLE, 10, 400.0)},
		{ANIMATION_ID::BOSS_ONE_GROUND_SLAM_INIT, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_GROUND_SLAM_FOLLOW, ANIMATION_TYPE_ID::CYCLE, 1, 250.0)},
		{ANIMATION_ID::BOSS_ONE_GROUND_SLAM_RISE, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_GROUND_SLAM_RISE, ANIMATION_TYPE_ID::FREEZE_ON_LAST, 8, 250.0)},
		{ANIMATION_ID::BOSS_ONE_GROUND_SLAM_FOLLOW, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_GROUND_SLAM_FOLLOW, ANIMATION_TYPE_ID::CYCLE, 1, 450.0)},
		{ANIMATION_ID::BOSS_ONE_GROUND_SLAM_FALL, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_GROUND_SLAM_FALL, ANIMATION_TYPE_ID::FREEZE_ON_LAST, 13, 250.0)},
		{ANIMATION_ID::BOSS_ONE_GROUND_SLAM_LAND, AnimationConfig(TEXTURE_ASSET_ID::BOSS_ONE_GROUND_SLAM_LAND, ANIMATION_TYPE_ID::FREEZE_ON_LAST, 9, 250.0)}
	};

	void updateTimer(AnimateRequest &animateRequest, const AnimationConfig& animationConfig, float elapsed_ms);
};