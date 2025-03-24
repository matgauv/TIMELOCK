#pragma once
#include "systems/ISystem.hpp"
#include "json.hpp"
#include "tinyECS/components.hpp"
using namespace std;
using namespace nlohmann;

class LevelParsingSystem : public ISystem
{
public:
    void init(GLFWwindow* window) override;
    void step(float elapsed_ms) override;
    void late_step(float elapsed_ms) override;

    LevelParsingSystem()
    {
    }
private:
    GLFWwindow* window = nullptr;
    json json_data;
    json tile_id_array;
    int stride;

    const std::unordered_map<std::string, TEXTURE_ASSET_ID> level_ground_map =
    {
        {"Level_0", TEXTURE_ASSET_ID::D_TUTORIAL_GROUND},
        {"Level_1", TEXTURE_ASSET_ID::A_TUTORIAL_GROUND},
        {"Level_2", TEXTURE_ASSET_ID::DECEL_LEVEL_GROUND},
        {"Level_3", TEXTURE_ASSET_ID::BOSS_LEVEL_ONE_GROUND}
    };

    bool parse_json();
    void init_level_background();
    void init_player_and_camera();
    void init_level_entities();
    void init_platforms(json platforms, bool moving);
    void init_boundaries(json boundaries);
    void init_partof(json partof);
    void init_spikes(json spikes);
    void init_projectiles(json projectiles);
    void init_cannons(json cannons);
    void init_ladders(json ladders);
    void init_checkpoints(json checkpoints);
    void init_doors(json doors);
    void init_pipes(json pipes);
    void init_pipeparts(json parts);
    void init_breakable_platforms(json breakables);
    void init_chains(json chains);
    void init_pendulums(json pendulums);
    void init_gears(json gears);
    void init_gear_spawners(json gear_spawners);
    void init_spikeballs(json spikeballs);

    bool extract_full_platform_dimensions(json platform, vec2& dimensions);
    bool extract_platform_attributes(json platform, vec2& dimensions, vec2& startPos, bool& rounded);
    bool extract_path_attributes(json platform, vector<Path>& paths, vec2& startPos, vec2& dimensions, bool& rounded);
    bool extract_boundary_attributes(json boundary, vec2& dimensions, vec2& position);
    bool extract_door_position(json door, vec2& position);

    vec2 convert_and_centralize_position(json pos, int conversion_factor);
    vec2 centralize_position(vec2 pos, int conversion_factor, bool is_x_axis);
    bool validate_custom_field(json attribute, string attribute_name, string entity, vector<string> sub_attributes = {});
    void print_parsing_error(string& error, string entity);
};
