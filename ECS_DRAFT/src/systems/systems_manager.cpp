#include "systems_manager.hpp"
#include "../common.hpp"
#include <iostream>
#include <chrono>
using Clock = std::chrono::high_resolution_clock;

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		std::cerr << error << ": " << desc << std::endl;
	}
}

bool SystemsManager::is_over() const {
    return bool(glfwWindowShouldClose(window));
}

void SystemsManager::run_game_loop() {
    auto t = Clock::now();

	float physics_accumulator = 0.0f;
	const float physics_step = 1000.0f / 120.0f;

    while (!is_over()) {
        // processes system messages, if this wasn't present the window would become unresponsive
        glfwPollEvents();

        // calculate elapsed times in milliseconds from the previous iteration
        auto now = Clock::now();
        float elapsed_ms =
            (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
        t = now;

    	physics_accumulator += elapsed_ms;



        for (ISystem* system : systems) {
            system->step(elapsed_ms);
        }

    	while (physics_accumulator >= physics_step) {
    		for (ISystem* system : fixed_systems)
    		{
    			system->step(physics_step);
    			system->late_step(physics_step); // TODO: late step later...
    		}

    		physics_accumulator -= physics_step;
    	}


        for (ISystem* system : systems) {
            system->late_step(elapsed_ms);
        }
    }
}

void SystemsManager::register_system(ISystem* system) {
    systems.push_back(system);
    system->init(this->window);
};

void SystemsManager::register_fixed_system(ISystem* system)
{
	fixed_systems.push_back(system);
	system->init(this->window);
}



// Note, this has a lot of OpenGL specific things, could be moved to the renderer
void SystemsManager::create_window() {

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
		return;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
	// glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);		// GLFW 3.3+
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);		// GLFW 3.3+

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "TIMELOCK", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "ERROR: Failed to glfwCreateWindow in systems_manager.cpp" << std::endl;
	}
}