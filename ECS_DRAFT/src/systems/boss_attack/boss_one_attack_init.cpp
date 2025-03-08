#include "boss_one_attack_init.hpp"
#include "../world/world_init.hpp"
#include <iostream>

// Creates 4 projectiles in total. A projectile is created every 0.5 seconds.
// The projectiles should have a normal speed such that the player does not need to use decelerate to avoid this.
// Once all 4 projectiles have been created, place the attack on its cooldown (fine tune later).
void useBossOneRegularProjectile(Entity& boss_entity, Entity& player_entity, float elapsed_time) {

    Boss& boss = registry.bosses.get(boss_entity);
    Motion& boss_motion = registry.motions.get(boss_entity);
    Motion& player_motion = registry.motions.get(player_entity);

    assert(registry.bossAttackLists.components.size() <= 1);
    auto& table = registry.bossAttackLists.components[0].boss_attack_table;

    Entity*& attack_entity_ptr = getBossAttackEntity(boss);
    BossAttack& attack_info = registry.bossAttacks.get(*attack_entity_ptr);

    // Set the flag and start time
    if (!attack_info.is_in_use) {
        attack_info.is_in_use = true;
        attack_info.attack_start_time = std::chrono::high_resolution_clock::now();

    } else {

        if (attack_info.num_of_attacks < attack_info.max_num_of_attacks) {
            // still need to attack

            attack_info.in_between_timer_ms -= elapsed_time;

            if (attack_info.in_between_timer_ms <= 0.f) {

                int is_boss_to_player_left = boss_motion.position.x - player_motion.position.x > 0 ? -1 : 1;

                vec2 pos = vec2(boss_motion.position.x + is_boss_to_player_left * (BOSS_ONE_BB_WIDTH_PX + PROJECTILE_WIDTH_PX), player_motion.position.y);
                vec2 size = vec2(PROJECTILE_WIDTH_PX, PROJECTILE_HEIGHT_PX);
                vec2 velocity = vec2(PROJECTILE_SPEED * attack_info.velocity_modifier.x * is_boss_to_player_left, 0.f);

                create_projectile(pos, size, velocity);

                attack_info.num_of_attacks++;

                attack_info.in_between_timer_ms = attack_info.in_between_delay_ms[attack_info.num_of_attacks]; // reset the timer
            }

        } else {
            // attack is done

            attack_info.is_in_use = false;
            attack_info.cooldown_ms = BOSS_ONE_REGULAR_PROJECTILE_COOLDOWN_MS;
        }
    }
}

// Identical to the regular projectile attack but the projectile velocity is faster.
// The projectiles should have a faster speed such that the player should use decelerate to avoid this
// Once all 4 projectiles have been created, place the attack on its cooldown (fine tune later)
void useBossOneFastProjectile(Entity& boss_entity, Entity& player_entity, float elapsed_time) {
    
    Boss& boss = registry.bosses.get(boss_entity);
    Motion& boss_motion = registry.motions.get(boss_entity);
    Motion& player_motion = registry.motions.get(player_entity);

    assert(registry.bossAttackLists.components.size() <= 1);
    auto& table = registry.bossAttackLists.components[0].boss_attack_table;

    Entity*& attack_entity_ptr = getBossAttackEntity(boss);
    BossAttack& attack_info = registry.bossAttacks.get(*attack_entity_ptr);

    // Set the flag and start time
    if (!attack_info.is_in_use) {
        attack_info.is_in_use = true;
        attack_info.attack_start_time = std::chrono::high_resolution_clock::now();

    } else {

        if (attack_info.num_of_attacks < attack_info.max_num_of_attacks) {
            // still need to attack

            attack_info.in_between_timer_ms -= elapsed_time;

            if (attack_info.in_between_timer_ms <= 0.f) {

                int is_boss_to_player_left = boss_motion.position.x - player_motion.position.x > 0 ? -1 : 1;

                vec2 pos = vec2(boss_motion.position.x + is_boss_to_player_left * (BOSS_ONE_BB_WIDTH_PX + PROJECTILE_WIDTH_PX), player_motion.position.y);
                vec2 size = vec2(PROJECTILE_WIDTH_PX, PROJECTILE_HEIGHT_PX);
                vec2 velocity = vec2(PROJECTILE_SPEED * attack_info.velocity_modifier.x * is_boss_to_player_left, 0.f);

                create_projectile(pos, size, velocity);

                attack_info.num_of_attacks++;

                attack_info.in_between_timer_ms = attack_info.in_between_delay_ms[attack_info.num_of_attacks]; // reset the timer
            }

        } else {
            // attack is done

            attack_info.is_in_use = false;
            attack_info.cooldown_ms = BOSS_ONE_FAST_PROJECTILE_COOLDOWN_MS;
        }
    }
}

// Creates 3 (fast?) projectiles above the boss in a horizontal line at the same time
// The first projectile travels towards the player after 3 seconds
// The second projectile travels towards the player after another 1 second
// The third projectile travels towards the player after another 0.5 second
// Once all 3 projectiles have been fired, place the attack on its cooldown
void useBossOneDelayedProjectile(Entity& boss_entity, Entity& player_entity, float elapsed_time) {

}

void useBossOneGroundSlam(Entity& boss_entity, Entity& player_entity, float elapsed_time) {
    // TODO:
    // Give an indicator (texture) that the boss is going to jump into the air
    // While in the air, increase the boss's x-velocity and make the boss easily follow the player
    // The boss will do a total of 3 ground slams
    // The first slam occurs after 3 seconds
    // The boss then moves back into the air and follow the player
    // The second slam occurs after another 2 seconds
    // The boss then moves back into the air and follow the player
    // The third slam occurs after another 5 seconds
    // Once the last slam is done, place the attack on its cooldown
    // 0.5 seconds right before each attack, give an indicator that the attack is about to occur (a different texture for the boss)
    // (will fine tune whether the ground slam should be affected by time control)
}

// may not use this attack... ?
void useBossOneDashAttack(Entity& boss_entity, Entity& player_entity, float elapsed_time) {
    // TODO:
    // Give an indicator that the boss is going to start dashing
    // After a 1 second delay, start the dash
    // The initial direction is determined by the player direction relative to the boss
    // During the dash, the boss has increased x-velocity
    // When the boss reaches the end of the areana, 
}

Entity*& getBossAttackEntity(Boss& boss) {
    assert(registry.bossAttackLists.components.size() <= 1);
    auto& table = registry.bossAttackLists.components[0].boss_attack_table;

    assert(table.size() == (int) BOSS_ATTACK_ID::TOTAL_COUNT);

    return table.at(getBossAttackId(boss));
}

int getBossAttackId(Boss& boss) {
    if (boss.boss_state == BOSS_STATE::BOSS1_REGULAR_PROJECTILE_ATTACK_STATE) return (int) BOSS_ATTACK_ID::BOSS1_REGULAR_PROJECTILE;
    
    if (boss.boss_state == BOSS_STATE::BOSS1_DELAYED_PROJECTILE_ATTACK_STATE) return (int) BOSS_ATTACK_ID::BOSS1_DELAYED_PROJECTILE;

    if (boss.boss_state == BOSS_STATE::BOSS1_FAST_PROJECTILE_ATTACK_STATE) return (int) BOSS_ATTACK_ID::BOSS1_FAST_PROJECTILE;

    if (boss.boss_state == BOSS_STATE::BOSS1_GROUND_SLAM_ATTACK_STATE) return (int) BOSS_ATTACK_ID::BOSS1_GROUND_SLAM;

    if (boss.boss_state == BOSS_STATE::BOSS1_DASH_ATTACK_STATE) return (int) BOSS_ATTACK_ID::BOSS1_DASH_ATTACK;

    return -1;
}