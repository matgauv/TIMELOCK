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

        std::cout << spawner.obstacle_id << "FROM create" <<  std::endl;


        PhysicsObject& phys = registry.physicsObjects.get(gear);
        phys.angular_damping = 0.01f;
        phys.angular_velocity = -10.0f;
        phys.apply_air_resistance = false;
        phys.apply_rotation = true;
        phys.apply_gravity = true;
        phys.mass = 100.0f;
        phys.bounce = 0.0f;

        Motion& motion = registry.motions.get(gear);
        motion.velocity = spawner.velocity;
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
                registry.remove_all_components_of(obstacle);
                spawner.obstacle_id = 0;
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
                std::cout << "removing: current pos" << motion.position.x << " " << motion.position.y  << "end pos: " << spawner.end_position.x << " , " << spawner.end_position.y << std::endl;
                registry.remove_all_components_of(obstacle);
                spawner.obstacle_id = 0;
                spawner.time_left_ms = spawner.lifetime_ms;
            } else {
                //motion.velocity = spawner.velocity;
            }
        }

    }
}

