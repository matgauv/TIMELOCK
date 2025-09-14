#include "world_system.hpp"

void WorldSystem::init(GLFWwindow* window) {

	this->window = window;

	// Set title
	glfwSetWindowTitle(window, "TIME LOCK | NaN fps");

	// Create a single GameState entity
	registry.gameStates.emplace(game_state_entity);

	// Create a single LevelState entity
	LevelState& levelState = registry.levelStates.emplace(level_state_entity);

	// This will be the first level we load when the game is started.
	levelState.curr_level_folder_name = "Level_8";
	levelState.shouldLoad = true;

	Entity flag_entity = Entity();
	registry.flags.emplace(flag_entity);

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
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	background_music = Mix_LoadMUS(audio_path("time_ambient.wav").c_str());
	slow_down_effect = Mix_LoadWAV(audio_path("slow.wav").c_str());
	speed_up_effect = Mix_LoadWAV(audio_path("speedup.wav").c_str());

	sound_effects.push_back(slow_down_effect);
	sound_effects.push_back(speed_up_effect);

	if (background_music == nullptr || slow_down_effect == nullptr || speed_up_effect == nullptr) {
		fprintf(stderr, "Failed to load sounds -- make sure the data directory is present");
		if (background_music) Mix_FreeMusic(background_music);
		if (slow_down_effect) Mix_FreeChunk(slow_down_effect);
		if (speed_up_effect) Mix_FreeChunk(speed_up_effect);

		background_music = nullptr;
		slow_down_effect = nullptr;
		speed_up_effect = nullptr;

		sound_effects.clear();

		Mix_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	return true;
}