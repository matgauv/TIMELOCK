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
        void handleBossOneState(Entity& boss_entity, float elapsed_ms);
        void handleBossTwoState(Entity& boss_entity, float elpased_ms);
        void handleFinalBossState(Entity& boss_entity, float elapsed_ms);

        BOSS_STATE handleBossOneChooseAttackState(Entity& boss_entity, float elapsed_ms, bool is_in_phase_two);

        bool isAttackOffCooldown(BossAttackList& table, BOSS_ATTACK_ID attack_id);

        std::default_random_engine rng;
	    std::uniform_real_distribution<float> uniform_dist;
};