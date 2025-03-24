#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "./physics_utils.h"

void handle_rotational_dynamics(Entity& object_entity, Entity& other_entity, const vec2& collision_normal, float step_seconds);
void handle_physics_collision(float step_seconds, Entity& entityA, Entity& entityB, Collision collision, std::vector<unsigned int>& grounded);
void apply_air_resistance(Entity entity, Motion& motion, float step_seconds);
void apply_gravity(Entity& entity, Motion& motion, float step_seconds);
void rotate_projectile(Entity& entity, Motion& motion, float step_seconds);
void update_pendulum(Entity& entity, float step_seconds);
void update_pendulum_rods();
void resolve_collision_position(Entity& entityA, Entity& entityB, Collision& collision, float inv_mass_a, float inv_mass_b);
void move_object_along_path(Entity& entity, Motion& motion, float step_seconds);