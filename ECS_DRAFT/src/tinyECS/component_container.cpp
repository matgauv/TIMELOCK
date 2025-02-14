// internal
#include "component_container.hpp"

// All we need to store besides the containers is the id of every entity and callbacks to be able to remove entities across containers
unsigned int Entity::id_count = 1;