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

struct FlagState {
	bool fly = false;
	bool no_clip = false;
};

// Player component
struct Player
{
	vec2 spawn_point;
	float timer = 0.0;
	// Consider expanding the fields with a state variable (idle, walking, in air, dead, etc.)
	PLAYER_STATE state = PLAYER_STATE::ALIVE;
	// Potentially transfer acceleration/deceleration controls to Player as well

	float jumping_valid_time = -1.0f;
};

struct SpawnPoint
{
	SPAWN_POINT_STATE state = SPAWN_POINT_STATE::UNVISITED;
};

// CanNon Towers
enum class CANNON_TOWER_STATE {
	IDLE = 0,
	AIMING = IDLE + 1,
	LOADING = AIMING + 1,
	FIRING = LOADING + 1
};

struct CannonTower {
	CANNON_TOWER_STATE state = CANNON_TOWER_STATE::IDLE;
	float detection_range = CANNON_TOWER_DETECTION_RANGE;
	float timer = 0.0f;
	Entity barrel_entity;
};

struct CannonBarrel {
	float angle = 0.0f;
};

// Platform component
struct Platform
{
};

// this component indicates that the platform has a geometry, and we should consider the rounded corners when computing collisions
// (all rigid objects are "platforms" but some have the visual appearance of rounded corners)
struct PlatformGeometry {
	int num_tiles;
};

struct onGround
{
	unsigned int other_id;
	onGround(unsigned int other_id) : other_id(other_id) {}
};

// Boundary component
struct Boundary
{
};

// Swinging Pendulum
struct Pendulum {
	vec2 pivot_point;
	float length;
	float current_angle;
	float angular_velocity;
	float damping = 0.0f; // optional, 0-1
};

struct PendulumRod {
	unsigned int bob_id;
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
	float horizontal_offset = 0.0f;
};

// PhysicsObject means that the component will obey physics
// use for physics based objects
struct PhysicsObject
{
	// TOGGLES (true means property will be applied)
	bool apply_gravity = true;
	bool apply_rotation = false;
	bool apply_air_resistance = true;
	bool apply_friction = true;

	// Properties to set:

	// general guide is 20-100ish, though you can go beyond. Extremely high or low values can lead to unexpected behaviour.
	// mass = 0 means the object is FIXED and will not move in response to a collision. (though any set velocity in the motion will still be applied)
	float mass = 0.1f;

	// friction will resist the object's motion on collision. Should be set in the 0-1 range.
	float friction = STATIC_FRICTION;

	// bounce controls how much energy is lost due to a collision.
	//   - 0.0 means 100% energy is lost (no bounce)
	//   - 1.0 means no energy is lost (bonce forever)
	// NOTE: in a collision the minmum "bounce" between the two colliding objects will be used for calculation
	float bounce = PHYSICS_OBJECT_BOUNCE;

	// controls how air resistance slows down the object. Should eb in the 0-1 range. (think of as air friction)
	// NOTE: the 'effective area' is also considered in air resistance calculations, so larger objects will slow down more.
	float drag_coefficient = 0.2f;

	// the amount of energy lost when rotating. Set this to 0 to have an object rotate forever!
	float angular_damping = 0.8f;

	// INTERNAL PROPERTIES
	// these are just used to keep track of information, no need to set manually as they will be calculated automatically!
	float moment_of_inertia = 0.0f;
	float angular_velocity = 0.0f;
};


struct NonPhysicsCollider {

};

struct RotatingGear {
	float angular_velocity = 0.0f;
};

struct ObstacleSpawner {
	vec2 velocity;
	vec2 start_position;
	vec2 end_position;
	vec2 size;
	unsigned int obstacle_id = 0;
	std::string obstacle_type;

	float time_left_ms = 10000.0f;
	float lifetime_ms = 10000.0f;
};


// All data relevant to the shape and motion of entities
struct Motion {
	vec2  position = { 0, 0 };
	float angle    = 0;
	vec2  scale    = { 10, 10 };
	float frequency = 0.f;
	float velocityModifier = 1.0f;
	vec2  velocity = {0.0f, 0.0f};
	std::vector<vec2> cached_vertices;
	std::vector<vec2> cached_axes;
	bool cache_invalidated = true;
};

struct PivotPoint {
	vec2 offset = { 0, 0 };
};

// This is added to a player who is walking.
struct Walking {
	bool is_left = false;
};

// this struct is added to a player who is climbing
struct Climbing {
	bool is_moving = false;
	bool is_up = false;
	bool was_climbing = false; // climbing in previous frame
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
	unsigned int other_id; // the second object involved in the collision
	vec2 overlap;
	vec2 normal;;
	Collision(unsigned int other, vec2 overlap, vec2 normal) {
		this->other_id = other;
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

struct SubMesh {
	Mesh* original_mesh;
	std::vector<vec2> cached_vertices;
	std::vector<vec2> cached_axes;
	vec2 offset;
	float rotation = 0.0f;
	vec2 scale_ratio = {1.0f, 1.0f};
	vec2 world_pos = { 0, 0 };
	bool cache_invalidated = true;
};

struct CompositeMesh {
	std::vector<SubMesh> meshes;
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

// A struct indicating that an entity is a Text (for tutorial)
struct Text
{
	Entity textEntity;
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

// struct indicating that entity is a Ladder
struct Ladder {

};

// struct indicating that entity is a Pipe
struct Pipe {
	std::string direction;
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
	vec2 offset = vec2{0.0f,0.0f};
};

// exit door to go to next level.
struct Door
{
	bool opened = false;
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
	CHAIN = METAL_BACKGROUND + 1,
	HEX = CHAIN + 1,
	SPAWNPOINT_UNVISITED = HEX + 1,
	SPAWNPOINT_ACTIVATE = SPAWNPOINT_UNVISITED + 1,
	SPAWNPOINT_DEACTIVATE = SPAWNPOINT_ACTIVATE + 1,
	SPAWNPOINT_REACTIVATE = SPAWNPOINT_DEACTIVATE + 1,
	CANNON_TOWER = SPAWNPOINT_REACTIVATE + 1,
	BARREL = CANNON_TOWER + 1,
	D_TUTORIAL_GROUND = BARREL + 1,
	A_TUTORIAL_GROUND = D_TUTORIAL_GROUND + 1,
	DECEL_LEVEL_GROUND = A_TUTORIAL_GROUND +1,
	BOSS_LEVEL_ONE_GROUND = DECEL_LEVEL_GROUND + 1,
	TILE = BOSS_LEVEL_ONE_GROUND + 1,
	WASD = TILE + 1,
	DECEL = WASD + 1,
	DECEL2 = DECEL + 1,
	ACCEL = DECEL2 + 1,
	PENDULUM = ACCEL + 1,
	PENDULUM_ARM = PENDULUM + 1,
	GEAR = PENDULUM_ARM + 1,
	SPIKEBALL = GEAR + 1,
	BREAKABLE_FRAGMENTS = SPIKEBALL + 1,
	TEXTURE_COUNT = BREAKABLE_FRAGMENTS + 1,
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	TEXTURED = COLOURED + 1,
  	LINE = TEXTURED + 1,
  	SCREEN = LINE + 1,
	HEX = SCREEN + 1,
	TILE = HEX + 1,
	PARTICLE_INSTANCED = TILE + 1,
	EFFECT_COUNT = PARTICLE_INSTANCED + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SPRITE = 0,
	DEBUG_LINE = SPRITE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	HEX = SCREEN_TRIANGLE + 1,
	PLAYER = HEX + 1,
	PLATFORM = PLAYER + 1,
	GEOMETRY_COUNT = PLATFORM + 1
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
	BREAKABLE_FRAGMENTS = SPAWNPOINT_REACTIVATE + 1,
	ANIMATION_COUNT = BREAKABLE_FRAGMENTS + 1
};
const int animation_count = (int)ANIMATION_ID::ANIMATION_COUNT;

enum class ANIMATION_TYPE_ID {
	CYCLE = 0,
	FREEZE_ON_RANDOM = CYCLE + 1,
	FREEZE_ON_LAST = FREEZE_ON_RANDOM + 1,
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
	std::string next_level_folder_name;
	TEXTURE_ASSET_ID ground;
	bool shouldLoad = false;
	vec2 dimensions;
};


// Particles

enum class PARTICLE_ID {
	COLORED = 0,
	SAMPLED_TEXTURE = COLORED + 1,
	BREAKABLE_FRAGMENTS = SAMPLED_TEXTURE + 1,
	PARTICLE_TYPE_COUNT = BREAKABLE_FRAGMENTS + 1
};

const int particle_type_count = (int)PARTICLE_ID::PARTICLE_TYPE_COUNT;

struct Particle {
	PARTICLE_ID particle_id;

	// Motion properties are isolated from Physics system to avoid sub-stepping particles
	vec2 position;
	float angle;
	vec2 scale;
	vec2 velocity;
	float ang_velocity = 0.0;

	float life;
	float timer = 0.0;
	float alpha = 1.0;
	vec2 fade_in_out = { 0.0, 0.0 };
	vec2 shrink_in_out = { 0.0, 0.0 };

	// Wind -> constant addition to velocity
	// Gravity -> contant acceleration
	// Turbulence -> randomized acceleration
	float wind_influence = 0.0;
	float gravity_influence = 0.0;
	float turbulence_influence = 0.0;
};

struct ParticleSystemState {
	vec2 wind_field = { 0.0, 0.0 };
	vec2 gravity_field = {0.0, GRAVITY};

	float turbulence_strength = 0.0;
	float turbulence_scale = 1.0;
};