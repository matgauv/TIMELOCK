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

    bool parse_json();
    void init_level_background();
    void init_player_and_camera();
    void init_level_entities();
    void init_platforms(json platforms, bool moving);

    void extract_full_platform_dimensions(json platform, vec2& dimensions);
    void extract_platform_attributes(json platform, vec2& dimensions, vec2& startPos);
    void extract_path_attributes(json platform, vector<Path>& paths, vec2& startPos, vec2& dimensions);

    vec2 convert_and_centralize_position(json pos, int conversion_factor);
};
