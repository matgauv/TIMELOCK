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

	void apply_gravity(Entity& entity, Motion& motion, float elapsed_ms);
	void player_walk(Entity& entity, Motion& motion, float elapsed_ms);
};