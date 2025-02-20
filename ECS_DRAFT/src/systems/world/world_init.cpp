#include "world_init.hpp"
#include "../../tinyECS/registry.hpp"

// TODO parse file descriptor to create level with render requests
// For now, hard coded to just put a platform on the screen...
void load_level(std::string descriptor_path) {
    (void)descriptor_path;
    vec2 initial_pos = { 50.0f, 50.0f };
    create_player(initial_pos, {50.0f, 50.0f});
    create_camera(initial_pos, { 1.0f, 1.0f }); // TODO: potential open-scene zoom in
    create_background({ WINDOW_WIDTH_PX * 2.0f, WINDOW_HEIGHT_PX * 2.0f }, TEXTURE_ASSET_ID::SAMPLE_BACKGROUND);
    create_static_platform({WINDOW_WIDTH_PX / 2.0f, WINDOW_HEIGHT_PX - 50.0f}, {WINDOW_WIDTH_PX, 100.0f});
    create_static_platform({25.0f, WINDOW_HEIGHT_PX - 100.0f}, {WINDOW_WIDTH_PX /4.0f, 100.0f});

    Path forward = Path({200.0f, 380.0f}, {350.0f, 380.0f}, 2.5);
    Path backwards = Path({350.0f, 380.0f}, {200.0f, 380.0f}, 2.5);
    std::vector<Path> movements = {forward, backwards};
    create_moving_platform({100.0f, 20.0f}, movements);
}

Entity create_player(vec2 position, vec2 scale) {
    Entity entity = Entity();

    registry.players.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0.0f};
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

Entity create_moving_platform(vec2 scale, std::vector<Path> movements) {
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = movements[0].start;
    motion.scale = scale;
    motion.velocity = {0, 0}; // physics system will calculate this...
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
    Platform& platform = registry.platforms.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);

    (void)platform; // TODO: use this if we need to

    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0};
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
    Camera& camera = registry.cameras.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);

    motion.position = position;
    motion.scale = scale;

    return entity;
}

Entity create_background(vec2 scene_dimensions, TEXTURE_ASSET_ID texture_id) {
    Entity entity = Entity();
    Motion& motion = registry.motions.emplace(entity);
    motion.position = scene_dimensions * 0.5f * BACKGROUND_DEPTH;
    motion.scale = scene_dimensions * BACKGROUND_DEPTH;

    registry.renderRequests.insert(entity, {
        texture_id,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    registry.layers.insert(entity, { LAYER_ID::BACKGROUND });


    return entity;
}