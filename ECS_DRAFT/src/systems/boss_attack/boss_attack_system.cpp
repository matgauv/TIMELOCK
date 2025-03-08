#include "boss_attack_system.hpp"
#include "../world/world_init.hpp"
#include "boss_one_attack_init.hpp"
#include <iostream>

BossAttackSystem::BossAttackSystem() {
    // rng = std::default_random_engine(std::random_device()());
}

BossAttackSystem::~BossAttackSystem() {

}

void BossAttackSystem::init(GLFWwindow* window) {
    this->window = window;
}

void BossAttackSystem::step(float elapsed_ms) {
    assert(registry.gameStates.components.size() <= 1);
    GameState& gameState = registry.gameStates.components[0];

    // TODO:
    // add attack cooldown related logic

    if (gameState.is_in_boss_fight) {
        // this assumes that the boss to be looked at is always the first boss component
        Boss& boss = registry.bosses.components[0];
        Entity& boss_entity = registry.bosses.entities[0];

        Player& player = registry.players.components[0];
        Entity& player_entity = registry.players.entities[0];

        // call the appropriate helper function to handle specific boss attack logic
        if (boss.boss_id == BOSS_ID::FIRST) {
            handleBossOneAttack(boss_entity, player_entity, elapsed_ms);
        }
        
        if (boss.boss_id == BOSS_ID::SECOND) {
            handleBossTwoAttack(boss_entity, player_entity, elapsed_ms);
        }

        if (boss.boss_id == BOSS_ID::FINAL) {
            handleFinalBossAttack(boss_entity, player_entity, elapsed_ms);
        }
    }
}

void BossAttackSystem::late_step(float elapsed_ms) {
    (void) elapsed_ms;
}

void BossAttackSystem::handleBossOneAttack(Entity& boss_entity, Entity& player_entity, float elapsed_time) {
    // get the boss component
    Boss& boss = registry.bosses.get(boss_entity);
    Motion& boss_motion = registry.motions.get(boss_entity);

    // NOTE: boss_motion.selfVelocity is a vec2, this is the acceleration caused by yourself
    // NOTE: the velocityModifier is just a single float number

    // call the appropriate helper function based on the attack state
    if (boss.boss_state == BOSS_STATE::BOSS1_GROUND_SLAM_ATTACK_STATE) {
        useBossOneGroundSlam(boss_entity, player_entity, elapsed_time);

    } else if (boss.boss_state == BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE) {
        useBossOneDelayedProjectile(boss_entity, player_entity, elapsed_time);

    } else if (boss.boss_state == BOSS_STATE::BOSS1_FAST_PROJECTILE_ATTACK_STATE) {
        useBossOneFastProjectile(boss_entity, player_entity, elapsed_time);

    } else if (boss.boss_state == BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE) {
        useBossOneRegularProjectile(boss_entity, player_entity, elapsed_time);

    } else if (boss.boss_state == BOSS_STATE::BOSS1_DASH_ATTACK_STATE) {
        useBossOneDashAttack(boss_entity, player_entity, elapsed_time);
    }
}

void BossAttackSystem::handleBossTwoAttack(Entity& boss_entity, Entity& player_entity, float elapsed_time) {
    // TODO: to be implemented in future milestone
    (void) elapsed_time;
}

void BossAttackSystem::handleFinalBossAttack(Entity& boss_entity, Entity& player_entity, float elapsed_time) {
    // TODO: to be implemented in future milestone
    (void) elapsed_time;
}