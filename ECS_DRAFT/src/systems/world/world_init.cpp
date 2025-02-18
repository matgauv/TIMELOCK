#include "world_init.hpp"
#include "../../tinyECS/registry.hpp"
#include <iostream>

// TODO parse file descriptor to create level with render requests
// For now, hard coded to just put a platform on the screen...
void load_level(std::string descriptor_path) {
    (void)descriptor_path;
    vec2 initial_pos = { 50.0f, 50.0f };
    create_player(initial_pos, {50.0f, 50.0f});
    create_platform({WINDOW_WIDTH_PX / 2.0f, WINDOW_HEIGHT_PX - 50.0f}, {WINDOW_WIDTH_PX, 100.0f});
    create_platform({25.0f, WINDOW_HEIGHT_PX - 100.0f}, {WINDOW_WIDTH_PX /4.0f, 100.0f});
    create_camera(initial_pos, { 1.0f, 1.0f }); // TODO: potential open-scene zoom in
}

Entity create_player(vec2 position, vec2 scale) {
    Entity entity = Entity();

    Player &player = registry.players.emplace(entity);
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

    return entity;
}

Entity create_platform(vec2 position, vec2 scale) {
    Entity entity = Entity();
    Platform& platform = registry.platforms.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);

    (void)platform; // TODO: use this if we need to

    motion.position = position;
    motion.scale = scale;
    motion.velocity = {0, 0}; // TODO static platforms for now.
    motion.angle = 0.0f;

    registry.renderRequests.insert(entity, {
        TEXTURE_ASSET_ID::BLACK,
        EFFECT_ASSET_ID::TEXTURED,
        GEOMETRY_BUFFER_ID::SPRITE
    });

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