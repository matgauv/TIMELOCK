#pragma once

#include "../../../common.hpp"
#include "../../../tinyECS/component_container.hpp"
#include "../../../tinyECS/components.hpp"
#include "../../../tinyECS/registry.hpp"
#include "systems/world/world_system.hpp"


void pipe_step(float elapsed_ms);
void screw_step(float elapsed_ms);

void fire_screw(vec2 position, vec2 velocity);

void destroy_screw(const Entity entity);