// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "../physics/physics_system.hpp"
#include "../player/player_system.hpp"

// create the world
WorldSystem::WorldSystem()
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {

	if (this->play_sound) {
		// Destroy music components
		if (background_music != nullptr)
			Mix_FreeMusic(background_music);

		for (Mix_Chunk* effect : sound_effects) {
			if (effect != nullptr)
				Mix_FreeChunk(effect);
		}

		Mix_CloseAudio();
	}

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

void WorldSystem::init(GLFWwindow* window) {

	this->window = window;

	// Set title
	glfwSetWindowTitle(window, "TIME LOCK | NaN fps");

	// Create a single GameState entity
	registry.gameStates.emplace(game_state_entity);

	if (this->play_sound && !start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds." << std::endl;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

	if (this->play_sound) {
		// start playing background music indefinitely
		std::cout << "Starting music..." << std::endl;
		Mix_PlayMusic(background_music, -1);
	}

	// Set all states to default
    restart_game();
}

// From A2
void WorldSystem::update_window_caption(float elapsed_ms) {
	// Potentially expand functionalities regarding levels

	int fps = (int)(1 / elapsed_ms);

	std::stringstream title_ss;
	title_ss << "TIME LOCK | " << (elapsed_ms <= 1.0E-3 ? "NaN" : std::to_string((int)(1.0 / (0.001 * elapsed_ms)))) << " fps";

	glfwSetWindowTitle(window, title_ss.str().c_str());
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
	auto now = std::chrono::high_resolution_clock::now();

	if (gameState.time_control_start_time != std::chrono::time_point<std::chrono::high_resolution_clock>{}) {
		float duration = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - gameState.time_control_start_time)).count() / 1000;

		if (gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED && duration >= ACCELERATION_DURATION_MS) {
			control_time(true, false);
		}

		if (gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED && duration >= DECELERATION_DURATION_MS) {
			control_time(false, false);
		}
	}

	// Update acceleration and deceleration cooldown time
	gameState.accelerate_cooldown_ms = std::max(0.f, gameState.accelerate_cooldown_ms - elapsed_ms_since_last_update);
	gameState.decelerate_cooldown_ms = std::max(0.f, gameState.decelerate_cooldown_ms - elapsed_ms_since_last_update);


	// TODO: prob don't need to loop any frame, only when transitions are taking place...
	for (Entity entity : registry.timeControllables.entities) {
		TimeControllable& tc = registry.timeControllables.get(entity);
		Motion& motion = registry.motions.get(entity);
		float start = NORMAL_FACTOR;

		// this is a bit ugly but covers the 4 cases where we go from accelerated/decelerated to normal/decelerated/accelerated
		if (motion.velocityModifier < NORMAL_FACTOR && tc.target_time_control_factor != DECELERATE_FACTOR) {
			start = DECELERATE_FACTOR;
		} else if (motion.velocityModifier > NORMAL_FACTOR && tc.target_time_control_factor != ACCELERATE_FACTOR) {
			start = ACCELERATE_FACTOR;
		}

		lerpTimeState(start, tc.target_time_control_factor, motion, gameState.time_control_start_time);

		// handles breakable wall degradation here
		if (registry.breakables.has(entity)) {
			Breakable& breakable = registry.breakables.get(entity);

			Entity& player_entity = registry.players.entities[0];
			Motion& player_motion = registry.motions.get(player_entity);
			Motion& breakable_tc_entity_motion = registry.motions.get(entity);

			if (getDistance(player_motion, breakable_tc_entity_motion) <= TIME_CONTROL_VICINITY_THRESHOLD) {
				// speed up the degrade speed and decrement health
				std::cout << "Player is close enough to the breakable platform" << std::endl;

				if (tc.can_be_accelerated && gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED) {
					std::cout << "Subtracting: " << breakable.degrade_speed_per_ms * tc.target_time_control_factor * elapsed_ms_since_last_update << " health from breakable platform" << std::endl;
					breakable.health += breakable.degrade_speed_per_ms * tc.target_time_control_factor * elapsed_ms_since_last_update;
				}
			}

			std::cout << "Breakable platform has " << breakable.health << " health left" << std::endl;

			if (breakable.health <= 0.f) {
				registry.remove_all_components_of(entity);
			}
		}
	}

	// Can potentially remove
	if (gameState.game_running_state == GAME_RUNNING_STATE::SHOULD_RESET) {
		restart_game();
	}
}

void WorldSystem::check_player_killed() {
	const Player& player = registry.players.components[0];
	GameState& gameState = registry.gameStates.components[0];

	// Disable time control during dead & respawn
	if (player.state == PLAYER_STATE::DEAD || player.state == PLAYER_STATE::RESPAWNED) {
		if (gameState.game_time_control_state != TIME_CONTROL_STATE::NORMAL) {
			control_time(false, false);
		}
	}
}

void WorldSystem::check_scene_transition() {
	const Player& player = registry.players.components[0];
	GameState& gameState = registry.gameStates.components[0];
	ScreenState& screenState = registry.screenStates.components[0];

	if ((player.state == PLAYER_STATE::DEAD && gameState.game_scene_transition_state != SCENE_TRANSITION_STATE::TRANSITION_OUT)) {
		// Conditions for start transition out

		gameState.game_scene_transition_state = SCENE_TRANSITION_STATE::TRANSITION_OUT;
		screenState.scene_transition_factor = 0.0f;
	}
	else if ((player.state != PLAYER_STATE::DEAD && gameState.game_scene_transition_state == SCENE_TRANSITION_STATE::TRANSITION_OUT)) {
		// Conditions for start transition in
		gameState.game_scene_transition_state = SCENE_TRANSITION_STATE::TRANSITION_IN;
	}
}

// M1 interpolation implementation
void WorldSystem::lerpTimeState(float start, float target, Motion& motion, std::chrono::time_point<std::chrono::high_resolution_clock> effectStartTime) {

	float currentTime = std::chrono::duration<float, std::milli>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();
	float startTime = std::chrono::duration<float, std::milli>(
			effectStartTime.time_since_epoch()
		).count();

	float time = (currentTime - startTime) / ACCELERATION_EMERGE_MS;

	if (time > 1.0) time = 1.0;

	motion.velocityModifier = lerpToTarget(start, target, time); // actual lerp function is in common.cpp
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

	// debugging for memory/component leaks
	registry.list_all_components();

	assert(registry.gameStates.components.size() <= 1);
	GameState& gameState = registry.gameStates.components[0];
	gameState.accelerate_cooldown_ms = 0.f;
	gameState.decelerate_cooldown_ms = 0.f;
	gameState.game_time_control_state = TIME_CONTROL_STATE::NORMAL;
	gameState.game_running_state = GAME_RUNNING_STATE::RUNNING;
	gameState.game_scene_transition_state = SCENE_TRANSITION_STATE::TRANSITION_IN;
	gameState.time_control_start_time = std::chrono::time_point<std::chrono::high_resolution_clock>{};
	gameState.is_in_boss_fight = 0;

	// TODO:
	// Maybe the game state should also keep track of current level and player spawning position?

	load_level("");
}

// World initialization
bool WorldSystem::start_and_load_sounds() {

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	background_music = Mix_LoadMUS(audio_path("time_ambient.wav").c_str());
	slow_down_effect = Mix_LoadWAV(audio_path("slow.wav").c_str());
	speed_up_effect = Mix_LoadWAV(audio_path("speedup.wav").c_str());

	sound_effects.push_back(slow_down_effect);
	sound_effects.push_back(speed_up_effect);

	if  (background_music == nullptr || slow_down_effect == nullptr || speed_up_effect == nullptr) {
		fprintf(stderr, "Failed to load sounds -- make sure the data directory is present");
		return false;
	}

	return true;
}

void WorldSystem::control_time(bool accelerate, bool activate) {
	GameState& gameState = registry.gameStates.components[0];

	if (activate) {
		// don't activate if cooldown hasn't expired!
		if ((accelerate && gameState.accelerate_cooldown_ms > 0.0) ||
			(!accelerate && gameState.decelerate_cooldown_ms > 0.0)) {
			return;
		}

		if (accelerate) {
			gameState.game_time_control_state = TIME_CONTROL_STATE::ACCELERATED;
			gameState.accelerate_cooldown_ms = ACCELERATION_COOLDOWN_MS;
			playSoundIfEnabled(speed_up_effect);
		} else {
			gameState.game_time_control_state = TIME_CONTROL_STATE::DECELERATED;
			gameState.decelerate_cooldown_ms = DECELERATION_COOLDOWN_MS;
			playSoundIfEnabled(slow_down_effect);
		}

		gameState.time_control_start_time = std::chrono::high_resolution_clock::now();
	} else {
		gameState.game_time_control_state = TIME_CONTROL_STATE::NORMAL;
		if (accelerate) {
			playSoundIfEnabled(slow_down_effect);
		} else {
			playSoundIfEnabled(speed_up_effect);
		}
	}

	for (uint i = 0; i < registry.timeControllables.components.size(); i++) {
		TimeControllable& tc = registry.timeControllables.components[i];
		Entity& entity = registry.timeControllables.entities[i];

		// set the target time control factor, `step` will lerp towards whatever we set here
		if (activate) {
			tc.target_time_control_factor = accelerate ? ACCELERATE_FACTOR : DECELERATE_FACTOR;
		} else {
			tc.target_time_control_factor = NORMAL_FACTOR;
		}

		// become harmful when activating acceleration and can become harmful or when deactivating deceleration (and became harmless when decellerating)
		if ((activate && accelerate && tc.can_become_harmful) ||
			(!activate && !accelerate && tc.can_become_harmless)) {
			registry.harmfuls.emplace(entity);
		}

		// become harmless when activating deceleration and can become harmless or when deactivating acceleration (and became harmful during acceleration)
		if ((activate && !accelerate && tc.can_become_harmless) ||
			(!activate && accelerate && tc.can_become_harmful)) {
			registry.harmfuls.remove(entity);
		}
	}
}


void WorldSystem::player_walking(bool walking, bool is_left) {
	Entity& player = registry.players.entities[0];

	if (walking) {
		// if already walking, just update direction. Otherwise, add component.
		if (registry.walking.has(player)) {
			Walking& walking_component = registry.walking.get(player);
			walking_component.is_left = is_left;
		} else {
			Walking& walking_component = registry.walking.emplace(player);
			walking_component.is_left = is_left;
		}

		PlayerSystem::set_walking(is_left);
	} else {
		if (registry.walking.has(player)) {
			Walking& walking_component = registry.walking.get(player);
			// if current walking component is in the direction of this player walk stop call, remove it and stop walking
			if (walking_component.is_left == is_left) {
				registry.walking.remove(player);
				PlayerSystem::set_standing(is_left);
			}
		}
	}
}

// TODO: this should be handled by physics?
void WorldSystem::player_jump() {
	Entity& player = registry.players.entities[0];

	if (!registry.falling.has(player)) {
		if (registry.motions.has(player))
		{
			Motion& motion = registry.motions.get(player);
			motion.velocity.y -= JUMP_VELOCITY;
			registry.falling.emplace(player);
		}

	}

}

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
	}

	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE) {
			if (debugging.in_debug_mode) {
				debugging.in_debug_mode = false;
			}
			else {
				debugging.in_debug_mode = true;
			}
		}
	}

	// The following actions only available when player is alive
	// Could extend to case of game pause
	const Player& player = registry.players.components[0];
	if (player.state != PLAYER_STATE::ALIVE) {
		return;
	}

	GameState& gameState = registry.gameStates.components[0];

	// Activate acceleration
	if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
		if (gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED) {
			control_time(true, false);
		} else {
			control_time(true, true);
		}
	}

	// Activate deceleration
	if (key == GLFW_KEY_W && action == GLFW_RELEASE)
	{
		if (gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED) {
			control_time(false, false);
		} else {
			control_time(false, true);
		}
	}

	if (key == GLFW_KEY_RIGHT) {
		if (action == GLFW_PRESS) {
			player_walking(true, false);
		} else if (action == GLFW_RELEASE) {
			player_walking(false, false);
		}
	}

	if (key == GLFW_KEY_LEFT) {
		if (action == GLFW_PRESS) {
			player_walking(true, true);
		} else if (action == GLFW_RELEASE) {
			player_walking(false, true);
		}
	}

	if (key == GLFW_KEY_UP) {
		player_jump();
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {

	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {
	// on button press
	if (action == GLFW_PRESS) {
		std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
	}
}

void WorldSystem::playSoundIfEnabled(Mix_Chunk* sound) {
	if (this->play_sound) {
		Mix_PlayChannel(-1, sound, 0);
	}
}