#pragma once

#include "../../../common.hpp"
#include "../../../tinyECS/component_container.hpp"
#include "../../../tinyECS/components.hpp"
#include "../../../tinyECS/registry.hpp"
#include "../../particle/particle_system.hpp"
#include "../../camera/camera_system.hpp"

void update_boss_halo(const Entity boss_entity, const Boss& boss);

void emit_broken_parts(const Motion& boss_motion);

void slam_effect(const Motion& boss_motion);

void emit_gathering_particle(vec2 center, float radius, float life, vec3 color);
