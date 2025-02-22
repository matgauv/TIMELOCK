#include "world_init.hpp"
#include "../../tinyECS/registry.hpp"

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
    create_player(initial_pos, {50.0f, 50.0f});
    create_camera(initial_pos, { 1.0f, 1.0f }); // TODO: potential open-scene zoom in

    float boundaryWidth = WINDOW_WIDTH_PX * 10.0f;
    float boundaryHeight = WINDOW_HEIGHT_PX * 5.0f;
    create_background({ boundaryWidth, boundaryHeight }, TEXTURE_ASSET_ID::SAMPLE_BACKGROUND);


    // level boundaries
    create_static_platform({boundaryWidth/2.0f, 0.0f}, {boundaryWidth, 1.0f});
    create_static_platform({boundaryWidth/2.0f, boundaryHeight}, {boundaryWidth, 1.0f});
    create_static_platform({0.0f,boundaryHeight/2.0f}, {1.0f, boundaryHeight});
    create_static_platform({boundaryWidth, boundaryHeight/2.0f}, {1.0f, boundaryHeight});

    // starting platform
    create_static_platform({ xStart, sceneHeight}, {500.0f, 100.0f});

    vec2 moving_plat_size = {200.0f, 20.0f};

    Path moving_plat_1_forwards = Path({xStart +  400.0f, sceneHeight}, {xStart + 600.0f, sceneHeight}, 0.2);
    Path moving_plat_1_backwards = Path({xStart +  600.0f, sceneHeight}, {xStart + 400.0f, sceneHeight}, 0.2);
    std::vector<Path> moving_plat_1_movements = {moving_plat_1_forwards, moving_plat_1_backwards};
    create_moving_platform(moving_plat_size, moving_plat_1_movements);

    Path moving_plat_2_forwards = Path({xStart +  850.0f, sceneHeight}, {xStart + 1050.0f, sceneHeight}, 0.2);
    Path moving_plat_2_backwards = Path({xStart +  1050.0f, sceneHeight}, {xStart + 850.0f, sceneHeight}, 0.2);
    std::vector<Path> moving_plat_2_movements = {moving_plat_2_backwards, moving_plat_2_forwards};
    create_moving_platform(moving_plat_size, moving_plat_2_movements);

    create_static_platform({ xStart + 1500.0f, sceneHeight}, {500.0f, 100.0f});

    Path moving_plat_3_up = Path({xStart +  1900.0f, sceneHeight}, {xStart + 1900.0f, sceneHeight - 500.0f}, 1.5);
    Path moving_plat_3_down = Path({xStart +  1900.0f, sceneHeight - 500.0f}, {xStart + 1900.0f, sceneHeight}, 1.5);
    std::vector<Path> moving_plat_3_movements = {moving_plat_3_up, moving_plat_3_down};
    create_moving_platform(moving_plat_size, moving_plat_3_movements);

    create_static_platform({ xStart + 2500.0f, sceneHeight - 900.0f}, {500.0f, 100.0f});

    Path moving_plat_4_forwards = Path({xStart + 2750.0f, sceneHeight - 800.0f}, {xStart + 2850.0f, sceneHeight - 800.0f}, 0.1);
    Path moving_plat_4_backwards = Path({xStart + 2850.0f, sceneHeight - 800.0f}, {xStart + 2750.0f, sceneHeight - 800.0f}, 0.1);
    std::vector<Path> moving_plat_4_movements = {moving_plat_4_forwards, moving_plat_4_backwards};
    create_moving_platform(moving_plat_size, moving_plat_4_movements);

    Path moving_plat_5_forwards = Path({xStart + 3050.0f, sceneHeight - 800.0f}, {xStart + 3150.0f, sceneHeight - 800.0f}, 0.1);
    Path moving_plat_5_backwards = Path({xStart +  3150.0f, sceneHeight - 800.0f}, {xStart + 3050.0f, sceneHeight - 800.0f}, 0.1);
    std::vector<Path> moving_plat_5_movements = {moving_plat_5_backwards, moving_plat_5_forwards};
    create_moving_platform(moving_plat_size, moving_plat_5_movements);

    create_static_platform({ xStart + 2950.0f, sceneHeight - 600.0f}, {500.0f, 100.0f});
    create_static_platform({ xStart + 2950.0f + 250.0f + 125.0f, sceneHeight - 600.0f - 50.0f + 5.0f}, {250.0f, 10.0f});

    create_physics_object({xStart + 3000.0f, sceneHeight - 650.0f},{50.0f, 50.0f}, 5.0f);
}

Entity create_player(vec2 position, vec2 scale) {
    Entity entity = Entity();

    registry.players.emplace(entity);

    PhysicsObject& object = registry.physicsObjects.emplace(entity);
    object.weight = 500.0f;

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.selfVelocity = {0, 0.0f};
    motion.angle = 0.0f;

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

Entity create_physics_object(vec2 position, vec2 scale, float weight) {
    Entity entity = Entity();

    PhysicsObject& object = registry.physicsObjects.emplace(entity);
    object.weight = weight;

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.selfVelocity = {0.0f, 0.0f};
    motion.angle = 0.0f;

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
    motion.selfVelocity = {0, 0}; // physics system will calculate this...
    motion.angle = 0.0f;

    registry.platforms.emplace(entity);

    MovementPath& movementPath = registry.movementPaths.emplace(entity);
    movementPath.paths = movements;

    registry.renderRequests.insert(entity, {
        TEXTURE_ASSET_ID::BLACK,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, { LAYER_ID::MIDGROUND });

    Deceleratable& deceleration_config = registry.deceleratables.emplace(entity);
    deceleration_config.can_become_harmless = false;

    Acceleratable& acceleration_config = registry.acceleratables.emplace(entity);
    acceleration_config.can_become_harmful = false;

    return entity;
}

Entity create_static_platform(vec2 position, vec2 scale) {
    Entity entity = Entity();

    registry.platforms.emplace(entity);

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.selfVelocity = {0, 0};
    motion.angle = 0.0f;

    registry.renderRequests.insert(entity, {
        TEXTURE_ASSET_ID::BLACK,
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

Entity create_background(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id) {
    Entity entity = Entity();
    Motion& motion = registry.motions.emplace(entity);
    motion.position = scene_dimensions * 0.5f /** BACKGROUND_DEPTH*/;
    motion.scale = scene_dimensions * BACKGROUND_DEPTH;

    registry.renderRequests.insert(entity, {
        texture_id,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, { LAYER_ID::BACKGROUND });


    return entity;
}

Entity create_projectile(vec2 pos, vec2 size, vec2 velocity)
{
	auto entity = Entity();

	Projectile& projectile = registry.projectiles.emplace(entity);
	
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.selfVelocity = velocity;
	motion.position = pos;
	motion.scale = size;
    
    Deceleratable& deceleratable = registry.deceleratables.emplace(entity);
    deceleratable.can_become_harmless = 1;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::SAMPLE_PROJECTILE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

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