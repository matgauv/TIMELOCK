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
    create_static_platform({700.0f, WINDOW_HEIGHT_PX - 100.0f}, {WINDOW_WIDTH_PX /4.0f, 100.0f});

    Path forward = Path({200.0f, 380.0f}, {350.0f, 380.0f}, 1.5);
    Path up = Path({350.0f, 380.0f}, {350.0f, 100.0f}, 1.5);
    Path down = Path({350.0f, 100.0f}, {350.0f, 380.0f}, 1.5);
    Path backwards = Path({350.0f, 380.0f}, {200.0f, 380.0f}, 1.5);
    std::vector<Path> movements = {forward, up, down, backwards};
    create_moving_platform({100.0f, 20.0f}, movements);

    Path top_forward = Path({450.0f, 100.0f}, {600.0f, 100.0f}, 0.1);
    Path top_backward = Path({600.0f, 100.0f}, {450.0f, 100.0f}, 0.1);
    std::vector<Path> movements_top_platform = {top_forward, top_backward};
    create_moving_platform({100.0f, 20.0f}, movements_top_platform);

    Path top2_forward = Path({700.0f, 100.0f}, {1000.0f, 0.0f}, 3.0);
    Path top2_backward = Path({1000.0f, 0.0f}, {700.0f, 100.0f}, 3.0);
    std::vector<Path> movements_top2_platform = {top2_forward, top2_backward};
    create_moving_platform({100.0f, 20.0f}, movements_top2_platform);
    //create_physics_object({300.0f, -150.0f}, {50.0f, 50.0f}, 5.0f);
    create_physics_object({150.0f, -200.0f}, {50.0f, 50.0f}, 5.0f);
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

Entity create_projectile(vec2 pos, vec2 size, vec2 velocity)
{
	auto entity = Entity();

	Projectile& projectile = registry.projectiles.emplace(entity);
	
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
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