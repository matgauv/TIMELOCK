#include "world_system.hpp"
#include "world_init.hpp"


// Player motions
void WorldSystem::player_walking(bool walking, bool is_left) {
	if (registry.players.size() > 0) {
		Entity& player = registry.players.entities[0];

		if (walking) {
			// if already walking, just update direction. Otherwise, add component.
			if (registry.walking.has(player)) {
				Walking& walking_component = registry.walking.get(player);
				walking_component.is_left = is_left;
			}
			else {
				Walking& walking_component = registry.walking.emplace(player);
				walking_component.is_left = is_left;
			}

		}
		else {
			if (registry.walking.has(player)) {
				Walking& walking_component = registry.walking.get(player);
				// if current walking component is in the direction of this player walk stop call, remove it and stop walking
				if (walking_component.is_left == is_left) {
					registry.walking.remove(player);
				}
			}
		}
	}

}

// TODO: this should be handled by physics?
void WorldSystem::player_jump() {
	if (registry.players.size() > 0) {
		Entity& player = registry.players.entities[0];

		//if (registry.onGrounds.has(player)) {
		if (PlayerSystem::can_jump()) {
			if (registry.motions.has(player))
			{
				Motion& motion = registry.motions.get(player);
				motion.velocity.y -= JUMP_VELOCITY;

				PlayerSystem::set_jumping_validity(false);
			}

		}
	}
}


// Breakable Platforms
void WorldSystem::degrade_breakable_platform(const Entity& entity, TimeControllable& tc, GameState& gameState, float elapsed_ms_since_last_update)
{
	Breakable& breakable = registry.breakables.get(entity);

	Entity& player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);
	Motion& breakable_tc_entity_motion = registry.motions.get(entity);

	if (getDistance(player_motion, breakable_tc_entity_motion) <= TIME_CONTROL_VICINITY_THRESHOLD) {
		// speed up the degrade speed and decrement health

		if (tc.can_be_accelerated && gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED) {
			breakable.health += breakable.degrade_speed_per_ms * tc.target_time_control_factor * elapsed_ms_since_last_update;
		}
	}

	if (breakable.health <= 0.f) {
		destroy_breakable_platform(entity);
	}
}

void WorldSystem::destroy_breakable_platform(Entity entity) {
	const Motion& motion = registry.motions.get(entity);
	const float fragment_size = std::min(std::min(motion.scale.x, motion.scale.y), (float)TILE_TO_PIXELS);
	const int fragment_count = (int)(motion.scale.x * motion.scale.y / (fragment_size * fragment_size));
	const vec2 player_pos = registry.motions.get(registry.players.entities[0]).position;

	// Fragments
	for (int i = 0; i < fragment_count; i++) {
		vec2 fragment_position = random_sample_rectangle(motion.position, motion.scale);

		ParticleSystem::spawn_particle(PARTICLE_ID::BREAKABLE_FRAGMENTS,
			fragment_position,
			0.0f, vec2{ fragment_size, fragment_size },
			30.0f * safe_normalize(fragment_position - player_pos) + rand_direction() * 15.0f,
			800.0, 1.0f, { 0.0f, 500.0f });
	}

	// Random dusts
	for (int i = 0; i < 30; i++) {
		vec2 dust_position = random_sample_rectangle(motion.position, motion.scale);

		ParticleSystem::spawn_particle(vec3{ 0.15f, 0.15f, 0.15f },
			dust_position,
			0.0f, vec2{ 2.0, 2.0 },
			rand_direction() * 30.0f,
			500.0, 0.8f, { 0.0f, 0.0f }, { 0.0f, 200.0f },
			0.0, 1.0);
	}

	registry.remove_all_components_of(entity);
}