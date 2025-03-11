#pragma once

#include <json.hpp>

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "tinyECS/components.hpp"

using namespace nlohmann;

void load_level(std::string descriptor_path);
void demo_level();

Entity create_player(vec2 position, vec2 scale);
Entity create_physics_object(vec2 position, vec2 scale, float mass);
Entity create_camera(vec2 position, vec2 scale);
Entity create_background(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_parallaxbackground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_foreground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_levelground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_static_platform(vec2 position, vec2 scale, json& tile_id_array, int stride);
Entity create_level_boundary(vec2 position, vec2 scale);
Entity create_world_boundary(vec2 position, vec2 scale);
Entity create_moving_platform(vec2 scale, std::vector<Path> movements, vec2 initial_position, json& tile_id_array, int stride);
Entity create_projectile(vec2 pos, vec2 size, vec2 velocity);
Entity create_bolt(vec2 pos, vec2 size, vec2 velocity, bool default_gravity);
Entity create_first_boss();
Entity create_spawnpoint(vec2 pos, vec2 size);
Entity create_spike(vec2 position, vec2 scale, json tile_id_array, int stride);
Entity create_partof(vec2 position, vec2 scale, json tile_id_array, int stride);
Entity create_breakable_static_platform(vec2 position, vec2 scale, bool should_break_instantly, float degrade_speed, bool is_time_controllable, json& tile_id_array, int stride);
Entity create_time_controllable_breakable_static_platform(vec2 position, vec2 scale, bool should_break_instantly, float degrade_speed, json& tile_id_array, int stride);
float getDistance(const Motion& one, const Motion& other);
Entity create_canon_tower(vec2 pos);
int get_tile_index(int pos_x, int pos_y, int offset_x, int offset_y, int stride);
Entity create_tutorial_text(vec2 position, vec2 size, TEXTURE_ASSET_ID texture_id);
