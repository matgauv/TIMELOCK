#include "boss_attack_system.hpp"
#include "../world/world_init.hpp"
#incldue "boss_one_attack_init.hpp"
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

        if (boss.health > 0.f) {
            boss.attack_cooldown_ms -= elapsed_ms;

            Motion& motion = registry.motions.get(boss_entity);
            
            if (boss.attack_cooldown_ms <= 0.f) {
                attack(motion.position);
                boss.attack_cooldown_ms = BOSS_ATTACK_COOLDOWN_MS;
            }
        }
    }
}

void BossSystem::late_step(float elapsed_ms) {
    (void) elapsed_ms;
}