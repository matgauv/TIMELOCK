#include "world_init.hpp"
#include "../../tinyECS/registry.hpp"
#include "systems/rendering/render_system.hpp"

// TODO parse file descriptor to create level with render requests
// For now, hard coded to just put a platform on the screen...
void load_level(std::string descriptor_path) {
    (void)descriptor_path;

    // demo level for M1 video
    demo_level();
}

void demo_level() {
    float sceneHeight = WINDOW_HEIGHT_PX * 2.0f;
    float xStart = 250.0f;
    vec2 initial_pos = {xStart, sceneHeight/2.0f + 300.0f};
    

    float boundaryWidth = WINDOW_WIDTH_PX * 3.0f;
    float boundaryHeight = WINDOW_HEIGHT_PX * 3.0f;
    create_parallaxbackground({ boundaryWidth, boundaryHeight }, TEXTURE_ASSET_ID::GEARS_BACKGROUND);
    create_background({ boundaryWidth, boundaryHeight }, TEXTURE_ASSET_ID::METAL_BACKGROUND);
    create_foreground({ boundaryWidth, boundaryHeight }, TEXTURE_ASSET_ID::CHAIN_BACKGROUND);

    // float boltsize = 75.f;
    // create_bolt({ 325.0f, sceneHeight / 2.0f + 500.0f }, { boltsize, boltsize }, { 0.0f, 0.0f });
    // create_bolt({ 325.0f, sceneHeight / 2.0f + 300.0f }, { boltsize, boltsize }, { 0.0f, 0.0f });
    // create_bolt({ 325.0f, sceneHeight / 2.0f + 150.0f }, { boltsize, boltsize }, { 0.0f, 0.0f });

    // create one non-time-controllable breakable platform
    float breakable_size = 75.f;
    create_breakable_static_platform({ 325.0f, sceneHeight / 2.0f + 625.0f }, {breakable_size, breakable_size}, true, 0.0f);

    // create one time-controllable breakable platform
    create_time_controllable_breakable_static_platform({ 1800.0f, sceneHeight - 100 }, {100.0f, 20.0f}, false, -0.05f);


    // initial_pos = {0,0};
    create_player(initial_pos, PLAYER_SCALE);
    create_camera(initial_pos, { 1.0f, 1.0f }); // TODO: potential open-scene zoom in


    // level boundaries
    create_static_platform({boundaryWidth/2.0f, 0.0f}, {boundaryWidth, 1.0f}, true);
    create_static_platform({boundaryWidth/2.0f, boundaryHeight}, {boundaryWidth, 1.0f}, true);
    create_static_platform({0.0f,boundaryHeight/2.0f}, {1.0f, boundaryHeight}, true);
    create_static_platform({boundaryWidth, boundaryHeight/2.0f}, {1.0f, boundaryHeight}, true);

    // starting platform
    create_static_platform({ xStart, sceneHeight}, {500.0f, 100.0f}, false);
    create_spawnpoint({ xStart - 200, sceneHeight - 110 }, SPAWNPOINT_SCALE);
    create_spawnpoint({ xStart + 200, sceneHeight - 110}, SPAWNPOINT_SCALE);
    create_spawnpoint({ xStart + 1300.0f, sceneHeight - 110 }, SPAWNPOINT_SCALE);

    // lil roof to test vertical collisions
    create_static_platform({xStart - 100.0f, sceneHeight - 125.0f}, {100.0f, 20.0f}, false);

    vec2 moving_plat_size = {200.0f, 20.0f};

    Path moving_plat_1_forwards = Path({xStart +  400.0f, sceneHeight}, {xStart + 600.0f, sceneHeight}, 2.0f);
    Path moving_plat_1_backwards = Path({xStart +  600.0f, sceneHeight}, {xStart + 400.0f, sceneHeight}, 2.0f);
    std::vector<Path> moving_plat_1_movements = {moving_plat_1_forwards, moving_plat_1_backwards};
    create_moving_platform(moving_plat_size, moving_plat_1_movements);

    Path moving_plat_2_forwards = Path({xStart +  850.0f, sceneHeight}, {xStart + 1050.0f, sceneHeight}, 0.2f);
    Path moving_plat_2_backwards = Path({xStart +  1050.0f, sceneHeight}, {xStart + 850.0f, sceneHeight}, 0.2f);
    std::vector<Path> moving_plat_2_movements = {moving_plat_2_backwards, moving_plat_2_forwards};
    create_moving_platform(moving_plat_size, moving_plat_2_movements);

    create_static_platform({ xStart + 1500.0f, sceneHeight}, {500.0f, 100.0f}, false);

    Path moving_plat_3_up = Path({xStart +  1900.0f, sceneHeight}, {xStart + 1900.0f, sceneHeight - 500.0f}, 1.5f);
    Path moving_plat_3_down = Path({xStart +  1900.0f, sceneHeight - 500.0f}, {xStart + 1900.0f, sceneHeight}, 1.5f);
    std::vector<Path> moving_plat_3_movements = {moving_plat_3_up, moving_plat_3_down};
    create_moving_platform(moving_plat_size, moving_plat_3_movements);

    create_static_platform({ xStart + 2500.0f, sceneHeight - 900.0f}, {500.0f, 100.0f}, false);

    Path moving_plat_4_forwards = Path({xStart + 2750.0f, sceneHeight - 800.0f}, {xStart + 2850.0f, sceneHeight - 800.0f}, 0.1f);
    Path moving_plat_4_backwards = Path({xStart + 2850.0f, sceneHeight - 800.0f}, {xStart + 2750.0f, sceneHeight - 800.0f}, 0.1f);
    std::vector<Path> moving_plat_4_movements = {moving_plat_4_forwards, moving_plat_4_backwards};
    create_moving_platform(moving_plat_size, moving_plat_4_movements);

    Path moving_plat_5_forwards = Path({xStart + 3050.0f, sceneHeight - 800.0f}, {xStart + 3150.0f, sceneHeight - 800.0f}, 0.1f);
    Path moving_plat_5_backwards = Path({xStart +  3150.0f, sceneHeight - 800.0f}, {xStart + 3050.0f, sceneHeight - 800.0f}, 0.1f);
    std::vector<Path> moving_plat_5_movements = {moving_plat_5_backwards, moving_plat_5_forwards};
    create_moving_platform(moving_plat_size, moving_plat_5_movements);

    create_static_platform({ xStart + 2950.0f, sceneHeight - 600.0f}, {500.0f, 100.0f}, false);
    create_static_platform({ xStart + 2950.0f - 250.0f - 125.0f, sceneHeight - 600.0f - 50.0f + 5.0f}, {250.0f, 10.0f}, false);

}

Entity create_player(vec2 position, vec2 scale) {
    Entity entity = Entity();

    Player &player = registry.players.emplace(entity);
    player.spawn_point = position;

    PhysicsObject& object = registry.physicsObjects.emplace(entity);
    object.mass = 15.0f;

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0.0f};
    motion.angle = 0.0f;

    Blocked& blocked = registry.blocked.emplace(entity);
    blocked.normal = vec2(0, 0);

    registry.falling.emplace(entity);

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

    registry.falling.emplace(entity);

    registry.renderRequests.insert(entity, {
        TEXTURE_ASSET_ID::OBJECT,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, {LAYER_ID::MIDGROUND});

    return entity;
}

Entity create_moving_platform(vec2 scale, std::vector<Path> movements) {
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = movements[0].start;
    motion.scale = scale;
    motion.velocity = {0, 0}; // physics system will calculate this...
    motion.angle = 0.0f;

    Blocked& blocked = registry.blocked.emplace(entity);
    blocked.normal = vec2(0, 0);

    registry.platforms.emplace(entity);

    MovementPath& movementPath = registry.movementPaths.emplace(entity);
    movementPath.paths = movements;

    registry.renderRequests.insert(entity, {
        TEXTURE_ASSET_ID::BLACK,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });
    registry.timeControllables.emplace(entity);

    return entity;
}

Entity create_static_platform(vec2 position, vec2 scale, bool isBoundary, float angle) {
    Entity entity = Entity();

    registry.platforms.emplace(entity);

    if (isBoundary) {
        registry.boundaries.emplace(entity);
    }

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
    motion.angle = angle;

    Blocked& blocked = registry.blocked.emplace(entity);
    blocked.normal = vec2(0, 0);

    registry.renderRequests.insert(entity, {
        isBoundary ? TEXTURE_ASSET_ID::BOUNDARY : TEXTURE_ASSET_ID::BLACK,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });


    return entity;
}

Entity create_camera(vec2 position, vec2 scale) {
    Entity entity = Entity();
    registry.cameras.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);

    motion.position = position;
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

Entity create_bolt(vec2 pos, vec2 size, vec2 velocity)
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
    object.mass = 20.0f;

    registry.falling.emplace(entity);

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

Entity create_breakable_static_platform(vec2 position, vec2 scale, bool should_break_instantly, float degrade_speed, float angle, bool is_time_controllable) {
    Entity entity = create_static_platform(position, scale, false, angle);
    Breakable& breakable = registry.breakables.emplace(entity);
    breakable.health = 1000.f;
    breakable.degrade_speed_per_ms = degrade_speed;
    breakable.should_break_instantly = should_break_instantly;

    RenderRequest& renderRequest = registry.renderRequests.get(entity);
    renderRequest.used_texture = is_time_controllable? TEXTURE_ASSET_ID::OBJECT : TEXTURE_ASSET_ID::GREY_CIRCLE;

    return entity;
}

Entity create_time_controllable_breakable_static_platform(vec2 position, vec2 scale, bool should_break_instantly, float degrade_speed, float angle) {
    Entity entity = create_breakable_static_platform(position, scale, should_break_instantly, degrade_speed, angle, true);

    TimeControllable& timeControllable = registry.timeControllables.emplace(entity);
    timeControllable.can_be_accelerated = true;
    timeControllable.can_be_decelerated = true;
    timeControllable.can_become_harmful = false;
    timeControllable.can_become_harmless = false;
    timeControllable.target_time_control_factor = 100000.f;

    return entity;
}

float getDistance(Motion& one, Motion& other) {
    return sqrt(pow(one.position.x - other.position.x, 2) + pow(one.position.y - other.position.y, 2));
}