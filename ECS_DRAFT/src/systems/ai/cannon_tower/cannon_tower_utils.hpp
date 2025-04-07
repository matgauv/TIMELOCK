#pragma once

#include "../../../common.hpp"
#include "../../../tinyECS/component_container.hpp"
#include "../../../tinyECS/components.hpp"
#include "../../../tinyECS/registry.hpp"

#include "../../particle/particle_system.hpp"

void cannon_tower_step(float elapsed_ms);

void idle_step(Entity tower_entity, CannonTower& tower, float elapsed_ms);

void aiming_step(Entity tower_entity, CannonTower& tower, float elapsed_ms);

void loading_step(Entity tower_entity, CannonTower& tower, float elapsed_ms);

void firing_step(Entity tower_entity, CannonTower& tower, float elapsed_ms);

bool player_detected(Entity tower_entity, CannonTower& tower);

void cannon_fire(Entity tower_entity, float angle);