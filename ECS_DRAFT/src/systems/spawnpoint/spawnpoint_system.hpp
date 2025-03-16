#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"

#include "systems/ISystem.hpp"

// Spawn Point Systemp; manages Player interaction with spawn points
class SpawnPointSystem : public ISystem
{
public:
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;

	SpawnPointSystem()
	{
	}
private:
	GLFWwindow* window = nullptr;

	bool update_spawnpoints(Entity active_spawnpoint);
	void activate_spawnpoint(Entity entity);
	void deactivate_spawnpoint(Entity entity);
	void reactivate_spawnpoint(Entity entity);
	void set_player_spawnpoint(Entity spawnpoint_entity);
};