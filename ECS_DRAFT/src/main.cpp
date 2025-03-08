#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// internal
#include "systems/systems_manager.hpp"
#include "systems/ai/ai_system.hpp"
#include "systems/physics/physics_system.hpp"
#include "systems/camera/camera_system.hpp"
#include "systems/animation/animation_system.hpp"
#include "systems/rendering/render_system.hpp"
#include "systems/world/world_system.hpp"
#include "systems/boss/boss_system.hpp"
#include "systems/spawnpoint/spawnpoint_system.hpp"

// Entry point
int main(int argc, char *argv[])
{

	bool play_sound = true;
	if (argc == 2 && strcmp(argv[1], "--nosound") == 0)
	{
		play_sound = false;
	}

	SystemsManager system_manager;
	if (system_manager.get_window() == nullptr) {
		return EXIT_FAILURE;
	}

	// global systems
	AISystem	  ai_system;
	WorldSystem   world_system;
	RenderSystem  renderer_system;
	PhysicsSystem physics_system;
	CameraSystem camera_system;
	AnimationSystem animation_system;
	BossSystem boss_system;
	SpawnPointSystem spawnpoint_system;

	world_system.setSound(play_sound);

	// register order is the order steps (and then late steps) will be called
	system_manager.register_system(&world_system);
	system_manager.register_system(&ai_system);
	system_manager.register_system(&spawnpoint_system);
	system_manager.register_system(&boss_system);
	system_manager.register_system(&physics_system);
	system_manager.register_system(&camera_system);
	system_manager.register_system(&animation_system);
	system_manager.register_system(&renderer_system); // render system should prob stay last (?)

	system_manager.run_game_loop();
	return EXIT_SUCCESS;
}
