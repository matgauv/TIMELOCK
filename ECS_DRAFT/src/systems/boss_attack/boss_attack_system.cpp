#include "boss_attack_system.hpp"
#include "../world/world_init.hpp"
#incldue "boss_one_attack_init.hpp"
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
    (void) elapsed_ms;
}

void BossAttackSystem::late_step(float elapsed_ms) {
    (void) elapsed_ms;
}