#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tinyECS/component_container.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string level_ground_path(const std::string& folder_name) {return PROJECT_SOURCE_DIR + std::string("../LDtk/") + folder_name + std::string("/Ground.png");}
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};


#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

//
// game constants
//
const int WINDOW_WIDTH_PX = 1280;
const int WINDOW_HEIGHT_PX = 720;

// FPS Counter Update Period
const float FPS_COUNTER_UPDATE_PERIOD_MS = 1000.0f;

// Spawn Points
const float SPAWNPOINT_DETECTION_RANGE = 80.0;
const float SPAWNPOINT_CHARGE_TIME_MS = 500.0;
const vec2 SPAWNPOINT_SCALE = { 25, 60 };

// TIME CONTROL
const float ACCELERATE_FACTOR = 2.0f;
const float DECELERATE_FACTOR = 0.2f;
const float NORMAL_FACTOR = 1.0f;

// For Breakable Wall
const float TIME_CONTROL_VICINITY_THRESHOLD = 150.f;

// TODO: increase these for game...
const float ACCELERATION_COOLDOWN_MS = 1500.0f;
const float DECELERATION_COOLDOWN_MS = 1000.0f;

const float ACCELERATION_DURATION_MS = 10000.0f;
const float DECELERATION_DURATION_MS = 10000.0f;

// Screen shader effect
const float ACCELERATION_EMERGE_MS = 150.0f;
const float DECELERATION_EMERGE_MS = 150.0f;

// Physics
const float M_TO_PIXELS = 80.0f; // 50 px is 1m
const float GRAVITY = 1250.0f;
const float GRAVITY_JUMP_ASCENT = 475.0f;
const float OBJECT_MAX_FALLING_SPEED = 1000.0f;

const float STATIC_FRICTION = 0.1f;
const float DYNAMIC_FRICTION = 0.015f;
const float BOLT_FRICTION = 0.1f;
const float AIR_RESISTANCE = 250.0f;
const float JUMP_VELOCITY = 380.0f;
const float AIR_DENSITY = 0.25f;

const float PHYSICS_OBJECT_BOUNCE = 0.2f;
const float DEFAULT_MASS = 10.0f;

const float PLATFORM_SLIP_ANGLE = 45.0f;
const float PLAYER_MAX_WALK_ANGLE = 80.0f;

const float DISTANCE_TO_DROP_BOLT = 150.0f;

// Player Statistics
const vec2 PLAYER_SCALE = { 50.0f, 50.0f };
const float PLAYER_MAX_FALLING_SPEED = 1000.0f;
const float PLAYER_MAX_WALKING_SPEED = 220.0f;
const float PLAYER_CLIMBING_SPEED = 150.0f;

const float PLAYER_WALK_ACCELERATION = 1200.0f;
const float PLAYER_WALK_LADDER_ACCELERATION = PLAYER_WALK_ACCELERATION / 3.0f;

const float DEAD_REVIVE_TIME_MS = 500.0f;

const float JUMPING_VALID_TIME_MS = 3000.0f;

// Fore, mid, background Depths; used for scaling only
const float FOREGROUND_DEPTH = 0.5f;
const float MIDGROUND_DEPTH = 0.75f;
const float BACKGROUND_DEPTH = 0.9f;
const float PARALLAXBACKGROUND_DEPTH = 1.5f;

// Camera motion properties
const float CAMERA_MAX_SPEED = PLAYER_MAX_FALLING_SPEED * 1.2f;
const float CAMERA_TRACE_RANGE = WINDOW_WIDTH_PX * 0.25f; // out of this range, camera will trace at max speed
const float CAMERA_VEL_LERP_FACTOR = 0.05f;
const float CAMERA_DEFAULT_SCALING = 0.7f;
const float CAMERA_MIN_SCALING = 0.2f;
const float CAMERA_MAX_SCALING = 5.0f;
const float CAMERA_BOUNDARY_PADDING = 0.75f;
const float CAMERA_SCREEN_SPACING_FOR_MOTION_RATIO = 1.0f / 10.0f;

// Projectile properties
const float PROJECTILE_WIDTH_PX = 40.0f;
const float PROJECTILE_HEIGHT_PX = 40.0f;
const float PROJECTILE_SPEED = (float) WINDOW_WIDTH_PX / 5.f; // projectile should travel across the entire screen in 5 seconds

// General boss battle related properties
const float PLAYER_ATTACK_DAMAGE = 20.0f;

// Level parsing constants
const int TILE_TO_PIXELS = 16;

// Boss 1 specific properties
const float BOSS_ONE_SPAWN_POINT_X = 400.f;
const float BOSS_ONE_SPAWN_POINT_Y = 720.f;
const float BOSS_ONE_ON_GROUND_Y_POSITION = 730.f; // TODO: need to verify this
const float BOSS_ONE_MAX_HEALTH = 100.f;
const float BOSS_ONE_X_VELOCITY_MULTIPLIER = 0.5f;
const float BOSS_ONE_MAX_X_VELOCITY = PLAYER_MAX_WALKING_SPEED;
const float BOSS_ONE_MIN_X_VELOCITY = BOSS_ONE_MAX_X_VELOCITY / 20.f;
const float BOSS_ONE_BB_WIDTH_PX = 50.f; // TODO: placeholder, we should adjust this once the actual texture is ready
const float BOSS_ONE_BB_HEIGHT_PX = 50.f; // TODO: placeholder, we should adjust this once the actual texture is ready
const float BOSS_ONE_MAX_TIME_UNTIL_EXHAUSTED_MS = 30000.f;

const float BOSS_ONE_MAX_WALK_DURATION_MS = 2000.f; // use 1000.f for testing purposes, otherwise use 5000.f
const float BOSS_ONE_MAX_EXHAUSTED_DURATION_MS = 10000.f;
const float BOSS_ONE_MAX_RECOVER_DURATION_MS = 2000.f;
const float BOSS_ONE_MAX_DAMAGED_DURATION_MS = 2000.f;

const float BOSS_ONE_MAX_NUM_OF_NON_DELAYED_PROJECTILE = 4;
const float BOSS_ONE_INTER_PROJECTILE_TIMER_MS = 1000.f;
const float BOSS_ONE_PROJECTILE_WIDTH_PX = 15.f;
const float BOSS_ONE_PROJECTILE_HEIGHT_PX = 15.f;

const float BOSS_ONE_REGULAR_PROJECTILE_VELOCITY = WINDOW_WIDTH_PX / 3.f;

const float BOSS_ONE_FAST_PROJECTILE_VELOCITY = BOSS_ONE_REGULAR_PROJECTILE_VELOCITY * 2.f;

// const float BOSS_ONE_DELAYED_PROJECTILE_Y_POSITION = WINDOW_HEIGHT_PX / 4.f;
const float BOSS_ONE_DELAYED_PROJECTILE_Y_POSITION = 700.f; // for testing purposes
// const float BOSS_ONE_FIRST_DELAYED_PROJECTILE_X_POSITION = WINDOW_WIDTH_PX * 0.25f;
const float BOSS_ONE_FIRST_DELAYED_PROJECTILE_X_POSITION = 300.f; // for testing purposes
const float BOSS_ONE_FIRST_DELAYED_PROJECTILE_TIMER_MS = 2500.f;
// const float BOSS_ONE_SECOND_DELAYED_PROJECTILE_X_POSITION = WINDOW_WIDTH_PX * 0.5f;
const float BOSS_ONE_SECOND_DELAYED_PROJECTILE_X_POSITION = 350.f; // for testing purposes
const float BOSS_ONE_SECOND_DELAYED_PROJECTILE_TIMER_MS = 4000.f;
// const float BOSS_ONE_THIRD_DELAYED_PROJECTILE_X_POSITION = WINDOW_WIDTH_PX * 0.75f;
const float BOSS_ONE_THIRD_DELAYED_PROJECTILE_X_POSITION = 400.f; // for testing purposes
const float BOSS_ONE_THIRD_DELAYED_PROJECTILE_TIMER_MS = 4500.f;

const float BOSS_ONE_DASH_VELOCITY = 100.f; // use 100.f for testing purposes, otherwise, the boss should be able to cross the screen in one second
const float BOSS_ONE_DASH_DURATION_MS = 4000.f;

const float BOSS_ONE_GROUND_SLAM_RISE_VELOCITY = -50.f; // for testing purposes, use -50.f, otherwise, the boss should take about 1~2 seconds to rise
const float BOSS_ONE_GROUND_SLAM_SLAM_VELOCITY = 300.f; // for testing purposes, use 300.f, otherwise, the boss should slam down in 0.25~0.5 seconds
const float BOSS_ONE_GROUND_SLAM_RISE_FINAL_Y_POSITION = 600.f; // for testing purposes, use 600.f, otherwise, the boss should be at the 1/3 point from the top
const float BOSS_ONE_FIRST_GROUND_SLAM_FOLLOW_DURATION_MS = 3000.f;
const float BOSS_ONE_SECOND_GROUND_SLAM_FOLLOW_DURATION_MS = 2000.f;
const float BOSS_ONE_THIRD_GROUND_SLAM_FOLLOW_DURATION_MS = 5000.f;
const float BOSS_ONE_GROUND_SLAM_LAND_DURATION_MS = 1000.f;
const float BOSS_ONE_GROUND_SLAM_IMPACT_WIDTH_PX = 20.f; // for testing purposes, use 20.f, otherwise, 100.f
const float BOSS_ONE_GROUND_SLAM_IMPACT_HEIGHT_PX = 50.f;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
// Cannon Tower
const float CANNON_TOWER_DETECTION_RANGE = 300.0f;
const float CANNON_TOWER_AIM_TIME_MS = 2750.0f;
const float CANNON_TOWER_LOAD_TIME_MS = 600.0f;
const float CANNON_TOWER_FIRE_TIME_MS = 1500.0f;

const float CANNON_TURN_SPEED = M_PI;

const vec2 CANNON_TOWER_SIZE = vec2{80, 120};

// barrel lies horizontally to the right at angle = 0
const vec2 CANNON_BARREL_SIZE = vec2{ 80, 30 };
const float CANNON_PROJECTILE_SPEED = 600.0f;
const vec2 CANNON_PROJECTILE_SIZE = vec2 {30.0f, 30.0f};

// door
const vec2 DOOR_SIZE = vec2 { 2.0f * TILE_TO_PIXELS, 3.0f * TILE_TO_PIXELS };

// Parsing constants
const float PARSING_CANNON_Y_POS_DIFF = (0.5f * TILE_TO_PIXELS) - (CANNON_TOWER_SIZE.y / 2);
const float PARSING_CHECKPOINT_Y_POS_DIFF = (0.5f * TILE_TO_PIXELS) - (SPAWNPOINT_SCALE.y / 2);

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

float lerpToTarget(float current, float target, float time);
bool gl_has_errors();
