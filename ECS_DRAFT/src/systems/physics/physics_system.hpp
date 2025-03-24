#pragma once

#include "../../common.hpp"
#include "systems/ISystem.hpp"
#include "physics_utils.h"
#include "collision_detection.h"
#include "collision_handlers.h"
#include "physics_simulation.h"
#include "player_mechanics.h"


class PhysicsSystem : public ISystem
{
public:
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;

	void setFreeFly(bool fly) {
		this->fly = fly;
	}

	PhysicsSystem()
	{
	}
private:
	GLFWwindow* window = nullptr;
	bool fly = false;

	void handle_collisions(float elapsed_ms);
	void detect_collisions();
};