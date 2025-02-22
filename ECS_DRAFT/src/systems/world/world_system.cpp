// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "../physics/physics_system.hpp"

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

		if (slow_down_effect != nullptr)
			Mix_FreeChunk(slow_down_effect);

		if (speed_up_effect != nullptr)
			Mix_FreeChunk(speed_up_effect);

		Mix_CloseAudio();
	}

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

void WorldSystem::init(GLFWwindow* window) {

	this->window = window;

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

// Update our game world
void WorldSystem::step(float elapsed_ms_since_last_update) {

	// Updating window title with points
	std::stringstream title_ss;
//	title_ss << "Points: " << points;
//	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	/* This part of code restricts the motion of entities;
	* It makes sense to apply a similar logic, but is currently restricting the action range of cameras;
	* May need to refine this in the furture (e.g., for certain projectiles, bosses, etc.)
	*/

	// Update info on acceleration and deceleration duration and trigger deactivate functions if needed
	assert(registry.gameStates.components.size() <= 1);
	GameState& gameState = registry.gameStates.components[0];
	auto now = std::chrono::high_resolution_clock::now();

	if (gameState.accelerate_start_time != std::chrono::time_point<std::chrono::high_resolution_clock>{}) {
		float duration = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - gameState.accelerate_start_time)).count() / 1000;

		if (duration >= ACCELERATION_DURATION_MS) {
			deactivate_acceleration();
		}
	}

	if (gameState.decelerate_start_time != std::chrono::time_point<std::chrono::high_resolution_clock>{}) {
		float duration = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - gameState.decelerate_start_time)).count() / 1000;

		if (duration >= DECELERATION_DURATION_MS) {
			deactivate_deceleration();
		}
	}

	// Update acceleration and deceleration cooldown time
	gameState.accelerate_cooldown_ms = std::max(0.f, gameState.accelerate_cooldown_ms - elapsed_ms_since_last_update);
	gameState.decelerate_cooldown_ms = std::max(0.f, gameState.decelerate_cooldown_ms - elapsed_ms_since_last_update);
}

void WorldSystem::late_step(float elapsed_ms) {
	(void) elapsed_ms;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {

	std::cout << "Restarting..." << std::endl;

	// Debugging for memory/component leaks
	registry.list_all_components();

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
	gameState.accelerate_start_time = std::chrono::time_point<std::chrono::high_resolution_clock>{};
	gameState.decelerate_start_time = std::chrono::time_point<std::chrono::high_resolution_clock>{};
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

	if  (background_music == nullptr || slow_down_effect == nullptr || speed_up_effect == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n make sure the data directory is present",
			audio_path("time_ambient.wav").c_str());
		return false;
	}

	return true;
}


// TODO can we make all four activate and deactivate for accelerate and decellerate one function?


void WorldSystem::activate_acceleration() {
    auto& acceleratable_registry = registry.acceleratables;

	// update time control state to accelerated
	assert(registry.gameStates.components.size() <= 1);
	GameState& gameState = registry.gameStates.components[0];

	gameState.game_time_control_state = TIME_CONTROL_STATE::ACCELERATED;

	// update accelerate start time
	gameState.accelerate_start_time = std::chrono::high_resolution_clock::now();

	// trigger acceleration
	for (uint i = 0; i < acceleratable_registry.components.size(); i++) {
		Acceleratable& curr = acceleratable_registry.components[i];
		Entity& entity = acceleratable_registry.entities[i];

		// update speed
		Motion& motion = registry.motions.get(entity);
		motion.frequency *= curr.factor;
		motion.velocityModifier *= curr.factor;

		// check if it can become harmful
		if (curr.can_become_harmful == 1) {
			Harmful& harmful = registry.harmfuls.emplace(entity);

			// Possible TODO:
			// Update the damage dealt to enemies/objects if needed in the future
		}
	}
}

void WorldSystem::activate_deceleration() {
    auto& deceleratable_registry = registry.deceleratables;

	// update time control state to decelerated
	assert(registry.gameStates.components.size() <= 1);
	GameState& gameState = registry.gameStates.components[0];

	gameState.game_time_control_state = TIME_CONTROL_STATE::DECELERATED;

	// update decelerate start time
	gameState.decelerate_start_time = std::chrono::high_resolution_clock::now();

	// trigger deceleration
	for (uint i = 0; i < deceleratable_registry.components.size(); i++) {
		Deceleratable& curr = deceleratable_registry.components[i];
		Entity& entity = deceleratable_registry.entities[i];

		// update speed
		Motion& motion = registry.motions.get(entity);
		motion.frequency *= curr.factor;
		motion.velocityModifier *= curr.factor;

		// check if it can become harmful
		if (curr.can_become_harmless == 1) {
			registry.harmfuls.remove(entity);
		}
	}

	if (this->play_sound) {
		Mix_PlayChannel(-1, slow_down_effect, 0);
	}
}

void WorldSystem::deactivate_acceleration() {
	auto& acceleratable_registry = registry.acceleratables;

	// update time control state to normal
	assert(registry.gameStates.components.size() <= 1);
	GameState& gameState = registry.gameStates.components[0];
	gameState.game_time_control_state = TIME_CONTROL_STATE::NORMAL;

	// update accelerate start time to default
	gameState.accelerate_start_time = std::chrono::time_point<std::chrono::high_resolution_clock>{};

	// deactivate acceleration
	for (uint i = 0; i < acceleratable_registry.components.size(); i++) {
		Acceleratable& curr = acceleratable_registry.components[i];
		Entity& entity = acceleratable_registry.entities[i];

		// update speed
		Motion& motion = registry.motions.get(entity);
		motion.frequency /= curr.factor;
		motion.velocityModifier /= curr.factor;

		// check if it can become harmful
		if (curr.can_become_harmful == 1) {
			registry.harmfuls.remove(entity);
		}
	}

	// start accelerate cooldown
	gameState.accelerate_cooldown_ms = ACCELERATION_COOLDOWN_MS;
}

void WorldSystem::deactivate_deceleration() {
	auto& decelerate_registry = registry.deceleratables;

	// update time control state to normal
	assert(registry.gameStates.components.size() <= 1);
	GameState& gameState = registry.gameStates.components[0];
	gameState.game_time_control_state = TIME_CONTROL_STATE::NORMAL;

	// update accelerate start time to default
	gameState.decelerate_start_time = std::chrono::time_point<std::chrono::high_resolution_clock>{};

	// deactivate deceleration
	for (uint i = 0; i < decelerate_registry.components.size(); i++) {
		Deceleratable& curr = decelerate_registry.components[i];
		Entity& entity = decelerate_registry.entities[i];

		// update speed
		Motion& motion = registry.motions.get(entity);
		motion.frequency /= curr.factor;
		motion.velocityModifier /= curr.factor;

		// check if it can become harmful
		if (curr.can_become_harmless == 1) {
			Harmful& harmful = registry.harmfuls.emplace(entity);

			// Possible TODO:
			// update the harmful entity damage value
		}
	}

	// start decelerate cooldown
	gameState.decelerate_cooldown_ms = DECELERATION_COOLDOWN_MS;
	if (this->play_sound) {
		Mix_PlayChannel(-1, speed_up_effect, 0);
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


		// TODO: this might not be the best approach to flip Player sprite;
		// Could potentially isolate all Player-related properties into Player component, and update Player system accordingly
		if (registry.renderRequests.has(player)) {
			registry.renderRequests.get(player).flipped = is_left;
		}

		if (registry.animateRequests.has(player)) {
			registry.animateRequests.get(player).used_animation = ANIMATION_ID::PLAYER_WALKING;
		}
	} else {
		if (registry.walking.has(player)) {
			Walking& walking_component = registry.walking.get(player);
			if (walking_component.is_left == is_left) {
				registry.walking.remove(player);
			}
		}

		if (registry.animateRequests.has(player)) {
			registry.animateRequests.get(player).used_animation = ANIMATION_ID::PLAYER_STANDING;
		}
	}
}

void WorldSystem::player_jump() {
	Entity& player = registry.players.entities[0];

	if (!registry.falling.has(player)) {
		Motion& motion = registry.motions.get(player);
		motion.selfVelocity.y = -JUMP_VELOCITY;
		registry.falling.emplace(player);
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

	GameState& gameState = registry.gameStates.components[0];

	// Activate acceleration
	if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {

		// todo can we just make these all one function that knows how to handle switching between states when the button is pressed?
		if (gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED)
		{
			deactivate_acceleration();
		} else if (gameState.game_time_control_state == TIME_CONTROL_STATE::NORMAL)
		{
			activate_acceleration();
		} else if (gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED)
		{
			deactivate_deceleration();
			activate_acceleration();
		}
	}

	// Activate deceleration
	if (key == GLFW_KEY_W && action == GLFW_RELEASE)
	{
		// todo see above
		if (gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED)
		{
			deactivate_deceleration();
		} else if (gameState.game_time_control_state == TIME_CONTROL_STATE::NORMAL)
		{
			activate_deceleration();
		} else if (gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED)
		{
			deactivate_acceleration();
			activate_deceleration();
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

	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
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