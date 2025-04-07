#pragma once

#include <json.hpp>

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "tinyECS/components.hpp"
#include "systems/rendering/render_system.hpp"


using namespace nlohmann;

inline std::unordered_map<std::string, std::unique_ptr<Mesh>> mesh_cache;

// Creation methods
Entity create_player(vec2 position, vec2 scale);
Entity create_deceleration_bar(vec2 position);
Entity create_physics_object(vec2 position, vec2 scale, float mass);
Entity create_camera(vec2 position, vec2 scale);
Entity create_background(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_parallaxbackground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_foreground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_levelground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);
Entity create_static_platform(vec2 position, vec2 scale, json& tile_id_array, int stride, bool rounded);
Entity create_level_boundary(vec2 position, vec2 scale);
Entity create_world_boundary(vec2 position, vec2 scale);
Entity create_clock_hole(vec2 position, vec2 scale);
Entity create_moving_platform(vec2 scale, std::vector<Path> movements, vec2 initial_position, json& tile_id_array, int stride, bool rounded);
Entity create_projectile(vec2 pos, vec2 size, vec2 velocity, bool delayed = false);
Entity create_bolt(vec2 pos, vec2 size, vec2 velocity, bool default_gravity, bool harmful);
Entity create_first_boss_test();
Entity create_spawnpoint(vec2 pos, vec2 size);
Entity create_spike(vec2 position, vec2 scale, json tile_id_array, int stride);
Entity create_ladder(vec2 position, vec2 scale, int height, json tile_id_array, int stride);
Entity create_partof(vec2 position, vec2 scale, json tile_id_array, int stride);
Entity create_breakable_static_platform(vec2 position, vec2 scale, bool should_break_instantly, float degrade_speed, bool is_time_controllable, json& tile_id_array, int stride);
Entity create_time_controllable_breakable_static_platform(vec2 position, vec2 scale, bool should_break_instantly, float degrade_speed, json& tile_id_array, int stride);
Entity create_cannon_tower(vec2 pos);
Entity create_tutorial_text(vec2 position, vec2 size, TEXTURE_ASSET_ID texture_id);
Entity create_door(vec2 position, bool open, json& tile_id_array, int stride);
Entity create_pipe_head(vec2 position, vec2 scale, std::string direction, json& tile_id_array, int stride, float time_offset = 0.0f);
Entity create_chain(vec2 position, vec2 scale);
Entity create_pendulum(vec2 pivot_position, float length, float initial_angle, float bob_radius);
Entity create_gear(vec2 position, vec2 size, bool fixed, float angular_velocity, float inital_angle);
Entity create_spikeball(vec2 position, vec2 size);
Entity create_spawner(std::string type, vec2 size, vec2 velocity, vec2 start_pos, vec2 end_pos);
Entity create_loading_screen();
Entity create_pause_buttons(vec2 pos, vec2 scale, float angle, TEXTURE_ASSET_ID texture_id);
Entity create_menu_screen();


// Helper methods
void remove_menu_screen();
float getDistance(const Motion& one, const Motion& other);
int get_tile_index(int pos_x, int pos_y, int offset_x, int offset_y, int stride);
