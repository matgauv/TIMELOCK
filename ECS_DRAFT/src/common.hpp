#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>
#include "float.h"

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

// Level parsing constants
const int TILE_TO_PIXELS = 16;

// FPS Counter Update Period
const float FPS_COUNTER_UPDATE_PERIOD_MS = 1000.0f;

// Render Settings
const int MAX_INSTANCE_COUNT = 2000;

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
const float JUMP_VELOCITY = 410.0f;
const float AIR_DENSITY = 0.25f;

const float PHYSICS_OBJECT_BOUNCE = 0.2f;
const float DEFAULT_MASS = 10.0f;

const float PLATFORM_SLIP_ANGLE = 45.0f;
const float PLAYER_MAX_WALK_ANGLE = 80.0f;

const float DISTANCE_TO_DROP_BOLT = 300.0f;

// Player Statistics
const vec2 PLAYER_SCALE = { 50.0f, 50.0f };
const float PLAYER_MAX_FALLING_SPEED = 1000.0f;
const float PLAYER_MAX_WALKING_SPEED = 220.0f;
const float PLAYER_CLIMBING_SPEED = 150.0f;
const float LADDER_TOP_OUT_THRESH = TILE_TO_PIXELS / 4.0f;
const float PLAYER_STATIC_FRICTION = 0.7f;

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

const float PROJECTILE_WIDTH_PX = 40.0f;
const float PROJECTILE_HEIGHT_PX = 40.0f;

const float FIRST_BOSS_PROJECTILE_SPEED = -200.0f;
const float BOSS_ATTACK_COOLDOWN_MS = 500.0f;

const float PLAYER_ATTACK_DAMAGE = 20.0f;



// platform stuff
const float PLATFORM_EDGE_MESH_SIZE = 3.75f;

// APPROX gear measurements
const float GEAR_CENTER_PX = 86.0f;
const float GEAR_TOOTH_WIDTH_PX = 19.0f;
const float GEAR_TOOTH_HEIGHT_PX = 29.0f;
const float GEAR_TOTAL_WIDTH = 124.0f;

const float GEAR_CENTER_RATIO = GEAR_CENTER_PX / GEAR_TOTAL_WIDTH;
const float GEAR_TOOTH_WIDTH_RATIO = GEAR_TOOTH_WIDTH_PX / GEAR_TOTAL_WIDTH;
const float GEAR_TOOTH_HEIGHT_RATIO = GEAR_TOOTH_HEIGHT_PX / GEAR_TOTAL_WIDTH;

// APPROAX spikeball measurements
const float SPIKE_HEIGHT_PX = 25.0f;
const float SPIKE_WIDTH_PX = 17.0f;
const float SPIKEBALL_CENTER_PX = 59.0f;
const float SPIKEBALL_TOTAL_PX = 109.0f;

const float SPIKE_HEIGHT_RATIO = SPIKE_HEIGHT_PX / SPIKEBALL_TOTAL_PX;
const float SPIKE_WIDTH_RATIO = SPIKE_WIDTH_PX / SPIKEBALL_TOTAL_PX;
const float SPIKEBALL_CENTER_RATIO = SPIKEBALL_CENTER_PX / SPIKEBALL_TOTAL_PX;


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
// Particles
const int PARTICLE_COUNT_LIMIT = 1000;
const float MAX_CAMERA_DISTANCE = 2000.0;
const float TURBULENCE_GRID_SIZE = MAX_CAMERA_DISTANCE / 32.0f;
const float TURBULENCE_EVOLUTION_SPEED = 1e-12f;
const int TURBULENCE_OCTAVES = 1;

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
float cubic_interpolation(float source, float target, float t);
float rand_float(float min = 0.0f, float max = 1.0f);
vec2 rand_direction();
vec2 rotate_2D(vec2 v, float angle_rad);
vec2 angle_to_direction(float angle_rad);
vec2 safe_normalize(vec2 v);

vec2 random_sample_rectangle(vec2 center, vec2 dimensions, float angle_radians = 0.0);
vec2 random_sample_ellipse(vec2 center, vec2 dimensions, float angle_radians = 0.0);

bool gl_has_errors();
