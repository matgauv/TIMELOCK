#pragma once

#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../rendering/render_system.hpp"

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// debugging red lines
Entity createLine(vec2 position, vec2 size);
