#include "world_system.hpp"

// on key callback
// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	if (registry.players.size() == 0) { return; } // level not loaded. TODO: set flag in the registry once level loading is done


	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
		return;
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
	if (player.state == PLAYER_STATE::RESPAWNED || player.state == PLAYER_STATE::DEAD) {
		return;
	}

	GameState& gameState = registry.gameStates.components[0];

	// Activate acceleration
	if (key == GLFW_KEY_EQUAL && action == GLFW_RELEASE) {
		/*
		if (gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED) {
			control_time(true, false, false);
		}
		else {
			control_time(true, true, false);
		}*/

		CameraSystem::shake_camera(20.0f, 15.0f);
	}

	// Activate deceleration
	if (key == GLFW_KEY_MINUS && action == GLFW_RELEASE)
	{
		if (gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED) {
			control_time(false, false, false);
		}
		else {
			control_time(false, true, false);
		}
	}

	if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) {
			player_walking(true, false);
			PlayerSystem::set_direction(false);
		}
		else if (action == GLFW_RELEASE) {
			player_walking(false, false);
		}
	}

	if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) {
			player_walking(true, true);
			PlayerSystem::set_direction(true);
		}
		else if (action == GLFW_RELEASE) {
			player_walking(false, true);
		}
	}

	Entity& player_entity = registry.players.entities[0];
	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			if (registry.climbing.has(player_entity)) {
				Climbing& climbing = registry.climbing.get(player_entity);
				climbing.is_up = true;
			}
			else {
				player_jump();
			}
			registry.players.components[0].w_is_held = true;
		} else {
			if (registry.climbing.has(player_entity)) {
				Climbing& climbing = registry.climbing.get(player_entity);
				climbing.is_up = false;
			}
			registry.players.components[0].w_is_held = false;
		}
	}



	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		LevelState& levelState = registry.levelStates.components[0];
		levelState.curr_level_folder_name = "Level_0";
		levelState.ground = TEXTURE_ASSET_ID::D_TUTORIAL_GROUND;
		levelState.shouldLoad = true;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		LevelState& levelState = registry.levelStates.components[0];
		levelState.curr_level_folder_name = "Level_1";
		levelState.ground = TEXTURE_ASSET_ID::DECEL_LEVEL_1_GROUND;
		levelState.shouldLoad = true;
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		LevelState& levelState = registry.levelStates.components[0];
		levelState.curr_level_folder_name = "Level_2";
		levelState.ground = TEXTURE_ASSET_ID::DECEL_LEVEL_2_GROUND;
		levelState.shouldLoad = true;
	}

	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		LevelState& levelState = registry.levelStates.components[0];
		levelState.curr_level_folder_name = "Level_3";
		levelState.ground = TEXTURE_ASSET_ID::DECEL_LEVEL_3_GROUND;
		levelState.shouldLoad = true;
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		LevelState& levelState = registry.levelStates.components[0];
		levelState.curr_level_folder_name = "Level_4";
		levelState.ground = TEXTURE_ASSET_ID::BOSS_TUTORIAL_GROUND;
		levelState.shouldLoad = true;
	}
	if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
		LevelState& levelState = registry.levelStates.components[0];
		levelState.curr_level_folder_name = "Level_5";
		levelState.ground = TEXTURE_ASSET_ID::BOSS_ONE_LEVEL_GROUND;
		levelState.shouldLoad = true;
	}

	if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
		LevelParsingSystem::schedule_reload();
	}

	FlagState& flag_state = registry.flags.components[0];

	// Fly controls (run ./TIMELOCK --fly):
	if (key == GLFW_KEY_RIGHT && flag_state.fly) {
		if (action == GLFW_PRESS) {
			player_walking(true, false);
			PlayerSystem::set_direction(false);
		}
		else if (action == GLFW_RELEASE) {
			player_walking(false, false);
		}
	}

	if (key == GLFW_KEY_LEFT && flag_state.fly) {
		if (action == GLFW_PRESS) {
			player_walking(true, true);
			PlayerSystem::set_direction(true);
		}
		else if (action == GLFW_RELEASE) {
			player_walking(false, true);
		}
	}

	if (key == GLFW_KEY_DOWN && flag_state.fly) {
		Motion& motion = registry.motions.get(registry.players.entities[0]);
		if (action == GLFW_PRESS) {
			motion.velocity.y = JUMP_VELOCITY * 5.0f;
		}
		else if (action == GLFW_RELEASE) {
			motion.velocity.y = 0;
		}
	}

	if (key == GLFW_KEY_UP && flag_state.fly) {
		Motion& motion = registry.motions.get(registry.players.entities[0]);
		if (action == GLFW_PRESS) {
			motion.velocity.y = -JUMP_VELOCITY * 5.0f;
		}
		else if (action == GLFW_RELEASE) {
			motion.velocity.y = 0;
		}
	}

	// keybinds for toggling debug controls
	if (key == GLFW_KEY_N && action == GLFW_PRESS) {
		flag_state.no_clip = !flag_state.no_clip;
		std::cout << "No-clip set to: " << flag_state.no_clip << std::endl;
	}

	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		flag_state.fly = !flag_state.fly;
		std::cout << "Fly set to: " << flag_state.fly << std::endl;
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