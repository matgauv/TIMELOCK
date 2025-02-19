#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "tinyECS/components.hpp"

void load_level(std::string descriptor_path);

Entity create_player(vec2 position, vec2 scale);
Entity create_static_platform(vec2 position, vec2 scale);
Entity create_moving_platform(vec2 scale, std::vector<Path> movements);
