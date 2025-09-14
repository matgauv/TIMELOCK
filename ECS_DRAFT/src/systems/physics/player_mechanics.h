#include "../../common.hpp"
#include "../../tinyECS/component_container.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/registry.hpp"
#include "./physics_utils.h"

void player_climb(Entity& entity, Motion& motion, float step_seconds);
void player_walk(Entity& entity, Motion& motion, float step_seconds);