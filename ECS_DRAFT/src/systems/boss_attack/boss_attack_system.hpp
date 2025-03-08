#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "systems/ISystem.hpp"
#include <random>

class BossAttackSystem : public ISystem
{
    public:
        BossAttackSystem();
        ~BossAttackSystem();

        void init(GLFWwindow* window) override;
        void step(float elapsed_ms) override;
        void late_step(float elapsed_ms) override;

    private:
        GLFWwindow* window = nullptr;

        void handleBossOneAttack(Entity& boss_entity, Entity& player_entity, float elapsed_time);
        void handleBossTwoAttack(Entity& boss_entity, Entity& player_entity, float elapsed_time);
        void handleFinalBossAttack(Entity& boss_entity, Entity& player_entity, float elapsed_time);

        // TODO: copied from boss_system, will determine whether they are needed or not in future M2 PR
        // std::default_random_engine rng;
	    // std::uniform_real_distribution<float> uniform_dist;
};