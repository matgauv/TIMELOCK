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

	static vec2 get_collision_overlap(Motion& a, Motion& b) {
		vec2 playerBB = {abs(a.scale.x), abs(a.scale.y)};
		vec2 objBB = {abs(b.scale.x), abs(b.scale.y)};

		vec2 playerHalf = playerBB * 0.5f;
		vec2 objHalf = objBB * 0.5f;

		vec2 delta = a.position - b.position;

		float overlapX = (playerHalf.x + objHalf.x) - fabs(delta.x);
		float overlapY = (playerHalf.y + objHalf.y) - fabs(delta.y);
		return {overlapX, overlapY};
	}

	PhysicsSystem()
	{
	}
private:
	GLFWwindow* window = nullptr;

	void apply_gravity(Entity& entity, Motion& motion, float step_seconds);
	void player_walk(Entity& entity, Motion& motion, float step_seconds);
	void move_object(Entity& entity, Motion& motion, float step_seconds);
};