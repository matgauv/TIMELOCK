#include <iostream>
#include "particle_system.hpp"

void ParticleSystem::init(GLFWwindow* window) {
	this->window = window;

	Entity entity = Entity();
	ParticleSystemState &state = registry.particleSystemStates.emplace(entity);
	//state.wind_field = {350.0, -50.0};
	state.turbulence_strength = 150.0f;
	state.turbulence_scale = TURBULENCE_GRID_SIZE;
}

void ParticleSystem::step(float elapsed_ms) {
	TIME_CONTROL_STATE current_state = registry.gameStates.components[0].game_time_control_state;

	float time_factor =
		((current_state == TIME_CONTROL_STATE::DECELERATED) ? DECELERATE_FACTOR :
			current_state == TIME_CONTROL_STATE::ACCELERATED ? ACCELERATE_FACTOR : NORMAL_FACTOR);

	const ParticleSystemState& system_state = registry.particleSystemStates.components[0];


	// Get current system time: https://www.geeksforgeeks.org/how-to-get-time-in-milliseconds-in-cpp/
	auto time_passage = std::chrono::system_clock::now().time_since_epoch();
	float system_time = (float)std::chrono::duration_cast<std::chrono::milliseconds>(time_passage).count() * TURBULENCE_EVOLUTION_SPEED;

	for (int i = registry.particles.size() - 1; i >= 0; i--) {
		const Entity entity = registry.particles.entities[i];
		Particle &particle = registry.particles.components[i];

		const float time_change_s = time_factor * elapsed_ms * 0.001f;

		if (particle.particle_id != PARTICLE_ID::CRACKING_RADIAL) {
			// Eliminate if out of camera range
			vec2 camera_pos = registry.motions.get(registry.cameras.entities[0]).position;
			if (glm::length(particle.position - camera_pos) > MAX_CAMERA_DISTANCE) {
				registry.remove_all_components_of(entity);
			}

			particle.timer += time_change_s * 1000.0f;

			// Eliminate if dead
			if (particle.timer > particle.life) {
				registry.remove_all_components_of(entity);
			}
		}

		// Update motion
		particle.position += (time_change_s * particle.velocity);
		particle.angle += (time_change_s * particle.ang_velocity);
		particle.angle = fmod(fmod(particle.angle, 360.0f) + 360.0f, 360.0f);

		if (abs(particle.wind_influence) > 1e-4) {
			particle.position += (time_change_s * system_state.wind_field * particle.wind_influence);
		}

		if (abs(particle.gravity_influence) > 1e-4) {
			particle.velocity += (time_change_s * system_state.gravity_field * particle.gravity_influence);
		}

		if (abs(particle.turbulence_influence) > 1e-4 && system_state.turbulence_strength > 1e-4 && system_state.turbulence_scale > 1e-2) {
			particle.velocity += 
				(time_change_s * system_state.turbulence_strength * angle_to_direction(
					M_PI * 2.0f * sample_from_turbulence(vec3(particle.position / system_state.turbulence_scale, system_time))));
		}

		// Handle different particles
		switch (particle.particle_id)
		{
			case PARTICLE_ID::COYOTE_PARTICLES:
				// Velocity decay
				particle.velocity *= 0.9f;
				break;
			default:
				break;
		}
	}
}

void ParticleSystem::late_step(float elapsed_ms) {
}

// Spawn with arbitrary particle id
unsigned int ParticleSystem::spawn_particle(
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
	
	if (handle_particle_type(entity, particle_id)) {
		return entity.id();
	}

	return 0;
}

// Spawn with color
unsigned int ParticleSystem::spawn_particle(
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

	return entity.id();
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
		case PARTICLE_ID::BREAKABLE_FRAGMENTS:
		{
			Particle& par = registry.particles.get(entity);
			par.gravity_influence = 0.5f;
			par.angle = rand_float(-15.f, 15.f);
			par.ang_velocity = rand_float(-50.f, 50.f);

			registry.animateRequests.emplace(entity).used_animation = ANIMATION_ID::BREAKABLE_FRAGMENTS;
			break;
		}
		case PARTICLE_ID::SCREW_FRAGMENTS:
		{
			Particle& par = registry.particles.get(entity);
			par.gravity_influence = 0.5f;
			par.angle = rand_float(-15.f, 15.f);
			par.ang_velocity = rand_float(-50.f, 50.f);

			registry.animateRequests.emplace(entity).used_animation = ANIMATION_ID::SCREW_FRAGMENTS;
			break;
		}
		case PARTICLE_ID::HEX_FRAGMENTS:
		{
			Particle& par = registry.particles.get(entity);
			par.gravity_influence = 0.5f;
			par.angle = rand_float(-15.f, 15.f);
			par.ang_velocity = rand_float(-50.f, 50.f);

			registry.animateRequests.emplace(entity).used_animation = ANIMATION_ID::HEX_FRAGMENTS;
			break;
		}
		case PARTICLE_ID::COYOTE_PARTICLES: {
			registry.animateRequests.emplace(entity).used_animation = ANIMATION_ID::COYOTE_PARTICLES;
			break;
		}
		case PARTICLE_ID::CRACKING_RADIAL: {
			Particle& par = registry.particles.get(entity);
			par.angle = (int)(rand_float(0.0, 4.0)) * 90.0f;

			registry.animateRequests.emplace(entity).used_animation = ANIMATION_ID::CRACKING_RADIAL;
			break;
		}
		case PARTICLE_ID::CRACKING_DOWNWARD: {
			registry.animateRequests.emplace(entity).used_animation = ANIMATION_ID::CRACKING_DOWNWARD;
			break;
		}
		case PARTICLE_ID::EXHALE: {
			Particle& par = registry.particles.get(entity);
			par.angle = rand_float(0.0f, 360.0f);
			par.ang_velocity = rand_float(10.0, 20.0f) * glm::sign(par.velocity.x);
			par.gravity_influence = -0.05f;

			registry.animateRequests.emplace(entity).used_animation = ANIMATION_ID::EXHALE;
			break;
		}
		case PARTICLE_ID::BROKEN_PARTS: {
			Particle& par = registry.particles.get(entity);
			par.angle = rand_float(0.0f, 360.0f);
			par.ang_velocity = rand_float(-20.0f, 20.0f);
			par.gravity_influence = 0.3f;

			registry.animateRequests.emplace(entity).used_animation = ANIMATION_ID::BROKEN_PARTS;
			break;
		}
		case PARTICLE_ID::CROSS_STAR: {
			Particle& par = registry.particles.get(entity);
			par.angle = rand_float(0.0f, 360.0f);
			par.ang_velocity = rand_float(-180.0f, 180.0f);
			break;
		}
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

// GPU-styled turbulence generation
vec3 ParticleSystem::seeded_pseudo_random(vec3 input) {
	// If you sense magical numbers here, we do exactly need them
	vec3 raw_result = 11.464f * vec3{ 
		abs(sinf(glm::dot(input, vec3{63.36f, -95.52f, 12.41f}))),
		abs(sinf(glm::dot(input, vec3{84.14f, 18.63f, 84.48f}))),
		abs(sinf(glm::dot(input, vec3{98.61f, 36.813f, 48.351f})))};

	return glm::fract(raw_result) * 2.0f - 1.0f;
}

float ParticleSystem::sample_from_gradient_noise(vec3 input) {
	vec3 grid_int = glm::floor(input);
	vec3 grid_frac = input - grid_int;

	// 8 grid points
	float grid_000 = glm::dot(seeded_pseudo_random(grid_int), grid_frac);
	float grid_100 = glm::dot(seeded_pseudo_random(grid_int + vec3{ 1.0f, 0.0f, 0.0f }), grid_frac - vec3{ 1.0f, 0.0f, 0.0f });
	float grid_010 = glm::dot(seeded_pseudo_random(grid_int + vec3{ 0.0f, 1.0f, 0.0f }), grid_frac - vec3{ 0.0f, 1.0f, 0.0f });
	float grid_110 = glm::dot(seeded_pseudo_random(grid_int + vec3{ 1.0f, 1.0f, 0.0f }), grid_frac - vec3{ 1.0f, 1.0f, 0.0f });
	float grid_001 = glm::dot(seeded_pseudo_random(grid_int + vec3{ 0.0f, 0.0f, 0.1f }), grid_frac - vec3{ 0.0f, 0.0f, 0.1f });
	float grid_101 = glm::dot(seeded_pseudo_random(grid_int + vec3{ 1.0f, 0.0f, 0.1f }), grid_frac - vec3{ 1.0f, 0.0f, 0.1f });
	float grid_011 = glm::dot(seeded_pseudo_random(grid_int + vec3{ 0.0f, 1.0f, 0.1f }), grid_frac - vec3{ 0.0f, 1.0f, 0.1f });
	float grid_111 = glm::dot(seeded_pseudo_random(grid_int + vec3{ 1.0f, 1.0f, 0.1f }), grid_frac - vec3{ 1.0f, 1.0f, 0.1f });

	// The GLSL smoothstep function: https://registry.khronos.org/OpenGL-Refpages/gl4/html/smoothstep.xhtml
	vec3 factor = grid_frac * grid_frac * (3.0f - 2.0f * grid_frac);

	return lerpToTarget(
		lerpToTarget(lerpToTarget(grid_000, grid_100, factor.x), lerpToTarget(grid_010, grid_110, factor.x), factor.y),
		lerpToTarget(lerpToTarget(grid_001, grid_101, factor.x), lerpToTarget(grid_011, grid_111, factor.x), factor.y), factor.z);
}

float ParticleSystem::sample_from_turbulence(vec3 coord) {
	float result = 0.0;
	float amplitude = 4.0;

	for (int i = 0; i < TURBULENCE_OCTAVES; i++) {
		// absolute ensures turbulencer behavior: https://thebookofshaders.com/13/
		result += (amplitude * abs(sample_from_gradient_noise(coord)));
		coord *= 0.5f;
		amplitude *= 2.0f;
	}

	return result;
}

void ParticleSystem::set_wind(float strength, vec2 direction) {
	registry.particleSystemStates.components[0].wind_field = strength * direction;
}

void ParticleSystem::set_gravity(float strength, vec2 direction) {
	registry.particleSystemStates.components[0].gravity_field = strength * direction;
}

void ParticleSystem::set_turbulence(float strength, float scale) {
	registry.particleSystemStates.components[0].turbulence_scale = scale;
	registry.particleSystemStates.components[0].turbulence_strength = strength;
}

// Consider migrate to world system
void ParticleSystem::emit_elliptical_particles(vec2 center, vec2 dimension, float angle_rad, int count, float local_speed, vec2 global_velocity, vec3 color, float size, float life) {
	for (int i = 0; i < count; i++) {
		vec2 disp_dir = rotate_2D(rand_direction() * dimension, angle_rad);

		vec2 velocity = local_speed * disp_dir + global_velocity;

		ParticleSystem::spawn_particle(color,
			center, 0.0f,
			vec2(size), velocity,
			life, 1.0f, { 0.0f, 0.35f * life }, { 0.1f * life, 0.35f * life });
	}
}