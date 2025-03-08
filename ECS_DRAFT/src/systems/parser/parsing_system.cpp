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
    create_player(initPos, {int(playerJson["width"]) * 2, int(playerJson["height"]) * 2});
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

        }
    }
}

void LevelParsingSystem::init_platforms(json platforms, bool moving) {
    for (json platform : platforms) {
        int fullSize = platform["customFields"]["size"];
        vec2 dimensions = {platform["width"], platform["height"]};
        vec2 currPos = {platform["x"], platform["y"]};

        for (int i = 0; i < fullSize; i++) {
            if (moving) {
            } else {
                create_static_platform(currPos, dimensions, false);
            }
            currPos.x += dimensions[0];
        }
    }
}