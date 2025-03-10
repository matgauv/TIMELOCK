#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "systems/ISystem.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem : public ISystem
{
public:
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;

	PhysicsSystem()
	{
	}
private:
	GLFWwindow* window = nullptr;

	void detect_collisions();
	void handle_collisions(float elapsed_ms);
	void handle_object_rigid_collision(Entity object_entity, Entity platform_entity, Collision collision, float step_seconds,  std::vector<unsigned int>& groundedEntities, std::vector<unsigned int>& onMovingPlatform);
	void handle_player_attack_collision(Entity player_entity, Entity attack_entity, Collision collision);
	void handle_player_boss_collision(Entity player_entity, Entity boss_entity, Collision collision);
	void handle_player_breakable_collision(Entity& player_entity, Entity& breakable_entity, Collision collision);
	void handle_physics_collision(float step_seconds, Entity entityA, Entity entityB, Collision collision,  std::vector<unsigned int>& groundedEntities);
	void handle_projectile_collision(Entity proj_entity, Entity other_entity);
	void apply_gravity(Entity& entity, Motion& motion, float step_seconds);
	void player_walk(Entity& entity, Motion& motion, float step_seconds);
	void move_object_along_path(Entity& entity, Motion& motion, float step_seconds);
	void rotate_projectile(Entity& entity, Motion& motion, float step_seconds);
	float clampToTarget(float value, float change, float target);
	bool in(std::vector<unsigned int> vec, unsigned int in);
	vec2 get_bounding_box(const Motion& motion);

	SIDE get_collision_side(Motion& a, Motion& b, vec2 overlap);
	vec2 get_collision_overlap(Motion& a, Motion& b);
	vec2 get_friction_impulse(vec2 relative_velocity, float total_inv_mass, float impulse_scalar, vec2 normal);

	vec2 get_friction(Entity& e, vec2& velocity, vec2& normal, float step_seconds, float mass, bool is_moving_platform);
	vec2 get_modified_velocity(Motion& m);
	bool is_grounded(float normal_y);
	void adjust_velocity_along_normal(Motion& motion, vec2& normal);
	void resolve_collision_position(Entity& entityA, Entity& entityB, Collision& collision);

	bool is_collision_between_player_and_boundary(Entity& one, Entity& other);
	bool is_collision_between_player_and_spike(Entity& one, Entity& other);
};