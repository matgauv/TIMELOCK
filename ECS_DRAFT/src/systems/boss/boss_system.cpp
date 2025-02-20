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
    aasert(registry.gameStates.components.size() <= 1);
    GameState& gameState = registry.gameStates.components[0];

    if (gameState.is_in_boss_fight) {
        // this assumes that the boss to be looked at is always the first boss component
        Boss& boss = registry.bosses.components[0];
        if (boss.health > 0.f) {
            boss.attack_cooldown_ms -= elapsed_ms;

            Motion& motion = registry.motions.get(boss);
            
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

void BossSystem::attack(vec2 boss_position) {
    // FUTURE TODO: use rng and uniformdist to determine which attack the boss will use next
    Player& player = registry.players.components[0];
    Motion& player_motion = registry.motions.get(player);
    
    create_projectile(vec2(boss_position.x, player_motion.position.y), 
        vec2(PROJECTILE_WIDTH_PX, PROJECTILE_HEIGHT_PX),
        vec2(boss_position.x < player_motion.position.x ? -FIRST_BOSS_PROJECTILE_SPEED : FIRST_BOSS_PROJECTILE_SPEED, 0.f));
}