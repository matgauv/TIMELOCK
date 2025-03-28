#include <iostream>
#include "ai_system.hpp"

void AISystem::init(GLFWwindow *window) {
	this->window = window;
}

void AISystem::step(float elapsed_ms) {
	cannon_tower_step(elapsed_ms);
	delayed_projectile_step(elapsed_ms);
	pipe_step(elapsed_ms);
	screw_step(elapsed_ms);
	obstacle_spawner_step(elapsed_ms);
}

void AISystem::late_step(float elapsed_ms) {
	(void)elapsed_ms;
}

