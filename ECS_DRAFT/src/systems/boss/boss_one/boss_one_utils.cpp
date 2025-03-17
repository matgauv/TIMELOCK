#include "boss_one_utils.hpp"
// #include <glm/trigonometric.hpp>
#include <iostream>
#include <cmath>

// Handles the state transition logic by checking the current boss state and then calling the corresponding helper
void boss_one_step(Entity& boss_entity, float elapsed_ms) {
    Boss& boss = registry.bosses.get(boss_entity);
    Motion& boss_motion = registry.motions.get(boss_entity);

    bool is_in_phase_two = (boss.health / BOSS_ONE_MAX_HEALTH) <= 0.5;

    // state transition logic
    switch (boss.boss_state) {
        case BOSS_STATE::BOSS1_IDLE_STATE:
            boss_one_idle_step(boss_entity, boss, elapsed_ms);
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
            boss_one_choose_attack_step(boss_entity, boss, boss_motion, elapsed_ms);
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

