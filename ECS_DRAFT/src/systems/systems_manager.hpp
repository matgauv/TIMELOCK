#pragma once

#include "ISystem.hpp"
#include "../common.hpp"
#include <vector>


class SystemsManager
{
  public:
    SystemsManager() {
      create_window();
    }

    void run_game_loop();
    void register_system(ISystem* system);
    void register_fixed_system(ISystem* system);

    void create_window();
    void set_window(GLFWwindow* window) {this->window = window;};
    GLFWwindow* get_window() const {return window;};

    bool is_over() const;
  private:
    std::vector<ISystem*> systems;
    std::vector<ISystem*> fixed_systems;
    GLFWwindow* window = nullptr;
};