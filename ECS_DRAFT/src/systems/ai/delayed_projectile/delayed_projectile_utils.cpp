#include "delayed_projectile_utils.hpp"
#include <glm/trigonometric.hpp>
#include <iostream>
#include <cmath>

void delayed_projectile_step(float elapsed_ms) {
    for (unsigned int i = 0; i < registry.delayeds.components.size(); i++) {
        Entity& entity = registry.delayeds.entities[i];
        Delayed& delayed = registry.delayeds.components[i];

        // decrement the timer
        delayed.timer_ms -= elapsed_ms;

        // if timer is up
        if (delayed.timer_ms <= 0.f && registry.projectiles.has(entity) && registry.motions.has(entity)) {
            // get delayed projectile motion
            Motion& motion = registry.motions.get(entity);

            // get player motion
            Entity& player_entity = registry.players.entities[0];
            Motion& player_motion = registry.motions.get(player_entity);

            // calculate the angle
            vec2 final_velocity = calculate_velocity_vector(motion.position, player_motion.position);

            // set the projectile velocity
            motion.velocity = final_velocity;

            registry.delayeds.remove(entity);
        }
    }
}

vec2 calculate_velocity_vector(vec2 projectile_position, vec2 player_position) {
    // calculate the difference between the two positions
    vec2 dist = player_position - projectile_position;

    // get the direction
    vec2 direction = glm::normalize(dist);

    // calculate the x and y velocity based on direction and the overall projectile velocity
    return BOSS_ONE_DELAYED_PROJECTILE_SPEED * direction;
}