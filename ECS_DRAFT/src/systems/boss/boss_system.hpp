#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "systems/ISystem.hpp"
#include <random>

class BossSystem : public ISystem
{
    public:
        BossSystem();
        ~BossSystem();

        void init(GLFWwindow* window) override;
        void step(float elapsed_ms) override;
        void late_step(float elapsed_ms) override;

    private:
        GLFWwindow* window = nullptr;
        void handleBossOneState(float elapsed_ms);
        void handleBossTwoState(float elpased_ms);
        void handleFinalBossState(float elapsed_ms);

        BOSS_STATE handleBossOneChooseAttackState(float elapsed_ms);

        bool isAttackOffCooldown(BossAttackList& table, BOSS_ATTACK_ID attack_id);

        std::default_random_engine rng;
	    std::uniform_real_distribution<float> uniform_dist;
};