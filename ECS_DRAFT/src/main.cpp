#define GL3W_IMPLEMENTATION
#include <gl3w.h>
#include <sstream>

// internal
#include "systems/systems_manager.hpp"
#include "systems/ai/ai_system.hpp"
#include "systems/physics/physics_system.hpp"
#include "systems/camera/camera_system.hpp"
#include "systems/animation/animation_system.hpp"
#include "systems/rendering/render_system.hpp"
#include "systems/world/world_system.hpp"
#include "systems/boss/boss_system.hpp"
#include "systems/parser/parsing_system.hpp"

// Entry point
int main(int argc, char *argv[])
{

	bool play_sound = true;
	bool fly = false;
	if (argc == 2)
	{
		std::vector<std::string> flags;
		std::stringstream ss (argv[1]);
		std::string item;

		while (getline (ss, item, ',')) {
			flags.push_back (item);
		}

		for (const string& flag : flags) {
			if (flag == "--fly") {
				fly = true;
			} else if (flag == "--nosound") {
				play_sound = false;
			}
		}
	}

	SystemsManager system_manager;
	if (system_manager.get_window() == nullptr) {
		return EXIT_FAILURE;
	}

	// global systems
	AISystem	  ai_system;
	WorldSystem   world_system;
	RenderSystem  renderer_system;
	LevelParsingSystem parsing_system;
	PhysicsSystem physics_system;
	CameraSystem camera_system;
	AnimationSystem animation_system;
	BossSystem boss_system;

	world_system.setSound(play_sound);
	world_system.setFreeFly(fly);
	physics_system.setFreeFly(fly);

	// register order is the order steps (and then late steps) will be called
	system_manager.register_system(&world_system);
	system_manager.register_system(&parsing_system);
	system_manager.register_system(&ai_system);
	system_manager.register_system(&boss_system);
	system_manager.register_system(&physics_system);
	system_manager.register_system(&camera_system);
	system_manager.register_system(&animation_system);
	system_manager.register_system(&renderer_system); // render system should prob stay last (?)

	system_manager.run_game_loop();
	return EXIT_SUCCESS;
}
