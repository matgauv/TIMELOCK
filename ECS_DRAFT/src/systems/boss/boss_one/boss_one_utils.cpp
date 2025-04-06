#include "boss_one_utils.hpp"
#include "../../world/world_init.hpp"
#include <iostream>
#include <cmath>
#include "../../player/player_system.hpp"


Entity create_first_boss() {
    auto entity = Entity();

    Boss& boss = registry.bosses.emplace(entity);
    boss.boss_id = BOSS_ID::FIRST;
    boss.boss_state = BOSS_STATE::BOSS1_IDLE_STATE;
    boss.can_be_damaged = false;
    boss.time_until_exhausted_ms = BOSS_ONE_MAX_TIME_UNTIL_EXHAUSTED_MS;
    boss.health = BOSS_ONE_MAX_HEALTH;
    boss.nextAttacks;

    FirstBoss& firstBoss = registry.firstBosses.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);

    // initially idle for testing purposes
    motion.velocity = vec2(0.f, 0.f);
    motion.scale = vec2(BOSS_ONE_BB_WIDTH_PX, BOSS_ONE_BB_HEIGHT_PX);

    TimeControllable& tc = registry.timeControllables.emplace(entity);

    // grab player position and use that as the spawning point
    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    // initial position
    motion.position = vec2(BOSS_ONE_SPAWN_POINT_X, BOSS_ONE_SPAWN_POINT_Y);

    // render request
    registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::GREY_CIRCLE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

    registry.layers.insert(entity, {LAYER_ID::MIDGROUND});

    AnimateRequest& animateRequest = registry.animateRequests.emplace(entity);
    animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_IDLE;

    HaloRequest& haloRequest = registry.haloRequests.emplace(entity);
    haloRequest.halo_color = BOSS_IDLE_HALO;
    haloRequest.target_color = BOSS_IDLE_HALO;

    return entity;
}

Entity create_snooze_button(vec2 boss_position) {
    auto entity = Entity();

    registry.snoozeButtons.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);
    motion.velocity = vec2(0.f, 0.f);
    
    // snooze button position should be based on the clock's position
    motion.position = boss_position - vec2(0.f, BOSS_ONE_BB_HEIGHT_PX / 2 - 6.f); // TODO: fine tune this
    
    registry.nonPhysicsColliders.emplace(entity);

    // render request
    registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::BOSS_ONE_SNOOZE_BUTTON,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

    registry.layers.insert(entity, {LAYER_ID::MIDGROUND});

    registry.haloRequests.emplace(entity);

    return entity;
}

// Handles the state transition logic by checking the current boss state and then calling the corresponding helper
void boss_one_step(Entity& boss_entity, float elapsed_ms, unsigned int random_num, std::default_random_engine& rng) {
    Boss& boss = registry.bosses.get(boss_entity);
    Motion& boss_motion = registry.motions.get(boss_entity);

    bool is_in_phase_two = (boss.health / BOSS_ONE_MAX_HEALTH) <= 0.8;

    boss.time_until_exhausted_ms -= elapsed_ms;

    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    // std::cout << "Player position is: (" << player_motion.position.x << ", " << player_motion.position.y << ")" << std::endl;
    // std::cout << "Boss position is: (" << boss_motion.position.x << ", " << boss_motion.position.y << ")" << std::endl;

    // state transition logic
    switch (boss.boss_state) {
        case BOSS_STATE::BOSS1_IDLE_STATE:
            boss_one_idle_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;

        case BOSS_STATE::BOSS1_MOVE_STATE:
            boss_one_move_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;

        case BOSS_STATE::BOSS1_EXHAUSTED_STATE:
            boss_one_exhausted_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;

        case BOSS_STATE::BOSS1_RECOVER_STATE:
            boss_one_recover_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_DAMAGED_STATE:
            boss_one_damaged_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;

        case BOSS_STATE::BOSS1_DEAD_STATE:
            boss_one_dead_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_CHOOSE_ATTACK_STATE:
            boss_one_choose_attack_step(boss_entity, boss, boss_motion, elapsed_ms, random_num, rng);
            break;
        
        case BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE:
            boss_one_regular_projectile_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_FAST_PROJECTILE_ATTACK_STATE:
            boss_one_fast_projectile_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
            
        case BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE:
            boss_one_delayed_projectile_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
            
        case BOSS_STATE::BOSS1_DASH_ATTACK_STATE:
            boss_one_dash_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
            
        case BOSS_STATE::BOSS1_GROUND_SLAM_INIT_1_STATE:
            boss_one_ground_slam_init_1_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;

        case BOSS_STATE::BOSS1_GROUND_SLAM_RISE_1_STATE:
            boss_one_ground_slam_rise_1_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_GROUND_SLAM_FOLLOW_1_STATE:
            boss_one_ground_slam_follow_1_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_GROUND_SLAM_SLAM_1_STATE:
            boss_one_ground_slam_slam_1_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
            
        case BOSS_STATE::BOSS1_GROUND_SLAM_LAND_1_STATE:
            boss_one_ground_slam_land_1_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_GROUND_SLAM_INIT_2_STATE:
            boss_one_ground_slam_init_2_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_GROUND_SLAM_RISE_2_STATE:
            boss_one_ground_slam_rise_2_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_GROUND_SLAM_FOLLOW_2_STATE:
            boss_one_ground_slam_follow_2_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_GROUND_SLAM_SLAM_2_STATE:
            boss_one_ground_slam_slam_2_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
            
        case BOSS_STATE::BOSS1_GROUND_SLAM_LAND_2_STATE:
            boss_one_ground_slam_land_2_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_GROUND_SLAM_INIT_3_STATE:
            boss_one_ground_slam_init_3_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;

        case BOSS_STATE::BOSS1_GROUND_SLAM_RISE_3_STATE:
            boss_one_ground_slam_rise_3_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_GROUND_SLAM_FOLLOW_3_STATE:
            boss_one_ground_slam_follow_3_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        case BOSS_STATE::BOSS1_GROUND_SLAM_SLAM_3_STATE:
            boss_one_ground_slam_slam_3_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
            
        case BOSS_STATE::BOSS1_GROUND_SLAM_LAND_3_STATE:
            boss_one_ground_slam_land_3_step(boss_entity, boss, boss_motion, elapsed_ms);
            break;
        
        default:
            break;
    }

    // TODO: check if there is any other variable in the boss component that we need to update?

    update_boss_halo(boss_entity, boss);
}

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
        float factor = std::clamp(boss.timer_ms / BOSS_ONE_MAX_DAMAGED_DURATION_MS, 0.0f, 1.0f) - 0.98f;
        factor = std::clamp(-16.0f * factor * factor + 1.0f, 0.0f, 1.0f); // -16(x-0.95)^4+1

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
    } else if (boss.boss_state == BOSS_STATE::BOSS1_DASH_ATTACK_STATE) {
        halo_request.target_color = BOSS_DASH_HALO;
    }
    else if (
        boss.boss_state == BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE ||
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
    else {
        halo_request.target_color = BOSS_NORMAL_HALO;
    }
}

// Handles the logic to transition into the MOVE state
void boss_one_idle_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Get player motion
    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);
    
    // find the distance between the player and the boss
    float dist = abs(boss_motion.position.x - player_motion.position.x);

    // transition to move if both are in screen
    if (dist <= 200.f) {
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        boss.timer_ms = BOSS_ONE_MAX_WALK_DURATION_MS; // walk for 5 seconds before attacking
        
        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_WALK;
    }
}

// Handles the boss motion and the transition logic to CHOOSE_ATTACK and EXHAUSTED state
void boss_one_move_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Get player motion
    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    // Use a helpfer function to determine the boss velocity
    boss_motion.velocity.x = calculate_boss_one_x_velocity(boss_motion.position.x, player_motion.position.x);
    boss_motion.velocity.y = 0;

    // If 30 seconds have passed, enter EXHAUSTED state
    if (boss.time_until_exhausted_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_EXHAUSTED_STATE;
        boss.can_be_damaged = true;
        boss.timer_ms = BOSS_ONE_MAX_EXHAUSTED_DURATION_MS;
        boss_motion.velocity.x = 0.f;

        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_EXHAUSTED;

        return;
    }

    // Otherwise, decrement the cooldown
    boss.timer_ms -= elapsed_ms;

    // If the boss has walked for 5 seconds, then enter CHOOSE ATTACK state
    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_CHOOSE_ATTACK_STATE;
    }

    // bool is_player_to_boss_left = player_motion.position.x <= boss_motion.position.x;
    // for testing purposes
    // if (boss.timer_ms <= 0.f) {
    //     // choose_dash_attack_test(boss_entity, boss, boss_motion);
    //     // choose_regular_projectile_attack_test(boss_entity, boss, boss_motion, is_player_to_boss_left);
    //     // choose_delayed_projectile_attack_test(boss_entity, boss, boss_motion);
    //     // choose_fast_projectile_attack_test(boss_entity, boss, boss_motion, is_player_to_boss_left);
    //     choose_ground_slam_test(boss_entity, boss, boss_motion);
    // }
}

// Handles the transition logic to RECOVER and DAMAGED states
void boss_one_exhausted_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    
    // create the snooze button if it is not there
    if (registry.snoozeButtons.components.size() == 0 && boss_motion.velocity.x == 0.f) {
        create_snooze_button(boss_motion.position);
    }

    // Decrement timer
    boss.timer_ms -= elapsed_ms;

    // Check for collision between snooze button and player
    assert(registry.firstBosses.components.size() <= 1);
    FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);
    
    if (firstBoss.player_collided_with_snooze_button) {
        boss.boss_state = BOSS_STATE::BOSS1_DAMAGED_STATE;
        boss.health -= PLAYER_ATTACK_DAMAGE;
        firstBoss.player_collided_with_snooze_button = false;
        boss.timer_ms = BOSS_ONE_MAX_DAMAGED_DURATION_MS;


        CameraSystem::shake_camera(20.0f, 10.0f);

        // Emit Particles
        // Broken parts
        for (int i = 0; i < 15; i++) {
            vec2 position = random_sample_ellipse(boss_motion.position, boss_motion.scale);

            ParticleSystem::spawn_particle(PARTICLE_ID::BROKEN_PARTS,
                position, 0.0f,
                rand_float(0.8f, 1.2f) * vec2(16.0f), vec2{ 5.f*(position.x - boss_motion.position.x), rand_float(-75.0f, -40.0f) },
                800.0f, 0.9f, { 0.0f, 300.0f });
        }

        // Potential fix for snooze button issue
        if (registry.snoozeButtons.size() > 0 && registry.motions.has(registry.snoozeButtons.entities[0])) {
            const vec2 snooze_button_pos = registry.motions.get(registry.snoozeButtons.entities[0]).position;
            // Emit particles
            for (int i = 0; i < 30; i++) {
                ParticleSystem::spawn_particle(BOSS_EXHAUST_HALO,
                    snooze_button_pos, 0.0f,
                    vec2(2.0f), rand_direction() * 50.0f,
                    500.0f, 1.0f, { 0.0f, 0.0f }, {50.0f, 50.0f});
            }

            registry.remove_all_components_of(registry.snoozeButtons.entities[0]);
        }

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_DAMAGED;
    }

    // Otherwise, if the timer is up, then the boss enters RECOVER STATE
    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_RECOVER_STATE;
        boss.can_be_damaged = false;
        boss.timer_ms = BOSS_ONE_MAX_RECOVER_DURATION_MS;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_RECOVERED;
    }

    // Breath particles
    if ((BOSS_EXHALE_PERIOD_MS - ((int)boss.timer_ms % (int)BOSS_EXHALE_PERIOD_MS)) < 50.0f) {
        // Reuse Coyote particle
        ParticleSystem::spawn_particle(PARTICLE_ID::EXHALE,
            boss_motion.position + vec2{0.0, 12.0f}, 0.0f, rand_float(0.5f, 1.0f) * vec2(16.0f),
            vec2{ rand_float(-25.0f, 25.0f), rand_float(10.0f, 15.0f) }, 1200.0f, 0.35f,
            { 50.0f, 800.0f }, {20.0f, 0.0f});
    }
}

// Handles the transition logic to MOVE state
void boss_one_recover_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Decrement the timer
    boss.timer_ms -= elapsed_ms;

    // Remove the snooze button
    for (Entity& button_entity : registry.snoozeButtons.entities) {
        registry.remove_all_components_of(button_entity);
    }

    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        boss.timer_ms = BOSS_ONE_MAX_WALK_DURATION_MS;

        boss.time_until_exhausted_ms = BOSS_ONE_MAX_TIME_UNTIL_EXHAUSTED_MS;
        
        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_WALK;
    }

    // Emit particles
    if (rand_float() < 0.1f) {
        ParticleSystem::spawn_particle(BOSS_RECOVER_HALO,
            random_sample_rectangle(boss_motion.position + vec2{0.0f, boss_motion.scale.y * 0.4f }, vec2{ boss_motion.scale.x, 30.0f }), 0.0f,
            vec2(1.2f, rand_float(15.f, 20.f)), vec2{0.0f, rand_float(-60.0f, -35.f)},
            800.0f, 0.8f, { 200.0f, 300.0f });
    }
}

// Handles the transition logic to RECOVER or DEAD state
void boss_one_damaged_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Decrement timer
    boss.timer_ms -= elapsed_ms;

    // Remove the snooze button
    for (Entity& button_entity : registry.snoozeButtons.entities) {
        registry.remove_all_components_of(button_entity);
    }

    // If boss health has reached 0, transition to DEAD state
    if (boss.health <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_DEAD_STATE;
        return;
    }

    // Otherwise, if timer is up, transition to RECOVER state
    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_RECOVER_STATE;
        boss.timer_ms = BOSS_ONE_MAX_RECOVER_DURATION_MS;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_RECOVERED;
    }

    // Emit particles
    if (rand_float() < 0.1f) {
        vec2 position = random_sample_ellipse(boss_motion.position, boss_motion.scale);

        ParticleSystem::spawn_particle(PARTICLE_ID::BROKEN_PARTS,
            position, 0.0f,
            rand_float(0.8f, 1.2f) * vec2(12.0f), vec2{ 5.f*(position.x - boss_motion.position.x), rand_float(-75.0f, -40.0f) },
            800.0f, 0.8f, { 0.0f, 300.0f });
    }
}

// Handles the logic of ending the boss fight, cleaning up the entities and components if needed
void boss_one_dead_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Update the flag in game state
    assert(registry.gameStates.components.size() <= 1);
    GameState& gameState = registry.gameStates.components[0];

    gameState.is_in_boss_fight = false;

    // Remove the boss and its components
    registry.remove_all_components_of(boss_entity);

    // TODO: Open the path to the exit
}

// Handles the logic of choosing which attack to use and then transition to the corresponding attack state
// Transition to one of: REGULAR PROJECTILE, FAST PROJECTILE, DELAYED PROJECTILE, DASH or GROUND SLAM RISE 1 states
void boss_one_choose_attack_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms, unsigned int random_num, std::default_random_engine& rng) {

    chooseAttack(boss_entity, boss, boss_motion, elapsed_ms, random_num, rng);

}

// Creates 4 projectiles and transitions to MOVE state
void boss_one_regular_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    assert(registry.firstBosses.components.size() <= 1);
    FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);

    if (firstBoss.num_of_projectiles_created == BOSS_ONE_MAX_NUM_OF_NON_DELAYED_PROJECTILE && boss.boss_state != BOSS_STATE::BOSS1_MOVE_STATE) {

        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        firstBoss.num_of_projectiles_created = 0;
        boss.timer_ms = BOSS_ONE_MAX_WALK_DURATION_MS;

        // update the scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_WALK;

    } else {
        boss_one_regular_projectile_attack(boss_entity, boss, boss_motion, elapsed_ms);
    }
}

// Creates 4 fast projectiles and transitions to MOVE state
void boss_one_fast_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    assert(registry.firstBosses.components.size() <= 1);
    FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);

    if (firstBoss.num_of_projectiles_created == BOSS_ONE_MAX_NUM_OF_NON_DELAYED_PROJECTILE) {
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        firstBoss.num_of_projectiles_created = 0;
        boss.timer_ms = BOSS_ONE_MAX_WALK_DURATION_MS;

        // update the scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX;        

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_WALK;

    } else {
        boss_one_fast_projectile_attack(boss_entity, boss, boss_motion, elapsed_ms);
    }
}

// Creates 3 delayed projectiles and transitions to MOVE state
void boss_one_delayed_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        boss_one_delayed_projectile_attack(boss_entity, boss, boss_motion, elapsed_ms);
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);
        firstBoss.num_of_projectiles_created = 0;
        boss.timer_ms = BOSS_ONE_MAX_WALK_DURATION_MS * 2.f;
    
        // update the scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX;
                
        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_WALK;
    }

}

// Increases boss x velocity and transitions to MOVE state
void boss_one_dash_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Decrement the timer
    boss.timer_ms -= elapsed_ms;

    // Transitions to MOVE state if timer is up
    if (boss.timer_ms <= 0.f) {

        // boss becomes harmless
        registry.harmfuls.remove(boss_entity);
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = false;
        tc.can_become_harmless = false;

        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        boss.timer_ms = BOSS_ONE_MAX_WALK_DURATION_MS;

        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_WALK;
    }

    // Shake camera
    CameraSystem::shake_camera(5.0f, 10.0f);
}

void boss_one_ground_slam_init_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_RISE;

        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_1_STATE;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;
    }
}

// Updates boss motion, quickly decreases boss y coordinate (boss should move upward and stop)
// and transitions to GROUND SLAM FOLLOW 1 state
void boss_one_ground_slam_rise_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    if (boss_motion.position.y <= BOSS_ONE_GROUND_SLAM_RISE_FINAL_Y_POSITION) {
        boss_motion.position.y = BOSS_ONE_GROUND_SLAM_RISE_FINAL_Y_POSITION;
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_FOLLOW_1_STATE;
        boss.timer_ms = BOSS_ONE_FIRST_GROUND_SLAM_FOLLOW_DURATION_MS;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_FOLLOW;
    }
}

// Updates boss motion, boss follows the player based on their x coordinates for some time
// and transitions to GROUND SLAM SLAM 1 state
void boss_one_ground_slam_follow_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_SLAM_VELOCITY;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_SLAM_1_STATE;

        // boss becomes harmful
        if (!registry.harmfuls.has(boss_entity)) {
            registry.harmfuls.emplace(boss_entity);
        }

        // boss becomes time controllable
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = true;
        tc.can_become_harmless = true;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_FALL;

    } else {
        Entity& player_entity = registry.players.entities[0];
        Motion& player_motion = registry.motions.get(player_entity);
    
        boss_motion.velocity.x = 10.f * calculate_boss_one_x_velocity(boss_motion.position.x, player_motion.position.x);
    }
}

// Updates boss motion, quickly increases boss y coordinate (boss should move downward)
// and transitions to GROUND SLAM LAND 1 state
void boss_one_ground_slam_slam_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // moved the player boss collision logic here for now
    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    float player_left_bound = player_motion.position.x - PLAYER_BB_WIDTH_PX / 2.f;
    float player_right_bound = player_motion.position.x + PLAYER_BB_WIDTH_PX / 2.f;
    float player_top_bound = player_motion.position.y - PLAYER_BB_HEIGHT_PX / 2.f;
    float player_bot_bound = player_motion.position.y + PLAYER_BB_HEIGHT_PX / 2.f;

    float boss_left_bound = boss_motion.position.x - BOSS_ONE_BB_WIDTH_PX / 2.f;
    float boss_right_bound = boss_motion.position.x + BOSS_ONE_BB_WIDTH_PX / 2.f;
    float boss_top_bound = boss_motion.position.y - BOSS_ONE_BB_HEIGHT_PX / 2.f;
    float boss_bot_bound = boss_motion.position.y + BOSS_ONE_BB_HEIGHT_PX / 2.f;

    if (player_left_bound <= boss_right_bound && player_right_bound >= boss_left_bound && 
        player_bot_bound >= boss_top_bound && player_top_bound <= boss_bot_bound) {
        
        if (registry.harmfuls.has(boss_entity)) {
            PlayerSystem::kill();
        }
    }

    if (boss_motion.position.y >= BOSS_ONE_ON_GROUND_Y_POSITION - 20.f) {
        boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION - 20.f;
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_LAND_1_STATE;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_LAND_DURATION_MS;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_LAND;
    }
}

// Check if the player is within a specific range (impact range, define a constant for the width and height)
// and transitions to GROUND SLAM RISE 2 state
void boss_one_ground_slam_land_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    
    boss.timer_ms -= elapsed_ms;

    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    // handle collision
    float left_bound = boss_motion.position.x - BOSS_ONE_BB_WIDTH_PX / 2 - BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    float right_bound = boss_motion.position.x + BOSS_ONE_BB_WIDTH_PX / 2 + BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    GameState& gameState = registry.gameStates.components[0];

    bool is_to_hit_radius_left = left_bound <= player_motion.position.x;
    bool is_to_hit_radius_right = player_motion.position.x <= right_bound;
    bool is_not_in_decel_state = gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED;

    if (left_bound <= player_motion.position.x && player_motion.position.x <= right_bound && 
        player_motion.position.y >= PLAYER_ON_BOSS_GROUND_POSITION_Y_THRESHOLD &&
        gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED) {
        PlayerSystem::kill();
    }

    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_INIT_2_STATE;
        boss_motion.velocity.x = 0;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_INIT_DURATION_MS;

        // boss becomes harmless
        registry.harmfuls.remove(boss_entity);
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = false;
        tc.can_become_harmless = false;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_INIT;
    }
}

void boss_one_ground_slam_init_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    
    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_RISE;

        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_2_STATE;

        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;
    }
}

// Updates boss motion, quickly decreases boss y coordinate (boss should move upward and stop)
// and transitions to GROUND SLAM FOLLOW 2 state
void boss_one_ground_slam_rise_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    if (boss_motion.position.y <= BOSS_ONE_GROUND_SLAM_RISE_FINAL_Y_POSITION) {
        boss_motion.position.y = BOSS_ONE_GROUND_SLAM_RISE_FINAL_Y_POSITION;
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_FOLLOW_2_STATE;
        boss.timer_ms = BOSS_ONE_SECOND_GROUND_SLAM_FOLLOW_DURATION_MS;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_FOLLOW;
    }
}

// Updates boss motion, boss follows the player based on their x coordinates for some time
// and transitions to GROUND SLAM SLAM 2 state
void boss_one_ground_slam_follow_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_SLAM_VELOCITY;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_SLAM_2_STATE;

        // boss becomes harmful
        if (!registry.harmfuls.has(boss_entity)) {
            registry.harmfuls.emplace(boss_entity);
        }

        // boss becomes time controllable
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = true;
        tc.can_become_harmless = true;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_FALL;

    } else {
        Entity& player_entity = registry.players.entities[0];
        Motion& player_motion = registry.motions.get(player_entity);
    
        boss_motion.velocity.x = 10.f * calculate_boss_one_x_velocity(boss_motion.position.x, player_motion.position.x);
    }
}

// Updates boss motion, quickly increases boss y coordinate (boss should move downward)
// and transitions to GROUND SLAM LAND 2 state
void boss_one_ground_slam_slam_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    float player_left_bound = player_motion.position.x - PLAYER_BB_WIDTH_PX / 2.f;
    float player_right_bound = player_motion.position.x + PLAYER_BB_WIDTH_PX / 2.f;
    float player_top_bound = player_motion.position.y - PLAYER_BB_HEIGHT_PX / 2.f;
    float player_bot_bound = player_motion.position.y + PLAYER_BB_HEIGHT_PX / 2.f;

    float boss_left_bound = boss_motion.position.x - BOSS_ONE_BB_WIDTH_PX / 2.f;
    float boss_right_bound = boss_motion.position.x + BOSS_ONE_BB_WIDTH_PX / 2.f;
    float boss_top_bound = boss_motion.position.y - BOSS_ONE_BB_HEIGHT_PX / 2.f;
    float boss_bot_bound = boss_motion.position.y + BOSS_ONE_BB_HEIGHT_PX / 2.f;

    if (player_left_bound <= boss_right_bound && player_right_bound >= boss_left_bound && 
        player_bot_bound >= boss_top_bound && player_top_bound <= boss_bot_bound) {
        
        if (registry.harmfuls.has(boss_entity)) {
            PlayerSystem::kill();
        }
    }

    if (boss_motion.position.y >= BOSS_ONE_ON_GROUND_Y_POSITION - 20.f) {
        boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION - 20.f;
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_LAND_2_STATE;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_LAND_DURATION_MS;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_LAND;
    }
}

// Check if the player is within a specific range (impact range, define a constant for the width and height)
// and transitions to GROUND SLAM RISE 3 state
void boss_one_ground_slam_land_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    boss.timer_ms -= elapsed_ms;

    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    // handle collision
    float left_bound = boss_motion.position.x - BOSS_ONE_BB_WIDTH_PX / 2 - BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    float right_bound = boss_motion.position.x + BOSS_ONE_BB_WIDTH_PX / 2 + BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    GameState& gameState = registry.gameStates.components[0];

    bool is_to_hit_radius_left = left_bound <= player_motion.position.x;
    bool is_to_hit_radius_right = player_motion.position.x <= right_bound;
    bool is_not_in_decel_state = gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED;

    if (left_bound <= player_motion.position.x && player_motion.position.x <= right_bound && 
        player_motion.position.y >= PLAYER_ON_BOSS_GROUND_POSITION_Y_THRESHOLD &&
        gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED) {
        PlayerSystem::kill();
    }

    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_INIT_3_STATE;
        boss_motion.velocity.x = 0;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_INIT_DURATION_MS;

        registry.harmfuls.remove(boss_entity);
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = false;
        tc.can_become_harmless = false;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_INIT;
    }
}

void boss_one_ground_slam_init_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    
    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_RISE;

        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_3_STATE;

        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;
    }
}

// Updates boss motion, quickly decreases boss y coordinate (boss should move upward and stop)
// and transitions to GROUND SLAM FOLLOW 3 state
void boss_one_ground_slam_rise_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    if (boss_motion.position.y <= BOSS_ONE_GROUND_SLAM_RISE_FINAL_Y_POSITION) {
        boss_motion.position.y = BOSS_ONE_GROUND_SLAM_RISE_FINAL_Y_POSITION;
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_FOLLOW_3_STATE;
        boss.timer_ms = BOSS_ONE_THIRD_GROUND_SLAM_FOLLOW_DURATION_MS;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_FOLLOW;
    }
}

// Updates boss motion, boss follows the player based on their x coordinates for some time
// and transitions to GROUND SLAM SLAM 3 state
void boss_one_ground_slam_follow_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_SLAM_VELOCITY;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_SLAM_3_STATE;

         // boss becomes harmful
        if (!registry.harmfuls.has(boss_entity)) {
            registry.harmfuls.emplace(boss_entity);
        }

        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = true;
        tc.can_become_harmless = true;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_FALL;

    } else {
        Entity& player_entity = registry.players.entities[0];
        Motion& player_motion = registry.motions.get(player_entity);
    
        boss_motion.velocity.x = 10.f * calculate_boss_one_x_velocity(boss_motion.position.x, player_motion.position.x);
    }
}

// Updates boss motion, quickly increases boss y coordinate (boss should move downward)
// and transitions to GROUND SLAM LAND 3 state
void boss_one_ground_slam_slam_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    float player_left_bound = player_motion.position.x - PLAYER_BB_WIDTH_PX / 2.f;
    float player_right_bound = player_motion.position.x + PLAYER_BB_WIDTH_PX / 2.f;
    float player_top_bound = player_motion.position.y - PLAYER_BB_HEIGHT_PX / 2.f;
    float player_bot_bound = player_motion.position.y + PLAYER_BB_HEIGHT_PX / 2.f;

    float boss_left_bound = boss_motion.position.x - BOSS_ONE_BB_WIDTH_PX / 2.f;
    float boss_right_bound = boss_motion.position.x + BOSS_ONE_BB_WIDTH_PX / 2.f;
    float boss_top_bound = boss_motion.position.y - BOSS_ONE_BB_HEIGHT_PX / 2.f;
    float boss_bot_bound = boss_motion.position.y + BOSS_ONE_BB_HEIGHT_PX / 2.f;

    if (player_left_bound <= boss_right_bound && player_right_bound >= boss_left_bound && 
        player_bot_bound >= boss_top_bound && player_top_bound <= boss_bot_bound) {
        
        if (registry.harmfuls.has(boss_entity)) {
            PlayerSystem::kill();
        }
    }

    if (boss_motion.position.y >= BOSS_ONE_ON_GROUND_Y_POSITION - 20.f) {
        boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION - 20.f;
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_LAND_3_STATE;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_LAND_DURATION_MS;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_LAND;
    }
}

// Check if the player is within a specific range (impact range, define a constant for the width and height)
// and transitions to MOVE state
void boss_one_ground_slam_land_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    boss.timer_ms -= elapsed_ms;

    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    // handle collision
    float left_bound = boss_motion.position.x - BOSS_ONE_BB_WIDTH_PX / 2 - BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    float right_bound = boss_motion.position.x + BOSS_ONE_BB_WIDTH_PX / 2 + BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    GameState& gameState = registry.gameStates.components[0];

    bool is_to_hit_radius_left = left_bound <= player_motion.position.x;
    bool is_to_hit_radius_right = player_motion.position.x <= right_bound;
    bool is_not_in_decel_state = gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED;

    if (left_bound <= player_motion.position.x && player_motion.position.x <= right_bound && 
        player_motion.position.y >= PLAYER_ON_BOSS_GROUND_POSITION_Y_THRESHOLD &&
        gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED) {
        PlayerSystem::kill();
    }

    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        boss_motion.velocity.y = 0;
        boss.timer_ms = BOSS_ONE_MAX_WALK_DURATION_MS;

        // update scaling
        boss_motion.scale = vec2(BOSS_ONE_BB_WIDTH_PX, BOSS_ONE_BB_HEIGHT_PX);

        // boss becomes harmless
        boss.can_damage_player = false;
        if (registry.harmfuls.has(boss_entity)) {
            registry.harmfuls.remove(boss_entity);
        }
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = false;
        tc.can_become_harmless = false;

        // update the boss position
        boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION;

        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_WALK;
    }

}

// Uses a linear function to calcualte the x velocity of the boss based on distance from player
float calculate_boss_one_x_velocity(float boss_x, float player_x) {

    // Calculate the distance
    float dist = player_x - boss_x;

    // Calculate a tentative velocity using the multiplier
    float velocity = dist * BOSS_ONE_X_VELOCITY_MULTIPLIER;

    // Clamp the velocity at the set minimum if velocity is too small
    if (abs(velocity) < BOSS_ONE_MIN_X_VELOCITY) {
        velocity = std::copysignf(BOSS_ONE_MIN_X_VELOCITY, dist);

    } else if (abs(velocity) > BOSS_ONE_MAX_X_VELOCITY) {
        // Clamp the velocity at the set maximum if the velocity is too large in either direction
        velocity = std::copysignf(BOSS_ONE_MAX_X_VELOCITY, dist);
    }

    return velocity;
}

// Chooses the attack based on decision tree (distance between boss and player, and a random number between 0 and 100) and transitions to the corresponding state
void chooseAttack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elpased_ms, unsigned int random_num, std::default_random_engine& rng) {
    // helper function that chooses the attack state for boss one using a decision tree

    // grab player related info
    Player& player = registry.players.components[0];
    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    // float dist = abs(boss_motion.position.x - player_motion.position.x) / 400.f;
    bool is_in_phase_two = (boss.health / BOSS_ONE_MAX_HEALTH) <= 0.8f;

    bool is_player_to_boss_left = boss_motion.position.x >= player_motion.position.x;

    BOSS_ATTACK_ID id = get_next_attack(boss, rng);
    if (!is_in_phase_two) {
        while (is_phase_two_attack(id)) {
            id = get_next_attack(boss, rng);
        }
    }
    transition_to_attack_state(boss_entity, boss, boss_motion, is_player_to_boss_left, id);

}

void boss_one_regular_projectile_attack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    assert(registry.firstBosses.components.size() <= 1);
    FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);

    firstBoss.projectile_timer_ms -= elapsed_ms;

    if (firstBoss.num_of_projectiles_created < BOSS_ONE_MAX_NUM_OF_NON_DELAYED_PROJECTILE &&
        firstBoss.projectile_timer_ms <= 0.f) {

            Entity& player_entity = registry.players.entities[0];
            Motion& player_motion = registry.motions.get(player_entity);

            int direction = (player_motion.position.x <= boss_motion.position.x) ? -1 : 1;

            // update the animate request based on the direction
            AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
            animateRequest.used_animation = direction == -1 ? ANIMATION_ID::BOSS_ONE_PROJECTILE_LEFT : ANIMATION_ID::BOSS_ONE_PROJECTILE_RIGHT;

            // create a projectile
            vec2 pos = vec2(boss_motion.position.x + direction * BOSS_ONE_BB_WIDTH_PX / 2, player_motion.position.y);
            vec2 size = vec2(BOSS_ONE_PROJECTILE_WIDTH_PX, BOSS_ONE_PROJECTILE_HEIGHT_PX);
            vec2 velocity = vec2(BOSS_ONE_REGULAR_PROJECTILE_VELOCITY * direction, 0.f);
            create_projectile(pos, size, velocity);

            firstBoss.num_of_projectiles_created++;
            firstBoss.projectile_timer_ms = BOSS_ONE_INTER_PROJECTILE_TIMER_MS;
    }
}

void boss_one_fast_projectile_attack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    assert(registry.firstBosses.components.size() <= 1);
    FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);

    firstBoss.projectile_timer_ms -= elapsed_ms;

    if (firstBoss.num_of_projectiles_created < BOSS_ONE_MAX_NUM_OF_NON_DELAYED_PROJECTILE &&
        firstBoss.projectile_timer_ms <= 0.f) {

            Entity& player_entity = registry.players.entities[0];
            Motion& player_motion = registry.motions.get(player_entity);

            int direction = (player_motion.position.x <= boss_motion.position.x) ? -1 : 1;

            // update the animate request based on the direction
            AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
            animateRequest.used_animation = direction == -1 ? ANIMATION_ID::BOSS_ONE_PROJECTILE_LEFT : ANIMATION_ID::BOSS_ONE_PROJECTILE_RIGHT;

            // create a projectile
            vec2 pos = vec2(boss_motion.position.x + direction * BOSS_ONE_BB_WIDTH_PX / 2, player_motion.position.y);
            vec2 size = vec2(BOSS_ONE_PROJECTILE_WIDTH_PX, BOSS_ONE_PROJECTILE_HEIGHT_PX);
            vec2 velocity = vec2(BOSS_ONE_FAST_PROJECTILE_VELOCITY * direction, 0.f);
            create_projectile(pos, size, velocity);

            firstBoss.num_of_projectiles_created++;
            firstBoss.projectile_timer_ms = BOSS_ONE_INTER_PROJECTILE_TIMER_MS;
    }
}

void boss_one_delayed_projectile_attack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    assert(registry.firstBosses.components.size() <= 1);
    FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);

    // create three delayed projectiles
    if (firstBoss.num_of_projectiles_created < 3) {
        vec2 pos_1 = vec2(BOSS_ONE_FIRST_DELAYED_PROJECTILE_X_POSITION, BOSS_ONE_DELAYED_PROJECTILE_Y_POSITION);
        vec2 pos_2 = vec2(BOSS_ONE_SECOND_DELAYED_PROJECTILE_X_POSITION, BOSS_ONE_DELAYED_PROJECTILE_Y_POSITION);
        vec2 pos_3 = vec2(BOSS_ONE_THIRD_DELAYED_PROJECTILE_X_POSITION, BOSS_ONE_DELAYED_PROJECTILE_Y_POSITION);
        vec2 size = vec2(BOSS_ONE_PROJECTILE_WIDTH_PX, BOSS_ONE_PROJECTILE_HEIGHT_PX);
        create_delayed_projectile(pos_1, BOSS_ONE_FIRST_DELAYED_PROJECTILE_TIMER_MS);
        create_delayed_projectile(pos_2, BOSS_ONE_SECOND_DELAYED_PROJECTILE_TIMER_MS);
        create_delayed_projectile(pos_3, BOSS_ONE_THIRD_DELAYED_PROJECTILE_TIMER_MS);
        firstBoss.num_of_projectiles_created = 3;
    }
}

void create_delayed_projectile(vec2 pos, float timer_ms) {
    Entity entity = create_projectile(pos, vec2(BOSS_ONE_PROJECTILE_WIDTH_PX, BOSS_ONE_PROJECTILE_HEIGHT_PX), vec2(0.f, 0.f));

    // add Delayed component
    Delayed& delayed = registry.delayeds.emplace(entity);
    delayed.timer_ms = timer_ms;
}

void choose_regular_projectile_attack_test(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_player_to_boss_left) {
    boss.boss_state = BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE;

    // stop the boss from moving
    boss_motion.velocity.x = 0.f;

    // update scaling
    boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;

    // update the animate request
    AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
    animateRequest.used_animation = is_player_to_boss_left ? ANIMATION_ID::BOSS_ONE_PROJECTILE_LEFT : ANIMATION_ID::BOSS_ONE_PROJECTILE_RIGHT;
}

void choose_fast_projectile_attack_test(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_player_to_boss_left) {
    boss.boss_state = BOSS_STATE::BOSS1_FAST_PROJECTILE_ATTACK_STATE;

    // stop the boss from moving
    boss_motion.velocity.x = 0.f;

    // update the scaling
    boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;

    // update the animate request
    AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
    animateRequest.used_animation = is_player_to_boss_left ? ANIMATION_ID::BOSS_ONE_PROJECTILE_LEFT : ANIMATION_ID::BOSS_ONE_PROJECTILE_RIGHT;
}

void choose_delayed_projectile_attack_test(Entity& boss_entity, Boss& boss, Motion& boss_motion) {
    boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;
    boss.timer_ms = 2000.f;

    // stop the boss from moving
    boss_motion.velocity.x = 0.f;

    // update the scaling
    boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;

    // update the animate request
    AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
    animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_DELAYED_PROJECTILE;

    FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);
    firstBoss.num_of_projectiles_created = 0;

}

void choose_dash_attack_test(Entity& boss_entity, Boss& boss, Motion& boss_motion) {
    boss.boss_state = BOSS_STATE::BOSS1_DASH_ATTACK_STATE;
    boss_motion.velocity.x = std::copysignf(BOSS_ONE_DASH_VELOCITY, boss_motion.velocity.x);
    boss.timer_ms = BOSS_ONE_DASH_DURATION_MS;

    TimeControllable& tc = registry.timeControllables.get(boss_entity);
    tc.can_be_decelerated = true;
    tc.can_become_harmless = true;

    registry.harmfuls.emplace(boss_entity);

    // update the animate request
    AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
    animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_DASH;

}

void choose_ground_slam_test(Entity& boss_entity, Boss& boss, Motion& boss_motion) {
    boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_INIT_1_STATE;
    boss_motion.velocity.x = 0;
    boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION - 22.f;
    boss.timer_ms = BOSS_ONE_GROUND_SLAM_INIT_DURATION_MS;

    // update scale
    boss_motion.scale = vec2(BOSS_ONE_GROUND_SLAM_BB_WIDTH_PX, BOSS_ONE_GROUND_SLAM_BB_HEIGHT_PX);

    // update animate request
    AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
    animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_INIT;
}

BOSS_ATTACK_ID get_next_attack(Boss& boss, std::default_random_engine& rng) {
    std::cout << "Calling get_next_attack()" << std::endl;
    if (boss.nextAttacks.empty()) {
        std::cout << "nextAttacks is empty..." << std::endl;
        refill_nextAttacks(boss, rng);
    }
    BOSS_ATTACK_ID id = boss.nextAttacks.back();
    boss.nextAttacks.pop_back();
    return id;
}

void refill_nextAttacks(Boss& boss, std::default_random_engine& rng) {
    boss.nextAttacks.clear();
    for (int i = 0; i < BOSS_ONE_NEXT_ATTACKS_VECTOR_MAX_SIZE; i++) {
        boss.nextAttacks.push_back(static_cast<BOSS_ATTACK_ID>(i % (int) BOSS_ATTACK_ID::TOTAL_COUNT));
    }
    std::shuffle(boss.nextAttacks.begin(), boss.nextAttacks.end(), rng);
}

bool is_phase_two_attack(BOSS_ATTACK_ID attack_id) {
    return attack_id == BOSS_ATTACK_ID::BOSS1_DASH_ATTACK || attack_id == BOSS_ATTACK_ID::BOSS1_GROUND_SLAM;
}

void transition_to_attack_state(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_player_to_boss_left, BOSS_ATTACK_ID id) {
    
    if (id == BOSS_ATTACK_ID::BOSS1_DASH_ATTACK) {

        boss.boss_state = BOSS_STATE::BOSS1_DASH_ATTACK_STATE;
        boss_motion.velocity.x = std::copysignf(BOSS_ONE_DASH_VELOCITY, boss_motion.velocity.x);
        boss.timer_ms = BOSS_ONE_DASH_DURATION_MS;

        // boss becomes harmful during dash attack
        boss.can_damage_player = true; // TODO: remove this and use Harmful component instead

        // boss becomes time controllable
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = true;
        tc.can_become_harmless = true;

        // the boss needs to have harmful component and is time controllable?
        if (!registry.harmfuls.has(boss_entity)) {
            registry.harmfuls.emplace(boss_entity);
        }

        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_DASH;

    } else if (id == BOSS_ATTACK_ID::BOSS1_REGULAR_PROJECTILE) {

        boss.boss_state = BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE;

        // stop the boss from moving
        boss_motion.velocity.x = 0.f;

        // update scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = is_player_to_boss_left ? ANIMATION_ID::BOSS_ONE_PROJECTILE_LEFT : ANIMATION_ID::BOSS_ONE_PROJECTILE_RIGHT;

    } else if (id == BOSS_ATTACK_ID::BOSS1_FAST_PROJECTILE) {

        boss.boss_state = BOSS_STATE::BOSS1_FAST_PROJECTILE_ATTACK_STATE;

        // stop the boss from moving
        boss_motion.velocity.x = 0.f;

        // update the scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;        

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = is_player_to_boss_left ? ANIMATION_ID::BOSS_ONE_PROJECTILE_LEFT : ANIMATION_ID::BOSS_ONE_PROJECTILE_RIGHT;

    } else if (id == BOSS_ATTACK_ID::BOSS1_DELAYED_PROJECTILE) {

        boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;
        boss.timer_ms = 2000.f;

        FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);
        firstBoss.num_of_projectiles_created = 0;

        // stop the boss from moving
        boss_motion.velocity.x = 0.f;

        // update scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;        

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_DELAYED_PROJECTILE;

    } else if (id == BOSS_ATTACK_ID::BOSS1_GROUND_SLAM) {

        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_INIT_1_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION - 22.f;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_INIT_DURATION_MS;

        // update the scaling
        boss_motion.scale = vec2(BOSS_ONE_GROUND_SLAM_BB_WIDTH_PX, BOSS_ONE_GROUND_SLAM_BB_HEIGHT_PX);

        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_INIT;

    }

}



// =================== UNUSED HELPER FUNCTIONS ===================

bool is_short_ranged_attack(BOSS_ATTACK_ID attack_id) {
    return attack_id == BOSS_ATTACK_ID::BOSS1_GROUND_SLAM || attack_id == BOSS_ATTACK_ID::BOSS1_DELAYED_PROJECTILE;
}

bool is_medium_ranged_attack(BOSS_ATTACK_ID attack_id) {
    return attack_id == BOSS_ATTACK_ID::BOSS1_GROUND_SLAM || attack_id == BOSS_ATTACK_ID::BOSS1_DELAYED_PROJECTILE ||
        attack_id == BOSS_ATTACK_ID::BOSS1_REGULAR_PROJECTILE;
}

bool is_long_ranged_attack(BOSS_ATTACK_ID attack_id) {
    return attack_id == BOSS_ATTACK_ID::BOSS1_DASH_ATTACK || attack_id == BOSS_ATTACK_ID::BOSS1_FAST_PROJECTILE ||
        attack_id == BOSS_ATTACK_ID::BOSS1_DELAYED_PROJECTILE || attack_id == BOSS_ATTACK_ID::BOSS1_REGULAR_PROJECTILE;
}

// Chooses a long ranged attack and transitions to the next state
void chooseLongRangedAttack(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_in_phase_two, bool is_player_to_boss_left, unsigned int random_num) {

    // options: dash attack, fast projectile, delayed projectile, regular projectile
    // if random_num is <= 25, use dash
    // else if random_num is <= 50, use fast projectile
    // else if random_num is <= 75, use delayed projectile
    // otherwise, use regular projectile

    if (is_in_phase_two && random_num <= 25) {
        boss.boss_state = BOSS_STATE::BOSS1_DASH_ATTACK_STATE;
        boss_motion.velocity.x = std::copysignf(BOSS_ONE_DASH_VELOCITY, boss_motion.velocity.x);
        boss.timer_ms = BOSS_ONE_DASH_DURATION_MS;

        // boss becomes harmful during dash attack
        boss.can_damage_player = true; // TODO: remove this and use Harmful component instead

        // boss becomes time controllable
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = true;
        tc.can_become_harmless = true;

        // the boss needs to have harmful component and is time controllable?
        if (!registry.harmfuls.has(boss_entity)) {
            registry.harmfuls.emplace(boss_entity);
        }

        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_DASH;

    } else if (random_num <= 50) {
        boss.boss_state = BOSS_STATE::BOSS1_FAST_PROJECTILE_ATTACK_STATE;

        // stop the boss from moving
        boss_motion.velocity.x = 0.f;

        // update the scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;        

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = is_player_to_boss_left ? ANIMATION_ID::BOSS_ONE_PROJECTILE_LEFT : ANIMATION_ID::BOSS_ONE_PROJECTILE_RIGHT;

    } else if (random_num <= 75) {
        boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;
        boss.timer_ms = 2000.f;

        FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);
        firstBoss.num_of_projectiles_created = 0;

        // stop the boss from moving
        boss_motion.velocity.x = 0.f;

        // update scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;        

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_DELAYED_PROJECTILE;

    } else {
        boss.boss_state = BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE;

        // stop the boss from moving
        boss_motion.velocity.x = 0.f;

        // update scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = is_player_to_boss_left ? ANIMATION_ID::BOSS_ONE_PROJECTILE_LEFT : ANIMATION_ID::BOSS_ONE_PROJECTILE_RIGHT;

    }

}

// Chooses a medium ranged attack and transitions to the next state
void chooseMediumRangedAttack(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_in_phase_two, bool is_player_to_boss_left, unsigned int random_num) {
    
    // options: ground slam, delayed projectile, regular projectile
    // if random_num <= 50, use ground slam
    // if random_num <= 80, use delayed projectile
    // otherwise, use regular projectile

    if (is_in_phase_two && random_num <= 50) {
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_INIT_1_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION - 22.f;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_INIT_DURATION_MS;

        // update the scaling
        boss_motion.scale = vec2(BOSS_ONE_GROUND_SLAM_BB_WIDTH_PX, BOSS_ONE_GROUND_SLAM_BB_HEIGHT_PX);

        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_INIT;

    } else if (random_num <= 80) {
        boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;
        boss.timer_ms = 2000.f;

        // stop the boss from moving
        boss_motion.velocity.x = 0.f;

        // update scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;        

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_DELAYED_PROJECTILE;

    } else {
        boss.boss_state = BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE;

        // stop the boss from moving
        boss_motion.velocity.x = 0.f;

        // update scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;        

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = is_player_to_boss_left ? ANIMATION_ID::BOSS_ONE_PROJECTILE_LEFT : ANIMATION_ID::BOSS_ONE_PROJECTILE_RIGHT;

    }
}

// Chooses a short ranged attack and transitions to the next state
void chooseShortRangedAttack(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_in_phase_two, bool is_player_to_boss_left, unsigned int random_num) {

    // options: ground slam, delayed projectile
    // if random_num <= 60, use ground slam
    // otherwise, use delayed projectile

    if (is_in_phase_two && random_num <= 60) {
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_INIT_1_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION - 22.f;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_INIT_DURATION_MS;

        // update the scaling
        boss_motion.scale = vec2(BOSS_ONE_GROUND_SLAM_BB_WIDTH_PX, BOSS_ONE_GROUND_SLAM_BB_HEIGHT_PX);

        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_INIT;

    } else {
        boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;
        boss.timer_ms = 2000.f;

        // stop the boss from moving
        boss_motion.velocity.x = 0.f;

        // update the scaling
        boss_motion.scale.x = BOSS_ONE_BB_WIDTH_PX + 20.f;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_DELAYED_PROJECTILE;

    }
}