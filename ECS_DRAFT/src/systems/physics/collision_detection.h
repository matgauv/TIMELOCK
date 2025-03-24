#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "./physics_utils.h"


void compute_platform_verticies(Motion& motion, Entity& e, float& angle_cos, float& angle_sin);
void compute_composite_mesh_vertices(Motion& motion, Entity& e);
void compute_vertices(Motion& motion, Entity& e);
void compute_axes(Motion& motion, std::vector<vec2>& vertices);
Collision compute_convex_collision(const std::vector<vec2>& a_verts, const std::vector<vec2>& a_axes, const std::vector<vec2>& b_verts, const std::vector<vec2>& b_axes, const vec2& a_position, const vec2& b_position, Entity& a, Entity& b);
Collision compute_sat_collision(Motion& a_motion, Motion& b_motion, Entity& a, Entity& b);
bool compute_AABB_collision(const Motion& a_motion, const Motion& b_motion);
void collision_check(Entity& entity_i, Motion& motion_i, Entity& entity_j);
void detect_collisions();