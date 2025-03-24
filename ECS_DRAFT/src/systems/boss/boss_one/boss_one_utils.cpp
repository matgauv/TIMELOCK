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

    return entity;
}

Entity create_snooze_button(vec2 boss_position) {
    auto entity = Entity();

    registry.snoozeButtons.emplace(entity);
    Motion& motion = registry.motions.emplace(entity);
    motion.velocity = vec2(0.f, 0.f);
    
    // snooze button position should be based on the clock's position
    motion.position = boss_position - vec2(0.f, BOSS_ONE_BB_HEIGHT_PX / 2 - 6.f); // TODO: fine tune this
    
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

    return entity;
}

// Handles the state transition logic by checking the current boss state and then calling the corresponding helper
void boss_one_step(Entity& boss_entity, float elapsed_ms, unsigned int random_num) {
    Boss& boss = registry.bosses.get(boss_entity);
    Motion& boss_motion = registry.motions.get(boss_entity);

    bool is_in_phase_two = (boss.health / BOSS_ONE_MAX_HEALTH) <= 0.8;

    boss.time_until_exhausted_ms -= elapsed_ms;

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
            boss_one_choose_attack_step(boss_entity, boss, boss_motion, elapsed_ms, random_num);
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

}

// Hanldes the logic to transition into the MOVE state
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
        AnimateRequest& animateRequest = registry.animateRequests.emplace(boss_entity);
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
        // choose_dash_attack_test(boss_entity, boss, boss_motion);
        // choose_regular_projectile_attack_test(boss_entity, boss, boss_motion, is_player_to_boss_left);
        // choose_delayed_projectile_attack_test(boss_entity, boss, boss_motion);
        // choose_fast_projectile_attack_test(boss_entity, boss, boss_motion, is_player_to_boss_left);
        // choose_ground_slam_test(boss_entity, boss, boss_motion);
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
}

// Handles the transition logic to MOVE state
void boss_one_recover_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Decrement the timer
    boss.timer_ms -= elapsed_ms;

    // Remove the snooze button
    if (registry.snoozeButtons.components.size() == 1) {
        Entity& button_entity = registry.snoozeButtons.entities[0];
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
}

// Handles the transition logic to RECOVER or DEAD state
void boss_one_damaged_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Decrement timer
    boss.timer_ms -= elapsed_ms;

    // Remove the snooze button
    if (registry.snoozeButtons.components.size() == 1) {
        Entity& button_entity = registry.snoozeButtons.entities[0];
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
}

// Handles the logic of ending the boss fight, cleaning up the entities and components if needed
void boss_one_dead_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        // Update the flag in game state
        assert(registry.gameStates.components.size() <= 1);
        GameState& gameState = registry.gameStates.components[0];

        gameState.is_in_boss_fight = false;

        // Remove the boss and its components
        registry.remove_all_components_of(boss_entity);
    }

    // TODO: Open the path to the exit
}

// Handles the logic of choosing which attack to use and then transition to the corresponding attack state
// Transition to one of: REGULAR PROJECTILE, FAST PROJECTILE, DELAYED PROJECTILE, DASH or GROUND SLAM RISE 1 states
void boss_one_choose_attack_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms, unsigned int random_num) {

    chooseAttack(boss_entity, boss, boss_motion, elapsed_ms, random_num);

}

// Creates 4 projectiles and transitions to MOVE state
void boss_one_regular_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    assert(registry.firstBosses.components.size() <= 1);
    FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);

    if (firstBoss.num_of_projectiles_created == BOSS_ONE_MAX_NUM_OF_NON_DELAYED_PROJECTILE && boss.boss_state != BOSS_STATE::BOSS1_MOVE_STATE) {

        boss.num_of_attack_completed++;
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
        boss.num_of_attack_completed++;
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
        boss.num_of_attack_completed++;
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);
        firstBoss.num_of_projectiles_created = 0;
        boss.timer_ms = BOSS_ONE_MAX_WALK_DURATION_MS * 5.f;
    
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
        boss.num_of_attack_completed++;

        // boss becomes harmless
        // boss.can_damage_player = false;
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
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_LAND;

    } else {
        Entity& player_entity = registry.players.entities[0];
        Motion& player_motion = registry.motions.get(player_entity);
    
        boss_motion.velocity.x = 10.f * calculate_boss_one_x_velocity(boss_motion.position.x, player_motion.position.x);
    }
}

// Updates boss motion, quickly increases boss y coordinate (boss should move downward)
// and transitions to GROUND SLAM LAND 1 state
void boss_one_ground_slam_slam_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    if (boss_motion.position.y >= BOSS_ONE_ON_GROUND_Y_POSITION) {
        boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION;
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
    float left_bound = boss_motion.position.x - BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    float right_bound = boss_motion.position.x + BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    if (left_bound <= player_motion.position.x && player_motion.position.x <= right_bound) {
        PlayerSystem::kill();
    }

    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_2_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;

        // boss becomes harmless
        registry.harmfuls.remove(boss_entity);
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = false;
        tc.can_become_harmless = false;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_RISE;
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

    if (boss_motion.position.y >= BOSS_ONE_ON_GROUND_Y_POSITION) {
        boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION;
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
    float left_bound = boss_motion.position.x - BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    float right_bound = boss_motion.position.x + BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    if (left_bound <= player_motion.position.x && player_motion.position.x <= right_bound) {
        PlayerSystem::kill();
    }

    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_3_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;

        registry.harmfuls.remove(boss_entity);
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = false;
        tc.can_become_harmless = false;

        // update the animate request
        AnimateRequest& animateRequest = registry.animateRequests.get(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_RISE;
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

    if (boss_motion.position.y >= BOSS_ONE_ON_GROUND_Y_POSITION) {
        boss_motion.position.y = BOSS_ONE_ON_GROUND_Y_POSITION;
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
    float left_bound = boss_motion.position.x - BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    float right_bound = boss_motion.position.x + BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX;
    if (left_bound <= player_motion.position.x && player_motion.position.x <= right_bound) {
        PlayerSystem::kill();
    }

    if (boss.timer_ms <= 0.f) {
        boss.num_of_attack_completed++;
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        boss_motion.velocity.y = 0;

        // update scaling
        boss_motion.scale = vec2(BOSS_ONE_BB_WIDTH_PX, BOSS_ONE_BB_HEIGHT_PX);

        registry.harmfuls.remove(boss_entity);

        // boss becomes harmless
        boss.can_damage_player = false;
        if (registry.harmfuls.has(boss_entity)) {
            registry.harmfuls.remove(boss_entity);
        }
        TimeControllable& tc = registry.timeControllables.get(boss_entity);
        tc.can_be_decelerated = false;
        tc.can_become_harmless = false;

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
void chooseAttack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elpased_ms, unsigned int random_num) {
    // helper function that chooses the attack state for boss one using a decision tree

    // random_num is between 0 and 100
    // std::cout << "random_num is: " << random_num << std::endl;

    // grab player related info
    Player& player = registry.players.components[0];
    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    float dist = abs(boss_motion.position.x - player_motion.position.x) / 400.f;
    bool is_in_phase_two = (boss.health / BOSS_ONE_MAX_HEALTH) <= 0.6f;

    bool is_player_to_boss_left = boss_motion.position.x >= player_motion.position.x;
    // std::cout << "dist is: " << dist << ", " << abs(boss_motion.position.x - player_motion.position.x) << std::endl;

    if (dist < 0.25f) {
        // std::cout << "choosing short ranged attack" << std::endl;
        chooseShortRangedAttack(boss_entity, boss, boss_motion, is_in_phase_two, is_player_to_boss_left, random_num);
    } else if (dist > 0.55f) {
        // std::cout << "choosing long ranged attack" << std::endl;
        chooseLongRangedAttack(boss_entity, boss, boss_motion, is_in_phase_two, is_player_to_boss_left, random_num);
    } else {
        // std::cout << "choosing medium ranged attack" << std::endl;
       chooseMediumRangedAttack(boss_entity, boss, boss_motion, is_in_phase_two, is_player_to_boss_left, random_num);
    }
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
        AnimateRequest& animateRequest = registry.animateRequests.emplace(boss_entity);
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
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_1_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;

        // update the scaling
        boss_motion.scale = vec2(200.f, 200.f);

        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.emplace(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::ANIMATION_COUNT;

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
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_1_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;

        // update the scaling
        boss_motion.scale = vec2(200.f, 200.f);

        // update animate request
        AnimateRequest& animateRequest = registry.animateRequests.emplace(boss_entity);
        animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_RISE;

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

void boss_one_regular_projectile_attack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    assert(registry.firstBosses.components.size() <= 1);
    FirstBoss& firstBoss = registry.firstBosses.get(boss_entity);

    firstBoss.projectile_timer_ms -= elapsed_ms;

    if (firstBoss.num_of_projectiles_created < BOSS_ONE_MAX_NUM_OF_NON_DELAYED_PROJECTILE &&
        firstBoss.projectile_timer_ms <= 0.f) {

            Entity& player_entity = registry.players.entities[0];
            Motion& player_motion = registry.motions.get(player_entity);

            int direction = (player_motion.position.x <= boss_motion.position.x) ? -1 : 1;

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
    boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_1_STATE;
    boss_motion.velocity.x = 0;
    boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;

    // update animate request
    AnimateRequest& animateRequest = registry.animateRequests.emplace(boss_entity);
    animateRequest.used_animation = ANIMATION_ID::BOSS_ONE_GROUND_SLAM_RISE;
}