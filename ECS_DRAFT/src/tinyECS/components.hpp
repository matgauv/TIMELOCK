#pragma once
#include "common.hpp"
#include <vector>
#include <chrono>

enum class GAME_RUNNING_STATE {
	RUNNING = 0,
	PAUSED = RUNNING + 1,
	OVER = PAUSED + 1,
	SHOULD_RESET = OVER + 1,
};

enum class TIME_CONTROL_STATE {
	NORMAL = 0,
	ACCELERATED = NORMAL + 1,
	DECELERATED = ACCELERATED + 1
};

enum class SCENE_TRANSITION_STATE {
	TRANSITION_OUT = 0,
	TRANSITION_IN = TRANSITION_OUT + 1
};

enum class BOSS_ID {
	FIRST = 0,
	SECOND = FIRST + 1,
	FINAL = SECOND + 1
};

enum class SPAWN_POINT_STATE {
	UNVISITED = 0,
	VISITED = UNVISITED + 1,
	ACTIVE = VISITED + 1
};

enum class PLAYER_STATE {
	// TODO: expand this to all possible states and transfer control to player system
	RESPAWNED = 0,
	ALIVE = RESPAWNED + 1,
	DEAD = ALIVE + 1
};

// Player component
struct Player
{
	vec2 spawn_point;
	float timer = 0.0;
	// Consider expanding the fields with a state variable (idle, walking, in air, dead, etc.)
	PLAYER_STATE state = PLAYER_STATE::ALIVE;
	// Potentially transfer acceleration/deceleration controls to Player as well
};

struct SpawnPoint
{
	SPAWN_POINT_STATE state = SPAWN_POINT_STATE::UNVISITED;
};

enum class CANON_TOWER_STATE {
	IDLE = 0,
	AIMING = IDLE + 1,
	FIRING = AIMING + 1
};

struct CanonTower {
	CANON_TOWER_STATE state = CANON_TOWER_STATE::IDLE;
	float detection_range = CANON_TOWER_DETECTION_RANGE;
	float timer = 0.0f;
	Entity barrel_entity;
};

struct CanonBarrel {
	float angle = 0.0f;
};

// Platform component
struct Platform
{
};

struct onGround
{
	Entity* ground_entity;
	onGround (Entity* entity) {
		ground_entity  = entity;
	}
};

// Boundary component
struct Boundary
{
};

// Path for moving sprite
struct Path {
	vec2 start;
	vec2 end;
	float duration;
	vec2 velocity;


	Path(const vec2& start, const vec2& end, float duration)
		: start(start), end(end), duration(duration),
		  velocity((end - start) / duration)
	{}
};

// A collection of paths for a moving sprite
struct MovementPath
{
	std::vector<Path> paths;
	int currentPathIndex = 0;
};

// Camera
struct Camera
{

};

// PhysicsObject means that the component will obey physics
// use for physics based objects
struct PhysicsObject
{
	float mass;
	float friction = STATIC_FRICTION;
	bool apply_gravity = true;
};


// All data relevant to the shape and motion of entities
struct Motion {
	vec2  position = { 0, 0 };
	float angle    = 0;
	vec2  scale    = { 10, 10 };
	float frequency = 0.f;
	float velocityModifier = 1.0f;
	vec2  velocity = {0.0f, 0.0f};
};

// This is added to a player who is walking.
struct Walking {
	bool is_left = false;
};

// This is added to a player entity when they collide with a wall to block them from walking through the wall.
struct Blocked {
	vec2 normal = { 0, 0 };
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
	Entity* other; // the second object involved in the collision
	vec2 overlap;
	vec2 normal;;
	Collision(Entity* other, vec2 overlap, vec2 normal) {
		this->other = other;
		this->overlap = overlap;
		this->normal = normal;
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
	float acceleration_factor = -1.0;
	float deceleration_factor = -1.0;
	float scene_transition_factor = -1.0;
};

// A struct that includes the necessary properties of the current game state
struct GameState {
	GAME_RUNNING_STATE game_running_state = GAME_RUNNING_STATE::RUNNING;
	TIME_CONTROL_STATE game_time_control_state = TIME_CONTROL_STATE::NORMAL;
	SCENE_TRANSITION_STATE game_scene_transition_state = SCENE_TRANSITION_STATE::TRANSITION_IN;
	float accelerate_cooldown_ms = 0.f;
	float decelerate_cooldown_ms = 0.f;
	float time_until_alarm_clock_ms = 300000.0f; // 5 minutes
	std::chrono::time_point<std::chrono::high_resolution_clock> time_control_start_time = std::chrono::time_point<std::chrono::high_resolution_clock>{};
	bool is_in_boss_fight = 0;
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
	vec2 uv;
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

// Marks an entity as responsive to time control
// will accelerate/decelerate when time control is used
struct TimeControllable
{
	float target_time_control_factor = NORMAL_FACTOR;
	bool can_be_accelerated = true;
	bool can_be_decelerated = true;
	bool can_become_harmful = 0;
	bool can_become_harmless = 0;
};

// A struct indicating that an entity can deal damage to other objects
// player is one-shot death
struct Harmful
{
	float damage = 0.f;
};

// A struct indicating that an entity is a Bolt (for mesh collision)
struct Bolt
{
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

// A struct indicating that an entity is a water drop
struct WaterDrop
{

};

// A struct indicating that an entity is a spike
struct Spike
{

};

// A struct indicating that an entity is breakable
struct Breakable
{
	float health;
	float degrade_speed_per_ms; // should be negative
	bool should_break_instantly; 
};

// A struct indicating that an entity is an enemy boss
struct Boss
{
	BOSS_ID boss_id;
	float health;
	float attack_cooldown_ms = 500.0f;
};

// A struct indicating that an entity is tile
struct Tile
{
	int id;
	unsigned int parent_id;
	int offset;
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
	SAMPLE_BACKGROUND = GREY_CIRCLE + 1,
	PLAYER_WALKING_V1 = SAMPLE_BACKGROUND + 1,
	PLAYER_STANDING_V1 = PLAYER_WALKING_V1 + 1,
	PLAYER_KILL = PLAYER_STANDING_V1 + 1,
	PLAYER_RESPAWN = PLAYER_KILL + 1,
	SAMPLE_PROJECTILE = PLAYER_RESPAWN + 1,
	OBJECT = SAMPLE_PROJECTILE + 1,
	BOUNDARY = OBJECT + 1,
	GEARS_BACKGROUND = BOUNDARY + 1,
	METAL_BACKGROUND = GEARS_BACKGROUND + 1,
	CHAIN_BACKGROUND = METAL_BACKGROUND + 1,
	HEX = CHAIN_BACKGROUND + 1,
	SPAWNPOINT_UNVISITED = HEX + 1,
	SPAWNPOINT_ACTIVATE = SPAWNPOINT_UNVISITED + 1,
	SPAWNPOINT_DEACTIVATE = SPAWNPOINT_ACTIVATE + 1,
	SPAWNPOINT_REACTIVATE = SPAWNPOINT_DEACTIVATE + 1,
	D_TUTORIAL_GROUND = SPAWNPOINT_REACTIVATE + 1,
	A_TUTORIAL_GROUND = D_TUTORIAL_GROUND + 1,
	TILE = A_TUTORIAL_GROUND + 1,
	TEXTURE_COUNT = TILE + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	TEXTURED = COLOURED + 1,
  	LINE = TEXTURED + 1,
  	SCREEN = LINE + 1,
	HEX = SCREEN + 1,
	TILE = HEX + 1,
	EFFECT_COUNT = TILE + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SPRITE = 0,
	DEBUG_LINE = SPRITE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	HEX = SCREEN_TRIANGLE + 1,
	GEOMETRY_COUNT = HEX + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class ANIMATION_ID {
	PLAYER_WALKING = 0,
	PLAYER_STANDING = PLAYER_WALKING + 1,
	PLAYER_KILL = PLAYER_STANDING + 1,
	PLAYER_RESPAWN = PLAYER_KILL + 1,
	SPAWNPOINT_ACTIVATE = PLAYER_RESPAWN + 1,
	SPAWNPOINT_DEACTIVATE = SPAWNPOINT_ACTIVATE + 1,
	SPAWNPOINT_REACTIVATE = SPAWNPOINT_DEACTIVATE + 1,
	ANIMATION_COUNT = SPAWNPOINT_REACTIVATE + 1
};
const int animation_count = (int)ANIMATION_ID::ANIMATION_COUNT;

enum class ANIMATION_TYPE_ID {
	CYCLE = 0,
	FREEZE_ON_LAST = CYCLE + 1,
	ANIMATION_TYPE_COUNT = FREEZE_ON_LAST + 1
};

enum class LAYER_ID {
	PARALLAXBACKGROUND = 0,
	BACKGROUND = PARALLAXBACKGROUND + 1,
	MIDGROUND = BACKGROUND + 1,
	FOREGROUND = MIDGROUND + 1
};

struct Layer {
	LAYER_ID layer = LAYER_ID::MIDGROUND;
};

struct RenderRequest {
	TEXTURE_ASSET_ID   used_texture  = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID    used_effect   = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	bool flipped = false; // TODO: it might be better to isolate this property into a new component
};

struct AnimateRequest {
	ANIMATION_ID used_animation;
	float timer = 0.0;
	vec2 tex_u_range = { 0.0, 1.0 };
};

struct LevelState {
	std::string curr_level_folder_name;
	TEXTURE_ASSET_ID ground;
	bool shouldLoad = false;
};