#pragma once
#include <vector>

#include "component_container.hpp"
#include "components.hpp"

class ECSRegistry
{
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Platform> platforms;
	ComponentContainer<OnPlatform> onPlatforms;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<GameState> gameStates;
	ComponentContainer<TimeControllable> timeControllables;
	ComponentContainer<Harmful> harmfuls;
	ComponentContainer<Bolt> bolts;
	ComponentContainer<Pendulum> pendulums;
	ComponentContainer<Gear> gears;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Rock> rocks;
	ComponentContainer<WaterDrop> waterdrops;
	ComponentContainer<Falling> falling;
	ComponentContainer<Walking> walking;
	ComponentContainer<Blocked> blocked;
	ComponentContainer<Camera> cameras;
	ComponentContainer<Layer> layers;
	ComponentContainer<AnimateRequest> animateRequests;
	ComponentContainer<MovementPath> movementPaths;
	ComponentContainer<Boss> bosses;
	ComponentContainer<PhysicsObject> physicsObjects;
	ComponentContainer<Boundary> boundaries;
	ComponentContainer<BossAttack> bossAttacks;
	ComponentContainer<BossAttackList> bossAttackLists;

	// constructor that adds all containers for looping over them
	ECSRegistry()
	{
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&platforms);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&gameStates);
		registry_list.push_back(&timeControllables);
		registry_list.push_back(&harmfuls);
		registry_list.push_back(&bolts);
		registry_list.push_back(&pendulums);
		registry_list.push_back(&gears);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&rocks);
		registry_list.push_back(&waterdrops);
		registry_list.push_back(&falling);
		registry_list.push_back(&walking);
		registry_list.push_back(&blocked);
		registry_list.push_back(&cameras);
		registry_list.push_back(&layers);
		registry_list.push_back(&animateRequests);
		registry_list.push_back(&movementPaths);
		registry_list.push_back(&bosses);
		registry_list.push_back(&physicsObjects);
		registry_list.push_back(&boundaries);
		registry_list.push_back(&bossAttacks);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;