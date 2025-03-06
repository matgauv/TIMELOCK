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


		if (registry.falling.has(entity)) {
			apply_gravity(entity, motion, step_seconds);
		}

		if (registry.players.has(entity)	) {
			player_walk(entity, motion, step_seconds);
		}

		if (registry.movementPaths.has(entity)) {
			move_object_along_path(entity, motion, step_seconds);
		}

		if (registry.bolts.has(entity)) {
			rotate_projectile(entity, motion, step_seconds); // temporary for visual test
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


float sign(float num) {
	return (num > 0.0f) - (num < 0.0f);
}


// Returns the overlap in the x and y direction of motions a and b
vec2 PhysicsSystem::get_collision_overlap(Motion& a, Motion& b) {
	vec2 playerBB = {abs(a.scale.x), abs(a.scale.y)};
	vec2 objBB = {abs(b.scale.x), abs(b.scale.y)};

	vec2 playerHalf = playerBB * 0.5f;
	vec2 objHalf = objBB * 0.5f;

	vec2 delta = a.position - b.position;

	float overlapX = (playerHalf.x + objHalf.x) - fabs(delta.x);
	float overlapY = (playerHalf.y + objHalf.y) - fabs(delta.y);

	vec2 overlap;
	if (abs(overlapX) < abs(overlapY)) {
		overlap = {overlapX * sign(delta.x), 0.0f};
	} else {
		overlap = {0.0f, overlapY * sign(delta.y)};
	}

	return overlap;
}

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 PhysicsSystem::get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// returns NONE if no collision, otherwise returns the side from the perspective of motion a
SIDE PhysicsSystem::get_collision_side(Motion& a, Motion& b, vec2 overlap) {
	vec2 delta = a.position - b.position;
	SIDE result = SIDE::NONE;
	if (fabs(overlap.x) >= 0 && fabs(overlap.y) >= 0) {
		if (fabs(overlap.x) < fabs(overlap.y)) {
			result = (delta.x > 0) ? SIDE::LEFT : SIDE::RIGHT;
		} else {
			result = (delta.y > 0) ? SIDE::TOP : SIDE::BOTTOM;
		}
	}
	return result;
}


// returns the mesh verticies if a mesh exists, otherwise, returns verticies of a square defined by the scale of the object (BB)
std::vector<vec2> get_vertices(Entity& e)
{
	if (registry.meshPtrs.has(e))
	{
		Mesh* mesh = registry.meshPtrs.get(e);
		std::vector<vec2> vertices;
		vertices.reserve(mesh->vertices.size());
		for (auto & vertex : mesh->vertices)
			vertices.push_back({vertex.position.x, vertex.position.y});

		return vertices;
	} else
	{
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
		// TODO: does this happen before or after? (are we using TRS?)
		for (vec2& vertex : vertices) {
			vertex += motion.position;
		}
		return vertices;
	}
}

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


// TODO: where to define this? can we just use the collison component? storing here now bc im lazzzyyy
struct CollisionResult
{
	bool collided = false;
	vec2 normal;
	float depth;
};

// see here for details https://dyn4j.org/2010/01/sat/
// computes collision between any two convex shapes (MUST be convex)
CollisionResult compute_sat_collision(Entity& a, Entity& b)
{
	std::vector<vec2> a_verts = get_vertices(a);
	std::vector<vec2> b_verts = get_vertices(b);

	// now we need to get all of the axis (edge normals) from both shapes
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
			return CollisionResult{false, vec2{0,0}, 0};
		}

		float overlap = std::min(a_max - b_min, b_max - a_min);
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
	return {true, smallest_axis, min_overlap};
}


// detect collisions between all moving entities.
void PhysicsSystem::detect_collisions() {

    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i < motion_container.components.size(); i++)
	{
		Entity entity_i = motion_container.entities[i];


		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j < motion_container.components.size(); j++)
		{
			Entity entity_j = motion_container.entities[j];
			CollisionResult result = compute_sat_collision(entity_i, entity_j);

			if (result.collided)
			{
				Collision& collision = registry.collisions.emplace_with_duplicates(entity_i, entity_j, result.normal * result.depth, result.normal);
				collision.other = entity_j;
				collision.overlap = result.normal * result.depth;
				collision.normal = result.normal;
			}
		}
	}
}

// will move an object along its movement path, rotating between the different paths specified in the component
void PhysicsSystem::move_object_along_path(Entity& entity, Motion& motion, float step_seconds) {
	MovementPath& movement_path = registry.movementPaths.get(entity);

	Path currentPath = movement_path.paths[movement_path.currentPathIndex];

	if (abs(motion.position.x - currentPath.end.x) <= abs(currentPath.velocity.x * motion.velocityModifier * step_seconds) &&
		abs(motion.position.y - currentPath.end.y) <= abs(currentPath.velocity.y * motion.velocityModifier * step_seconds)) {
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

// accelerates the entity by GRAVITY until it reaches the max_speed. currently only one max speed.
// TODO: should this be applied velocity?
void PhysicsSystem::apply_gravity(Entity& entity, Motion& motion, float step_seconds) {
	float max_fall_speed = OBJECT_MAX_FALLING_SPEED;
	if (registry.players.has(entity)) {
		max_fall_speed = PLAYER_MAX_FALLING_SPEED;
	}

	motion.velocity.y = clampToTarget(motion.velocity.y, GRAVITY * M_TO_PIXELS * step_seconds, max_fall_speed);
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

		motion.velocity += accel * step_seconds;
		motion.velocity.x = clamp(motion.velocity.x, -PLAYER_MAX_WALKING_SPEED, PLAYER_MAX_WALKING_SPEED);
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
	std::vector<unsigned int> onMovingPlatform = {};

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
			handle_object_platform_collision(one, other, collision, step_seconds, groundedEntities, onMovingPlatform);
		} else if (registry.physicsObjects.has(other) && registry.platforms.has(one)) {
			// std::cout << "  colliding with platform: " << registry.platforms.has(one) << std::endl;
		//	collision.overlap *= -1; //swap sides since coll is from perspective of one (left<->right) (top <-> bottom)
			handle_object_platform_collision(other, one, collision, step_seconds, groundedEntities, onMovingPlatform);
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

		// If object is not grounded, it should be falling.
		if(!in(groundedEntities, entity.id())) {
			if (!registry.falling.has(entity)) {
				registry.falling.emplace(entity);
			}
		} else {
			registry.falling.remove(entity);
			// TODO air resistance?
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}


// Handles collision between a PhysicsObject entity and a Platform entity.
void PhysicsSystem::handle_object_platform_collision(Entity object_entity, Entity platform_entity, Collision collision, float step_seconds, std::vector<unsigned int>& groundedEntities, std::vector<unsigned int>& onMovingPlatform)
{
	vec2 normal = collision.normal;
	float collision_depth = length(collision.overlap);

	// resolve collision
	Motion& obj_motion = registry.motions.get(object_entity);
	obj_motion.position += normal * collision_depth;


	if (!registry.blocked.has(object_entity))
	{
		std::cout<< "MIssing blocked ??" <<std::endl;
	} else
	{
		Blocked& blocked = registry.blocked.get(object_entity);
		blocked.normal = normal;
	}



	float vel_along_normal = dot(obj_motion.velocity, normal);

	if (vel_along_normal < 0)
	{
		obj_motion.velocity -= (vel_along_normal * normal);
	}

	float weight = DEFAULT_WEIGHT;

	if (registry.physicsObjects.has(object_entity)) {
		weight = registry.physicsObjects.get(object_entity).weight;
	}

	vec2 platform_velocity = registry.motions.has(platform_entity)
		  ? vec2(registry.motions.get(platform_entity).velocity.x, 0.0f)
		  : vec2(0.0f);

	if (-normal.y > (PLATFORM_SLIP_ANGLE * (M_PI / 180)))
	{
		bool is_moving_platform = registry.movementPaths.has(platform_entity);
		vec2 relative_vel = obj_motion.velocity - platform_velocity;

		vec2 new_relative_vel = get_friction(
			object_entity,
			relative_vel,
			normal,
			step_seconds,
			weight,
			is_moving_platform
		);

		obj_motion.velocity = new_relative_vel + platform_velocity;
		groundedEntities.push_back(object_entity.id());

		// Calculate fling velocity based on horizontal platform movement
		float platform_speed = abs(platform_velocity.x);
		if (platform_speed > 200.0f) {
			// Calculate surface tangent direction
			vec2 tangent = normalize(vec2(normal.y, -normal.x));

			// Fling in platform's movement direction scaled by surface alignment
			float surface_alignment = abs(dot(normalize(platform_velocity), tangent));
			obj_motion.velocity.x += platform_velocity.x * 0.4f * surface_alignment;
		}
	}

	if (registry.movementPaths.has(object_entity))
	{
		onMovingPlatform.push_back(object_entity.id());
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
// DISCLAIMER: I barely understand physics, but after questioning chatGPT and reading some basic articles, I put together this code which is a bit more rigerous than the previous iteration (and a lot cleaner)
void PhysicsSystem::handle_physics_collision(float step_seconds, Entity entityA, Entity entityB, Collision collision, std::vector<unsigned int>& grounded)
{
	Motion& motionA = registry.motions.get(entityA);
	Motion& motionB = registry.motions.get(entityB);

	PhysicsObject& physA = registry.physicsObjects.get(entityA);
	PhysicsObject& physB = registry.physicsObjects.get(entityB);

	const vec2 normal = collision.normal;
	const float collision_depth = length(collision.overlap);

	// resolve collision based on mass (way to avoid the ugly switch statement from previous iteration)
	// this also improves from previous code to properly handle the case where both objects are moving!!!
	float a_inv_weight = 1.0f / physA.weight;
	float b_inv_weight = 1.0f / physB.weight;
	const float total_inv_mass = a_inv_weight * b_inv_weight;
	motionA.position += normal * collision_depth * (a_inv_weight * total_inv_mass);
	motionB.position -= normal * collision_depth * (b_inv_weight * total_inv_mass); // - because normal is from A -> B

	// now get the relative velocities
	vec2 vel_relative = motionB.velocity - motionA.velocity;
	float vel_along_normal = dot(vel_relative, normal);


	// we only care if they are moving towards each other
	if (vel_along_normal > 0.0f)
	{
		return;
	}

	// compute the impulse
	float impulse_scalar = -(1 + PHYSICS_OBJECT_BOUNCE) * vel_along_normal;
	impulse_scalar /= total_inv_mass;

	vec2 impulse = impulse_scalar * normal;
	motionA.velocity -= impulse * a_inv_weight; // because normal points A->B but A is moving towards B
	motionB.velocity += impulse * b_inv_weight;

	vec2 friction_impulse = get_friction_impulse(vel_relative, total_inv_mass, impulse_scalar, normal);
	motionA.velocity -= friction_impulse;
	motionB.velocity += friction_impulse;

	// finally, detect if they are on the ground! (or an angled platform)
	if (dot (normal, {0,1}) > (PLATFORM_SLIP_ANGLE * (M_PI / 180)))
	{
		if(a_inv_weight > b_inv_weight) grounded.push_back(entityA.id());
		if(b_inv_weight >= a_inv_weight) grounded.push_back(entityB.id());
	}

}

// proper friction using coulomb's law
// https://www.tribonet.org/wiki/laws-of-friction/
vec2 PhysicsSystem::get_friction_impulse(vec2 relative_velocity, float total_inv_mass, float impulse_scalar, vec2 normal)
{
	vec2 friction_impulse = {0,0};
	vec2 tangent = (relative_velocity) - (normal * (relative_velocity));

	if (pow(length(tangent), 2) > 0.0001f)
	{
		tangent = normalize(tangent);

		float tangent_impulse_mag = -dot(relative_velocity, tangent);
		tangent_impulse_mag /= total_inv_mass;

		float mu = STATIC_FRICTION;
		friction_impulse = tangent * clamp(tangent_impulse_mag, -impulse_scalar * mu, impulse_scalar * mu);
	}
	return friction_impulse;
}

vec2 PhysicsSystem::get_friction(Entity& e, vec2& velocity, vec2& normal, float step_seconds, float weight, bool is_moving_platform) {
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

	float impulse = friction * (weight * GRAVITY) * step_seconds;
	impulse = std::min(impulse, tangent_speed);

	vec2 tangent_dir = normalize(velocity_tangent);
	return velocity - tangent_dir * impulse;
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
