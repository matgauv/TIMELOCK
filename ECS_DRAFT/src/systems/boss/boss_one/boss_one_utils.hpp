#pragma once

#include "../../../common.hpp"
#include "../../../tinyECS/component_container.hpp"
#include "../../../tinyECS/components.hpp"
#include "../../../tinyECS/registry.hpp"

// Creates the first boss entity and its relevant components
Entity create_first_boss();

// Creates the snooze button for the boss in EXHAUSTED state
Entity create_snooze_button(vec2 boss_position);

// the main step function
void boss_one_step(Entity& boss_entity, float elapsed_ms, unsigned int random_num);

void boss_one_idle_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_move_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_exhausted_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_recover_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_damaged_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_dead_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_choose_attack_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms, unsigned int random_num);

void boss_one_regular_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_regular_projectile_attack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_fast_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_fast_projectile_attack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_delayed_projectile_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_delayed_projectile_attack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_dash_step(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

void boss_one_dash_attack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elapsed_ms);

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

float calculate_boss_one_x_velocity(float boss_x, float player_x);

void chooseAttack(Entity& boss_entity, Boss& boss, Motion& boss_motion, float elpased_ms, unsigned int random_num);

void chooseLongRangedAttack(Entity& boss_entity, Boss& boss, Motion& moss_motion, bool is_in_phase_two, bool is_player_to_boss_left, unsigned int random_num);

void chooseMediumRangedAttack(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_in_phase_two, bool is_player_to_boss_left, unsigned int random_num);

void chooseShortRangedAttack(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_in_phase_two, bool is_player_to_boss_left, unsigned int random_num);

void create_delayed_projectile(vec2 pos, float timer_ms);

// helper functions for testing purposes
void choose_regular_projectile_attack_test(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_player_to_boss_left);

void choose_fast_projectile_attack_test(Entity& boss_entity, Boss& boss, Motion& boss_motion, bool is_player_to_boss_left);

void choose_delayed_projectile_attack_test(Entity& boss_entity, Boss& boss, Motion& boss_motion);

void choose_dash_attack_test(Entity& boss_entity, Boss& boss, Motion& boss_motion);

void choose_ground_slam_test(Entity& boss_entity, Boss& boss, Motion& boss_motion);