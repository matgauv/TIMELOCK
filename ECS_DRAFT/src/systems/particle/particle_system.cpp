#include "particle_system.hpp"

void ParticleSystem::init(GLFWwindow* window) {
	this->window = window;
}

void ParticleSystem::step(float elapsed_ms) {
	TIME_CONTROL_STATE current_state = registry.gameStates.components[0].game_time_control_state;

	float time_factor =
		((current_state == TIME_CONTROL_STATE::DECELERATED) ? DECELERATE_FACTOR :
			current_state == TIME_CONTROL_STATE::ACCELERATED ? ACCELERATE_FACTOR : NORMAL_FACTOR);

	for (int i = registry.particles.size() - 1; i >= 0; i--) {
		const Entity entity = registry.particles.entities[i];
		Particle &particle = registry.particles.components[i];

		// Eliminate if out of camera range
		vec2 camera_pos = registry.motions.get(registry.cameras.entities[0]).position;
		if (glm::length(particle.position - camera_pos) > MAX_CAMERA_DISTANCE) {
			registry.remove_all_components_of(entity);
		}
		
		particle.timer += (time_factor * elapsed_ms);

		// Eliminate if dead
		if (particle.timer > particle.life) {
			registry.remove_all_components_of(entity);
		}

		// Update motion
		particle.position += (time_factor * elapsed_ms * particle.velocity * 0.001f);
		particle.angle += (time_factor * elapsed_ms * particle.ang_velocity * 0.001f);
		particle.angle = fmod(fmod(particle.angle, 360.0f) + 360.0f, 360.0f);
	}
}

void ParticleSystem::late_step(float elapsed_ms) {
}

// Spawn with arbitrary particle id
bool ParticleSystem::spawn_particle(
	PARTICLE_ID particle_id, 
	vec2 pos, float angle, vec2 scale, vec2 velocity,
	float life,
	float alpha, vec2 fade_in_out, vec2 shrink_in_out,
	float wind_influence, float gravity_influence, float turbulence_influence) {

	if (registry.particles.size() > PARTICLE_COUNT_LIMIT) {
		return false;
	}

	if (fade_in_out[0] < 0 || fade_in_out[1] < 0 || (fade_in_out[0] + fade_in_out[1] > life) ||
		shrink_in_out[0] < 0 || shrink_in_out[1] < 0 || (shrink_in_out[0] + shrink_in_out[1] > life)) {
		return false;
	}

	Entity entity = set_basic_particle(
		particle_id,
		pos, angle, scale, velocity,
		life,
		alpha, fade_in_out, shrink_in_out,
		wind_influence, gravity_influence, turbulence_influence);
	
	return handle_particle_type(entity, particle_id);
}

// Spawn with color
bool ParticleSystem::spawn_particle(
	vec3 color,
	vec2 pos, float angle, vec2 scale, vec2 velocity,
	float life,
	float alpha, vec2 fade_in_out, vec2 shrink_in_out,
	float wind_influence, float gravity_influence, float turbulence_influence) {

	if (registry.particles.size() > PARTICLE_COUNT_LIMIT) {
		return false;
	}

	if (fade_in_out[0] < 0 || fade_in_out[1] < 0 || (fade_in_out[0] + fade_in_out[1] > life) ||
		shrink_in_out[0] < 0 || shrink_in_out[1] < 0 || (shrink_in_out[0] + shrink_in_out[1] > life)) {
		return false;
	}

	Entity entity = set_basic_particle(
		PARTICLE_ID::COLORED,
		pos, angle, scale, velocity,
		life,
		alpha, fade_in_out, shrink_in_out,
		wind_influence, gravity_influence, turbulence_influence);

	registry.colors.emplace(entity, color);

	return true;
}

Entity ParticleSystem::set_basic_particle(
	PARTICLE_ID particle_id,
	vec2 pos, float angle, vec2 scale, vec2 velocity,
	float life,
	float alpha, vec2 fade_in_out, vec2 shrink_in_out,
	float wind_influence, float gravity_influence, float turbulence_influence) {

	Entity entity = Entity();

	Particle& particle = registry.particles.emplace(entity);
	particle.particle_id = particle_id;
	particle.alpha = alpha;
	particle.life = life;
	particle.timer = 0;
	particle.fade_in_out = fade_in_out;
	particle.shrink_in_out = shrink_in_out;
	particle.wind_influence = wind_influence;
	particle.gravity_influence = gravity_influence;
	particle.turbulence_influence = turbulence_influence;

	particle.angle = angle;
	particle.position = pos;
	particle.scale = scale;
	particle.velocity = velocity;

	return entity;
}

bool ParticleSystem::handle_particle_type(Entity entity, PARTICLE_ID particle_id) {
	bool success = true;

	switch(particle_id) {
		case PARTICLE_ID::COLORED:
			registry.colors.emplace(entity, vec3(0.0));
			break;
		case PARTICLE_ID::SAMPLED_TEXTURE:
			break;
		default:
			success = false;
			break;
	}

	if (success) {
		registry.renderRequests.insert(entity, {
			TEXTURE_ASSET_ID::BLACK,
			EFFECT_ASSET_ID::PARTICLE_INSTANCED,
			GEOMETRY_BUFFER_ID::SPRITE});
	}
	else {
		registry.remove_all_components_of(entity);
	}

	return success;
}