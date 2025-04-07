// Header
#include "world_system.hpp"

// create the world
WorldSystem::WorldSystem()
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {

	if (this->play_sound) {
		Mix_HaltMusic();
		Mix_HaltChannel(-1);
		// Destroy music components
		if (background_music != nullptr)
			Mix_FreeMusic(background_music);
		background_music = nullptr;

		for (Mix_Chunk* effect : sound_effects) {
			if (effect != nullptr)
				Mix_FreeChunk(effect);
			effect = nullptr;
		}

		sound_effects.clear();

		Mix_CloseAudio();
		Mix_Quit();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		SDL_Quit();
	}

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
	glfwTerminate();
}



// Update our game world
void WorldSystem::step(float elapsed_ms_since_last_update) {
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Update fps info
	fps_timer += elapsed_ms_since_last_update;
	if (fps_timer > FPS_COUNTER_UPDATE_PERIOD_MS) {
		fps_timer = 0.0;
		update_window_caption(elapsed_ms_since_last_update);
	}

	/* This part of code restricts the motion of entities;
	* It makes sense to apply a similar logic, but is currently restricting the action range of cameras;
	* May need to refine this in the future (e.g., for certain projectiles, bosses, etc.)
	*/

	// Update info on acceleration and deceleration duration and deactivate if needed
	assert(registry.gameStates.components.size() <= 1);

	check_player_killed();
	check_scene_transition();

	GameState& gameState = registry.gameStates.components[0];
	
	// Update acceleration and deceleration timer
	if (gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED) {
		if (gameState.accelerate_timer > 0.0) {
			gameState.accelerate_timer -= elapsed_ms_since_last_update;

			if (gameState.accelerate_timer <= 0.0) {
				control_time(true, false);
			}
		}
	} else { 
		if (gameState.accelerate_timer < 0.0) {
			gameState.accelerate_timer += elapsed_ms_since_last_update;

			if (gameState.accelerate_timer >= 0.0) {
				gameState.accelerate_timer = 0.0;
			}
		}
	}

	if (gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED) {
		if (gameState.decelerate_timer > 0.0) {
			gameState.decelerate_timer -= elapsed_ms_since_last_update;

			if (gameState.decelerate_timer <= 0.0) {
				control_time(false, false);
			}
		}
	} else {
		if (gameState.decelerate_timer < 0.0) {
			gameState.decelerate_timer += elapsed_ms_since_last_update;

			if (gameState.decelerate_timer >= 0.0) {
				gameState.decelerate_timer = 0.0;
			}
		}
	}
	


	// TODO: prob don't need to loop any frame, only when transitions are taking place...
	for (int i = 0; i < registry.timeControllables.size(); i++) {
		const Entity entity = registry.timeControllables.entities[i];
		
		TimeControllable& tc = registry.timeControllables.components[i];
		Motion& motion = registry.motions.get(entity);
		float start = NORMAL_FACTOR;
		float effective_time = 0.0f;

		// this is a bit ugly but covers the 4 cases where we go from accelerated/decelerated to normal/decelerated/accelerated
		if (motion.velocityModifier < NORMAL_FACTOR && tc.target_time_control_factor != DECELERATE_FACTOR) {
			start = DECELERATE_FACTOR;
			effective_time = gameState.decelerate_timer;
		} else if (motion.velocityModifier > NORMAL_FACTOR && tc.target_time_control_factor != ACCELERATE_FACTOR) {
			start = ACCELERATE_FACTOR;
			effective_time = gameState.accelerate_timer;
		}

		lerpTimeState(start, tc.target_time_control_factor, motion, effective_time);
		
		update_time_control_properties(gameState.game_time_control_state, tc, entity);
	}

	// Can potentially remove
	if (gameState.game_running_state == GAME_RUNNING_STATE::SHOULD_RESET) {
		restart_game();
	}
}




void WorldSystem::late_step(float elapsed_ms) {
	(void) elapsed_ms;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {

	std::cout << "Restarting..." << std::endl;

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Reset fps timer
	fps_timer = 0.0;

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Remove all particles
	while (registry.particles.entities.size() > 0)
		registry.remove_all_components_of(registry.particles.entities.back());

	// debugging for memory/component leaks
	registry.list_all_components();

	assert(registry.gameStates.components.size() <= 1);
	GameState& gameState = registry.gameStates.components[0];
	gameState.accelerate_timer = 0.f;
	gameState.accelerate_timer = 0.f;
	gameState.game_time_control_state = TIME_CONTROL_STATE::NORMAL;

	//gameState.game_running_state = GAME_RUNNING_STATE::MENU;
	gameState.game_running_state = GAME_RUNNING_STATE::OUTRO;
	
	gameState.game_scene_transition_state = SCENE_TRANSITION_STATE::TRANSITION_IN;
	gameState.is_in_boss_fight = 0;

	LevelState& levelState = registry.levelStates.components[0];
	levelState.shouldLoad = true;

	// Check if particle system initialized
	if (registry.particleSystemStates.size() >= 1) {
		ParticleSystemState& particleState = registry.particleSystemStates.components[0];
		particleState.wind_field = { 0.0f, 0.0f };
		particleState.gravity_field = { 0.0f, GRAVITY };
		particleState.turbulence_scale = 1.0f;
		particleState.turbulence_strength = 0.0f;
	}
	// TODO:
	// Maybe the game state should also keep track of current level and player spawning position?

	//load_level("");
}
