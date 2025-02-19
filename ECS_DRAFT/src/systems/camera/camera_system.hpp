#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "systems/ISystem.hpp"

// Camera System which follows specified Motion target
class CameraSystem : public ISystem
{
public:
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;

	CameraSystem()
	{
	}
private:
	GLFWwindow* window = nullptr;

	void follow(Motion& cam_motion, vec2 target);
};