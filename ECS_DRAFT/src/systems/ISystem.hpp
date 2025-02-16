#pragma once
#include "../common.hpp"

class ISystem
{
  public:
    virtual ~ISystem() = default;
    virtual void init(GLFWwindow* window) = 0;
    virtual void step(float elapsed_ms) = 0;
    virtual void late_step(float elapsed_ms) = 0;
};
