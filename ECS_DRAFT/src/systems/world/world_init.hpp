#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "tinyECS/components.hpp"

void load_level(std::string descriptor_path);
void demo_level();

Entity create_player(vec2 position, vec2 scale);
Entity create_physics_object(vec2 position, vec2 scale, float mass);
Entity create_camera(vec2 position, vec2 scale);
Entity create_background(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_parallaxbackground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_foreground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_static_platform(vec2 position, vec2 scale, bool isBoundary, float angle = 0.0f);
Entity create_moving_platform(vec2 scale, std::vector<Path> movements);
Entity create_projectile(vec2 pos, vec2 size, vec2 velocity);
Entity create_bolt(vec2 pos, vec2 size, vec2 velocity);
Entity create_first_boss();
Entity create_spawnpoint(vec2 pos, vec2 size);
Entity create_spike(vec2 position, vec2 scale);
Entity create_breakable_static_platform(vec2 position, vec2 scale, bool should_break_instantly, float degrade_speed, float angle = 0.0f, bool is_time_controllable = false);
Entity create_time_controllable_breakable_static_platform(vec2 position, vec2 scale, bool should_break_instantly, float degrade_speed, float angle = 0.0f);
float getDistance(Motion& one, Motion& other);