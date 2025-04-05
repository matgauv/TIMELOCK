#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"

#include "systems/ISystem.hpp"

class UiSystem : public ISystem
{
public:
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;

	UiSystem()
	{
	}
private:
	GLFWwindow* window = nullptr;

	void deceleration_bar_step(float elapsed_ms);
};