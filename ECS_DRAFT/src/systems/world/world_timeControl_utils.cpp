#include "world_system.hpp"

// M1 interpolation implementation
void WorldSystem::lerpTimeState(float start, float target, Motion& motion, float effective_time) {
	float time = 1.0;

	if ((0.0f < effective_time && effective_time < DECELERATION_DURATION_MS)) {
		// Lerp effects on activating
		time = std::min(1.0f, (DECELERATION_DURATION_MS - effective_time) / DECELERATION_EMERGE_MS);
	} else if (0.0f > effective_time && -effective_time < DECELERATION_COOLDOWN_MS) {
		// Lerp effects on cooling down
		time = std::min(1.0f, (DECELERATION_COOLDOWN_MS + effective_time) / DECELERATION_EMERGE_MS);
	}

	motion.velocityModifier = lerpToTarget(start, target, time); // actual lerp function is in common.cpp
}

bool WorldSystem::set_time_control_state(bool accelerate, bool activate, bool force_cooldown_reset) {
	GameState& gameState = registry.gameStates.components[0];

	if (activate) {
		// don't activate if cooldown hasn't expired!
		if ((accelerate && gameState.accelerate_timer < 0.0) ||
			(!accelerate && gameState.decelerate_timer < 0.0)) {
			return false;
		}

		if (accelerate) {
			gameState.game_time_control_state = TIME_CONTROL_STATE::ACCELERATED;
			gameState.accelerate_timer = ACCELERATION_DURATION_MS;
		}
		else {
			gameState.game_time_control_state = TIME_CONTROL_STATE::DECELERATED;
			gameState.decelerate_timer = DECELERATION_DURATION_MS;
		}

		return true;
	}
	else {
		/*
		if ((accelerate && gameState.game_time_control_state != TIME_CONTROL_STATE::ACCELERATED) ||
			(!accelerate && gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED)) {
			return false;
		}*/

		gameState.game_time_control_state = TIME_CONTROL_STATE::NORMAL;
		if (accelerate) {
			gameState.accelerate_timer = (force_cooldown_reset ? 0.0f :
				-ACCELERATION_COOLDOWN_MS * std::clamp((ACCELERATION_DURATION_MS - gameState.accelerate_timer) / ACCELERATION_DURATION_MS, 0.0f, 1.0f));
		}
		else {
			gameState.decelerate_timer = (force_cooldown_reset ? 0.0f : 
				-DECELERATION_COOLDOWN_MS * std::clamp((DECELERATION_DURATION_MS - gameState.decelerate_timer) / DECELERATION_DURATION_MS, 0.0f, 1.0f));
		}

		return true;
	}
}

// TODO: setting harmful component in trigger-based control_time function can be redundant;
// we have to configure newly summoned entities to obey their harmful/harmless rules anyways
// (e.g., when summoning a projectile during decel, we have to ensure it does not have a harmful component)
void WorldSystem::control_time(bool accelerate, bool activate, bool force_cooldown_reset) {
	if (set_time_control_state(accelerate, activate, force_cooldown_reset)) {
		if ((activate && accelerate) || (!activate && !accelerate)) {
			playSoundIfEnabled(speed_up_effect);
		}
		else {
			playSoundIfEnabled(slow_down_effect);
		}
	}
}


void WorldSystem::update_time_control_properties(TIME_CONTROL_STATE timeControlState, TimeControllable& tc, const Entity& entity)
{
	// become harmful when activating acceleration and can become harmful or when deactivating deceleration (and became harmless when decellerating)
	if ((timeControlState == TIME_CONTROL_STATE::ACCELERATED && tc.can_become_harmful) ||
		(timeControlState != TIME_CONTROL_STATE::DECELERATED && tc.can_become_harmless)) {
		if (!registry.harmfuls.has(entity)) {
			registry.harmfuls.emplace(entity);
		}
	}

	// become harmless when activating deceleration and can become harmless or when deactivating acceleration (and became harmful during acceleration)
	if ((timeControlState == TIME_CONTROL_STATE::DECELERATED && tc.can_become_harmless) ||
		(timeControlState != TIME_CONTROL_STATE::ACCELERATED && tc.can_become_harmful)) {
		if (registry.harmfuls.has(entity)) {
			registry.harmfuls.remove(entity);
		}
	}

	if (timeControlState == TIME_CONTROL_STATE::DECELERATED) {
		tc.target_time_control_factor = DECELERATE_FACTOR;
	}
	else if (timeControlState == TIME_CONTROL_STATE::ACCELERATED) {
		tc.target_time_control_factor = ACCELERATE_FACTOR;
	}
	else {
		tc.target_time_control_factor = NORMAL_FACTOR;
	}
}