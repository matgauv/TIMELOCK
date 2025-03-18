#include "boss_one_utils.hpp"
// #include <glm/trigonometric.hpp>
#include <iostream>
#include <cmath>

// Handles the state transition logic by checking the current boss state and then calling the corresponding helper
void boss_one_step(Entity& boss_entity, float elapsed_ms, unsigned int random_num) {
    Boss& boss = registry.bosses.get(boss_entity);
    Motion& boss_motion = registry.motions.get(boss_entity);

    bool is_in_phase_two = (boss.health / BOSS_ONE_MAX_HEALTH) <= 0.8;

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
    // TODO: if the camera is going to be on the entire boss areana already, then we can directly
    //       transition to MOVE state

    // Get player motion
    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);
    
    // find the distance between the player and the boss
    float dist = abs(boss_motion.position.x - player_motion.position.x);

    // transition to move if both are in screen
    if (dist <= WINDOW_WIDTH_PX) {
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        boss.timer_ms = BOSS_ATTACK_COOLDOWN_MS; // walk for 5 seconds before attacking
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

    // If the boss has used 5 attacks, enter EXHAUSTED state
    if (boss.num_of_attack_completed == 5) {
        boss.boss_state = BOSS_STATE::BOSS1_EXHAUSTED_STATE;
        boss.can_be_damaged = true;
        boss.timer_ms = BOSS_ONE_MAX_EXHAUSTED_DURATION_MS;
        boss_motion.velocity.x = 0.f;
        return;
    }

    // Otherwise, decrement the cooldown
    boss.timer_ms -= elapsed_ms;

    // If the boss has walked for 5 seconds, then enter CHOOSE ATTACK state
    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_CHOOSE_ATTACK_STATE;
    }
}

// Handles the transition logic to RECOVER and DAMAGED states
void boss_one_exhausted_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    
    // Update the 

    // Decrement timer
    boss.timer_ms -= elapsed_ms;

    // TODO: Check if there is a collision between a specific part of the boss and the player...
    // Should this be done here or let Physics system handle it?
    // If the player can land on the snooze button, then the player should be able to bounce off it
    
    // TODO: need to be able to check if this collision has happened

    // Otherwise, if the timer is up, then the boss enters RECOVER STATE
    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_RECOVER_STATE;
        boss.can_be_damaged = false;
        boss.timer_ms = BOSS_ONE_MAX_RECOVER_DURATION_MS;
    }
}

// Handles the transition logic to MOVE state
void boss_one_recover_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Decrement the timer
    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        Entity& player_entity = registry.players.entities[0];
        Motion& player_motion = registry.motions.get(player_entity);
        boss_motion.velocity.x = calculate_boss_one_x_velocity(boss_motion.position.x, player_motion.position.x);
        boss.timer_ms = BOSS_ONE_MAX_WALK_DURATION_MS;
    }
}

// Handles the transition logic to RECOVER or DEAD state
void boss_one_damaged_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Decrement timer
    boss.timer_ms -= elapsed_ms;

    // Decrement boss health
    boss.health -= PLAYER_ATTACK_DAMAGE;

    // If boss health has reached 0, transition to DEAD state
    if (boss.health <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_DEAD_STATE;
        return;
    }

    // Otherwise, if timer is up, transition to RECOVER state
    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_RECOVER_STATE;
        boss.timer_ms = BOSS_ONE_MAX_RECOVER_DURATION_MS;
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
    boss.num_of_attack_completed++;
    boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
}

// Creates 4 fast projectiles and transitions to MOVE state
void boss_one_fast_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    boss.num_of_attack_completed++;
    boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
}

// Creates 3 delayed projectiles and transitions to MOVE state
void boss_one_delayed_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    boss.num_of_attack_completed++;
    boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
}

// Increases boss x velocity and transitions to MOVE state
void boss_one_dash_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    // Decrement the timer
    boss.timer_ms -= elapsed_ms;

    // Transitions to MOVE state if timer is up
    if (boss.timer_ms <= 0.f) {
        boss.num_of_attack_completed++;

        // boss becomes harmless
        boss.can_damage_player = false;

        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        boss.timer_ms = BOSS_ONE_MAX_WALK_DURATION_MS;
    }
}

// Updates boss motion, quickly decreases boss y coordinate (boss should move upward and stop)
// and transitions to GROUND SLAM FOLLOW 1 state
void boss_one_ground_slam_rise_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    if (boss_motion.position.y == BOSS_ONE_GROUND_SLAM_RISE_FINAL_Y_POSITION) {
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_FOLLOW_1_STATE;
        boss.timer_ms = BOSS_ONE_FIRST_GROUND_SLAM_FOLLOW_DURATION_MS;
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
    } else {
        Entity& player_entity = registry.players.entities[0];
        Motion& player_motion = registry.motions.get(player_entity);
    
        boss_motion.velocity.x = calculate_boss_one_x_velocity(boss_motion.position.x, player_motion.position.x);
    }
}

// Updates boss motion, quickly increases boss y coordinate (boss should move downward)
// and transitions to GROUND SLAM LAND 1 state
void boss_one_ground_slam_slam_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    if (boss_motion.position.y == BOSS_ONE_ON_GROUND_Y_POSITION) {
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_LAND_1_STATE;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_LAND_DURATION_MS;
    }
}

// Check if the player is within a specific range (impact range, define a constant for the width and height)
// and transitions to GROUND SLAM RISE 2 state
void boss_one_ground_slam_land_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {
    
    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_2_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;
    }
}

// Updates boss motion, quickly decreases boss y coordinate (boss should move upward and stop)
// and transitions to GROUND SLAM FOLLOW 2 state
void boss_one_ground_slam_rise_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    if (boss_motion.position.y == BOSS_ONE_GROUND_SLAM_RISE_FINAL_Y_POSITION) {
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_FOLLOW_2_STATE;
        boss.timer_ms = BOSS_ONE_SECOND_GROUND_SLAM_FOLLOW_DURATION_MS;
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
    } else {
        Entity& player_entity = registry.players.entities[0];
        Motion& player_motion = registry.motions.get(player_entity);
    
        boss_motion.velocity.x = calculate_boss_one_x_velocity(boss_motion.position.x, player_motion.position.x);
    }
}

// Updates boss motion, quickly increases boss y coordinate (boss should move downward)
// and transitions to GROUND SLAM LAND 2 state
void boss_one_ground_slam_slam_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    if (boss_motion.position.y == BOSS_ONE_ON_GROUND_Y_POSITION) {
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_LAND_2_STATE;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_LAND_DURATION_MS;
    }
}

// Check if the player is within a specific range (impact range, define a constant for the width and height)
// and transitions to GROUND SLAM RISE 3 state
void boss_one_ground_slam_land_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_3_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;
    }
}

// Updates boss motion, quickly decreases boss y coordinate (boss should move upward and stop)
// and transitions to GROUND SLAM FOLLOW 3 state
void boss_one_ground_slam_rise_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    if (boss_motion.position.y == BOSS_ONE_GROUND_SLAM_RISE_FINAL_Y_POSITION) {
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_FOLLOW_3_STATE;
        boss.timer_ms = BOSS_ONE_THIRD_GROUND_SLAM_FOLLOW_DURATION_MS;
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
    } else {
        Entity& player_entity = registry.players.entities[0];
        Motion& player_motion = registry.motions.get(player_entity);
    
        boss_motion.velocity.x = calculate_boss_one_x_velocity(boss_motion.position.x, player_motion.position.x);
    }
}

// Updates boss motion, quickly increases boss y coordinate (boss should move downward)
// and transitions to GROUND SLAM LAND 3 state
void boss_one_ground_slam_slam_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    if (boss_motion.position.y == BOSS_ONE_ON_GROUND_Y_POSITION) {
        boss_motion.velocity.y = 0;
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_LAND_3_STATE;
        boss.timer_ms = BOSS_ONE_GROUND_SLAM_LAND_DURATION_MS;
    }
}

// Check if the player is within a specific range (impact range, define a constant for the width and height)
// and transitions to MOVE state
void boss_one_ground_slam_land_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms) {

    boss.timer_ms -= elapsed_ms;

    if (boss.timer_ms <= 0.f) {
        boss.num_of_attack_completed++;
        boss.boss_state = BOSS_STATE::BOSS1_MOVE_STATE;
        boss_motion.velocity.y = 0;

        // boss becomes harmless
        boss.can_damage_player = false;
    }

}

// Uses a non-linear function to calcualte the x velocity of the boss based on distance from player
float calculate_boss_one_x_velocity(float boss_x, float player_x) {
    // Use sigmoid
    // Calculate the distance
    float dist = boss_x - player_x;

    // Calculate a tentative velocity using the multiplier
    float velocity = dist * BOSS_ONE_X_VELOCITY_MULTIPLIER;

    // Clamp the velocity at the set minimum if velocity is too small
    if (abs(velocity) < BOSS_ONE_MIN_X_VELOCITY) {
        velocity = BOSS_ONE_MIN_X_VELOCITY * std::copysignf(1.f, dist);
    }

    // Clamp the velocity at the set maximum if the velocity is too large in either direction
    if (velocity > BOSS_ONE_MAX_X_VELOCITY) {
        velocity = BOSS_ONE_MAX_X_VELOCITY;
    }

    if (velocity < BOSS_ONE_MAX_X_VELOCITY) {
        velocity = -BOSS_ONE_MAX_X_VELOCITY;
    }

    return velocity;
}

// Chooses the attack based on decision tree (distance between boss and player, and a random number between 0 and 100) and transitions to the corresponding state
void chooseAttack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elpased_ms, unsigned int random_num) {
    // helper function that chooses the attack state for boss one using a decision tree

    // random_num is between 0 and 100

    // grab player related info
    Player& player = registry.players.components[0];
    Entity& player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);

    float dist = abs(boss_motion.position.x - player_motion.position.x) / WINDOW_WIDTH_PX;
    bool is_in_phase_two = (boss.health / BOSS_ONE_MAX_HEALTH) <= 0.6f;

    if (dist < 0.3f) {
        chooseShortRangedAttack(boss, boss_motion, is_in_phase_two, random_num);
    } else if (dist > 0.7f) {
        chooseLongRangedAttack(boss_entity, boss, boss_motion, is_in_phase_two, random_num);
    } else {
       chooseMediumRangedAttack(boss, boss_motion, is_in_phase_two, random_num);
    }
}

// Chooses a long ranged attack and transitions to the next state
void chooseLongRangedAttack(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_in_phase_two, unsigned int random_num) {

    // options: dash attack, fast projectile, delayed projectile, regular projectile
    // if random_num is <= 25, use dash
    // else if random_num is <= 50, use fast projectile
    // else if random_num is <= 75, use delayed projectile
    // otherwise, use regular projectile

    if (is_in_phase_two && random_num <= 25) {
        boss.boss_state = BOSS_STATE::BOSS1_DASH_ATTACK_STATE;
        boss_motion.velocity.x = std::copysignf(boss_motion.velocity.x, BOSS_ONE_DASH_VELOCITY);
        boss.timer_ms = BOSS_ONE_DASH_DURATION_MS;

        // boss becomes harmful during dash attack
        boss.can_damage_player = true;

    } else if (random_num <= 50) {
        boss.boss_state = BOSS_STATE::BOSS1_FAST_PROJECTILE_ATTACK_STATE;
    } else if (random_num <= 75) {
        boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;
    } else {
        boss.boss_state = BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE;
    }

}

// Chooses a medium ranged attack and transitions to the next state
void chooseMediumRangedAttack(Boss& boss, Motion& boss_motion, bool is_in_phase_two, unsigned int random_num) {
    
    // options: ground slam, delayed projectile, regular projectile
    // if random_num <= 50, use ground slam
    // if random_num <= 80, use delayed projectile
    // otherwise, use regular projectile

    if (is_in_phase_two && random_num <= 50) { 
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_1_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;
    } else if (random_num <= 80) {
        boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;
    } else {
        boss.boss_state = BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE;
    }
}

// Chooses a short ranged attack and transitions to the next state
void chooseShortRangedAttack(Boss& boss, Motion& boss_motion, bool is_in_phase_two, unsigned int random_num) {

    // options: ground slam, delayed projectile
    // if random_num <= 60, use ground slam
    // otherwise, use delayed projectile

    if (is_in_phase_two && random_num <= 60) {
        boss.boss_state = BOSS_STATE::BOSS1_GROUND_SLAM_RISE_1_STATE;
        boss_motion.velocity.x = 0;
        boss_motion.velocity.y = BOSS_ONE_GROUND_SLAM_RISE_VELOCITY;
    } else {
        boss.boss_state = BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE;
    }
}