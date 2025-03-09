#include "canon_tower_utils.hpp"
#include <glm/trigonometric.hpp>

void canon_tower_step(float elapsed_ms) {
	for (int i = 0; i < registry.canonTowers.size(); i++) {
		const Entity tower_entity = registry.canonTowers.entities[i];
		CanonTower &tower = registry.canonTowers.components[i];

		switch (tower.state) {
			case CANON_TOWER_STATE::IDLE:
				idle_step(tower_entity, tower, elapsed_ms);
				break;
			case CANON_TOWER_STATE::AIMING:
				aiming_step(tower_entity, tower, elapsed_ms);
				break;
			case CANON_TOWER_STATE::FIRING:
				break;
			default:
				break;
		}

		if (tower.timer > 0) {
			tower.timer -= (elapsed_ms * registry.timeControllables.get(tower_entity).target_time_control_factor);

			if (tower.timer < 0) {
				tower.timer == 0;
			}
		}

		// Update barrel motion
		CanonBarrel& barrel = registry.canonBarrels.get(tower.barrel_entity);
		Motion& barrel_motion = registry.motions.get(tower.barrel_entity);
		const Motion& tower_motion = registry.motions.get(tower_entity);

		barrel_motion.angle = barrel.angle;
		barrel_motion.velocity *= 0;

		/*
		glm::rotate
		float s_a = sin(glm::radians(barrel.angle));
		float c_a = cos(barrel.angle);
		barrel_motion.position =
			tower_motion.position +
			0.5f * CANON_BARREL_SIZE[0] * vec2{};
			*/
	}
}

void idle_step(Entity tower_entity, CanonTower &tower, float elapsed_ms) {
	if (player_detected(tower_entity, tower)) {
		tower.state = CANON_TOWER_STATE::AIMING;
		tower.timer = CANON_TOWER_AIM_TIME_MS;
	}
}

void aiming_step(Entity tower_entity, CanonTower& tower, float elapsed_ms) {
	if (!player_detected(tower_entity, tower)) {
		tower.state = CANON_TOWER_STATE::IDLE;
		tower.timer = 0;

		return;
	}

	// TODO: aim the barrel in a more natural way
	// TODO: allow arbitrary orientation of the tower
	Entity player_entity = registry.players.entities[0];
	const Motion& player_motion = registry.motions.get(player_entity);

	const Motion& tower_motion = registry.motions.get(tower_entity);
	
	vec2 disp = player_motion.position - tower_motion.position;


	CanonBarrel &barrel = registry.canonBarrels.get(tower.barrel_entity);
	if (glm::length(disp) == 0.0f) {
		barrel.angle = 0.0f;
	}
	else {
		barrel.angle = atan2f(disp[1], disp[0]);
	}
}

bool player_detected(Entity tower_entity, CanonTower& tower) {
	Entity player_entity = registry.players.entities[0];
	const Motion& player_motion = registry.motions.get(player_entity);

	const Motion& tower_motion = registry.motions.get(tower_entity);

	// TODO: Currently not checking if anything is blocking the vision
	// TODO: potentially restrict angle of canon barrel
	return (glm::length(player_motion.position - tower_motion.position) < tower.detection_range);
}