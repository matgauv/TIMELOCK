#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"

void useBossOneRegularProjectile(Entity& boss_entity, Entity& player_entity, float elapsed_time);
void useBossOneFastProjectile(Entity& boss_entity, Entity& player_entity, float elapsed_time);
void useBossOneDelayedProjectile(Entity& boss_entity, Entity& player_entity, float elapsed_time);
void useBossOneGroundSlam(Entity& boss_entity, Entity& player_entity, float elapsed_time);
void useBossOneDashAttack(Entity& boss_entity, Entity& player_entity, float elapsed_time);
Entity*& getBossAttackEntity(Boss& boss);
int getBossAttackId(Boss& boss);
Entity* getDelayedEntity(int i);