#pragma once

#include "../../common.hpp"
#include <string>
#include "../../tinyECS/component_container.hpp"

void load_level(std::string descriptor_path);

Entity create_player(vec2 position, vec2 scale);
Entity create_platform(vec2 position, vec2 scale);
Entity create_camera(vec2 position, vec2 scale);