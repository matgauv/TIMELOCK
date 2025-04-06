#include "boss_one_vfx_utils.hpp"

void update_boss_halo(const Entity boss_entity, const Boss& boss) {
    if (!registry.haloRequests.has(boss_entity)) {
        return;
    }

    HaloRequest& halo_request = registry.haloRequests.get(boss_entity);

    if (boss.boss_state == BOSS_STATE::BOSS1_IDLE_STATE) {
        halo_request.target_color = BOSS_IDLE_HALO;
    }
    else if (
        boss.boss_state == BOSS_STATE::BOSS1_DAMAGED_STATE ||
        boss.boss_state == BOSS_STATE::BOSS1_DEAD_STATE) {
        float factor = glm::clamp(boss.timer_ms / BOSS_ONE_MAX_DAMAGED_DURATION_MS, 0.0f, 1.0f) - 0.98f;
        factor = glm::clamp(-16.0f * factor * factor + 1.0f, 0.0f, 1.0f); // -16(x-0.95)^4+1

        halo_request.halo_color = factor * vec4(3.0f) + (1.0f - factor) * BOSS_DAMAGED_HALO;
        halo_request.target_color = halo_request.halo_color;
    }
    else if (boss.boss_state == BOSS_STATE::BOSS1_EXHAUSTED_STATE) {
        vec4 color = BOSS_EXHAUST_HALO;
        color.a = (0.1f * sinf(boss.timer_ms * 2.0f * M_PI / BOSS_EXHALE_PERIOD_MS) + .9f);
        halo_request.halo_color = color;
        halo_request.target_color = color;

        if (registry.snoozeButtons.size() > 0 && registry.haloRequests.has(registry.snoozeButtons.entities[0])) {
            HaloRequest& snooze_button_halo = registry.haloRequests.get(registry.snoozeButtons.entities[0]);
            snooze_button_halo.halo_color = color;
            snooze_button_halo.target_color = color;
        }
    }
    else if (boss.boss_state == BOSS_STATE::BOSS1_RECOVER_STATE) {
        halo_request.target_color = BOSS_RECOVER_HALO;
    }
    else if (boss.boss_state == BOSS_STATE::BOSS1_DASH_ATTACK_STATE) {
        halo_request.target_color = BOSS_DASH_HALO;
    }
    else if (
        boss.boss_state == BOSS_STATE::BOSS1_FAST_PROJECTILE_ATTACK_STATE ||
        boss.boss_state == BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE ||
        boss.boss_state == BOSS_STATE::BOSS1_GROUND_SLAM_LAND_1_STATE ||
        boss.boss_state == BOSS_STATE::BOSS1_GROUND_SLAM_LAND_2_STATE ||
        boss.boss_state == BOSS_STATE::BOSS1_GROUND_SLAM_LAND_3_STATE ||
        boss.boss_state == BOSS_STATE::BOSS1_GROUND_SLAM_SLAM_1_STATE ||
        boss.boss_state == BOSS_STATE::BOSS1_GROUND_SLAM_SLAM_2_STATE ||
        boss.boss_state == BOSS_STATE::BOSS1_GROUND_SLAM_SLAM_3_STATE) {
        halo_request.target_color = BOSS_ATTACK_HALO;
    }
    else if ( boss.boss_state == BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE){
        halo_request.target_color = BOSS_SUMMONING_HALO;
    }
    else {
        halo_request.target_color = BOSS_NORMAL_HALO;
    }
}

void emit_broken_parts(const Motion& boss_motion) {
    vec2 position = random_sample_ellipse(boss_motion.position - vec2{ 0.0, 30.0f }, boss_motion.scale - vec2{0.0, 15.0f});

    ParticleSystem::spawn_particle(PARTICLE_ID::BROKEN_PARTS,
        position, 0.0f,
        rand_float(0.8f, 1.2f) * vec2(16.0f), vec2{ 5.f * (position.x - boss_motion.position.x), rand_float(-150.0f, -100.0f) },
        800.0f, 0.9f, { 0.0f, 300.0f });
}

void slam_effect(const Motion& boss_motion) {
    // Crack particle
    
    float ground_level = BOSS_ONE_ON_GROUND_Y_POSITION + 0.5f * BOSS_ONE_BB_HEIGHT_PX;
    vec2 position = vec2{boss_motion.position.x, ground_level + 0.5f * SLAM_CRACKING_SIZE}; // Offset to ground position

    ParticleSystem::spawn_particle(PARTICLE_ID::CRACKING_DOWNWARD,
        position, 0.0f,
        vec2(SLAM_CRACKING_SIZE), vec2(0.0f),
        1500.0f, 1.0f, { 0.0f, 1000.0f });

    // Dust particles
    for (int i = 0; i < 15; i++) {
        ParticleSystem::spawn_particle(vec3(1.f, 1.f, 0.9f),
            random_sample_rectangle(vec2{ boss_motion.position.x , ground_level }, vec2{ BOSS_ONE_BB_HEIGHT_PX * 1.5f , 2.0f}), 0.0f,
            vec2(2.0f), vec2(0.0f, rand_float(-400.0f, -300.0f)),
            800.0f, 0.8f, { 0.0f, 400.0f }, {0.0f, 0.0f}, 0.0f, 0.5f);
    }

    // Shake camera
    CameraSystem::shake_camera(15.0f, 10.0f);
}

void emit_gathering_particle(vec2 center, float radius, float life, vec3 color) {
    if (life <= 1e-4f) {
        return;
    }

    float speed = radius / life;
    vec2 offset_direction = rand_direction();

    vec2 position = center + radius * offset_direction;

    ParticleSystem::spawn_particle(color,
        position, 0.0f,
        vec2(2.5f), -offset_direction * speed * 1000.0f,
        life, 1.0f, { 0.1f * life, 0.0f }, { 0.0f, 0.35f * life });
}

// Consider migrate to world system
void emit_elliptical_particles(vec2 center, vec2 dimension, float angle_rad, int count, float local_speed, vec2 global_velocity, vec3 color, float size, float life) {
    for (int i = 0; i < count; i++) {
        vec2 disp_dir = rotate_2D(rand_direction() * dimension, angle_rad);

        vec2 velocity = local_speed * disp_dir + global_velocity;

        ParticleSystem::spawn_particle(color,
            center, 0.0f,
            vec2(size), velocity,
            life, 1.0f, { 0.0f, 0.35f * life}, { 0.1f * life, 0.35f * life });
    }
}