#pragma once

// internal
#include "../../common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "../rendering/render_system.hpp"
#include "../particle/particle_system.hpp"

// Container for all our entities and game logic.
// Individual rendering / updates are deferred to the update() methods.
class WorldSystem : public ISystem
{
public:
	WorldSystem();

	// starts the game
	void init(GLFWwindow* window) override;

	// steps the game ahead by ms milliseconds
	void step(float elapsed_ms) override;

	// steps the game ahead, runs after all other components have stepped
	void late_step(float elapsed_ms) override;

	// releases all associated resources
	~WorldSystem();

	// Getter for the GameState entity
	Entity getGameStateEntity() { return game_state_entity; }

	void setSound(bool play_sound) { this->play_sound = play_sound; }

	void setFreeFly(bool fly) {
		this->fly = fly;
	}

	// Should likely aggregate these into a utils class
	static void destroy_breakable_platform(Entity entity);

private:
	// starts and loads music and sound effects
	bool start_and_load_sounds();

	// control acceleration/deceleration
	void control_time(bool accelerate, bool activate);

	void lerpTimeState(float start, float factor, Motion& motion, std::chrono::time_point<std::chrono::high_resolution_clock> effectStartTime);

	void player_walking(bool walking, bool is_left);
	void player_jump();

	
	float mouse_pos_x = 0.0f;
	float mouse_pos_y = 0.0f;

	// input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button_pressed(int button, int action, int mods);

	void playSoundIfEnabled(Mix_Chunk* sound);

	// fps update; update per second to avoid flickering number
	float fps_timer = 0.0f;
	void update_window_caption(float elapsed_ms);

	// restart level
	void restart_game();

	// Player kill and respawn
	void check_player_killed();

	void check_scene_transition();

	// GameState entity
	Entity game_state_entity;

	// LevelState entity
	Entity level_state_entity;

	// OpenGL window handle
	GLFWwindow* window;

	// Game state
	float current_speed;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* slow_down_effect;
	Mix_Chunk* speed_up_effect;
	std::vector<Mix_Chunk*> sound_effects;

	bool play_sound = true;
	bool fly = false;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
