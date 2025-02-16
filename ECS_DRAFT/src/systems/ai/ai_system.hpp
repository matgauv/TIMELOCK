#pragma once

#include "../rendering/render_system.hpp"
#include "../../tinyECS/registry.hpp"

class AISystem : public ISystem
{
public:
	AISystem() {}
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;
private:
	GLFWwindow* window = nullptr;
};