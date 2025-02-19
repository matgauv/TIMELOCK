#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../../ext/stb_image/stb_image.h"

enum class GAME_RUNNING_STATE {
	RUNNING = 0,
	PAUSED = RUNNING + 1,
	OVER = PAUSED + 1
};

enum class TIME_CONTROL_STATE {
	NORMAL = 0,
	ACCELERATED = NORMAL + 1,
	DECELERATED = ACCELERATED + 1
};


// Player component
struct Player
{
};

// Platform component
struct Platform
{
};


// All data relevant to the shape and motion of entities
struct Motion {
	vec2  position = { 0, 0 };
	float angle    = 0;
	vec2  velocity = { 0, 0 };
	vec2  scale    = { 10, 10 };
	float frequency = 0.f;
};


// This is added to a player who is free-falling through the air.
struct Falling
{
};

// This is added to a player who is walking.
struct Walking {
	bool is_left = false;
};

// This is added to a player entity when they collide with a wall to block them from walking through the wall.
struct Blocked {
	bool left = false;
	bool right = false;
};

// counterclockwise (sides for collisions)
enum SIDE {
	LEFT = 0,
	BOTTOM = 1,
	RIGHT = 2,
	TOP = 3,
	NONE = 4
};

// Structure to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	SIDE side;
	Collision(Entity& other, SIDE side) {
		this->other = other;
		this->side = side;
	};
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

// A struct that includes the necessary properties of the current game state
struct GameState {
	GAME_RUNNING_STATE game_running_state = GAME_RUNNING_STATE::RUNNING;
	TIME_CONTROL_STATE game_time_control_state = TIME_CONTROL_STATE::NORMAL;
	float accelerate_cooldown_ms = 0.f;
	float decelerated_cooldown_ms = 0.f;
	float time_until_alarm_clock_ms = 300000.0f; // 5 minutes
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};


// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// A struct indicating that an entity can be accelerated by the player's ability
struct Acceleratable
{
	float factor = 2.f;
	bool can_become_harmful = 0;
};

// A struct indicating that an entity can be decelerated by the player's ability
struct Deceleratable
{
	float factor = 0.f;
	bool can_become_harmless = 0;
};

// A struct indicating that an entity can deal damage to other objects
// player is one-shot death
struct Harmful
{
	float damage = 0.f;
};

// A struct indicating that an entity is a swinging pendulum
struct Pendulum
{

};

// A struct indicating that an entity is a clock gear
struct Gear
{

};

// A struct indicating that an entity is a projectile
struct Projectile
{

};

// A struct indicating that an entity is a rock
struct Rock
{

};

// A struct indiciating that an entity is a water drop
struct WaterDrop
{

};


/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	BLACK = 0,
	GREY_CIRCLE = BLACK + 1,
	TEXTURE_COUNT = GREY_CIRCLE + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	TEXTURED = COLOURED + 1,
  	LINE = TEXTURED + 1,
  	SCREEN = LINE + 1,
	EFFECT_COUNT = SCREEN + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SPRITE = 0,
	DEBUG_LINE = SPRITE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID   used_texture  = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID    used_effect   = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

