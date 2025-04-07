//
// Created by d on 2025-03-24.
//

#include "obstacle_spawner.h"

#include <iostream>

// TODO: this is prob a temporary system! just need smthing for M3. this is probably similar to Yixian's screw platforms but the sun is rising and I have not slept

void create_obstacle(ObstacleSpawner& spawner) {
    if (spawner.obstacle_type == "GEAR") {
        Entity gear = create_gear(spawner.start_position, spawner.size, false, 0.0f, 0.0f);

        spawner.obstacle_id = gear.id();

        PhysicsObject& phys = registry.physicsObjects.get(gear);
        phys.angular_damping = 0.00f;
        phys.angular_velocity = (spawner.velocity.x) / 100.0f;
        phys.apply_air_resistance = false;
        phys.apply_rotation = true;
        phys.apply_gravity = true;
        phys.mass = 100.0f;
        phys.bounce = 0.2f;
        phys.friction = 0.0f;

        Motion& motion = registry.motions.get(gear);
        motion.velocity = spawner.velocity;
    } else if (spawner.obstacle_type == "SPIKEBALL") {
        Entity spikeball = create_spikeball(spawner.start_position, spawner.size);
        spawner.obstacle_id = spikeball.id();

        PhysicsObject& phys = registry.physicsObjects.get(spikeball);
        phys.angular_damping = 0.0f;
        phys.friction = 0.00f;
        phys.angular_velocity = (spawner.velocity.x) / 100.0f;
        phys.apply_air_resistance = false;
        phys.apply_rotation = true;
        phys.apply_gravity = true;
        phys.mass = 100.0f;
        phys.bounce = 0.4f;

        Motion& motion = registry.motions.get(spikeball);
        motion.velocity = spawner.velocity;

        spikeball_effects(spawner.start_position);
    }
}


void obstacle_spawner_step(float elapsed_ms) {
    for (uint i = 0; i < registry.obstacleSpawners.size(); i++) {
          ObstacleSpawner& spawner = registry.obstacleSpawners.components[i];

        if (spawner.obstacle_id != 0) {
            spawner.time_left_ms -= elapsed_ms;
        }


        if (spawner.time_left_ms <= 0.0f) {
            if (spawner.obstacle_id != 0) {
                Entity obstacle = Entity(spawner.obstacle_id);
                const vec2 obstacle_last_pos = registry.motions.get(obstacle).position;
                registry.remove_all_components_of(obstacle);
                spawner.obstacle_id = 0;

                if (spawner.obstacle_type == "SPIKEBALL") {
                    spikeball_effects(obstacle_last_pos);
                }
            }
            // Reset the timer for the next obstacle.
            spawner.time_left_ms = spawner.lifetime_ms;
            continue;
        }

        if (spawner.obstacle_id == 0) {
            create_obstacle(spawner);
        } else {
            Entity obstacle = Entity(spawner.obstacle_id);
            Motion& motion = registry.motions.get(obstacle);

            vec2 toEnd = spawner.end_position - motion.position;
            if (dot(toEnd, spawner.velocity) < 0.0f) {
                const vec2 obstacle_last_pos = registry.motions.get(obstacle).position;
                if (spawner.obstacle_type == "SPIKEBALL") {
                    spikeball_effects(obstacle_last_pos);
                }

                registry.remove_all_components_of(obstacle);
                spawner.obstacle_id = 0;
                spawner.time_left_ms = spawner.lifetime_ms;
            } else {
                // last min hacky M3 fix lol TODO change
                motion.velocity.x = spawner.velocity.x;
            }
        }

        // Spawning indicators
        if (spawner.lifetime_ms - spawner.time_left_ms > OBSTACLE_SPAWNER_REST_TIME_MS) {
            float factor = std::clamp(spawner.time_left_ms / spawner.lifetime_ms, 0.0f, 1.0f);

            if (spawner.obstacle_type == "SPIKEBALL") {
                vec2 disp = rand_direction();
                vec2 par_pos = spawner.start_position + disp * spawner.size.x * 0.75f;

                ParticleSystem::spawn_particle(vec3(0.95f, 0.0f, 0.0f),
                    par_pos, 0.0, 10.0f * vec2(1.0f - factor + 0.2f), -disp * 10.0f, 400.0f, 1.0f, { 0.0f, 0.0f }, { 25.0f, 150.0f });
            }
        }
    }
}

void spikeball_effects(vec2 pos) {
    for (int i = 0; i < 30; i++) {
        vec2 vel = random_sample_ellipse(vec2(0.0f), vec2(1.0f)) * 200.0f;

        ParticleSystem::spawn_particle(rand_float() > 0.5 ? vec3(0.01f) : vec3(0.95f),
            pos, 0.0, vec2(rand_float(2.8f, 3.5f)), vel, 500.0f, 1.0f, { 0.0f, 0.0f }, { 100.0f, 200.0f });
    }
}