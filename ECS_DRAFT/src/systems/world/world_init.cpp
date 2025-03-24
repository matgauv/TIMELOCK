#include "world_init.hpp"

#include <iostream>

#include "../../tinyECS/registry.hpp"
#include "systems/rendering/render_system.hpp"

Entity create_player(vec2 position, vec2 scale) {
    Entity entity = Entity();

    Player &player = registry.players.emplace(entity);
    player.spawn_point = position;

    PhysicsObject& object = registry.physicsObjects.emplace(entity);
    object.mass = 40.0f;

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0.0f};
    motion.angle = 0.0f;

    Blocked& blocked = registry.blocked.emplace(entity);
    blocked.normal = vec2(0, 0);

    registry.renderRequests.insert(entity, {
        TEXTURE_ASSET_ID::GREY_CIRCLE,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, {LAYER_ID::MIDGROUND});

    AnimateRequest& animation = registry.animateRequests.emplace(entity);
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

    Blocked& blocked = registry.blocked.emplace(entity);
    blocked.normal = vec2(0, 0);

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

    Blocked& blocked = registry.blocked.emplace(entity);
    blocked.normal = vec2(0, 0);

    MovementPath& movementPath = registry.movementPaths.emplace(entity);
    movementPath.paths = movements;

    registry.platforms.emplace(entity);
    registry.timeControllables.emplace(entity);

    int num_tiles = scale.x / TILE_TO_PIXELS; // only allows for 1 tile tall platforms atm
    int starting_tile_pos = initial_position.x - (0.5 * scale.x) + (0.5 * TILE_TO_PIXELS);

    for (int i = 0; i < num_tiles; i++) {
        Entity tile_entity = Entity();

        int tile_arr_index = get_tile_index(starting_tile_pos, initial_position.y, i, 0, stride);

        Tile& tile_component = registry.tiles.emplace(tile_entity);
        tile_component.offset.x = i;
        tile_component.parent_id = entity.id();
        tile_component.id = tile_id_array[tile_arr_index];

        registry.renderRequests.insert(tile_entity, {
            TEXTURE_ASSET_ID::TILE,
            EFFECT_ASSET_ID::TILE,
            GEOMETRY_BUFFER_ID::SPRITE
        });

        registry.layers.insert(tile_entity, { LAYER_ID::MIDGROUND });
    }

    if (rounded) {
        PlatformGeometry &platform_geometry = registry.platformGeometries.emplace(entity);
        platform_geometry.num_tiles = num_tiles;
    }

    return entity;
}

Entity create_static_platform(vec2 position, vec2 scale, json& tile_id_array, int stride, bool rounded) {
    Entity entity = Entity();

    registry.platforms.emplace(entity);


    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
    motion.angle = 0;

    Blocked& blocked = registry.blocked.emplace(entity);
    blocked.normal = vec2(0, 0);


    int num_tiles = scale.x / TILE_TO_PIXELS; // only allows for 1 tile tall platforms atm
    int starting_tile_pos = position.x - (0.5 * scale.x) + (0.5 * TILE_TO_PIXELS);

    for (int i = 0; i < num_tiles; i++) {
        Entity tile_entity = Entity();

        int tile_arr_index = get_tile_index(starting_tile_pos, position.y, i, 0, stride);


        Tile& tile_component = registry.tiles.emplace(tile_entity);
        tile_component.offset.x = i;
        tile_component.parent_id = entity.id();
        tile_component.id = tile_id_array[tile_arr_index];


        registry.renderRequests.insert(tile_entity, {
            TEXTURE_ASSET_ID::TILE,
            EFFECT_ASSET_ID::TILE,
            GEOMETRY_BUFFER_ID::SPRITE
        });

        registry.layers.insert(tile_entity, { LAYER_ID::MIDGROUND });
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

        registry.layers.insert(tile_entity, { LAYER_ID::MIDGROUND });
    }

    return entity;
}

Entity create_level_boundary(vec2 position, vec2 scale) {
    Entity entity = Entity();

    registry.platforms.emplace(entity);

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
    motion.angle = 0;

    Blocked& blocked = registry.blocked.emplace(entity);
    blocked.normal = vec2(0, 0);

    return entity;
}

Entity create_world_boundary(vec2 position, vec2 scale) {
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
    motion.angle = 0;

    registry.boundaries.emplace(entity);

    return entity;
}

Entity create_camera(vec2 position, vec2 scale) {
    Entity entity = Entity();
    registry.cameras.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);

    motion.position = CameraSystem::restricted_boundary_position(position, scale);;
    motion.scale = scale;


    return entity;
}

Entity create_parallaxbackground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id) {
    Entity entity = Entity();
    Motion& motion = registry.motions.emplace(entity);
    motion.position = scene_dimensions * 0.5f; // PARALLAXBACKGROUND_DEPTH;
    motion.scale = scene_dimensions *1.5f; // PARALLAXBACKGROUND_DEPTH;
    
    registry.renderRequests.insert(entity, {
        texture_id,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, { LAYER_ID::PARALLAXBACKGROUND });

    return entity;
}

Entity create_background(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id) {
    Entity entity = Entity();
    Motion& motion = registry.motions.emplace(entity);
    motion.position = scene_dimensions * 0.5f ;// BACKGROUND_DEPTH;
    motion.scale = scene_dimensions *1.5f;// BACKGROUND_DEPTH;

    registry.renderRequests.insert(entity, {
        texture_id,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, { LAYER_ID::BACKGROUND });

    return entity;
}

Entity create_foreground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id) {
    Entity entity = Entity();
    Motion& motion = registry.motions.emplace(entity);
    motion.position = scene_dimensions * 0.5f ; // / FOREGROUND_DEPTH;
    motion.scale = scene_dimensions / FOREGROUND_DEPTH *0.75f;

    registry.renderRequests.insert(entity, {
        texture_id,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, { LAYER_ID::FOREGROUND });

    return entity;
}

Entity create_levelground(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id) {
    Entity entity = Entity();
    Motion& motion = registry.motions.emplace(entity);
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

    Blocked& blocked = registry.blocked.emplace(entity);
    blocked.normal = vec2(0, 0);


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

    Blocked& blocked = registry.blocked.emplace(entity);
    blocked.normal = vec2(0, 0);

    PhysicsObject& object = registry.physicsObjects.emplace(entity);
    object.mass = 25.0f;
    object.apply_gravity = default_gravity;
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

    Pipe& pipe = registry.pipes.emplace(entity);
    pipe.direction = direction;

    int tile_arr_index = get_tile_index(position.x, position.y, 0, 0, stride);

    Entity tile_entity = Entity();

    Tile& tile = registry.tiles.emplace(tile_entity);
    tile.parent_id = entity.id();
    tile.offset = {0, 0};
    tile.id = tile_id_array[tile_arr_index];

    registry.renderRequests.insert(tile_entity, {
                TEXTURE_ASSET_ID::TILE,
                EFFECT_ASSET_ID::TILE,
                GEOMETRY_BUFFER_ID::SPRITE
            });

    registry.layers.insert(tile_entity, { LAYER_ID::MIDGROUND });

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

float getDistance(const Motion& one, const Motion& other) {
    return glm::length(one.position - other.position);
}

int get_tile_index(int pos_x, int pos_y, int offset_x, int offset_y, int stride) {
    int tile_coord_y = (pos_y / TILE_TO_PIXELS) + offset_y;
    int tile_coord_x = (pos_x / TILE_TO_PIXELS) + offset_x;
    return tile_coord_x + tile_coord_y * stride;
}

