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
	void handle_object_platform_collision(Entity object_entity, Entity platform_entity, Collision collision, float step_seconds,  std::vector<unsigned int>& groundedEntities, std::vector<unsigned int>& onMovingPlatform);
	void handle_player_attack_collision(Entity player_entity, Entity attack_entity, Collision collision);
	void handle_player_boss_collision(Entity player_entity, Entity boss_entity, Collision collision);
	void handle_physics_collision(float step_seconds, Entity entityA, Entity entityB, Collision collision,  std::vector<unsigned int>& groundedEntities);
	void apply_gravity(Entity& entity, Motion& motion, float step_seconds);
	void player_walk(Entity& entity, Motion& motion, float step_seconds);
	void move_object_along_path(Entity& entity, Motion& motion, float step_seconds);
	float clampToTarget(float value, float change, float target);
	float applyLerp(float current, float diff, float target);
	float lerpToTarget(float current, float target, float time);
	bool in(std::vector<unsigned int> vec, unsigned int in);
	vec2 get_bounding_box(const Motion& motion);

	SIDE get_collision_side(Motion& a, Motion& b);
	vec2 get_collision_overlap(Motion& a, Motion& b);

};