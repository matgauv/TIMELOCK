#include "boss_system.hpp"
#include "../world/world_init.hpp"
#include <iostream>

BossSystem::BossSystem() {
    rng = std::default_random_engine(std::random_device()());
}

BossSystem::~BossSystem() {

}

void BossSystem::init(GLFWwindow* window) {
    this->window = window;
}

void BossSystem::step(float elapsed_ms) {
    assert(registry.gameStates.components.size() <= 1);
    GameState& gameState = registry.gameStates.components[0];

    if (gameState.is_in_boss_fight) {
        // this assumes that the boss to be looked at is always the first boss component
        Boss& boss = registry.bosses.components[0];
        Entity& boss_entity = registry.bosses.entities[0];

        // call the appropriate helper function to handle the state changes
        if (boss.boss_id == BOSS_ID::FIRST) {
            handleBossOneState(boss_entity, elapsed_ms);
        }
        
        if (boss.boss_id == BOSS_ID::SECOND) {
            handleBossTwoState(boss_entity, elapsed_ms);
        }

        if (boss.boss_id == BOSS_ID::FINAL) {
            handleFinalBossState(boss_entity, elapsed_ms);
        }
    }
}

void BossSystem::late_step(float elapsed_ms) {
    (void) elapsed_ms;
}

// void BossSystem::attack(vec2 boss_position) {
//     // FUTURE TODO: use rng and uniformdist to determine which attack the boss will use next
//     Entity player = registry.players.entities[0];
//     Motion& player_motion = registry.motions.get(player);
    
//     create_projectile(vec2(boss_position.x, player_motion.position.y), 
//         vec2(PROJECTILE_WIDTH_PX, PROJECTILE_HEIGHT_PX),
//         vec2(boss_position.x < player_motion.position.x ? -FIRST_BOSS_PROJECTILE_SPEED : FIRST_BOSS_PROJECTILE_SPEED, 0.f));
// }

void BossSystem::handleBossOneState(Entity& boss_entity, float elapsed_ms) {

    Boss& boss = registry.bosses.get(boss_entity);

    bool is_in_phase_two = (boss.health / BOSS_ONE_MAX_HEALTH) <= 0.5;
    
    if (boss.health <= 0.f) {
        // transition to dead state when health reaches 0
        boss.boss_state = BOSS_STATE::BOSS1_DEAD_STATE;

    } else if (boss.attack_cooldown_ms > 0.f) {
        // if the boss is alive and their attack cooldown has not reached zero yet, decrement the cooldown
        boss.attack_cooldown_ms -= elapsed_ms;
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;

    } else if (boss.attack_cooldown_ms <= 0.f &&  boss.boss_state == BOSS_STATE::BOSS1_MOVE_STATE) {
        // set the boss state to choose attack state
        boss.boss_state = BOSS_STATE::BOSS1_CHOOSE_ATTACK_STATE;

    } else if (boss.boss_state == BOSS_STATE::BOSS1_CHOOSE_ATTACK_STATE) {
        // call the helper to choose which attack to use
        BOSS_STATE chosen_state = handleBossOneChooseAttackState(boss_entity, elapsed_ms, is_in_phase_two);
        boss.boss_state = chosen_state;
    }

    // else keep the current state
}

BOSS_STATE BossSystem::handleBossOneChooseAttackState(Entity& boss_entity, float elpased_ms, bool is_in_phase_two) {
    // helper function that chooses the attack state for boss one using a decision tree

    // grab boss related info
    Boss& boss = registry.bosses.get(boss_entity);
    Motion& boss_motion = registry.motions.get(boss_entity);
    assert(registry.bossAttackLists.has(boss_entity));
    BossAttackList& bossAttackList = registry.bossAttackLists.get(boss_entity);

    // grab player related info
    Player& player = registry.players.components[0];
    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    float dist = abs(boss_motion.position.x - player_motion.position.x) / WINDOW_WIDTH_PX;

    if (dist < 0.3f) {
        // choose short range attack

        if (is_in_phase_two && isAttackOffCooldown(bossAttackList, BOSS_ATTACK_ID::BOSS1_GROUND_SLAM)) {
            // if ground slam is ready, use it
            boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_ATTACK_STATE;
            
        } else if (isAttackOffCooldown(bossAttackList, BOSS_ATTACK_ID::BOSS1_DELAYED_PROJECTILE)) {
            // else if delayed projectile is ready, use it
            boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;

        } else {
            // else just set to move state
            boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        }

    } else if (dist > 0.7f) {
        // choose long range attack

        if (is_in_phase_two && isAttackOffCooldown(bossAttackList, BOSS_ATTACK_ID::BOSS1_DASH_ATTACK)) {
            // if dash attack is ready, use it
            boss.boss_state = BOSS_STATE::BOSS1_DASH_ATTACK_STATE;

        } else if (isAttackOffCooldown(bossAttackList, BOSS_ATTACK_ID::BOSS1_FAST_PROJECTILE)) {
            // else if fast projectile is ready, use it
            boss.boss_state = BOSS_STATE::BOSS1_FAST_PROJECTILE_ATTACK_STATE;

        } else if (isAttackOffCooldown(bossAttackList, BOSS_ATTACK_ID::BOSS1_DELAYED_PROJECTILE)) {
            // else if delayed projectile is ready, use it
            boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;

        } else if (isAttackOffCooldown(bossAttackList, BOSS_ATTACK_ID::BOSS1_REGULAR_PROJECTILE)) {
            // else if regular projectile is ready, use it
            boss.boss_state = BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE;

        } else {
            // else just set to move state
            boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        }

    } else {
        // choose medium range attack
        if (is_in_phase_two && isAttackOffCooldown(bossAttackList, BOSS_ATTACK_ID::BOSS1_GROUND_SLAM)) {
            // if ground slam is ready, use it
            boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_ATTACK_STATE;

        } else if (isAttackOffCooldown(bossAttackList, BOSS_ATTACK_ID::BOSS1_DELAYED_PROJECTILE)) {
            // else if delayed projectile is ready, use it
            boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;
            
        } else if (isAttackOffCooldown(bossAttackList, BOSS_ATTACK_ID::BOSS1_REGULAR_PROJECTILE)) {
            // if regular projectile is ready, use it
            boss.boss_state = BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE;

        } else {
            // else just set to move state
            boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        }
    }
}

void BossSystem::handleBossTwoState(Entity& boss_entity, float elapsed_ms) {
    // TODO: to be implemented in future milestone
    (void) elapsed_ms;
}

void BossSystem::handleFinalBossState(Entity& boss_entity, float elapsed_ms) {
    // TODO: to be implemented in future milestone
    (void) elapsed_ms;
}

bool BossSystem::isAttackOffCooldown(BossAttackList& table, BOSS_ATTACK_ID attack_id) {
    return (table.boss_attack_table.at((int) attack_id)).cooldown_ms <= 0.f;
}