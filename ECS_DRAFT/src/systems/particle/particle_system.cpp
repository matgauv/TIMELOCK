#include "particle_system.hpp"

void ParticleSystem::init(GLFWwindow* window) {
	this->window = window;
}

void ParticleSystem::step(float elapsed_ms) {
	for (int i = registry.particles.size() - 1; i >= 0; i--) {
		const Entity entity = registry.particles.entities[i];
		Particle &particle = registry.particles.components[i];
		Motion& motion = registry.motions.get(entity);
		
		particle.timer += motion.velocityModifier * elapsed_ms;

		if (particle.timer > particle.life) {
			registry.remove_all_components_of(entity);
		}
	}
}

void ParticleSystem::late_step(float elapsed_ms) {
}

// Spawn with arbitrary particle id
bool ParticleSystem::spawn_particle(
	PARTICLE_ID particle_id, 
	vec2 pos, float angle, vec2 scale, vec2 velocity, 
	float life, 
	float alpha, vec2 fade_in_out) {

	if (registry.particles.size() > PARTICLE_COUNT_LIMIT) {
		return false;
	}

	Entity entity = set_basic_particle(
		particle_id,
		pos, angle, scale, velocity,
		life,
		alpha, fade_in_out);
	
	return handle_particle_type(entity, particle_id);
}

// Spawn with color
bool ParticleSystem::spawn_particle(
	vec3 color,
	vec2 pos, float angle, vec2 scale, vec2 velocity,
	float life,
	float alpha, vec2 fade_in_out) {

	if (registry.particles.size() > PARTICLE_COUNT_LIMIT) {
		return false;
	}

	Entity entity = set_basic_particle(
		PARTICLE_ID::COLORED,
		pos, angle, scale, velocity,
		life,
		alpha, fade_in_out);

	registry.colors.emplace(entity, color);

	return true;
}

Entity ParticleSystem::set_basic_particle(
	PARTICLE_ID particle_id,
	vec2 pos, float angle, vec2 scale, vec2 velocity,
	float life,
	float alpha, vec2 fade_in_out) {

	Entity entity = Entity();

	Particle& particle = registry.particles.emplace(entity);
	particle.particle_id = particle_id;
	particle.alpha = alpha;
	particle.life = life;
	particle.timer = 0;

	Motion& motion = registry.motions.emplace(entity);
	motion.angle = angle;
	motion.position = pos;
	motion.scale = scale;
	motion.velocity = velocity;

	TimeControllable& tc = registry.timeControllables.emplace(entity);

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