#pragma once

#include "../../../common.hpp"
#include "../../../tinyECS/component_container.hpp"
#include "../../../tinyECS/components.hpp"
#include "../../../tinyECS/registry.hpp"

// the main step function
void boss_one_step(Entity& boss_entity, float elapsed_ms);

void boss_one_idle_step(Entity& boss_entity, Boss& boss, float elapsed_ms);

void boss_one_move_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_exhausted_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_recover_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_damaged_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_dead_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_choose_attack_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_regular_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_fast_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_delayed_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_dash_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_rise_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_follow_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_slam_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_land_1_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_rise_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_follow_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_slam_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_land_2_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_rise_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_follow_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_slam_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_ground_slam_land_3_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);


