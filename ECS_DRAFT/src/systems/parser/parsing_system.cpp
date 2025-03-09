#include <iostream>
#include "parsing_system.hpp"
#include "../world/world_init.hpp"
#include "tinyECS/registry.hpp"
#include <fstream>

void LevelParsingSystem::init(GLFWwindow *window) {
    this->window = window;
}

void LevelParsingSystem::step(float elapsed_ms) {
    LevelState& level_state = registry.levelStates.components[0];
    if (!level_state.shouldLoad) return;

    // clear current level (remove all entities)
    while (registry.motions.entities.size() > 0) {
        registry.remove_all_components_of(registry.motions.entities.back());
    }

    if (!parse_json()) {
        cout << "Error: could not parse JSON" << endl;
        return;
    }

    init_level_background();
    init_player_and_camera();
    init_level_entities();

    level_state.shouldLoad = false;
}

void LevelParsingSystem::late_step(float elapsed_ms) {

}

bool LevelParsingSystem::parse_json() {
    LevelState& level_state = registry.levelStates.components[0];
    string filename = PROJECT_SOURCE_DIR + std::string("data/levels/") + level_state.curr_level_file_name;
    ifstream level_file(filename);
    if (!level_file) {
        cout << "Error could not open file " << filename << endl;
        return false;
    }

    level_file >> json_data;
    level_file.close();
    return true;
}

void LevelParsingSystem::init_level_background() {
    // TODO: static w, h values -- should change (maybe parse from level file).
    float w = WINDOW_WIDTH_PX * 5.0f;
    float h = WINDOW_HEIGHT_PX * 5.0f;
    create_parallaxbackground({w, h}, TEXTURE_ASSET_ID::GEARS_BACKGROUND);
    create_background({w, h}, TEXTURE_ASSET_ID::METAL_BACKGROUND);
    create_foreground({ w, h}, TEXTURE_ASSET_ID::CHAIN_BACKGROUND);
}

void LevelParsingSystem::init_player_and_camera() {
    json playerJson = json_data["entities"]["Player"][0];
    vec2 initPos = vec2(playerJson["x"], playerJson["y"]);
    create_player(initPos, {int(playerJson["width"]), int(playerJson["height"])});
    create_camera(initPos, {1.0f, 1.0f});
}

void LevelParsingSystem::init_level_entities() {
    json entities = json_data["entities"];

    // TODO: This is really ugly -- can prob clean this up with a map or smt.
    for (auto& [entity_type, entity_list] : entities.items()) {
        if (entity_type == "Platform") {
            init_platforms(entity_list, false);
        } else if (entity_type == "MovingPlatform") {
            init_platforms(entity_list, true);
        } else if (entity_type == "Spike") {

        } else if (entity_type == "Door") {

        } else if (entity_type == "Projectile") {

        } else if (entity_type == "Pipe") {

        } else if (entity_type == "Boundary") {
            init_boundaries(entity_list);
        }
    }
}

void LevelParsingSystem::init_boundaries(json boundaries) {
    for (json boundary : boundaries) {
        vec2 dimensions;
        vec2 position;
        extract_boundary_attributes(boundary, dimensions, position);
        create_boundary(position, dimensions);
    }
}

void LevelParsingSystem::init_platforms(json platforms, bool moving) {
    for (json platform : platforms) {
        vec2 dimensions;
        vec2 startPos;
        if (moving) {
            vector<Path> path;
            extract_path_attributes(platform, path, startPos, dimensions);
            create_moving_platform(dimensions, path, startPos);
        } else {
            extract_platform_attributes(platform, dimensions, startPos);
            create_static_platform(startPos, dimensions);
        }
    }
}

void LevelParsingSystem::extract_boundary_attributes(json boundary, vec2& dimensions, vec2& position) {
    int x = boundary["x"];
    int y = boundary["y"];
    vec2 start_pos = vec2{x / TILE_TO_PIXELS, y / TILE_TO_PIXELS};

    json end_pos_json = boundary["customFields"]["length"];
    vec2 end_pos = {end_pos_json["cx"], end_pos_json["cy"]};

    int size;
    string direction = boundary["customFields"]["direction"];

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
}


void LevelParsingSystem::extract_full_platform_dimensions(json platform, vec2& dimensions) {
    int full_size = platform["customFields"]["size"];
    int full_width = full_size * static_cast<int>(platform["width"]);
    dimensions = {full_width, platform["height"]};
}

void LevelParsingSystem::extract_platform_attributes(json platform, vec2& dimensions, vec2& startPos) {
    extract_full_platform_dimensions(platform, dimensions);

    int left_x = static_cast<int>(platform["x"]) - (static_cast<int>(platform["width"]) / 2);
    int start_x = left_x + (static_cast<int>(dimensions[0]) / 2);
    startPos = {start_x, platform["y"]};
}

void LevelParsingSystem::extract_path_attributes(json platform, vector<Path>& paths, vec2& init_pos_in_path, vec2& dimensions) {
    extract_full_platform_dimensions(platform, dimensions);

    // currently all position values in json are relative to leftmost tile in platform -- need to centralize position on x-axis.
    int conversion_factor = (static_cast<int>(dimensions.x) / 2) - (static_cast<int>(platform["width"]) / 2);

    json start_pos_json = platform["customFields"]["start"];
    vec2 start_pos = convert_and_centralize_position(start_pos_json, conversion_factor);

    json end_pos_json = platform["customFields"]["end"];
    vec2 end_pos = convert_and_centralize_position(end_pos_json, conversion_factor);

    json init_pos_json = platform["customFields"]["position"];
    init_pos_in_path = convert_and_centralize_position(init_pos_json, conversion_factor);

    // TODO: hardcoded duration value -- should prob parse from json
    Path forward = Path(start_pos, end_pos, 0.5);
    Path backward = Path(end_pos, start_pos, 0.5);
    paths.push_back(forward);
    paths.push_back(backward);
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