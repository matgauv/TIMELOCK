#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "systems/ISystem.hpp"

#include "cannon_tower/cannon_tower_utils.hpp"
#include "pipe/pipe_utils.hpp"

class AISystem : public ISystem
{
public:
	AISystem() {}
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;
private:
	GLFWwindow* window = nullptr;

	// A collection of AI-related functions; executed in specific order?
};