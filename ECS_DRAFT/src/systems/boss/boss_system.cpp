#include "boss_system.hpp"
#include "../world/world_init.hpp"
#include <iostream>

BossSystem::BossSystem() {
    rng = std::default_random_engine(std::random_device()());
    nextAttacks = std::vector<BOSS_ATTACK_ID>(28); // a vector of size 28 
    std::cout << "Initializing boss sytem nextAttacks vector" << std::endl;
    for (int i = 0; i < nextAttacks.size(); i++) {
        std::cout << "Iteration i = " << i << std::endl;
        std::cout << "Adding " << i % (int) BOSS_ATTACK_ID::TOTAL_COUNT << " to boss sytem nextAttacks vector" << std::endl;
        nextAttacks.push_back(static_cast<BOSS_ATTACK_ID>(i % (int) BOSS_ATTACK_ID::TOTAL_COUNT));
    }
    std::shuffle(nextAttacks.begin(), nextAttacks.end(), rng);
}

BossSystem::~BossSystem() {

}

void BossSystem::init(GLFWwindow* window) {
    this->window = window;
}

void BossSystem::step(float elapsed_ms) {
    assert(registry.gameStates.components.size() <= 1);
    GameState& gameState = registry.gameStates.components[0];

    if (registry.bosses.entities.size() == 1) {
        Entity& boss_entity = registry.bosses.entities[0];
        Entity& player_entity = registry.players.entities[0];
        Motion& player_motion = registry.motions.get(player_entity);
        Motion& boss_motion = registry.motions.get(boss_entity);
    
        if (abs(player_motion.position.x - boss_motion.position.x) <= 200.f) {
            gameState.is_in_boss_fight = true;
        }
    } 

    if (gameState.is_in_boss_fight) {
        // this assumes that the boss to be looked at is always the first boss component
        Boss& boss = registry.bosses.components[0];
        Entity& boss_entity = registry.bosses.entities[0];

        // call the appropriate helper function to handle the state changes
        if (boss.boss_id == BOSS_ID::FIRST) {
            unsigned int random_num = (unsigned int) (uniform_dist(rng) * 100);
            boss_one_step(boss_entity, elapsed_ms, random_num, rng);
        }
    }
}

void BossSystem::late_step(float elapsed_ms) {
    (void) elapsed_ms;
}