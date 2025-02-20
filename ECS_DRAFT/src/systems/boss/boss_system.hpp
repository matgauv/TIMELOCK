#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "systems/ISystem.hpp"

class BossSystem : public ISystem
{
    public:
        BossSystem();
        ~BossSystem();

        void init(GLFWwindow* window) override;
        void step(float elapsed_ms) override;
        void late_step(float elapsed_ms) override;

    private:
        void attack(vec2 boss_position);

        std::default_random_engine rng;
	    std::uniform_real_distribution<float> uniform_dist;
};