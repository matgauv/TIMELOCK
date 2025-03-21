#pragma once

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

	static bool spawn_particle(
		PARTICLE_ID particle_id, 
		vec2 pos, float angle, vec2 scale, vec2 velocity, 
		float life, 
		float alpha = 1.0, vec2 fade_in_out = vec2{ 0.0, 0.0 }, vec2 shrink_in_out = vec2{ 0.0, 0.0 },
		float wind_influence = 0.0, float gravity_influence = 0.0, float turbulence_influence = 0.0);

	static bool spawn_particle(vec3 color, vec2 pos, float angle, vec2 scale, vec2 velocity,
		float life,
		float alpha = 1.0, vec2 fade_in_out = vec2{ 0.0, 0.0 }, vec2 shrink_in_out = vec2{ 0.0, 0.0 },
		float wind_influence = 0.0, float gravity_influence = 0.0, float turbulence_influence = 0.0);

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
};