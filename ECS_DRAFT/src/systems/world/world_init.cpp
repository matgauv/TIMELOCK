#include "world_init.hpp"

#include <cfloat>
#include <iostream>
#include <glm/detail/func_trigonometric.inl>

#include "../../tinyECS/registry.hpp"
#include "systems/rendering/render_system.hpp"

Entity create_player(vec2 position, vec2 scale) {
    Entity entity = Entity();

    Player &player = registry.players.emplace(entity);
    player.spawn_point = position;

    PhysicsObject &object = registry.physicsObjects.emplace(entity);
    object.mass = 40.0f;
    object.friction = PLAYER_STATIC_FRICTION;
    object.bounce = 0.0f; // player should NOT bounce

    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0.0f};
    motion.angle = 0.0f;

    registry.renderRequests.insert(entity, {
                                       TEXTURE_ASSET_ID::GREY_CIRCLE,
                                       EFFECT_ASSET_ID::TEXTURED,
                                       GEOMETRY_BUFFER_ID::SPRITE
                                   });

    registry.layers.insert(entity, {LAYER_ID::MIDGROUND});

    AnimateRequest &animation = registry.animateRequests.emplace(entity);
    animation.used_animation = ANIMATION_ID::PLAYER_STANDING;

    // DEBUGGING ONLY
    /*
    Deceleratable& deceleration_config = registry.deceleratables.emplace(entity);
    deceleration_config.can_become_harmless = false;

    Acceleratable& acceleration_config = registry.acceleratables.emplace(entity);
    acceleration_config.can_become_harmful = false;
    */

    return entity;
}

Entity create_physics_object(vec2 position, vec2 scale, float mass) {
    Entity entity = Entity();

    PhysicsObject& object = registry.physicsObjects.emplace(entity);
    object.mass = mass;

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0.0f, 0.0f};
    motion.angle = 0.0f;


    registry.renderRequests.insert(entity, {
        TEXTURE_ASSET_ID::OBJECT,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, {LAYER_ID::MIDGROUND});

    return entity;
}

Entity create_moving_platform(vec2 scale, std::vector<Path> movements, vec2 initial_position, json& tile_id_array, int stride, bool rounded) {
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = initial_position;
    motion.scale = scale;
    motion.velocity = {0, 0}; // physics system will calculate this...
    motion.angle = 0.0f;

    MovementPath& movementPath = registry.movementPaths.emplace(entity);
    movementPath.paths = movements;

    PhysicsObject& physics_object = registry.physicsObjects.emplace(entity);
    physics_object.mass = 0.0f;
    physics_object.apply_gravity = false;
    physics_object.apply_rotation = false;
    physics_object.friction = 0.25;

    registry.platforms.emplace(entity);
    registry.timeControllables.emplace(entity);

    int num_tiles = scale.x / TILE_TO_PIXELS; // only allows for 1 tile tall platforms atm
    int starting_tile_pos = initial_position.x - (0.5 * scale.x) + (0.5 * TILE_TO_PIXELS);

    for (int i = 0; i < num_tiles; i++) {
        Entity tile_entity = Entity();

        int tile_arr_index = get_tile_index(starting_tile_pos, initial_position.y, i, 0, stride);

        Tile &tile_component = registry.tiles.emplace(tile_entity);
        tile_component.offset.x = i;
        tile_component.parent_id = entity.id();
        tile_component.id = tile_id_array[tile_arr_index];

        registry.renderRequests.insert(tile_entity, {
                                           TEXTURE_ASSET_ID::TILE,
                                           EFFECT_ASSET_ID::TILE,
                                           GEOMETRY_BUFFER_ID::SPRITE
                                       });

        registry.layers.insert(tile_entity, {LAYER_ID::MIDGROUND});
    }

    if (rounded) {
        PlatformGeometry &platform_geometry = registry.platformGeometries.emplace(entity);
        platform_geometry.num_tiles = num_tiles;
    }

    return entity;
}

Entity create_static_platform(vec2 position, vec2 scale, json &tile_id_array, int stride, bool rounded) {
    Entity entity = Entity();

    registry.platforms.emplace(entity);


    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
    motion.angle = 0;

    PhysicsObject& physics_object = registry.physicsObjects.emplace(entity);
    physics_object.apply_gravity = false;
    physics_object.mass = 0.0f;
    physics_object.apply_rotation = false;
    physics_object.friction = 0.25;

    int num_tiles = scale.x / TILE_TO_PIXELS; // only allows for 1 tile tall platforms atm
    int starting_tile_pos = position.x - (0.5 * scale.x) + (0.5 * TILE_TO_PIXELS);

    for (int i = 0; i < num_tiles; i++) {
        Entity tile_entity = Entity();

        int tile_arr_index = get_tile_index(starting_tile_pos, position.y, i, 0, stride);


        Tile &tile_component = registry.tiles.emplace(tile_entity);
        tile_component.offset.x = i;
        tile_component.parent_id = entity.id();
        tile_component.id = tile_id_array[tile_arr_index];


        registry.renderRequests.insert(tile_entity, {
                                           TEXTURE_ASSET_ID::TILE,
                                           EFFECT_ASSET_ID::TILE,
                                           GEOMETRY_BUFFER_ID::SPRITE
                                       });

        registry.layers.insert(tile_entity, {LAYER_ID::MIDGROUND});
    }

    if (rounded) {
        PlatformGeometry &platform_geometry = registry.platformGeometries.emplace(entity);
        platform_geometry.num_tiles = num_tiles;
    }


    return entity;
}

Entity create_ladder(vec2 position, vec2 scale, int height, json tile_id_array, int stride) {
    Entity entity = Entity();
    registry.ladders.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);
    vec2 ladder_scale = {scale.x, scale.y * height};
    motion.scale = ladder_scale;
    motion.position = {position.x, position.y + (0.5 * TILE_TO_PIXELS) - (0.5 * ladder_scale.y)};
    motion.velocity = {0, 0};
    motion.angle = 0.f;

    registry.nonPhysicsColliders.emplace(entity);

    int start_tile_y = motion.position.y - (0.5 * ladder_scale.y) + (0.5 * TILE_TO_PIXELS);
    for (int i = 0; i < height; i++) {
        Entity tile_entity = Entity();
        int tile_arr_index = get_tile_index(position.x, start_tile_y, 0, i, stride);

        Tile& tile_component = registry.tiles.emplace(tile_entity);
        tile_component.offset.y = i;
        tile_component.parent_id = entity.id();
        tile_component.id = tile_id_array[tile_arr_index];

        registry.renderRequests.insert(tile_entity, {
                                           TEXTURE_ASSET_ID::TILE,
                                           EFFECT_ASSET_ID::TILE,
                                           GEOMETRY_BUFFER_ID::SPRITE
                                       });

        registry.layers.insert(tile_entity, {LAYER_ID::MIDGROUND});
    }

    return entity;
}

Entity create_level_boundary(vec2 position, vec2 scale) {
    Entity entity = Entity();

    registry.platforms.emplace(entity);

    PhysicsObject& physics_object = registry.physicsObjects.emplace(entity);
    physics_object.apply_gravity = false;
    physics_object.apply_rotation = false;
    physics_object.mass = 0.0f;
    physics_object.friction = 0.5;

    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
    motion.angle = 0;

    return entity;
}

Entity create_world_boundary(vec2 position, vec2 scale) {
    Entity entity = Entity();

    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
    motion.angle = 0;

    registry.nonPhysicsColliders.emplace(entity);

    registry.boundaries.emplace(entity);

    return entity;
}

Entity create_camera(vec2 position, vec2 scale) {
    Entity entity = Entity();
    registry.cameras.emplace(entity);
    Motion &motion = registry.motions.emplace(entity);

    motion.position = CameraSystem::restricted_boundary_position(position, scale);;
    motion.scale = scale;


    return entity;
}

Entity create_parallaxbackground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id) {
    Entity entity = Entity();
    Motion &motion = registry.motions.emplace(entity);
    motion.position = scene_dimensions * 0.5f; // PARALLAXBACKGROUND_DEPTH;
    motion.scale = scene_dimensions * 1.5f; // PARALLAXBACKGROUND_DEPTH;

    registry.renderRequests.insert(entity, {
                                       texture_id,
                                       EFFECT_ASSET_ID::TEXTURED,
                                       GEOMETRY_BUFFER_ID::SPRITE
                                   });

    registry.layers.insert(entity, {LAYER_ID::PARALLAXBACKGROUND});

    return entity;
}

Entity create_background(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id) {
    Entity entity = Entity();
    Motion &motion = registry.motions.emplace(entity);
    motion.position = scene_dimensions * 0.5f; // BACKGROUND_DEPTH;
    motion.scale = scene_dimensions * 1.5f; // BACKGROUND_DEPTH;

    registry.renderRequests.insert(entity, {
                                       texture_id,
                                       EFFECT_ASSET_ID::TEXTURED,
                                       GEOMETRY_BUFFER_ID::SPRITE
                                   });

    registry.layers.insert(entity, {LAYER_ID::BACKGROUND});

    return entity;
}

Entity create_foreground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id) {
    Entity entity = Entity();
    Motion &motion = registry.motions.emplace(entity);
    motion.position = scene_dimensions * 0.5f; // / FOREGROUND_DEPTH;
    motion.scale = scene_dimensions / FOREGROUND_DEPTH * 0.75f;

    registry.renderRequests.insert(entity, {
                                       texture_id,
                                       EFFECT_ASSET_ID::TEXTURED,
                                       GEOMETRY_BUFFER_ID::SPRITE
                                   });

    registry.layers.insert(entity, {LAYER_ID::FOREGROUND});

    return entity;
}

Entity create_levelground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id) {
    Entity entity = Entity();
    Motion &motion = registry.motions.emplace(entity);
    vec2 position = {scene_dimensions.x - (TILE_TO_PIXELS), scene_dimensions.y - (TILE_TO_PIXELS)};

    motion.position = position * 0.5f; // / FOREGROUND_DEPTH;
    motion.scale = scene_dimensions;


    registry.renderRequests.insert(entity, {
                                       texture_id,
                                       EFFECT_ASSET_ID::TEXTURED,
                                       GEOMETRY_BUFFER_ID::SPRITE
                                   });

  registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

    return entity;
}

Entity create_tutorial_text(vec2 position, vec2 size, TEXTURE_ASSET_ID texture_id) {

    Entity entity = Entity();

    Motion& text_motion = registry.motions.emplace(entity);

    text_motion.position = position; 
    text_motion.scale = size;


    registry.renderRequests.insert(entity, {
        texture_id,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
        });

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

    return entity;
}

Entity create_projectile(vec2 pos, vec2 size, vec2 velocity)
{
	auto entity = Entity();

	Projectile& projectile = registry.projectiles.emplace(entity);

	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = pos;
	motion.scale = size;

    TimeControllable& tc = registry.timeControllables.emplace(entity);
    tc.can_become_harmless = true;
    tc.can_be_accelerated = false;

    registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::SAMPLE_PROJECTILE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

	return entity;
}

Entity create_bolt(vec2 pos, vec2 size, vec2 velocity, bool default_gravity)
{
	auto entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = pos;
	motion.scale = size;

    PhysicsObject& object = registry.physicsObjects.emplace(entity);
    object.mass = 25.0f;
    object.apply_gravity = default_gravity;
    object.apply_rotation = false;
    object.friction = BOLT_FRICTION;
    object.drag_coefficient = 0.01;

    registry.bolts.emplace(entity);

    registry.colors.insert(
        entity, 
        { 1.0f, 1.0f, 1.0f }
    );

    registry.renderRequests.insert(
        entity,
        {
            TEXTURE_ASSET_ID::HEX,
			EFFECT_ASSET_ID::HEX,
			GEOMETRY_BUFFER_ID::HEX
		}
	);

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

	return entity;
}

Entity create_first_boss() {
    auto entity = Entity();

    Boss& boss = registry.bosses.emplace(entity);
    boss.boss_id = BOSS_ID::FIRST;
    boss.health = 1000.0f;
    boss.attack_cooldown_ms = 500.0f;

    Motion& motion = registry.motions.emplace(entity);
    motion.position = vec2(WINDOW_WIDTH_PX / 4 * 3, WINDOW_HEIGHT_PX / 4 * 3);

    registry.renderRequests.insert(
        entity,
        {
            TEXTURE_ASSET_ID::TEXTURE_COUNT,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );

    return entity;
}

Entity create_spawnpoint(vec2 pos, vec2 size) {
    Entity entity = Entity();

    SpawnPoint& spawnpoint = registry.spawnPoints.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);
    motion.position = pos;
    motion.scale = size;

    registry.renderRequests.insert(
        entity,
        {
            TEXTURE_ASSET_ID::SPAWNPOINT_UNVISITED,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

    return entity;
}

Entity create_cannon_tower(vec2 pos) {
    Entity entity = Entity();

    CannonTower& tower = registry.cannonTowers.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);
    motion.position = pos;
    motion.scale = CANNON_TOWER_SIZE;

    registry.renderRequests.insert(
        entity,
        {
            TEXTURE_ASSET_ID::CANNON_TOWER,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );

    registry.timeControllables.emplace(entity);

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

    Entity barrel_entity = Entity();
    tower.barrel_entity = barrel_entity;
    registry.cannonBarrels.emplace(barrel_entity);

    Motion& barrel_motion = registry.motions.emplace(barrel_entity);
    barrel_motion.position = pos + vec2{CANNON_BARREL_SIZE[0] * 0.5f, 0};
    barrel_motion.scale = CANNON_BARREL_SIZE;

    registry.renderRequests.insert(
        barrel_entity,
        {
            TEXTURE_ASSET_ID::BARREL,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );

    registry.layers.insert(barrel_entity, { LAYER_ID::MIDGROUND});
    registry.timeControllables.emplace(barrel_entity);

    return entity;
}

Entity create_spike(vec2 position, vec2 scale, json tile_id_array, int stride) {
    Entity entity = Entity();

    registry.spikes.emplace(entity);

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
    motion.angle = 0.f;


    registry.nonPhysicsColliders.emplace(entity);

    int tile_arr_index = get_tile_index(position.x, position.y, 0, 0, stride);

    Tile& tile_component = registry.tiles.emplace(entity);
    tile_component.offset.x = 0;
    tile_component.parent_id = entity.id();
    tile_component.id = tile_id_array[tile_arr_index];

    registry.renderRequests.insert(entity, {
            TEXTURE_ASSET_ID::TILE,
            EFFECT_ASSET_ID::TILE,
            GEOMETRY_BUFFER_ID::SPRITE
        });

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

    return entity;
}




Entity create_partof(vec2 position, vec2 scale, json tile_id_array, int stride) {
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.angle = 0.0f;
    motion.velocity = {0.0f, 0.0f};

    int tile_arr_index = get_tile_index(position.x, position.y, 0, 0, stride);

    Tile& tile_component = registry.tiles.emplace(entity);
    tile_component.offset.x = 0;
    tile_component.parent_id = entity.id();
    tile_component.id = tile_id_array[tile_arr_index];

    registry.renderRequests.insert(entity, {
            TEXTURE_ASSET_ID::TILE,
            EFFECT_ASSET_ID::TILE,
            GEOMETRY_BUFFER_ID::SPRITE
        });

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

    return entity;
}

Entity create_breakable_static_platform(vec2 position, vec2 scale, bool should_break_instantly, float degrade_speed, bool is_time_controllable, json& tile_id_array, int stride) {
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0.0, 0.0};
    motion.angle = 0;

    registry.platforms.emplace(entity);

    PhysicsObject& phys = registry.physicsObjects.emplace(entity);
    phys.mass =0.0f;
    phys.apply_gravity = false;
    phys.apply_rotation = false;

    Breakable& breakable = registry.breakables.emplace(entity);
    breakable.health = 1000.f;
    breakable.degrade_speed_per_ms = degrade_speed;
    breakable.should_break_instantly = should_break_instantly;

    // TODO: need to add a proper texture for this
    registry.renderRequests.insert(entity, {
            is_time_controllable? TEXTURE_ASSET_ID::OBJECT : TEXTURE_ASSET_ID::GREY_CIRCLE,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

    return entity;
}

Entity create_time_controllable_breakable_static_platform(vec2 position, vec2 scale, bool should_break_instantly, float degrade_speed, json& tile_id_array, int stride) {
    Entity entity = create_breakable_static_platform(position, scale, should_break_instantly, degrade_speed,  true,  tile_id_array, stride);

    TimeControllable& timeControllable = registry.timeControllables.emplace(entity);
    timeControllable.can_be_accelerated = true;
    timeControllable.can_be_decelerated = true;
    timeControllable.can_become_harmful = false;
    timeControllable.can_become_harmless = false;
    timeControllable.target_time_control_factor = 100000.f;

    return entity;
}

Entity create_door(vec2 position, bool open, json& tile_id_array, int stride) {
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = DOOR_SIZE;
    motion.velocity = {0, 0};
    motion.angle = 0.0f;

    Door& door = registry.doors.emplace(entity);
    door.opened = open;

    registry.nonPhysicsColliders.emplace(entity);

    int width_in_tiles = DOOR_SIZE.x / TILE_TO_PIXELS;
    int height_in_tiles = DOOR_SIZE.y / TILE_TO_PIXELS;
    vec2 first_tile_pos =
    {
        position.x - (0.5 * DOOR_SIZE.x) + (0.5 * TILE_TO_PIXELS),
        position.y - (0.5 * DOOR_SIZE.y) + (0.5 * TILE_TO_PIXELS)
    };

    for (int i = 0; i < width_in_tiles; i++) {
        for (int j = 0; j < height_in_tiles; j++) {
            Entity tile_entity = Entity();

            int tile_arr_index = get_tile_index(first_tile_pos.x, first_tile_pos.y, i, j, stride);

            Tile& tile_component = registry.tiles.emplace(tile_entity);
            tile_component.offset = {i, j};
            tile_component.parent_id = entity.id();
            tile_component.id = tile_id_array[tile_arr_index];

            registry.renderRequests.insert(tile_entity, {
                TEXTURE_ASSET_ID::TILE,
                EFFECT_ASSET_ID::TILE,
                GEOMETRY_BUFFER_ID::SPRITE
            });

            registry.layers.insert(tile_entity, { LAYER_ID::MIDGROUND });
        }
    }

    return entity;
}

Entity create_pipe_head(vec2 position, vec2 scale, std::string direction, json& tile_id_array, int stride) {
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
    motion.angle = 0;

    registry.platforms.emplace(entity);

    PhysicsObject& phys = registry.physicsObjects.emplace(entity);
    phys.mass = 0.0f;
    phys.apply_gravity = false;
    phys.apply_rotation = false;

    Pipe& pipe = registry.pipes.emplace(entity);
    pipe.direction = direction;

    int tile_arr_index = get_tile_index(position.x, position.y, 0, 0, stride);

    Tile& tile = registry.tiles.emplace(entity);
    tile.parent_id = entity.id();
    tile.offset = {0, 0};
    tile.id = tile_id_array[tile_arr_index];

    registry.renderRequests.insert(entity, {
                TEXTURE_ASSET_ID::TILE,
                EFFECT_ASSET_ID::TILE,
                GEOMETRY_BUFFER_ID::SPRITE
            });

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

    return entity;
}

Entity create_chain(vec2 position, vec2 scale) {
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
    motion.angle = 0;

    registry.renderRequests.insert(entity, {
        TEXTURE_ASSET_ID::CHAIN,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, { LAYER_ID::FOREGROUND });

    return entity;
}


// TODO memory leak with the mesh allocation...
Mesh* createCircleMesh(Entity e, int segments) {
    Mesh *mesh = new Mesh();

    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = 0.5f * cosf(theta);
        float y = 0.5f * sinf(theta);

        ColoredVertex vertex;
        vertex.position.x = x;
        vertex.position.y = y;
        vertex.color = {1.0f, 1.0f, 1.0f};
        mesh->vertices.push_back(vertex);
    }

    return mesh;
}

Entity create_pendulum_string(vec2 start, vec2 end) {
    Entity entity = Entity();

    float angle = atan2(end.y - start.y, end.x - start.x);
    float length = glm::length(end - start);


    Motion &motion = registry.motions.emplace(entity);
    motion.scale = {1.0f, length};
    motion.angle = angle - 90.0f;
    motion.velocity = {0, 0};

    motion.position.x = start.x;
    motion.position.y = start.y + cos(angle - 90.0f) * (length / 2);

    PivotPoint &pivot_point = registry.pivotPoints.emplace(entity);
    pivot_point.offset = {0.0f, -length / 2.0f};

    registry.renderRequests.insert(entity, {
                                       TEXTURE_ASSET_ID::PENDULUM_ARM,
                                       EFFECT_ASSET_ID::TEXTURED,
                                       GEOMETRY_BUFFER_ID::SPRITE
                                   });

    registry.layers.insert(entity, {LAYER_ID::MIDGROUND});

    return entity;
}


Entity create_pendulum(vec2 pivot_position, float length, float initial_angle, float bob_radius) {
    Entity entity = Entity();

    float bob_x = pivot_position.x + length * sin(initial_angle - 90.0f);
    float bob_y = pivot_position.y + length * cos(initial_angle - 90.0f);

    Motion &motion = registry.motions.emplace(entity);
    motion.position = {bob_x, bob_y};
    motion.angle =0.0f;
    motion.velocity = {0.0f, 0.0f};
    motion.scale = {bob_radius * 2, bob_radius * 2};

    Pendulum &pendulum = registry.pendulums.emplace(entity);
    pendulum.pivot_point = pivot_position;
    pendulum.length = length;
    pendulum.current_angle = initial_angle;
    pendulum.angular_velocity = 0.0f;
    pendulum.damping = 0.0f;

    // these properties don't actually determine how the pendulum swings, they do determine how collisions behave
    PhysicsObject &physics_object = registry.physicsObjects.emplace(entity);
    physics_object.apply_gravity = false;
    physics_object.drag_coefficient = 0.0f;
    physics_object.mass = 10.0f;

    TimeControllable& tc = registry.timeControllables.emplace(entity);
    tc.can_be_accelerated = true;
    tc.can_be_decelerated = true;

    // TODO: performance, share mesh!! no need to have the same mesh over and over again...
    Mesh* mesh = createCircleMesh(entity, 16);
    registry.meshPtrs.emplace(entity, mesh);


    Entity rod = create_pendulum_string(pivot_position, motion.position);
    PendulumRod &rod_component = registry.pendulumRods.emplace(rod);
    rod_component.bob_id = entity.id();

    registry.renderRequests.insert(entity, {
                                       TEXTURE_ASSET_ID::PENDULUM,
                                       EFFECT_ASSET_ID::TEXTURED,
                                       GEOMETRY_BUFFER_ID::SPRITE
                                   });
    registry.layers.insert(entity, {LAYER_ID::MIDGROUND});
    return entity;
}


Mesh* get_mesh_from_file(std::string filename) {
    Mesh* mesh = new Mesh();
    Mesh::loadFromOBJFile(filename, mesh->vertices, mesh->vertex_indices, mesh->original_size);
    return mesh;
}

// TODO: we also have a mesh cache in the registry, but if meshes are just for collisions, it feels weird/annyoing to load them in the render system...
// this just prevents us from loading the mesh every time we create an object
Mesh* get_mesh(std::string filepath) {
    auto it = mesh_cache.find(filepath);
    if (it != mesh_cache.end()) {
        return it->second;
    }

    Mesh* mesh = get_mesh_from_file(data_path() + filepath);
    mesh_cache[filepath] = mesh;
    return mesh;
}


Entity create_gear(vec2 position, vec2 size) {
    Entity entity = Entity();

    registry.gears.emplace(entity);

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = size;
    motion.angle = M_PI;
    motion.velocity = {0.0f, 0.0f};
    motion.cache_invalidated = true;

    PhysicsObject& physics_object = registry.physicsObjects.emplace(entity);
    physics_object.apply_gravity = true;
    physics_object.mass = 90.0f;
    physics_object.friction = 0.01f;
    physics_object.apply_rotation = true;

    CompositeMesh& compositeMesh = registry.compositeMeshes.emplace(entity);

    float inner_radius = (size.x / 2.0f) * GEAR_CENTER_RATIO;
    float tooth_length = (size.x / 2.0f) * GEAR_TOOTH_RATIO;


    Mesh* mesh = createCircleMesh(entity, 16);
    SubMesh sub_mesh = SubMesh{};
    sub_mesh.original_mesh = mesh;
    sub_mesh.scale_ratio = {GEAR_CENTER_RATIO, GEAR_CENTER_RATIO};

    compositeMesh.meshes.push_back(sub_mesh);

    // TODO: this is real bad, only should do once total (not per gear)...
    Mesh* nesw_tooth = get_mesh("/meshes/step-jagged-ne-sw.obj");
    Mesh* nwse_tooth = get_mesh("/meshes/step-jagged-nw-se.obj");
    Mesh* ew_tooth = get_mesh("/meshes/step-teeth.obj");

    // lil helper to add the tooth
    auto add_tooth = [&](Mesh* tooth_mesh, float angle_rad, float scale_ratio, bool flip = false) {
        SubMesh tooth = SubMesh{};
        tooth.original_mesh = tooth_mesh;
        tooth.scale_ratio = {GEAR_TOOTH_RATIO, GEAR_TOOTH_RATIO};

        tooth.rotation = (flip ? 180.0 : 0.0f);

        tooth.offset = {
            (inner_radius + tooth_length) * cos(angle_rad),
            (inner_radius + tooth_length) * sin(angle_rad)
        };

        compositeMesh.meshes.push_back(tooth);
    };

    // angles, TODO put these in header?
    float west_angle = 0.0f;
    float southwest_angle = M_PI * 0.33f;
    float southeast_angle = M_PI * 0.66f;
    float east_angle = M_PI;
    float northwest_angle = M_PI * 1.33f;
    float northeast_angle = M_PI * 1.66f;

     add_tooth(ew_tooth, east_angle, GEAR_TOOTH_RATIO, false);
     add_tooth(nesw_tooth, northeast_angle, GEAR_TOOTH_RATIO, false);
     add_tooth(nwse_tooth, southeast_angle, GEAR_TOOTH_RATIO, false);

     add_tooth(ew_tooth, west_angle, GEAR_TOOTH_RATIO, true);
     add_tooth(nesw_tooth, southwest_angle, GEAR_TOOTH_RATIO, true);
     add_tooth(nwse_tooth, northwest_angle, GEAR_TOOTH_RATIO, true);

    registry.renderRequests.insert(entity, {
        TEXTURE_ASSET_ID::GEAR,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });
    registry.layers.insert(entity, {LAYER_ID::MIDGROUND});

    return entity;
}

Entity create_spikeball(vec2 position, vec2 size) {
   Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = size;
    motion.angle = 0.0f;
    motion.velocity = {0.0f, 0.0f};
    motion.cache_invalidated = true;

    PhysicsObject& physics_object = registry.physicsObjects.emplace(entity);
    physics_object.apply_gravity = true;
    physics_object.mass = 90.0f;
    physics_object.friction = 0.01f;
    physics_object.apply_rotation = true;

    CompositeMesh& compositeMesh = registry.compositeMeshes.emplace(entity);

    float inner_radius = ((size.x * SPIKEBALL_CENTER_RATIO) / 2.0f);
    float spike_height = (size.x) * SPIKE_HEIGHT_RATIO;
    float spike_width = (size.x) * SPIKE_WIDTH_PX;

    Mesh* mesh = createCircleMesh(entity, 16);
    SubMesh sub_mesh = SubMesh{};
    sub_mesh.original_mesh = mesh;
    sub_mesh.scale_ratio = {SPIKEBALL_CENTER_RATIO, SPIKEBALL_CENTER_RATIO};

    compositeMesh.meshes.push_back(sub_mesh);

    int num_spikes = 8;
    float angle = 0.0f;
    float angle_diff =  360.0f / num_spikes;

    auto add_spike = [&](Mesh* spike_mesh, float angle_deg, float scale_ratio, bool flip = false) {
        SubMesh spike = SubMesh{};
        spike.original_mesh = spike_mesh;
        spike.scale_ratio = vec2{SPIKE_WIDTH_RATIO, SPIKE_HEIGHT_RATIO};

         spike.rotation = angle_deg + 90.0f;

        float offset_len = inner_radius + (spike_height / 2.0f);

        spike.offset = {
            (offset_len * cos(radians(angle_deg))),
            (offset_len * sin(radians(angle_deg)))
        };

        compositeMesh.meshes.push_back(spike);
    };

    Mesh* spike_mesh = get_mesh("/meshes/spikeball-spikes.obj");

    for (int i = 0; i < num_spikes; i++) {
        add_spike(spike_mesh, angle, SPIKE_WIDTH_RATIO, false);
        angle += angle_diff;
    }

    registry.renderRequests.insert(entity, {
        TEXTURE_ASSET_ID::SPIKEBALL,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });
    registry.layers.insert(entity, {LAYER_ID::MIDGROUND});
    return entity;
}



float getDistance(const Motion& one, const Motion& other) {
    return glm::length(one.position - other.position);
}

int get_tile_index(int pos_x, int pos_y, int offset_x, int offset_y, int stride) {
    int tile_coord_y = (pos_y / TILE_TO_PIXELS) + offset_y;
    int tile_coord_x = (pos_x / TILE_TO_PIXELS) + offset_x;
    return tile_coord_x + tile_coord_y * stride;
}

