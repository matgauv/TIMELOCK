#pragma once
#include "systems/ISystem.hpp"

class LevelParsingSystem : public ISystem
{
public:
    void init(GLFWwindow* window) override;
    void step(float elapsed_ms) override;
    void late_step(float elapsed_ms) override;

    LevelParsingSystem()
    {
    }
private:
    GLFWwindow* window = nullptr;
};
