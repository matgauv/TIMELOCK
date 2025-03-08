#include "boss_one_attack_init.hpp"
#include "../world/world_init.hpp"
#include <iostream>

void useBossOneRegularProjectile() {
    // TODO:
    // create 4 projectiles, a projectile is created every 0.5 seconds (therefore the total time should be 2 seconds)
    // the projectiles should have a normal speed such that the player does not need to use decelerate to avoid this
    // once all 4 projectiles have been created, place the attack on its cooldown (fine tune later)
}

void useBossOneFastProjectile() {
    // TODO:
    // call useBossOneRegularProjectile() helper
    // increase the velocity modifier so that it is much faster
    // may want to decrease the in-between-delay down to 0.25 seconds
    // once all 4 projectiles have been created, place the attack on its cooldown (fine tune later)
    // the projectile can be affected by time control
}

void useBossOneDelayedProjectile() {
    // TODO:
    // create 3 projectiles above the boss in a horizontal line
    // the first projectile travels towards the player after 3 seconds
    // the second projectile travels towards the player after another 1 second
    // the third projectile travels towards the player after another 0.5 second
    // once all 3 projectiles have been fired, place the attack on its cooldown
    // the projectiles can be affected by time control

}

void useBossOneGroundSlam() {
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
void useBossOneDashAttack() {
    // TODO:
    // Give an indicator that the boss is going to start dashing
    // After a 1 second delay, start the dash
    // The initial direction is determined by the player direction relative to the boss
    // During the dash, the boss has increased x-velocity
    // When the boss reaches the end of the areana, 
}