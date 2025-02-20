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
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
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

	if (background_music == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n make sure the data directory is present",
			audio_path("time_ambient.wav").c_str());
		return false;
	}

    return true;
}

void WorldSystem::init(GLFWwindow* window) {

	this->window = window;
	if (!start_and_load_sounds()) {
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

	// start playing background music indefinitely
	std::cout << "Starting music..." << std::endl;
	Mix_PlayMusic(background_music, -1);

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

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	/* This part of code restricts the motion of entities;
	* It makes sense to apply a similar logic, but is currently restricting the action range of cameras;
	* May need to refine this in the furture (e.g., for certain projectiles, bosses, etc.)
	*/

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	    Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motions_registry.entities[i]) && !registry.cameras.has(motions_registry.entities[i])) // don't remove the player or camera
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

}

void WorldSystem::late_step(float elapsed_ms) {
	handle_collisions();
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

	load_level("");
}

void WorldSystem::handle_player_object_collision(Entity player_entity, Entity object_entity, Collision collision, bool* playerIsGrounded) {
	Motion& player_motion = registry.motions.get(player_entity);
	Motion& object_motion = registry.motions.get(object_entity);
	vec2 overlap = PhysicsSystem::get_collision_overlap(player_motion, object_motion);

	if (collision.side == SIDE::LEFT || collision.side == SIDE::RIGHT) {
		if (!registry.blocked.has(player_entity)) {
			registry.blocked.emplace(player_entity);
		}
		Blocked& blocked = registry.blocked.get(player_entity);
		if (collision.side == SIDE::LEFT) {
			blocked.left = true;
			if (player_motion.velocity.x < 0)
				player_motion.velocity.x = 0.0f;
			player_motion.position.x += overlap.x;
		}
		else {
			blocked.right = true;
			if (player_motion.velocity.x > 0)
				player_motion.velocity.x = 0.0f;
			player_motion.position.x -= overlap.x;
		}
	}

	if (collision.side == SIDE::BOTTOM) {
		player_motion.velocity.y = 0.0f;
		registry.falling.remove(player_entity);
		*playerIsGrounded = true;
		player_motion.position.y -= overlap.y;

		if (registry.movementPaths.has(object_entity)) {
			MovementPath& movementPath = registry.movementPaths.get(object_entity);
			Path& currPath = movementPath.paths[movementPath.currentPathIndex];
			player_motion.baseVelocity = currPath.velocity;
		}
	} else if (collision.side == SIDE::TOP) {
		// stops the player from "sticking" to the bottom of a platform when they jump up into it
		// if player's y velocity is positive (i.e. player is falling), don't set velocity to 0 to avoid hanging.
		player_motion.velocity.y = max(player_motion.velocity.y, 0.0f);
	}

}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	ComponentContainer<Collision>& collision_container = registry.collisions;
    bool playerIsGrounded = false;

	for (uint i = 0; i < collision_container.components.size(); i++) {
		Entity& one = collision_container.entities[i];
		Collision& collision = collision_container.components[i];
		Entity& other = collision.other;

		// check player collisions (TODO: abstract this into logic for any falling component?)
		if (registry.players.has(one) && registry.platforms.has(other)) {
			handle_player_object_collision(one, other, collision, &playerIsGrounded);
		} else if (registry.players.has(other) && registry.platforms.has(one)) {
			// TODO: swap left/right, top/bottom collisions since player is the other...
			handle_player_object_collision(other, one, collision, &playerIsGrounded);
		}
	}

	// after checking all collisions, if player is not marked as grounded they should be falling again.
	if (!playerIsGrounded) {
		Entity& player = registry.players.entities[0];
		if (!registry.falling.has(player)) {
			registry.falling.emplace(player);
			registry.blocked.remove(player);
		}
		Motion& player_motion = registry.motions.get(player);
		player_motion.baseVelocity = {0, 0};
	}


	// Remove all collisions from this simulation step
	registry.collisions.clear();
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
	} else {
		if (registry.walking.has(player)) {
			Walking& walking_component = registry.walking.get(player);
			if (walking_component.is_left == is_left) {
				registry.walking.remove(player);
			}
		}
	}
}

void WorldSystem::player_jump() {
	Entity& player = registry.players.entities[0];

	if (!registry.falling.has(player)) {
		Motion& motion = registry.motions.get(player);
		motion.velocity.y = -JUMP_VELOCITY;
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