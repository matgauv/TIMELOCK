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

    bool parse_json();
    void init_level_background();
    void init_player_and_camera();
    void init_level_entities();
    void init_platforms(json platforms, bool moving);
    void init_boundaries(json boundaries);
    void init_partof(json partof);
    void init_spikes(json spikes);

    void extract_full_platform_dimensions(json platform, vec2& dimensions);
    void extract_platform_attributes(json platform, vec2& dimensions, vec2& startPos);
    void extract_path_attributes(json platform, vector<Path>& paths, vec2& startPos, vec2& dimensions);
    void extract_boundary_attributes(json boundary, vec2& dimensions, vec2& position);

    vec2 convert_and_centralize_position(json pos, int conversion_factor);
    vec2 centralize_position(vec2 pos, int conversion_factor, bool is_x_axis);
};
