#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"

#include "systems/ISystem.hpp"

// Player System; manages Player statistics
class PlayerSystem : public ISystem
{
public:
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;

	void set_spawnpoint(vec2 spawnpoint);
	void static kill();

	PlayerSystem()
	{
	}
private:
	GLFWwindow* window = nullptr;

	void static player_respawn();
};