#include "world_system.hpp"

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


// TODO: setting harmful component in trigger-based control_time function can be redundant;
// we have to configure newly summoned entities to obey their harmful/harmless rules anyways
// (e.g., when summoning a projectile during decel, we have to ensure it does not have a harmful component)
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
		}
		else {
			gameState.game_time_control_state = TIME_CONTROL_STATE::DECELERATED;
			gameState.decelerate_cooldown_ms = DECELERATION_COOLDOWN_MS;
			playSoundIfEnabled(slow_down_effect);
		}

		gameState.time_control_start_time = std::chrono::high_resolution_clock::now();
	}
	else {
		gameState.game_time_control_state = TIME_CONTROL_STATE::NORMAL;
		if (accelerate) {
			playSoundIfEnabled(slow_down_effect);
		}
		else {
			playSoundIfEnabled(speed_up_effect);
		}
	}

	/*
	for (uint i = 0; i < registry.timeControllables.components.size(); i++) {
		TimeControllable& tc = registry.timeControllables.components[i];
		Entity& entity = registry.timeControllables.entities[i];

		// set the target time control factor, `step` will lerp towards whatever we set here
		if (activate) {
			tc.target_time_control_factor = accelerate ? ACCELERATE_FACTOR : DECELERATE_FACTOR;
		}
		else {
			tc.target_time_control_factor = NORMAL_FACTOR;
		}

		// become harmful when activating acceleration and can become harmful or when deactivating deceleration (and became harmless when decellerating)
		if ((activate && accelerate && tc.can_become_harmful) ||
			(!activate && !accelerate && tc.can_become_harmless)) {
			if (!registry.harmfuls.has(entity)) {
				registry.harmfuls.emplace(entity);
			}
		}

		// become harmless when activating deceleration and can become harmless or when deactivating acceleration (and became harmful during acceleration)
		if ((activate && !accelerate && tc.can_become_harmless) ||
			(!activate && accelerate && tc.can_become_harmful)) {
			if (registry.harmfuls.has(entity)) {
				registry.harmfuls.remove(entity);
			}
		}
	}
	*/
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