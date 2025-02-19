#include <iostream>
#include "time_control_system.hpp"
#include "../world/world_init.hpp"

void TimeControlSystem::init(GLFWwindow *window) {
	this->window = window;
}

void TimeControlSystem::step(float elapsed_ms) {
	(void)elapsed_ms;
}

void TimeControlSystem::late_step(float elapsed_ms) {
	(void)elapsed_ms;
}