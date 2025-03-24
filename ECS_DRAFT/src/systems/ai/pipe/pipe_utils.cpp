#include "pipe_utils.hpp"
#include <iostream>

void pipe_step(float elapsed_ms) {
	const vec2 player_pos = registry.motions.get(registry.players.entities[0]).position;

	for (int i = 0; i < registry.pipes.size(); i++) {
		const Entity entity = registry.pipes.entities[i];
		Pipe& pipe = registry.pipes.components[i];
		const Motion &motion = registry.motions.get(entity);

		pipe.timer -= elapsed_ms * registry.timeControllables.get(entity).target_time_control_factor;

		if (pipe.timer <= 0.0) {
			pipe.timer = PIPE_FIRING_PERIOD_MS;

			if (glm::length(player_pos - motion.position) < 3000.0f) {
				const vec2 screw_position = motion.position + vec2{ (TILE_TO_PIXELS + SCREW_SIZE.x + 1.0f) * pipe.direction_factor, 0.0f };
				fire_screw(
					screw_position,
					vec2{ pipe.direction_factor * SCREW_SPEED, 0.0f});
				std::cout << screw_position.x << ", " << screw_position.y << std::endl;
			}
		}
	}
}

void fire_screw(vec2 position, vec2 velocity) {
	auto proj_entity = Entity();

	PhysicsObject& object = registry.physicsObjects.emplace(proj_entity);
	object.mass = 100.0f;
	object.drag_coefficient = 0.01f;
	object.apply_gravity = false;
	object.apply_rotation = false;

	registry.harmfuls.emplace(proj_entity);
	registry.projectiles.emplace(proj_entity);

	Motion& motion = registry.motions.emplace(proj_entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = position;
	motion.scale = SCREW_SIZE;


	registry.renderRequests.insert(
		proj_entity,
		{
			TEXTURE_ASSET_ID::SCREW,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			velocity.x < 0
		}
	);

	registry.layers.insert(proj_entity, { LAYER_ID::MIDGROUND });

	TimeControllable& timeControllable = registry.timeControllables.emplace(proj_entity);
	timeControllable.can_become_harmless = true;
}