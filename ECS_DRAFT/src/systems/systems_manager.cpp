#include "systems_manager.hpp"
#include "../common.hpp"
#include <iostream>
#include <chrono>

#include "physics/physics_system.hpp"
#include "tinyECS/registry.hpp"
#include "world/world_init.hpp"
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
	const float physics_step = 1000.0f / 120.0f; // we step physics at 120 fps
	const int substep_count = 4; // for each of these 120 fps, we step physics 8 times in small sub steps

	// if the game is running really fast, we just step the physics system anyways..
	// https://gafferongames.com/post/fix_your_timestep/
	const float max_accumulator_ms = 250.0f;


	while (!is_over()) {
		glfwPollEvents();

		auto now = Clock::now();
		float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		GameState& gs = registry.gameStates.components[0];


		if (gs.game_running_state == GAME_RUNNING_STATE::MENU || gs.game_running_state == GAME_RUNNING_STATE::PAUSED) {
			if (registry.cameras.size() == 0) {
				// mock camera to load the menu screen
				create_camera({ 0, 0 }, { 1, 1 });
			}

			if (registry.menuScreens.size() == 0) {
				create_menu_screen();
			}

			// step the render system
			systems[systems.size() - 1]->step(elapsed_ms);
		}
		else if (gs.game_running_state == GAME_RUNNING_STATE::INTRO) 
		{
			if (registry.cutScenes.size() == 0) {
				create_intro_cutscene();
			}

			systems[systems.size() - 1]->step(elapsed_ms);
			systems[systems.size() - 2]->step(elapsed_ms);

			Entity entity = registry.cutScenes.entities[0];
			CutScene& cutscene = registry.cutScenes.get(entity);

			if (cutscene.state == 0) {
				registry.remove_all_components_of(entity);
				gs.game_running_state = GAME_RUNNING_STATE::RUNNING;
			}
		}
		else if (gs.game_running_state == GAME_RUNNING_STATE::OUTRO) {
			
			if (registry.cutScenes.size() == 0) {
				create_outro_cutscene();
			}

			// step the render/animation system
			systems[systems.size() - 1]->step(elapsed_ms);
			systems[systems.size() - 2]->step(elapsed_ms);

			Entity entity = registry.cutScenes.entities[0];
			CutScene& cutscene = registry.cutScenes.get(entity);
			if (cutscene.state == 0) {

				registry.remove_all_components_of(entity);
				gs.game_running_state = GAME_RUNNING_STATE::MENU;
			}
		}
		else {

			physics_accumulator += elapsed_ms;
			physics_accumulator = std::min(physics_accumulator, max_accumulator_ms);

			// step regular systems with the frame time
			for (ISystem* system : systems) {
				system->step(elapsed_ms);
			}

			// step physics systems if enough time has elapsed with fixed frame time
			while (physics_accumulator >= physics_step) {
				float substep_dt = physics_step / substep_count;

				// perform the physics step in sub steps for more consistent behaviour
				for (int i = 0; i < substep_count; ++i) {
					for (ISystem* system : fixed_systems) {
						system->step(substep_dt);
					}
				}
				physics_accumulator -= physics_step;

				// late step once (NOT FIXED)
				for (ISystem* system : fixed_systems) {
					system->late_step(physics_step);
				}
			}
		}

		// late step regular systems with frame time
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