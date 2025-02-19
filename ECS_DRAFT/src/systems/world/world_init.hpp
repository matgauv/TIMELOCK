#pragma once

#include "../../common.hpp"
#include <string>
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"

void load_level(std::string descriptor_path);

Entity create_player(vec2 position, vec2 scale);
Entity create_platform(vec2 position, vec2 scale);
Entity create_camera(vec2 position, vec2 scale);
Entity create_background(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id);