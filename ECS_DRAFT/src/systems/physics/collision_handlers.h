#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "./physics_utils.h"
#include "../player/player_system.hpp"
#include "../world/world_system.hpp"
//#include <glm/trigonometric.hpp>

void handle_player_ladder_collision(Entity& player_entity, Entity& ladder_entity, int step_seconds);
void drop_bolt_when_player_near(float dist_threshold);
void handle_player_boss_collision(Entity& player_entity, Entity& boss_entity, Collision& collision);
void handle_projectile_collision(Entity& proj_entity, Entity& other_entity);
void handle_player_attack_collision(Entity& player_entity, Entity& attack_entity, Collision& collision);
void handle_player_breakable_collision(Entity& breakable_entity, float elapsed_ms);
void handle_player_door_collision();