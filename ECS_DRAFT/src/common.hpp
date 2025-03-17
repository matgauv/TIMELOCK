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

// Render Settings
const int MAX_INSTANCE_COUNT = 10000;

// Spawn Points
const float SPAWNPOINT_DETECTION_RANGE = 80.0;
const float SPAWNPOINT_CHARGE_TIME_MS = 500.0;
const vec2 SPAWNPOINT_SCALE = { 60, 120 };

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
const float M_TO_PIXELS = 65.0f; // 50 px is 1m
const float GRAVITY = 9.8f * M_TO_PIXELS;
const float OBJECT_MAX_FALLING_SPEED = 1000.0f;

const float STATIC_FRICTION = 0.15f;
const float DYNAMIC_FRICTION = 0.015f;
const float BOLT_FRICTION = 0.1f;
const float AIR_RESISTANCE = 250.0f;
const float JUMP_VELOCITY = 305.0f;

const float PHYSICS_OBJECT_BOUNCE = 0.2f;
const float DEFAULT_MASS = 1.0f;

const float PLATFORM_SLIP_ANGLE = 45.0f;
const float PLAYER_MAX_WALK_ANGLE = 80.0f;

const float DISTANCE_TO_DROP_BOLT = 250.0f;

// Player Statistics
const vec2 PLAYER_SCALE = { 50.0f, 50.0f };
const float PLAYER_MAX_FALLING_SPEED = 1000.0f;
const float PLAYER_MAX_WALKING_SPEED = 305.0f;

const float PLAYER_WALK_ACCELERATION = 900.0f;

const float DEAD_REVIVE_TIME_MS = 500.0f;

const float JUMPING_VALID_TIME_MS = 100.0f;

// Fore, mid, background Depths; used for scaling only
const float FOREGROUND_DEPTH = 0.5f;
const float MIDGROUND_DEPTH = 0.75f;
const float BACKGROUND_DEPTH = 0.9f;
const float PARALLAXBACKGROUND_DEPTH = 1.5f;

// Camera motion properties
const float CAMERA_MAX_SPEED = PLAYER_MAX_FALLING_SPEED * 1.2f;
const float CAMERA_TRACE_RANGE = WINDOW_WIDTH_PX * 0.25f; // out of this range, camera will trace at max speed
const float CAMERA_VEL_LERP_FACTOR = 0.2f;

const float PROJECTILE_WIDTH_PX = 40.0f;
const float PROJECTILE_HEIGHT_PX = 40.0f;

const float FIRST_BOSS_PROJECTILE_SPEED = -200.0f;
const float BOSS_ATTACK_COOLDOWN_MS = 500.0f;

const float PLAYER_ATTACK_DAMAGE = 20.0f;

// Level parsing constants
const int TILE_TO_PIXELS = 16;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
// Canon Tower
const float CANON_TOWER_DETECTION_RANGE = 300.0f;
const float CANON_TOWER_AIM_TIME_MS = 2750.0f;
const float CANON_TOWER_LOAD_TIME_MS = 600.0f;
const float CANON_TOWER_FIRE_TIME_MS = 1500.0f;

const float CANON_TURN_SPEED = M_PI;

const vec2 CANON_TOWER_SIZE = vec2{80, 120};

// barrel lies horizontally to the right at angle = 0
const vec2 CANON_BARREL_SIZE = vec2{ 80, 30 };
const float CANON_PROJECTILE_SPEED = 600.0f;
const vec2 CANON_PROJECTILE_SIZE = vec2 {30.0f, 30.0f};


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
