#pragma once

#include <chrono>

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "systems/ISystem.hpp"

// Particle System
class ParticleSystem : public ISystem
{
public:
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;

	static unsigned int spawn_particle(
		PARTICLE_ID particle_id, 
		vec2 pos, float angle, vec2 scale, vec2 velocity, 
		float life, 
		float alpha = 1.0, vec2 fade_in_out = vec2{ 0.0, 0.0 }, vec2 shrink_in_out = vec2{ 0.0, 0.0 },
		float wind_influence = 0.0, float gravity_influence = 0.0, float turbulence_influence = 0.0);

	static unsigned int spawn_particle(vec3 color, vec2 pos, float angle, vec2 scale, vec2 velocity,
		float life,
		float alpha = 1.0, vec2 fade_in_out = vec2{ 0.0, 0.0 }, vec2 shrink_in_out = vec2{ 0.0, 0.0 },
		float wind_influence = 0.0, float gravity_influence = 0.0, float turbulence_influence = 0.0);

	static void set_wind(float strength, vec2 direction);
	static void set_gravity(float strength, vec2 direction);
	static void set_turbulence(float strength, float scale);

	static void emit_elliptical_particles(vec2 center, vec2 dimension, float angle_rad, int count, float local_speed, vec2 global_velocity, vec3 color, float size, float life);

	ParticleSystem()
	{
	}
private:
	GLFWwindow* window = nullptr;

	static Entity set_basic_particle(
		PARTICLE_ID particle_id,
		vec2 pos, float angle, vec2 scale, vec2 velocity,
		float life,
		float alpha = 1.0, vec2 fade_in_out = vec2{ 0.0, 0.0 }, vec2 shrink_in_out = vec2{0.0, 0.0},
		float wind_influence = 0.0, float gravity_influence = 0.0, float turbulence_influence = 0.0);

	static bool handle_particle_type(Entity entity, PARTICLE_ID particle_id);

	static vec3 seeded_pseudo_random(vec3 input);
	static float sample_from_gradient_noise(vec3 input);
	static float sample_from_turbulence(vec3 coord);
};