#pragma once

#include "common.hpp"
#include "../rendering/render_system.hpp"
#include "../../tinyECS/registry.hpp"

class AISystem
{
public:
	void step(float elapsed_ms);
};