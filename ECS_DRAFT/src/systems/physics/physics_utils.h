#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"

float lerp(float a, float b, float t);
float calculate_moment_of_inertia(Entity& entity);
vec2 closest_point_on_segment(const vec2& p, const vec2& a, const vec2& b);
bool is_on_ground(float normal_y);
float clampToTarget(float value, float change, float target);
bool in(std::vector<unsigned int>& vec, unsigned int id);
bool is_collision_between_player_and_boundary(Entity& one, Entity& other);
bool is_collision_between_player_and_spike(Entity& one, Entity& other);
bool player_harmful_collision(Entity& one, Entity& other);
void handle_player_breakable_collision(Entity& player_entity, Entity& breakable_entity, Collision collision);
std::pair<float, float> project(const std::vector<vec2>& verts, const vec2& axis);
std::vector<vec2>& get_vertices(Entity& e);
vec2 get_modified_velocity(Motion& m);
float get_modified_angular_velocity(Motion& m, PhysicsObject& physics);
std::vector<vec2>& get_axes(Entity& e);