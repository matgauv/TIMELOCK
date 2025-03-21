#pragma once

#include "../../../common.hpp"
#include "../../../tinyECS/component_container.hpp"
#include "../../../tinyECS/components.hpp"
#include "../../../tinyECS/registry.hpp"

void delayed_projectile_step(float elapsed_ms);

vec2 calculate_velocity_vector(vec2 projectile_position, vec2 player_position);