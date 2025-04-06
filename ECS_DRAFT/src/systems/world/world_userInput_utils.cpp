#include "world_system.hpp"
#include "world_init.hpp"

// on key callback
// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	if (registry.players.size() == 0) { return; } // level not loaded. TODO: set flag in the registry once level loading is done

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
	GameState& gameState = registry.gameStates.components[0];
	if (player.state == PLAYER_STATE::RESPAWNED || player.state == PLAYER_STATE::DEAD || gameState.game_running_state != GAME_RUNNING_STATE::RUNNING) {
		return;
	}

	// Activate acceleration
	if (key == GLFW_KEY_EQUAL && action == GLFW_RELEASE) {
		if (gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED) {
			control_time(true, false, false);
		}
		else {
			control_time(true, true, false);
		}
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
		if (registry.climbing.has(player_entity)) {
			Climbing& climbing = registry.climbing.get(player_entity);
			climbing.is_up = true;
		}
		else {
			player_jump();
		}


	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_W) {
		if (registry.climbing.has(player_entity)) {
			Climbing& climbing = registry.climbing.get(player_entity);
			climbing.is_up = false;
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
		levelState.ground = TEXTURE_ASSET_ID::A_TUTORIAL_GROUND;
		levelState.shouldLoad = true;
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		LevelState& levelState = registry.levelStates.components[0];
		levelState.curr_level_folder_name = "Level_2";
		levelState.ground = TEXTURE_ASSET_ID::DECEL_LEVEL_GROUND;
		levelState.shouldLoad = true;
	}

	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		LevelState& levelState = registry.levelStates.components[0];
		levelState.curr_level_folder_name = "Level_3";
		levelState.ground = TEXTURE_ASSET_ID::DECEL_LEVEL_GROUND;
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

	//pause screen
	if (key == GLFW_KEY_P) {
		if (gameState.game_running_state == GAME_RUNNING_STATE::PAUSED) {
			// do nothing
		} else if (gameState.game_running_state == GAME_RUNNING_STATE::MENU) {
            // do nothing
		} else {
			gameState.game_running_state = GAME_RUNNING_STATE::PAUSED;

            if (registry.cameras.entities.empty()) { return; }
			create_menu_screen();
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {

	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;

    if (registry.cameras.entities.empty()) {
		return;
	}

	GameState& gameState = registry.gameStates.components[0];

	if (gameState.game_running_state == GAME_RUNNING_STATE::PAUSED) {
		if (registry.menuScreens.size() == 0) {
			create_menu_screen();
		}

		MenuScreen& menu_screen = registry.menuScreens.components[0];
		RenderRequest& menu_button = registry.renderRequests.get(menu_screen.button_ids[1]);
		RenderRequest& resume_button = registry.renderRequests.get(menu_screen.button_ids[2]);

        if (mouse_pos_x >= 439 && mouse_pos_x <= 895 && mouse_pos_y >= 229 && mouse_pos_y <= 305) {
            menu_button.used_texture = TEXTURE_ASSET_ID::MENU_SELECTED;
            resume_button.used_texture = TEXTURE_ASSET_ID::RESUME;
		}
		else if (mouse_pos_x >= 439 && mouse_pos_x <= 895 && mouse_pos_y >= 360 && mouse_pos_y <= 438) {
			menu_button.used_texture = TEXTURE_ASSET_ID::MENU;
			resume_button.used_texture = TEXTURE_ASSET_ID::RESUME_SELECTED;
		}
		else {
			menu_button.used_texture = TEXTURE_ASSET_ID::MENU;
			resume_button.used_texture = TEXTURE_ASSET_ID::RESUME;
		}

	}

	if (gameState.game_running_state == GAME_RUNNING_STATE::MENU) {
		if (registry.menuScreens.size() == 0) {
			create_menu_screen();
		}

		MenuScreen& menu_screen = registry.menuScreens.components[0];
        RenderRequest& screen = registry.renderRequests.get(menu_screen.button_ids[0]);
		Motion& key_motion = registry.motions.get(menu_screen.button_ids[1]);
        Motion& camera_motion = registry.motions.get(registry.cameras.entities[0]);

		//if mouse hovers over start button
		if (mouse_pos_x >= 785 && mouse_pos_x <= 907 && mouse_pos_y >= 192 && mouse_pos_y <= 313) {
            screen.used_texture = TEXTURE_ASSET_ID::START_SELECTED;
            key_motion.angle = -27;
            key_motion.position = { camera_motion.position.x + 50.0f, camera_motion.position.y -30.0f };
		} // else if mouse hovers over exit button
		else if (mouse_pos_x >= 745 && mouse_pos_x <= 869 && mouse_pos_y >= 466 && mouse_pos_y <= 580) {
			screen.used_texture = TEXTURE_ASSET_ID::EXIT_SELECTED;
			key_motion.angle = 40;
			key_motion.position = { camera_motion.position.x + 42.0f, camera_motion.position.y +42.0f};
		}
		//if nothing hovered, return to middle position (regular screen,key,cover)
		else {
			screen.used_texture = TEXTURE_ASSET_ID::SCREEN;
			key_motion.angle = 0;
			key_motion.position = { camera_motion.position.x + 60.0f, camera_motion.position.y };
		}
	}
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {
	// on button press
	if (action == GLFW_PRESS) {
		std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
	}

    if (registry.cameras.entities.empty()) {
		return;
	}

	GameState& gameState = registry.gameStates.components[0];
	Entity camera_entity = registry.cameras.entities[0];
	Motion& camera_motion = registry.motions.get(camera_entity);

	// ----------PAUSE SCREEN-------------
	if (gameState.game_running_state == GAME_RUNNING_STATE::PAUSED) {
		//if menu pressed
		if (mouse_pos_x >= 439 && mouse_pos_x <= 895 && mouse_pos_y >= 229 && mouse_pos_y <= 305 && action == GLFW_PRESS) {
			//erase pause screen
			remove_menu_screen();

            //set game state to menu
			gameState.game_running_state = GAME_RUNNING_STATE::MENU;
		} // if resume pressed
		else if (mouse_pos_x >= 439 && mouse_pos_x <= 895 && mouse_pos_y >= 360 && mouse_pos_y <= 438 && action == GLFW_PRESS) {
			//erase pause screen
			remove_menu_screen();

            //set game state to running
			gameState.game_running_state = GAME_RUNNING_STATE::RUNNING;
		}

	}

	//-------------- MENU--------------
	if (gameState.game_running_state == GAME_RUNNING_STATE::MENU) {
		//if start pressed
		if (mouse_pos_x >= 785 && mouse_pos_x <= 907 && mouse_pos_y >= 192 && mouse_pos_y <= 313 && action == GLFW_PRESS) {
			remove_menu_screen();

			if (registry.players.size() == 0) {
				LevelState& ls = registry.levelStates.components[0];
				ls.shouldLoad = true;
			}

			gameState.game_running_state = GAME_RUNNING_STATE::RUNNING;
		} // else if exit pressed
		else if (mouse_pos_x >= 745 && mouse_pos_x <= 869 && mouse_pos_y >= 466 && mouse_pos_y <= 580 && action == GLFW_PRESS) {
			remove_menu_screen();

			// setting to over does nothing? so for now just calling close window
			gameState.game_running_state = GAME_RUNNING_STATE::OVER;
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		}
	}
}

void WorldSystem::playSoundIfEnabled(Mix_Chunk* sound) {
	if (this->play_sound) {
		Mix_PlayChannel(-1, sound, 0);
	}
}