#include "world_system.hpp"

void WorldSystem::check_player_killed() {
	const Player& player = registry.players.components[0];
	GameState& gameState = registry.gameStates.components[0];

	// Disable time control during dead & respawn
	if (player.state == PLAYER_STATE::DEAD || player.state == PLAYER_STATE::RESPAWNED) {
		if (gameState.game_time_control_state != TIME_CONTROL_STATE::NORMAL) {
			control_time(false, false, true);
		}
	}
}

void WorldSystem::check_scene_transition() {
	const Player& player = registry.players.components[0];
	GameState& gameState = registry.gameStates.components[0];
	ScreenState& screenState = registry.screenStates.components[0];
	LevelState& levelState = registry.levelStates.components[0];

	if ((player.state == PLAYER_STATE::DEAD && gameState.game_scene_transition_state != SCENE_TRANSITION_STATE::TRANSITION_OUT)) {
		// Conditions for start transition out

		gameState.game_scene_transition_state = SCENE_TRANSITION_STATE::TRANSITION_OUT;
		screenState.scene_transition_factor = 0.0f;
	}
	else if (((player.state != PLAYER_STATE::DEAD && levelState.reload_coutdown < 0.0)
		&& gameState.game_scene_transition_state == SCENE_TRANSITION_STATE::TRANSITION_OUT)) {
		// Conditions for start transition in
		gameState.game_scene_transition_state = SCENE_TRANSITION_STATE::TRANSITION_IN;
	}
}