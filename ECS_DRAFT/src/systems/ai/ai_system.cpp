#include <iostream>
#include "ai_system.hpp"
#include "../world/world_init.hpp"

void AISystem::init(GLFWwindow *window) {
	this->window = window;
}

void AISystem::step(float elapsed_ms) {
	cannon_tower_step(elapsed_ms);
	delayed_projectile_step(elapsed_ms);
}

void AISystem::late_step(float elapsed_ms) {
	(void)elapsed_ms;
}

