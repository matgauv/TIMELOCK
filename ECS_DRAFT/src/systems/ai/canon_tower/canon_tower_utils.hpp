#pragma once

#include "../../../common.hpp"
#include "../../../tinyECS/component_container.hpp"
#include "../../../tinyECS/components.hpp"
#include "../../../tinyECS/registry.hpp"

void canon_tower_step(float elapsed_ms);

void idle_step(Entity tower_entity, CanonTower& tower, float elapsed_ms);

void aiming_step(Entity tower_entity, CanonTower& tower, float elapsed_ms);

void loading_step(Entity tower_entity, CanonTower& tower, float elapsed_ms);

void firing_step(Entity tower_entity, CanonTower& tower, float elapsed_ms);

bool player_detected(Entity tower_entity, CanonTower& tower);

void canon_fire(Entity tower_entity, float angle);