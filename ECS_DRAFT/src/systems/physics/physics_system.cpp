// internal
#include "physics_system.hpp"

#include <cfloat>

#include "../world/world_init.hpp"
#include <iostream>

void PhysicsSystem::init(GLFWwindow* window) {
	this->window = window;
}

/*
 *	PhysicsSystem::step
 *		- Loops over every motion entity
 *		- Applies gravity to entities that are mid-air (falling)
 *		- Moves player entity if it's walking (i.e. the player is holding the walk key)
 *		- Updates positions of entities with movement paths (i.e. moving platforms).
 *		- Detects collisions between all entities.
 */
void PhysicsSystem::step(float elapsed_ms) {

	auto& motion_registry = registry.motions;
	float step_seconds = elapsed_ms / 1000.f;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];


		if (registry.physicsObjects.has(entity)) {
			if (registry.physicsObjects.get(entity).apply_gravity) apply_gravity(entity, motion, step_seconds);
		}

		if (registry.players.has(entity)	) {
			player_walk(entity, motion, step_seconds);
		}

		if (registry.movementPaths.has(entity)) {
			move_object_along_path(entity, motion, step_seconds);
		}

		motion.position += (motion.velocity * motion.velocityModifier) * step_seconds;
	}

	// clear blocked...
	for (Entity entity : registry.physicsObjects.entities) {
		if (registry.blocked.has(entity)) {
			Blocked& blocked = registry.blocked.get(entity);
			blocked.normal = vec2{0, 0};
		}
	}

	detect_collisions();
}

// Handles all collisions detected in PhysicsSystem::step
void PhysicsSystem::late_step(float elapsed_ms) {
	handle_collisions(elapsed_ms);
}


// returns the mesh verticies if a mesh exists, otherwise, returns verticies of a square defined by the scale of the object (BB)
std::vector<vec2> get_vertices(Entity& e)
{
	if (registry.meshPtrs.has(e))
	{
		Mesh* mesh = registry.meshPtrs.get(e);
		Motion& motion = registry.motions.get(e);
		std::vector<vec2> vertices;
		float angle_cos = cos(motion.angle);
		float angle_sin = sin(motion.angle);
		for (auto & vertex : mesh->vertices)
		{
			vec2 scaled = {vertex.position.x * motion.scale.x, vertex.position.y * motion.scale.y};
			vec2 rotated = {
				scaled.x * angle_cos - scaled.y * angle_sin,
				scaled.x * angle_sin + scaled.y * angle_cos
			};
			vec2 vertex_pos = vec2{(rotated.x) + motion.position.x , (rotated.y) + motion.position.y};

			vertices.push_back(vertex_pos);
		}

		return vertices;
	} else
	{
		// TODO: store this ? no need to compute EVERY FRAME -> other than the translation
		// No mesh, we assume square BB
		std::vector<vec2> vertices(4);
		Motion& motion = registry.motions.get(e);
		vec2 half_size = motion.scale * 0.5f;

		vertices[0] = { -half_size.x, -half_size.y };
		vertices[1] = {  half_size.x, -half_size.y };
		vertices[2] = {  half_size.x,  half_size.y };
		vertices[3] = { -half_size.x,  half_size.y };

		// if there's a rotation, we need to apply
		float angle_cos = cos(motion.angle);
		float angle_sin = sin(motion.angle);

		for (vec2& vertex : vertices)
		{
			// rotate around z axis using this matrix:
			//[ cos -sin ]
			//[ sin  cos ]
			vec2 rotated = {
				vertex.x * angle_cos - vertex.y * angle_sin,
				vertex.x * angle_sin + vertex.y * angle_cos
			};
			vertex = rotated;
		}

		// make sure to translate to world position
		for (vec2& vertex : vertices) {
			vertex += motion.position;
		}
		return vertices;
	}
}

// returns the normals of the edge between the verticies
std::vector<vec2> get_axis(std::vector<vec2>& vertices)
{
	std::vector<vec2> local_axes;
	for (int i = 0; i < vertices.size(); i++)
	{
		vec2 edge = vertices[(i+1)%vertices.size()] - vertices[i]; // modulo so we properly wrap around
		vec2 normal = normalize(vec2{-edge.y, edge.x});
		local_axes.push_back(normal);
	}
	return local_axes;
}

// projects the verticies onto the axis
// only return the min and max since the object is convex and the axis is 1 dimensional
std::pair<float, float> project(const std::vector<vec2> verts, const vec2& axis)
{
	float min = dot(axis, verts[0]);
	float max = min; // init to min

	for (const auto& vertex : verts)
	{
		float proj = dot(axis, vertex);
		min = std::min(min, proj);
		max = std::max(max, proj);
	}

	return std::make_pair(min, max);
}

// see here for details https://dyn4j.org/2010/01/sat/
// computes collision between any two convex shapes (MUST be convex)
Collision compute_sat_collision(Entity& a, Entity& b)
{
	std::vector<vec2> a_verts = get_vertices(a);
	std::vector<vec2> b_verts = get_vertices(b);

	// now we need to get all of the axis (edge normals) from both shapes
	// TODO: make this a set to reduce redundant calculations
	std::vector<vec2> axes = get_axis(a_verts);
	std::vector<vec2> b_axes = get_axis(b_verts);
	axes.insert(axes.end(), b_axes.begin(), b_axes.end());

	float min_overlap = FLT_MAX;
	vec2 smallest_axis;

	// SAT time!! (perform the main SAT check)
	// For every axis:
	//	1. project each shape onto the axis
	//  2. check overlap on the axis
	for (const auto& axis : axes) // const bc I watched a TikTok about a vulnerability if you don't use it
	{
		auto [a_min, a_max] = project(a_verts, axis);
		auto [b_min, b_max] = project(b_verts, axis);

		// by the theorem:
		// if there is any axis where the projections do not overlap, there cannot be a collision. :O
		if (a_max < b_min || b_max < a_min)
		{
			return Collision{b, vec2{0,0}, vec2{0,0}};
		}

		// min and max so that we handle the case where there one is fully contained in the other
		float overlap = std::min(a_max, b_max) - std::max(a_min, b_min);

		if (overlap < min_overlap)
		{
			min_overlap = overlap;
			smallest_axis = axis;
		}
	}

	// make sure that the normal points from A -> B
	Motion& motionA = registry.motions.get(a);
	Motion& motionB = registry.motions.get(b);
	vec2 center_a = motionA.position;
	vec2 center_b = motionB.position;
	vec2 direction = center_a - center_b;
	if (dot(direction, smallest_axis) < 0)
	{
		smallest_axis *= -1;
	}

	// the smallest axis is where the shapes overlap the least ->
	// the minimal amount we need to move one shape to resolve the collision
	// (the axis are represented by the normals)
	return {b, min_overlap * smallest_axis, smallest_axis};
}


// detect collisions between all moving entities.
void PhysicsSystem::detect_collisions() {

    ComponentContainer<PhysicsObject> &physics_objects = registry.physicsObjects;
	ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i < physics_objects.components.size(); i++)
	{
		Entity entity_i = physics_objects.entities[i];


		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = 0; j < motion_container.components.size(); j++)
		{
			Entity entity_j = motion_container.entities[j];
			if (entity_j.id() == entity_i.id()
				|| (registry.collisions.has(entity_i) && registry.collisions.get(entity_i).other == entity_j)
				|| (registry.collisions.has(entity_j) && registry.collisions.get(entity_j).other == entity_i)) continue;

			Collision result = compute_sat_collision(entity_i, entity_j);

			if (result.normal != vec2{0,0} && result.overlap != vec2{0,0})
			{
				registry.collisions.emplace_with_duplicates(entity_i, entity_j, result.overlap, result.normal);
			}
		}
	}
}

// will move an object along its movement path, rotating between the different paths specified in the component
void PhysicsSystem::move_object_along_path(Entity& entity, Motion& motion, float step_seconds) {
	MovementPath& movement_path = registry.movementPaths.get(entity);

	Path currentPath = movement_path.paths[movement_path.currentPathIndex];

	vec2 modified_velocity = get_modified_velocity(motion);

	if (abs(motion.position.x - currentPath.end.x) <= abs(modified_velocity.x * step_seconds) &&
		abs(motion.position.y - currentPath.end.y) <= abs(modified_velocity.y * step_seconds)) {
		if (movement_path.currentPathIndex == movement_path.paths.size() - 1) {
			movement_path.currentPathIndex = 0;
		} else {
			movement_path.currentPathIndex++;
		}
			currentPath = movement_path.paths[movement_path.currentPathIndex];
			motion.position = currentPath.start;
		}
	motion.velocity = currentPath.velocity;
}

// rotates the projectile based on its velocity
void PhysicsSystem::rotate_projectile(Entity& entity, Motion& motion, float step_seconds) {
	float angularSpeed = 40.0f;
	motion.angle -= angularSpeed * step_seconds;
}

// accelerates the entity by GRAVITY until it reaches the max_speed (terminal velocity)
// TODO:: more terminal velocities
void PhysicsSystem::apply_gravity(Entity& entity, Motion& motion, float step_seconds) {
	float max_fall_speed = OBJECT_MAX_FALLING_SPEED;
	if (registry.players.has(entity)) {
		max_fall_speed = PLAYER_MAX_FALLING_SPEED;
	}

	motion.velocity.y = clampToTarget(motion.velocity.y, GRAVITY * step_seconds, max_fall_speed);
}

// Moves the player left or right depending on the direction specified in the walking component
// will not move if blocked
void PhysicsSystem::player_walk(Entity& entity, Motion& motion, float step_seconds)
{
	if (registry.walking.has(entity))
	{
		Walking& walking = registry.walking.get(entity);
		vec2 desired_direction = {walking.is_left ? -1.0f : 1.0f, 0.0f};
		vec2 accel = desired_direction * PLAYER_WALK_ACCELERATION;

		vec2 platform_velocity = {0.0f, 0.0f};
		if (registry.onGrounds.has(entity)) {
			Entity platform = registry.onGrounds.get(entity).ground_entity;
			Motion& platform_motion = registry.motions.get(platform);
			platform_velocity = get_modified_velocity(platform_motion);
		}

		vec2 rel_velocity = motion.velocity - platform_velocity;
		rel_velocity += accel * step_seconds;

		// clamp relative velocity instead of world velocity
		rel_velocity.x = clamp(rel_velocity.x, -PLAYER_MAX_WALKING_SPEED, PLAYER_MAX_WALKING_SPEED);

		motion.velocity = rel_velocity + platform_velocity;
	}
}

/*
 * Handles collisions between entities, specifically:
 *		- PhysicsObject <-> Platform
 *		- PhysicsObject <-> PhysicsObject
 */
void PhysicsSystem::handle_collisions(float elapsed_ms) {
	ComponentContainer<Collision>& collision_container = registry.collisions;

	std::vector<unsigned int> groundedEntities = {};
	float step_seconds = elapsed_ms / 1000.0f;

	for (uint i = 0; i < collision_container.components.size(); i++) {
		Entity& one = collision_container.entities[i];
		Collision& collision = collision_container.components[i];
		Entity& other = collision.other;


		// if (!registry.blocked.has(one))
		// {
		// 	Blocked& blocked = registry.blocked.emplace(one);
		// 	blocked.normal = collision.normal;
		// }

		// order here is important so handle both cases sep
		if (registry.physicsObjects.has(one) && registry.platforms.has(other)) {
			// std::cout << "  colliding with platform: " << registry.platforms.has(other) << std::endl;
			handle_object_rigid_collision(one, other, collision, step_seconds, groundedEntities);
		} else if (registry.physicsObjects.has(other) && registry.platforms.has(one)) {
			// std::cout << "  colliding with platform: " << registry.platforms.has(one) << std::endl;
		//	collision.overlap *= -1; //swap sides since coll is from perspective of one (left<->right) (top <-> bottom)
			handle_object_rigid_collision(other, one, collision, step_seconds, groundedEntities);
		}

		if (registry.players.has(one) && registry.projectiles.has(other)) {
			// TODO: should handle_player_projectile_collision() be handle_player_attack_collision() ?
			handle_player_attack_collision(one, other, collision);
		} else if (registry.players.has(other) && registry.projectiles.has(one)) {
			handle_player_attack_collision(other, one, collision);
		}

		if (registry.players.has(one) && registry.bosses.has(other)) {
			handle_player_boss_collision(one, other, collision);
		} else if (registry.players.has(other) && registry.bosses.has(one)) {
			handle_player_boss_collision(other, one, collision);
		}

		// // if player touches boundary, reset the game
		// GameState& gameState = registry.gameStates.components[0];
		// if (registry.players.has(one) && registry.boundaries.has(other)) {
		// 	gameState.game_running_state = GAME_RUNNING_STATE::SHOULD_RESET;
		// } else if (registry.players.has(other) && registry.boundaries.has(one)) {
		// 	gameState.game_running_state = GAME_RUNNING_STATE::SHOULD_RESET;
		// }

		// if objects touch the boundary, remove them
		// if (registry.physicsObjects.has(one) && registry.boundaries.has(other)) {
		// 	registry.remove_all_components_of(one);
		// } else if (registry.physicsObjects.has(other) && registry.boundaries.has(one)) {
		// 	registry.remove_all_components_of(other);
		// }

	}

	for (uint i = 0; i < collision_container.components.size(); i++) {
		Entity& one = collision_container.entities[i];
		Collision& collision = collision_container.components[i];
		Entity& other = collision.other;

		if (registry.physicsObjects.has(one) && registry.physicsObjects.has(other)) {
			handle_physics_collision(step_seconds, one, other, collision, groundedEntities);
        }
	}

	for (int i = 0; i < registry.physicsObjects.entities.size(); i++){
		Entity& entity = registry.physicsObjects.entities[i];

		if(!in(groundedEntities, entity.id())) {
			registry.onGrounds.remove(entity);

			Motion& motion = registry.motions.get(entity);
			float diff = AIR_RESISTANCE * step_seconds;
			motion.velocity.x = clampToTarget(motion.velocity.x, diff, 0);
		//	motion.velocity.y = clampToTarget(motion.velocity.y, diff, 0);

			if (registry.players.has(entity)) std::cout << "adding falling" << std::endl;

		} else {
			if (registry.players.has(entity))
				std::cout << "removing falling" << std::endl;
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}


// Handles collision between a PhysicsObject entity and a Platform entity.
void PhysicsSystem::handle_object_rigid_collision(Entity object_entity, Entity platform_entity, Collision collision, float step_seconds, std::vector<unsigned int>& groundedEntities)
{
	Blocked& blocked = registry.blocked.get(object_entity);
	Motion& obj_motion = registry.motions.get(object_entity);

	vec2 normal = collision.normal;
	blocked.normal = normal;

	resolve_collision_position(object_entity, platform_entity, collision);

	adjust_velocity_along_normal(obj_motion, normal);

	float mass = DEFAULT_MASS;
	if (registry.physicsObjects.has(object_entity)) {
		mass = registry.physicsObjects.get(object_entity).mass;
	}

	Motion& platform_motion = registry.motions.get(platform_entity);
	vec2 platform_velocity = get_modified_velocity(platform_motion);
//	platform_velocity.y = 0.0f;


	if (should_slip(-normal.y)) {
		bool is_moving_platform = registry.movementPaths.has(platform_entity);
		vec2 relative_vel = obj_motion.velocity - platform_velocity;

		vec2 new_relative_vel = get_friction(
			object_entity,
			relative_vel,
			normal,
			step_seconds,
			mass,
			is_moving_platform
		);

		obj_motion.velocity = new_relative_vel + platform_velocity;

		// calculate fling velocity based on platform movement
		float platform_speed = length(platform_velocity);
		if (platform_speed > 250.0f) {
			vec2 tangent = normalize(vec2(normal.y, -normal.x));

			// Fling in platform's movement direction scaled by surface alignment
			float surface_alignment = abs(dot(normalize(platform_velocity), tangent));
			obj_motion.velocity += platform_velocity * 0.1f * surface_alignment;
		}
	}

	if (is_on_ground(-normal.y)) {
		groundedEntities.push_back(object_entity);
		if (registry.onGrounds.has(object_entity)) {
			registry.onGrounds.get(object_entity).ground_entity = platform_entity;
		} else {
			registry.onGrounds.emplace(object_entity, platform_entity);
		}
	}
}

void PhysicsSystem::handle_player_attack_collision(Entity player_entity, Entity attack_entity, Collision collision) {
	if (registry.harmfuls.has(attack_entity)) {
		assert(registry.gameStates.components.size() <= 1);
		GameState& gameState = registry.gameStates.components[0];
		gameState.game_running_state = GAME_RUNNING_STATE::OVER;
	}
}

void PhysicsSystem::handle_player_boss_collision(Entity player_entity, Entity boss_entity, Collision collision) {
	Boss& boss = registry.bosses.get(boss_entity);

	boss.health -= PLAYER_ATTACK_DAMAGE;

	if (boss.health <= 0.f) {
		assert(registry.gameStates.components.size() <= 1);
		GameState& gameState = registry.gameStates.components[0];
		gameState.is_in_boss_fight = 0;
		registry.bosses.remove(boss_entity);
	}
}

// Handles collision between two PhysicsObject entities.
void PhysicsSystem::handle_physics_collision(float step_seconds, Entity entityA, Entity entityB, Collision collision, std::vector<unsigned int>& grounded)
{
	Motion& motionA = registry.motions.get(entityA);
	Motion& motionB = registry.motions.get(entityB);

	PhysicsObject& physA = registry.physicsObjects.get(entityA);
	PhysicsObject& physB = registry.physicsObjects.get(entityB);

	vec2 normal = collision.normal;

	float a_inv_mass = (1.0f / physA.mass);
	float b_inv_mass = (1.0f / physB.mass);
	float total_inv_mass = a_inv_mass + b_inv_mass;
	resolve_collision_position(entityA, entityB, collision);

	const float velocity_bias = 0.1f; // Small separation velocity
	vec2 bias_velocity = normal * velocity_bias;

	motionA.velocity -= bias_velocity * a_inv_mass;
	motionB.velocity += bias_velocity * b_inv_mass;

	// now get the relative velocities
	vec2 vel_relative = motionB.velocity - motionA.velocity;
	float vel_along_normal = dot(vel_relative, normal);

	// compute the impulse
	float impulse_scalar = -(1.0f + PHYSICS_OBJECT_BOUNCE) * vel_along_normal;
	impulse_scalar /= (total_inv_mass);

	vec2 impulse = impulse_scalar * normal;
	motionA.velocity -= impulse * a_inv_mass; // because normal points A->B but A is moving towards B
	motionB.velocity += impulse * b_inv_mass;

    vec2 tangent = vel_relative - dot(vel_relative, normal) * normal;

    if (length(tangent) > 0.01f) {
        tangent = normalize(tangent);

        float friction_impulse_magnitude = (-dot(vel_relative, tangent) / total_inv_mass) * STATIC_FRICTION;

		// TODO: bit hacky
		vec2 friction_impulse = friction_impulse_magnitude * tangent;
		float vertical_scale = 0.5f;
		friction_impulse.y *= vertical_scale;

        motionA.velocity -= a_inv_mass * friction_impulse;
        motionB.velocity += b_inv_mass * friction_impulse;
    }
	// finally, detect if they are on the ground! (or an angled platform)
	if (is_on_ground(-normal.y))
	{
		grounded.push_back(entityA.id());
		registry.onGrounds.emplace(entityA, entityB);
	}

	if (is_on_ground(normal.y))
	{
		grounded.push_back(entityB.id());
		registry.onGrounds.emplace(entityB, entityA);
	}
}

// proper friction using coulomb's law
// https://www.tribonet.org/wiki/laws-of-friction/
vec2 PhysicsSystem::get_friction(Entity& e, vec2& velocity, vec2& normal, float step_seconds, float mass, bool is_moving_platform) {
	vec2 velocity_tangent = velocity - dot(velocity, normal) * normal;
	float tangent_speed = length(velocity_tangent);

	if (tangent_speed <= 0.0f) {
		return velocity;
	}

	float friction = STATIC_FRICTION;
	if (registry.walking.has(e)) {
		Walking& walking = registry.walking.get(e);
		if ((walking.is_left && velocity.x < 0.0f) || (!walking.is_left && velocity.x > 0.0f)) {
			friction = DYNAMIC_FRICTION;
		}
	}

	if (is_moving_platform) {
		friction *= 2.0f;
	}

	// friction normal force is determined by mass * gravity
	float impulse = friction * (mass * GRAVITY) * step_seconds;
	impulse = std::min(impulse, tangent_speed);

	vec2 tangent_dir = normalize(velocity_tangent);
	return velocity - tangent_dir * impulse;
}

void PhysicsSystem::resolve_collision_position(Entity& entityA, Entity& entityB, Collision& collision)
{
	Motion& motionA = registry.motions.get(entityA);
	Motion& motionB = registry.motions.get(entityB);

	float inv_mass_a = 0.0f;
	if (registry.physicsObjects.has(entityA))
	{
		inv_mass_a = 1.0f / registry.physicsObjects.get(entityA).mass;
	}

	float inv_mass_b = 0.0f;
	if (registry.physicsObjects.has(entityB))
	{
		inv_mass_b = 1.0f / registry.physicsObjects.get(entityB).mass;
	}

	const float total_inv_mass = inv_mass_a + inv_mass_b;
	if (total_inv_mass <= 0.0f) return;


	// leave the objects slightly colliding so that the collision is still triggered
	const float epsilon = 0.1f;
	vec2 resolution = collision.normal * (length(collision.overlap) - epsilon);
	motionA.position += resolution * (inv_mass_a / total_inv_mass);
	motionB.position -= resolution * (inv_mass_b / total_inv_mass);
}

void PhysicsSystem::adjust_velocity_along_normal(Motion& motion, vec2& normal)
{
	float vec_along_normal = dot(motion.velocity, normal);
	if (vec_along_normal < 0.0f)
	{
		motion.velocity -= normal * vec_along_normal;
	}
}

bool PhysicsSystem::should_slip(float normal_y)
{
	float slip_angle_rad = PLATFORM_SLIP_ANGLE * (M_PI / 180.0f);
	float threshold = cos(slip_angle_rad);
	return normal_y >= threshold;
}

bool PhysicsSystem::is_on_ground(float normal_y) {
	float ground_angle_rad = PLAYER_MAX_WALK_ANGLE * (M_PI / 180.0f);
	float threshold = cos(ground_angle_rad);
	return normal_y >= threshold;
}


vec2 PhysicsSystem::get_modified_velocity(Motion& m)
{
	return {m.velocity.x * m.velocityModifier, m.velocity.y * m.velocityModifier};
}


// function for interpolating object velocity, specifically when an object is on a moving platforms.
float PhysicsSystem::clampToTarget(float value, float change, float target) {
	change = abs(change);
	if (value > target) {
		return std::max(value - change, target);
	} else if (value < target) {
		return std::min(value + change, target);
	}
	return target;
}

// Helper function to check if an entity id is within a vector.
bool PhysicsSystem::in(std::vector<unsigned int> vec, unsigned int id) {
	return std::find(vec.begin(), vec.end(), id) != vec.end();
}
