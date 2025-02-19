#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "systems/ISystem.hpp"

class TimeControlSystem : public ISystem
{
public:
	TimeControlSystem() {}
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;
private:
	GLFWwindow* window = nullptr;
};