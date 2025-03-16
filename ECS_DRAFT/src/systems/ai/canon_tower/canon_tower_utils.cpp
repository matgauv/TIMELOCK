#include "canon_tower_utils.hpp"
#include <glm/trigonometric.hpp>
#include <iostream>
#include <cmath>

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
			case CANON_TOWER_STATE::LOADING:
				loading_step(tower_entity, tower, elapsed_ms);
				break;
			case CANON_TOWER_STATE::FIRING:
				firing_step(tower_entity, tower, elapsed_ms);
				break;
			default:
				break;
		}

		// Update timer
		if (tower.timer > 0) {
			tower.timer -= (elapsed_ms * registry.timeControllables.get(tower_entity).target_time_control_factor);

			if (tower.timer < 0) {
				tower.timer = 0;
			}
		}

		// Update barrel motion
		CanonBarrel& barrel = registry.canonBarrels.get(tower.barrel_entity);
		Motion& barrel_motion = registry.motions.get(tower.barrel_entity);
		const Motion& tower_motion = registry.motions.get(tower_entity);

		barrel_motion.angle = glm::degrees(barrel.angle);
		barrel_motion.velocity *= 0;

		float s_a = sin(barrel.angle);
		float c_a = cos(barrel.angle);
		barrel_motion.position =
			tower_motion.position +
			0.5f * barrel_motion.scale[0] * vec2{c_a, s_a};
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

	if (tower.timer <= 0) {
		tower.state = CANON_TOWER_STATE::LOADING;
		tower.timer = CANON_TOWER_LOAD_TIME_MS;

		return;
	}

	// TODO: aim the barrel in a more natural way
	// TODO: allow arbitrary orientation of the tower


	Entity player_entity = registry.players.entities[0];
	const Motion& player_motion = registry.motions.get(player_entity);

	const Motion& tower_motion = registry.motions.get(tower_entity);

	vec2 disp = player_motion.position - tower_motion.position;


	CanonBarrel& barrel = registry.canonBarrels.get(tower.barrel_entity);
	float target_angle = 0.0f;
	if (glm::length(disp) > 0.0f) {
		target_angle = atan2f(disp[1], disp[0]);
	}

	float turn = elapsed_ms / 1000.0 * CANON_TURN_SPEED;
	if (abs(barrel.angle - target_angle) <= turn ||
		abs(barrel.angle - target_angle + 2.0f * M_PI) <= turn ||
		abs(barrel.angle - target_angle - 2.0f * M_PI) <= turn) {
		barrel.angle = target_angle;
	}
	else {
		turn *= (
			abs(barrel.angle - target_angle) > M_PI ?
			(barrel.angle > target_angle ? 1 : -1) :
			(barrel.angle > target_angle ? -1 : 1));

		barrel.angle = fmod(barrel.angle + turn, 2.0f * M_PI);
		if (barrel.angle < 0) {
			barrel.angle += 2.0f * M_PI;
		}
	}
}

void loading_step(Entity tower_entity, CanonTower& tower, float elapsed_ms) {
	if (tower.timer <= 0.0f) {
		tower.state = CANON_TOWER_STATE::FIRING;
		tower.timer = CANON_TOWER_FIRE_TIME_MS;

		canon_fire(tower_entity, registry.canonBarrels.get(tower.barrel_entity).angle);

		return;
	}

	Motion& barrel_motion = registry.motions.get(tower.barrel_entity);

	// (1-t)^3
	float t = 1.0f - tower.timer / CANON_TOWER_LOAD_TIME_MS;
	float lerp_factor = (1.0f - t) * (1.0f - t) * (1.0f - t);
	barrel_motion.scale = CANON_BARREL_SIZE * (vec2{ 1.0f, 1.0f } *lerp_factor + vec2{ 0.6f, 1.6f } *(1.0f - lerp_factor));
}

// Currently more like a cooldown state
void firing_step(Entity tower_entity, CanonTower& tower, float elapsed_ms) {
	Motion& barrel_motion = registry.motions.get(tower.barrel_entity);
	if (tower.timer <= 0) {
		tower.state = CANON_TOWER_STATE::IDLE;
		tower.timer = 0;
		barrel_motion.scale = CANON_BARREL_SIZE;
	}

	if (tower.timer > 0.9f * CANON_TOWER_FIRE_TIME_MS) {
		// Thrust

		// (x-1)^4
		float t = 1.0f - (tower.timer - 0.9f * CANON_TOWER_FIRE_TIME_MS) / (0.1f * CANON_TOWER_FIRE_TIME_MS);
		float lerp_factor = (t - 1.0f) * (t - 1.0f)* (t - 1.0f)* (t - 1.0f);
		barrel_motion.scale = CANON_BARREL_SIZE * (
			vec2{ 0.6f, 1.6f } * lerp_factor + vec2{ 1.4f, 0.7f } *(1.0f - lerp_factor));

	}
	else {
		// Recoil
		
		// (t-1)^2
		float t = 1.0f - tower.timer / (0.9f * CANON_TOWER_FIRE_TIME_MS);
		float lerp_factor = (t - 1.0f) * (t - 1.0f);
		barrel_motion.scale = CANON_BARREL_SIZE * (
			vec2{ 1.4f, 0.7f } * lerp_factor + vec2{ 1.0f, 1.0f } * (1.0f - lerp_factor));
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

void canon_fire(Entity tower_entity, float angle) {
	// Currently a copy of create bolt
	CanonTower& tower = registry.canonTowers.get(tower_entity);

	auto proj_entity = Entity();

	Blocked& blocked = registry.blocked.emplace(proj_entity);
	blocked.normal = vec2(0, 0);

	PhysicsObject& object = registry.physicsObjects.emplace(proj_entity);
	object.mass = 10.0f;
	object.drag_coefficient = 0.01f;

	//registry.falling.emplace(entity);

	registry.bolts.emplace(proj_entity);
	registry.harmfuls.emplace(proj_entity);
	registry.projectiles.emplace(proj_entity);

	vec2 dir = vec2{ cos(angle), sin(angle) };
	Motion& motion = registry.motions.emplace(proj_entity);
	motion.angle = 0.f;
	motion.velocity = CANON_PROJECTILE_SPEED * dir;
	motion.position = registry.motions.get(tower_entity).position + dir * registry.motions.get(tower.barrel_entity).scale.x;
	motion.scale = CANON_PROJECTILE_SIZE;

	registry.colors.insert(
		proj_entity,
		{ 1.0f, 1.0f, 1.0f }
	);

	registry.renderRequests.insert(
		proj_entity,
		{
			TEXTURE_ASSET_ID::HEX,
			EFFECT_ASSET_ID::HEX,
			GEOMETRY_BUFFER_ID::HEX
		}
	);

	registry.layers.insert(proj_entity, { LAYER_ID::MIDGROUND });

	TimeControllable &timeControllable = registry.timeControllables.emplace(proj_entity);
	timeControllable.can_become_harmless = true;
}