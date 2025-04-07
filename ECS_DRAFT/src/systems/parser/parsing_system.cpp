#include <iostream>
#include "parsing_system.hpp"
#include "../world/world_init.hpp"
#include "tinyECS/registry.hpp"
#include "../boss/boss_one/boss_one_utils.hpp"
#include <fstream>
#include "systems/ai/pipe/pipe_utils.hpp"

void LevelParsingSystem::init(GLFWwindow *window) {
    this->window = window;
}

void LevelParsingSystem::step(float elapsed_ms) {
    LevelState& level_state = registry.levelStates.components[0];

    if (level_state.shouldReparseEntities) {
        init_level_entities(reparsable_entities);
        level_state.shouldReparseEntities = false;
        return;
    }

    if (level_state.reload_coutdown > 0.0f) {
        if (level_state.shouldLoad) {
            // Instant reload
            level_state.reload_coutdown = -1.0f;
        }
        else {
            level_state.reload_coutdown -= elapsed_ms;
            

            // Automatically reload next level
            if (level_state.reload_coutdown < 0.0f) {
                level_state.reload_coutdown = -1.0f;

                if (level_state.curr_level_folder_name != level_state.next_level_folder_name) {
                    level_state.curr_level_folder_name = level_state.next_level_folder_name;
                    level_state.shouldLoad = true;
                };
            }
        }
    }

    if (!level_state.shouldLoad) return;

    // remove all entities with motion (flush the current level)
    while (registry.motions.size() > 0) {
        registry.remove_all_components_of(registry.motions.entities.back());
    }

    // remove all renderRequests (for tiles)
    while (registry.renderRequests.size() > 0) {
        registry.remove_all_components_of(registry.renderRequests.entities.back());
    }

    // Remove all particles
    while (registry.particles.entities.size() > 0)
        registry.remove_all_components_of(registry.particles.entities.back());

    if (!parse_json()) {
        cout << "Error: could not parse JSON" << endl;
        return;
    }

    json next_levels = json_data["neighbourLevels"];
    if (!next_levels.empty()) {
        level_state.next_level_folder_name = next_levels[0];
    } else {
        cout << "Next level name not present in JSON" << endl;
        level_state.next_level_folder_name = level_state.curr_level_folder_name;
    }

    level_state.ground = level_ground_map.at(level_state.curr_level_folder_name);

    tile_id_array = json_data["layers"][0]["data"];
    stride = static_cast<int>(json_data["width"]) / TILE_TO_PIXELS;

    level_state.dimensions = vec2{ json_data["width"], json_data["height"] };
    init_level_background();
    init_level_entities(json_data["entities"]);
    init_player_and_camera();

    if (level_state.ground == TEXTURE_ASSET_ID::BOSS_ONE_LEVEL_GROUND) {
        for (Entity& e : registry.snoozeButtons.entities) {
            registry.remove_all_components_of(e);
        }
        if (registry.bosses.size() == 1) {
            registry.remove_all_components_of(registry.bosses.entities[0]);
        }
        create_first_boss();
        assert(registry.gameStates.components.size() <= 1);
        GameState& gameState = registry.gameStates.components[0];
        gameState.is_in_boss_fight = false;
    }
    else {
        if (registry.bosses.size() > 0) {
            registry.remove_all_components_of(registry.bosses.entities[0]);
        }

        assert(registry.gameStates.components.size() <= 1);
        GameState& gameState = registry.gameStates.components[0];
        gameState.is_in_boss_fight = false;
    }

    level_state.shouldLoad = false;
    level_state.reload_coutdown = -1.0f;

    // "Uninitialized value" to pass the first render step with large time_elapse
    // 3.0 = 1.0 factor + 2 * tolerances
    // registry.screenStates.components[0].scene_transition_factor = 3.0;
    // registry.gameStates.components[0].game_scene_transition_state = SCENE_TRANSITION_STATE::TRANSITION_IN;

    WorldSystem::set_time_control_state(false, false, true);
    WorldSystem::set_time_control_state(true, false, true);
}

void LevelParsingSystem::late_step(float elapsed_ms) {

}

/*
 * INITIALIZATION FUNCTIONS FOR PARSING AND CREATING OBJECTS FROM JSON
 */

void LevelParsingSystem::init_level_background() {
    // TODO: static w, h values -- should change (maybe parse from level file).
    LevelState& levelState = registry.levelStates.components[0];
    float scale_factor = json_data["width"] > json_data["height"] ? ceil(static_cast<float>(json_data["width"]) / BACKGROUND_WIDTH) : ceil(static_cast<float>(json_data["height"]) / BACKGROUND_HEIGHT);
    float background_w = BACKGROUND_WIDTH * scale_factor;
    float background_h = BACKGROUND_HEIGHT * scale_factor;
    create_parallaxbackground({background_w, background_h}, TEXTURE_ASSET_ID::GEARS_BACKGROUND);
    create_background({background_w, background_h}, TEXTURE_ASSET_ID::METAL_BACKGROUND);
    create_levelground({json_data["width"], json_data["height"]}, levelState.ground);

    // world boundaries
    float world_w = json_data["width"];
    float world_h = json_data["height"];
    float shift_differential = 0.5 * TILE_TO_PIXELS;

    // horizontal world boundaries (top/bottom)
    create_world_boundary({world_w / 2.0f - shift_differential, 0.0f - (0.25 * world_h) - shift_differential}, {world_w, 1.0f});
    create_world_boundary({world_w / 2.0f - shift_differential, world_h + (0.25 * world_h) - shift_differential}, {world_w, 1.0f});

    // vertical world boundaries (left/right)
    create_world_boundary({0.0f - shift_differential, world_h / 2.0f - shift_differential}, {1.0f, world_h * 1.5});
    create_world_boundary({world_w - shift_differential, world_h / 2.0f - shift_differential}, {1.0f, world_h * 1.5});
}

void LevelParsingSystem::init_player_and_camera() {
    json playerJson = json_data["entities"]["Player"][0];
    vec2 initPos = vec2(playerJson["x"], playerJson["y"]);
    create_player(initPos, { int(playerJson["width"]) * 1.5, int(playerJson["height"]) * 1.5});
    create_deceleration_bar(initPos + DECEL_BAR_OFFSET);
    create_camera(initPos, { 1.0f, 1.0f });

    // TEMP: for now, tutorial text is always shown
    // hardcode rn but maybe should pass position as text entity in ldtk?
    bool tutorial = true;
    if (tutorial) {
		if (json_data["identifier"] == "Level_0") {
            create_tutorial_text({ initPos.x + 2650, initPos.y }, {6000, 1500 }, TEXTURE_ASSET_ID::TUTORIAL_TEXT);
        }
        if (json_data["identifier"] == "Level_4") {
            create_tutorial_text({ initPos.x + 1750, initPos.y - 140}, { 4000, 600 }, TEXTURE_ASSET_ID::BOSS_TUTORIAL_TEXT);
        }
        else if (json_data["identifier"] == "Level_6") {
            create_tutorial_text({ initPos.x + 250, initPos.y - 150 }, { 450, 70 }, TEXTURE_ASSET_ID::ACCEL);
        }
    }
}

void LevelParsingSystem::init_level_entities(json entities) {
    // TODO: This is really ugly -- can prob clean this up with a map or smt.
    for (auto& [entity_type, entity_list] : entities.items()) {
        if (entity_type == "Platform") {
            init_platforms(entity_list, false);
        } else if (entity_type == "MovingPlatform") {
            init_platforms(entity_list, true);
        } else if (entity_type == "Spike") {
            init_spikes(entity_list);
        } else if (entity_type == "Door") {
            init_doors(entity_list);
        } else if (entity_type == "Projectile") {
            reparsable_entities["Projectile"] = entity_list;
            init_projectiles(entity_list);
        } else if (entity_type == "Pipe") {
            init_pipes(entity_list);
        } else if (entity_type == "PipePart") {
            init_pipeparts(entity_list);
        } else if (entity_type == "Boundary") {
            init_boundaries(entity_list);
        } else if (entity_type == "PartOf") {
            init_partof(entity_list);
        } else if (entity_type == "Cannon") {
            init_cannons(entity_list);
        } else if (entity_type == "Ladder") {
            init_ladders(entity_list);
        } else if (entity_type == "Checkpoint") {
            init_checkpoints(entity_list);
        } else if (entity_type == "Breakable") {
            reparsable_entities["Breakable"] = entity_list;
            init_breakable_platforms(entity_list);
        } else if (entity_type == "Chain") {
            init_chains(entity_list);
        } else if (entity_type == "Pendulum") {
            init_pendulums(entity_list);
        } else if (entity_type == "Gear") {
            init_gears(entity_list);
        } else if (entity_type == "Spikeball") {
            init_spikeballs(entity_list);
        } else if (entity_type == "Obstacle_spawner") {
            init_spawners(entity_list);
        }
    }
}

void LevelParsingSystem::init_chains(json chains) {
    LevelState& ls = registry.levelStates.components[0];
    for (json& chain : chains) {
        vec2 position = {chain["x"], ls.dimensions.y / 2};
        vec2 scale = {TILE_TO_PIXELS * ceil(ls.dimensions.y / 1500), ls.dimensions.y};
        create_chain(position, scale);
    }
}

void LevelParsingSystem::init_breakable_platforms(json breakables) {
    // clear all projectiles (for reparsing)
    while (registry.breakables.entities.size() > 0) {
        registry.remove_all_components_of(registry.breakables.entities.back());
    }

    for (json& breakable : breakables) {
        vec2 size;

        json json_full_size = breakable["customFields"]["size"];
        if (!validate_custom_field(json_full_size, "size", breakable["iid"])) {
            continue;
        }
        int full_size = json_full_size;

        json json_direction = breakable["customFields"]["direction"];
        if (!validate_custom_field(json_direction, "direction", breakable["iid"])) {
            continue;
        }
        string direction = json_direction;

        int conversion_factor;
        bool is_x_axis;
        if (direction == "up" || direction == "down")
        {
            size = {breakable["width"], static_cast<int>(breakable["height"]) * full_size};
            if (direction == "up") {
                conversion_factor = (static_cast<int>(breakable["height"]) / 2) - (static_cast<int>(size.y) / 2);
            } else {
                conversion_factor = (static_cast<int>(size.y) / 2) - (static_cast<int>(breakable["height"]) / 2);
            }
            is_x_axis = false;
        }
        else
        {
            size = {static_cast<int>(breakable["width"]) * full_size, breakable["height"]};
            if (direction == "left") {
                conversion_factor = (static_cast<int>(breakable["width"]) / 2) - (static_cast<int>(size.x) / 2);
            } else {
                conversion_factor = (static_cast<int>(size.x) / 2) - (static_cast<int>(breakable["width"]) / 2);
            }
            is_x_axis = true;
        }

        vec2 start_pos = {static_cast<int>(breakable["x"]) / TILE_TO_PIXELS, static_cast<int>(breakable["y"]) / TILE_TO_PIXELS};
        vec2 position = centralize_position(start_pos, conversion_factor, is_x_axis);

        create_time_controllable_breakable_static_platform(position, size, false, 2.0, tile_id_array, stride);
    }
}

void LevelParsingSystem::init_spawners(json gear_spawners) {
    for (json& gear_spawner : gear_spawners) {
        json customFields = gear_spawner["customFields"];
        vec2 velocity = vec2{customFields["x_velocity"], customFields["y_velocity"]};
        vec2 size = vec2{customFields["width"], customFields["height"]};

        vec2 start_pos = vec2{customFields["start_point"]["cx"], customFields["start_point"]["cy"]};
        start_pos *= TILE_TO_PIXELS;

        vec2 end_pos = vec2{customFields["end_point"]["cx"], customFields["end_point"]["cy"]};
        end_pos *= TILE_TO_PIXELS;

        std::string type = customFields["Type"];

        create_spawner(type, size, velocity, start_pos, end_pos);
    }
}



void LevelParsingSystem::init_pipes(json pipes) {
    for (json& pipe : pipes) {
        vec2 position = {pipe["x"], pipe["y"]};
        vec2 scale = {pipe["width"], pipe["height"]};

        json json_direction = pipe["customFields"]["direction"];
        if (!validate_custom_field(json_direction, "direction", pipe["iid"])) {
            continue;
        }

        string direction = json_direction;
        create_pipe_head(position, scale, direction, tile_id_array, stride);
    }
}

void LevelParsingSystem::init_pipeparts(json parts) {
    for (json& pipe_part : parts) {
        vec2 position = {pipe_part["x"], pipe_part["y"]};
        vec2 size = {pipe_part["width"], pipe_part["height"]};
        create_static_platform(position, size, tile_id_array, stride, false);
    }
}



void LevelParsingSystem::init_doors(json doors) {
    for (json door : doors) {
        json json_open = door["customFields"]["open"];
        if (!validate_custom_field(json_open, "open", door["iid"])) {
            continue;
        }
        bool open = json_open;

        vec2 position;
        extract_door_position(door, position);
        create_door(position, open, this->tile_id_array, this->stride);
    }
}

void LevelParsingSystem::init_checkpoints(json checkpoints) {
    for (json checkpoint : checkpoints) {
        vec2 position = {checkpoint["x"], static_cast<float>(checkpoint["y"]) + PARSING_CHECKPOINT_Y_POS_DIFF};
        create_spawnpoint(position, SPAWNPOINT_SCALE);
    }
}


void LevelParsingSystem::init_cannons(json cannons) {
    for (json cannon : cannons) {
        vec2 position = {cannon["x"], static_cast<float>(cannon["y"]) + PARSING_CANNON_Y_POS_DIFF};
        create_cannon_tower(position);
    }
}

void LevelParsingSystem::init_ladders(json ladders) {
    for (json ladder : ladders) {
        vec2 position = {ladder["x"], ladder["y"]};

        json json_length = ladder["customFields"]["length"];
        if (!validate_custom_field(json_length, "length", ladder["iid"], {"cx", "cy"})) {
            continue;
        }
        int height = abs(static_cast<int>(json_length["cy"]) - (position.y / TILE_TO_PIXELS)) + 1;

        vec2 dimensions = {ladder["width"], ladder["height"]};
        create_ladder(position, dimensions, height, tile_id_array, stride);
    }
}

void LevelParsingSystem::init_pendulums(json pendulum) {
    for (json pendulum : pendulum) {
        vec2 pivot_position = {pendulum["x"], pendulum["y"]};

        json json_end_pos = pendulum["customFields"]["length"];
        if (!validate_custom_field(json_end_pos, "length", pendulum["iid"], {"cx", "cy"})) {
            continue;
        }
        float end_pos_y = json_end_pos["cy"];
        float length = abs(pivot_position.y - (end_pos_y * TILE_TO_PIXELS));

        json json_initial_angle = pendulum["customFields"]["initial_angle"];
        json json_bob_radius = pendulum["customFields"]["bob_radius"];

        if (!validate_custom_field(json_initial_angle, "initial_angle", pendulum["iid"]) ||
            !validate_custom_field(json_bob_radius, "bob_radius", pendulum["iid"])) {
            continue;
        }

        float initial_angle = json_initial_angle;
        float bob_radius = json_bob_radius;

        create_pendulum(pivot_position, length, initial_angle, bob_radius);

    }
}

void LevelParsingSystem::init_spikeballs(json spikeballs) {
    for (json spikeball : spikeballs) {
        json json_width = spikeball["customFields"]["width"];
        json json_height = spikeball["customFields"]["height"];

        if (!validate_custom_field(json_width, "width", spikeball["iid"]) ||
            !validate_custom_field(json_height, "height", spikeball["iid"])) {
            continue;
        }
        vec2 position = {spikeball["x"], spikeball["y"]};
        vec2 size_px = {json_width, json_height};
        create_spikeball(position, size_px);
    }
}


void LevelParsingSystem::init_gears(json gears) {
    for (json gear : gears) {
        vec2 position = {gear["x"], gear["y"]};

        json json_gear_edge = gear["customFields"]["gear_edge"];
        if (!validate_custom_field(json_gear_edge, "gear_edge", gear["iid"], {"cx", "cy"})) {
            continue;
        }
        vec2 gear_edge_pos = {json_gear_edge["cx"], json_gear_edge["cy"]};
        float radius = abs(position.x  - (gear_edge_pos.x * TILE_TO_PIXELS));

        // TODO: error handling
        bool fixed = gear["customFields"]["fixed"];
        float angular_velocity = gear["customFields"]["angular_velocity"];
        float inital_angle = gear["customFields"]["initial_angle"];

        vec2 size_px = {radius * 2, radius * 2};
        create_gear(position, size_px, fixed, angular_velocity, inital_angle);
    }
}

void LevelParsingSystem::init_projectiles(json projectiles) {
    // clear all bolts (for reparsing)
    while (registry.bolts.entities.size() > 0) {
        registry.remove_all_components_of(registry.bolts.entities.back());
    }

    for (json projectile: projectiles) {
        vec2 position = vec2{projectile["x"], projectile["y"]};
        vec2 velocity = {0, 0};
        float scale = projectile["customFields"]["scale"];
        vec2 size = vec2 {scale, scale};
        // TODO: handle other meshtypes? (some are null in json rn so cannot parse)
        // string meshtype = "";
        // if (projectile["customFields"]["meshtype"]) meshtype = projectile["customFields"]["meshtype"];
        create_bolt(position, size, velocity, false, true);
    }
}

void LevelParsingSystem::init_boundaries(json boundaries) {
    for (json boundary : boundaries) {
        vec2 dimensions;
        vec2 position;
        if (!extract_boundary_attributes(boundary, dimensions, position)) {
            continue;
        }
        create_level_boundary(position, dimensions);
    }
}

void LevelParsingSystem::init_spikes(json spikes) {
    for (json spike : spikes) {
        json json_end_pos = spike["customFields"]["length"];
        if (!validate_custom_field(json_end_pos, "length", spike["iid"], {"cx", "cy"})) {
            continue;
        }
        vec2 end_pos = {static_cast<int>(json_end_pos["cx"]) * TILE_TO_PIXELS, static_cast<int>(json_end_pos["cy"]) * TILE_TO_PIXELS};

        json json_direction = spike["customFields"]["direction"];
        if (!validate_custom_field(json_direction, "direction", spike["iid"])) {
            continue;
        }
        string direction = json_direction;
        bool is_x_axis = direction == "left" || direction == "right";
        int pos_stride = TILE_TO_PIXELS * (direction == "left" || direction == "up" ? -1 : 1);

        vec2 dimensions = {spike["width"], spike["height"]};
        vec2 start_pos = {spike["x"], spike["y"]};
        int num_spikes = is_x_axis ? abs(end_pos.x - start_pos.x) / dimensions.x : abs(end_pos.y - start_pos.y) / dimensions.y;

        for (int i = 0; i <= num_spikes; i++) {
            vec2 position;
            if (is_x_axis) {
                position = {start_pos.x + (i * pos_stride), start_pos.y};
            } else {
                position = {start_pos.x, start_pos.y + (i * pos_stride)};
            }
            create_spike(position, dimensions, tile_id_array, stride);
        }
    }
}

void LevelParsingSystem::init_partof(json partof) {
    for (json pf : partof) {
        vec2 dimensions = {pf["width"], pf["height"]};
        vec2 position = {pf["x"], pf["y"]};
        create_partof(position, dimensions, tile_id_array, stride);
    }
}

void LevelParsingSystem::init_platforms(json platforms, bool moving) {
    for (json platform : platforms) {
        vec2 dimensions;
        vec2 startPos;
        bool rounded;
        if (moving) {
            vector<Path> path;
            if (!extract_path_attributes(platform, path, startPos, dimensions, rounded)) {
                continue;
            }
            create_moving_platform(dimensions, path, startPos, tile_id_array, stride, rounded);
        } else {
            if (!extract_platform_attributes(platform, dimensions, startPos, rounded)) {
                continue;
            }
            create_static_platform(startPos, dimensions, tile_id_array, stride, rounded);
        }
    }
}

/*
 * HELPERS FOR EXTRACTING INFORMATION FROM JSON
 */

bool LevelParsingSystem::extract_door_position(json door, vec2& position) {
    int x_start = static_cast<float>(door["x"]) - (0.5 * TILE_TO_PIXELS);
    int y_start = static_cast<float>(door["y"]) + (0.5 * TILE_TO_PIXELS);

    int x = x_start + (0.5 * DOOR_SIZE.x);
    int y = y_start - (0.5 * DOOR_SIZE.y);

    position = {x, y};
    return true;
}

bool LevelParsingSystem::extract_boundary_attributes(json boundary, vec2& dimensions, vec2& position) {
    int x = boundary["x"];
    int y = boundary["y"];
    vec2 start_pos = vec2{x / TILE_TO_PIXELS, y / TILE_TO_PIXELS};

    json end_pos_json = boundary["customFields"]["length"];
    if (!validate_custom_field(end_pos_json, "length", boundary["iid"])) {
        return false;
    }
    vec2 end_pos = {end_pos_json["cx"], end_pos_json["cy"]};

    int size;
    json json_direction = boundary["customFields"]["direction"];
    if (!validate_custom_field(json_direction, "direction", boundary["iid"])) {
        return false;
    }
    string direction = json_direction;

    int conversion_factor;
    bool is_x_axis;
    if (direction == "up" || direction == "down")
    {
        size = abs(end_pos.y - start_pos.y) + 1;
        dimensions = {boundary["width"], static_cast<int>(boundary["height"]) * size};

        if (direction == "up") {
            conversion_factor = (static_cast<int>(boundary["height"]) / 2) - (static_cast<int>(dimensions.y) / 2);
        } else {
            conversion_factor = (static_cast<int>(dimensions.y) / 2) - (static_cast<int>(boundary["height"]) / 2);
        }
        is_x_axis = false;
    }
    else
    {
        size = abs(end_pos.x - start_pos.x) + 1;
        dimensions = {static_cast<int>(boundary["width"]) * size, boundary["height"]};

        if (direction == "left") {
            conversion_factor = (static_cast<int>(boundary["width"]) / 2) - (static_cast<int>(dimensions.x) / 2);
        } else {
            conversion_factor = (static_cast<int>(dimensions.x) / 2) - (static_cast<int>(boundary["width"]) / 2);
        }
        is_x_axis = true;
    }

    position = centralize_position(start_pos, conversion_factor, is_x_axis);
    return true;
}


bool LevelParsingSystem::extract_full_platform_dimensions(json platform, vec2& dimensions) {
    json json_size = platform["customFields"]["size"];
    if (!validate_custom_field(json_size, "size", platform["iid"])) {
        return false;
    }
    int full_size = json_size;
    int full_width = full_size * static_cast<int>(platform["width"]);
    dimensions = {full_width, platform["height"]};

    return true;
}

bool LevelParsingSystem::extract_platform_attributes(json platform, vec2& dimensions, vec2& startPos, bool& rounded) {
    if (!extract_full_platform_dimensions(platform, dimensions)) {
        return false;
    }

    int left_x = static_cast<int>(platform["x"]) - (static_cast<int>(platform["width"]) / 2);
    int start_x = left_x + (static_cast<int>(dimensions[0]) / 2);
    startPos = {start_x, platform["y"]};

    json json_rounded = platform["customFields"]["rounded"];
    if (!validate_custom_field(json_rounded, "rounded", platform["iid"])) {
        return false;
    }
    rounded = json_rounded;

    return true;
}

bool LevelParsingSystem::extract_path_attributes(json platform, vector<Path>& paths, vec2& init_pos_in_path, vec2& dimensions, bool& rounded) {
    extract_full_platform_dimensions(platform, dimensions);

    // currently all position values in json are relative to leftmost tile in platform -- need to centralize position on x-axis.
    int conversion_factor = (static_cast<int>(dimensions.x) / 2) - (static_cast<int>(platform["width"]) / 2);

    json start_pos_json = platform["customFields"]["start"];
    if (!validate_custom_field(start_pos_json, "start", platform["iid"], {"cx", "cy"})) {
        return false;
    }
    vec2 start_pos = convert_and_centralize_position(start_pos_json, conversion_factor);

    json end_pos_json = platform["customFields"]["end"];
    if (!validate_custom_field(end_pos_json, "start", platform["iid"], {"cx", "cy"})) {
        return false;
    }
    vec2 end_pos = convert_and_centralize_position(end_pos_json, conversion_factor);

    int left_x = static_cast<int>(platform["x"]) - (static_cast<int>(platform["width"]) / 2);
    int start_x = left_x + (static_cast<int>(dimensions[0]) / 2);
    init_pos_in_path = {start_x, platform["y"]};

    // set duration to default value of 0.5 if no duration is set.
    json json_duration = platform["customFields"]["duration"];
    if (!validate_custom_field(json_duration, "duration", platform["iid"])) {
        return false;
    }
    float duration = json_duration;
    if (duration == 0.0) {
        duration = 0.5;
    }

    Path forward = Path(start_pos, end_pos, duration);
    Path backward = Path(end_pos, start_pos, duration);
    paths.push_back(forward);
    paths.push_back(backward);

    json json_rounded = platform["customFields"]["rounded"];
    if (!validate_custom_field(json_rounded, "rounded", platform["iid"])) {
        return false;
    }
    rounded = json_rounded;

    return true;
}

/*
 * OTHER HELPERS
 */

bool LevelParsingSystem::parse_json() {
    LevelState& level_state = registry.levelStates.components[0];
    string filename = PROJECT_SOURCE_DIR + std::string("../LDtk/") + level_state.curr_level_folder_name + string("/data.json");
    ifstream level_file(filename);
    if (!level_file) {
        cout << "Error could not open file " << filename << endl;
        return false;
    }

    level_file >> json_data;
    level_file.close();
    return true;
}

vec2 LevelParsingSystem::convert_and_centralize_position(json pos, int conversion_factor) {
    return vec2({
        static_cast<int>(pos["cx"]) * TILE_TO_PIXELS + conversion_factor,
        static_cast<int>(pos["cy"]) * TILE_TO_PIXELS
    });
}

vec2 LevelParsingSystem::centralize_position(vec2 pos, int conversion_factor, bool is_x_axis) {
    return vec2({
        pos.x * TILE_TO_PIXELS + (is_x_axis ? conversion_factor : 0),
        pos.y * TILE_TO_PIXELS + (!is_x_axis ? conversion_factor : 0)
    });
}

bool LevelParsingSystem::validate_custom_field(json attribute, string attribute_name, string entity_id, vector<string> sub_attributes) {
    if (attribute.is_null()) {
        string error = "NULL value for customField: " + attribute_name;
        print_parsing_error(error, entity_id);
        return false;
    }

    if (!sub_attributes.empty()) {
        for (const string& sub : sub_attributes) {
            if (!validate_custom_field(attribute[sub], sub, entity_id)) {
                return false;
            }
        }
    }

    return true;
}

void LevelParsingSystem::print_parsing_error(string& error, string entity_id) {
    LevelState& ls = registry.levelStates.components[0];
    cout << "\033[96m" << ls.curr_level_folder_name << ": ";

    cout << "\033[91m" << "Error when parsing entity " << "\033[93m" << entity_id << ": " << "\033[91m" << error << endl;

    // set the line color back to normal in console
    cout << "\033[0m";
}