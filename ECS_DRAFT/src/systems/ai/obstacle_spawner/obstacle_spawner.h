#include "../../../common.hpp"
#include "../../../tinyECS/component_container.hpp"
#include "../../../tinyECS/components.hpp"
#include "../../../tinyECS/registry.hpp"
#include "systems/ISystem.hpp"
#include "../../world/world_init.hpp"
#include "../../particle/particle_system.hpp"

void obstacle_spawner_step(float elapsed_ms);

void spikeball_effects(vec2 pos);