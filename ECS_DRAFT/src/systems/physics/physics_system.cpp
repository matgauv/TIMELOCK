// internal
#include "physics_system.hpp"

#include <cfloat>

#include "../world/world_init.hpp"
#include "../player/player_system.hpp"
#include <iostream>
#include <glm/detail/func_trigonometric.inl>

void compute_platform_verticies(Motion& motion, Entity& e, float& angle_cos, float& angle_sin) {
	PlatformGeometry& geo = registry.platformGeometries.get(e);
	Mesh* edge_mesh = registry.meshPtrs.get(e);

	const float total_tiles = geo.num_tiles;
	const float platform_center_offset = total_tiles * 0.5f * TILE_TO_PIXELS;

	std::vector<vec2> platform_vertices;

	// left edge
	for (auto& v : edge_mesh->vertices) {
		vec2 pos = v.position;
		platform_vertices.emplace_back(pos / 3.75f);
	}

	// middle flat seciton of the platform
	const float middle_start = 1.0f;
	const float middle_end = total_tiles - 1.0f;
	platform_vertices.insert(platform_vertices.end(), {
		{middle_start, -0.5f},
		{middle_end, -0.5f},
		{middle_end, 0.5f},
		{middle_start, 0.5f}
	});

	// right edge (mirrored left edge)
	for (auto& v : edge_mesh->vertices) {
		v.position /= 3.75;
		platform_vertices.emplace_back(
						(geo.num_tiles) - v.position.x,
						v.position.y
		);
	}

	vec2 pos = motion.position;
	pos.x -= (platform_center_offset);

	for (auto& vert : platform_vertices) {
		vec2 scaled = vert * (float)TILE_TO_PIXELS;
		// Apply rotation
		vec2 rotated = {
			scaled.x * angle_cos - scaled.y * angle_sin,
			scaled.x * angle_sin + scaled.y * angle_cos
		};
		// Apply position
		motion.cached_vertices.push_back(rotated + pos);
	}
}
void compute_composite_mesh_vertices(Motion& motion, Entity& e)
{
    CompositeMesh& compositeMesh = registry.compositeMeshes.get(e);

    float parentAngleRad = radians(motion.angle);
    float cosParent      = cos(parentAngleRad);
    float sinParent      = sin(parentAngleRad);

    for (SubMesh& sub_mesh : compositeMesh.meshes)
    {
        if (motion.cache_invalidated || sub_mesh.cache_invalidated)
        {
            sub_mesh.cached_vertices.clear();
            sub_mesh.cached_axes.clear();


        	// local rotation is how to orient the mesh relative to parent (ie. angle of spike)
            float localAngleRad = radians(sub_mesh.rotation);
            float cosLocal      = cos(localAngleRad);
            float sinLocal      = sin(localAngleRad);


            vec2 rotated_offset = {
                sub_mesh.offset.x * cosParent - sub_mesh.offset.y * sinParent,
                sub_mesh.offset.x * sinParent + sub_mesh.offset.y * cosParent,
            };

            sub_mesh.world_pos = motion.position + rotated_offset;

            for (auto& vertex : sub_mesh.original_mesh->vertices)
            {
                vec2 localScaled = {
                    vertex.position.x * sub_mesh.scale_ratio.x,
                    vertex.position.y * sub_mesh.scale_ratio.y
                };

                vec2 localRotated = {
                    localScaled.x * cosLocal - localScaled.y * sinLocal,
                    localScaled.x * sinLocal + localScaled.y * cosLocal
                };

                vec2 parentScaled = {
                    localRotated.x * motion.scale.x,
                    localRotated.y * motion.scale.y
                };

                vec2 finalRotated = {
                    parentScaled.x * cosParent - parentScaled.y * sinParent,
                    parentScaled.x * sinParent + parentScaled.y * cosParent
                };

                vec2 vertexPos = finalRotated + sub_mesh.world_pos;

                sub_mesh.cached_vertices.push_back(vertexPos);
            }

            for (size_t i = 0; i < sub_mesh.cached_vertices.size(); i++)
            {
                vec2 curr = sub_mesh.cached_vertices[i];
                vec2 next = sub_mesh.cached_vertices[(i + 1) % sub_mesh.cached_vertices.size()];
                vec2 edge = next - curr;

                vec2 normal = normalize(vec2{-edge.y, edge.x});
                sub_mesh.cached_axes.push_back(normal);
            }

            sub_mesh.cache_invalidated = false;
        }
    }
}



void compute_vertices(Motion& motion, Entity& e) {
	motion.cached_vertices.clear();
	float angle_cos = cos(radians(motion.angle));
	float angle_sin = sin(radians(motion.angle));

	// compute the verticies for each of the sub meshes...
	if (registry.compositeMeshes.has(e)) return compute_composite_mesh_vertices(motion, e);


	if (registry.meshPtrs.has(e))
	{
		if (registry.platformGeometries.has(e)) return compute_platform_verticies(motion, e, angle_cos, angle_sin);

		Mesh* mesh = registry.meshPtrs.get(e);
		motion.cached_vertices.reserve( mesh->vertices.size() );

		for (auto & vertex : mesh->vertices)
		{
			vec2 scaled = {vertex.position.x * motion.scale.x, vertex.position.y * motion.scale.y};
			vec2 rotated = {
				scaled.x * angle_cos - scaled.y * angle_sin,
				scaled.x * angle_sin + scaled.y * angle_cos
			};
			vec2 vertex_pos = vec2{(rotated.x) + motion.position.x , (rotated.y) + motion.position.y};

			motion.cached_vertices.push_back(vertex_pos);
		}

	} else
	{
		// No mesh, we assume square BB
		motion.cached_vertices.resize(4);

		vec2 half_size = motion.scale * 0.5f;

		motion.cached_vertices[0] = { -half_size.x, -half_size.y };
		motion.cached_vertices[1] = {  half_size.x, -half_size.y };
		motion.cached_vertices[2] = {  half_size.x,  half_size.y };
		motion.cached_vertices[3] = { -half_size.x,  half_size.y };

		// if there's a rotation, we need to apply

		for (vec2& vertex : motion.cached_vertices)
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

		for (vec2& vertex : motion.cached_vertices)
		{
			vertex = vertex + motion.position;
		}
	}
}

// returns the normals of the edge between the verticies
void compute_axes(Motion& motion, std::vector<vec2>& vertices)
{
	motion.cached_axes.clear();
	motion.cached_axes.reserve(vertices.size());
	for (int i = 0; i < vertices.size(); i++)
	{
		vec2 edge = vertices[(i+1)%vertices.size()] - vertices[i]; // modulo so we properly wrap around
		vec2 normal = normalize(vec2{-edge.y, edge.x});
		motion.cached_axes.push_back(normal);
	}
}



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

	drop_bolt_when_player_near(DISTANCE_TO_DROP_BOLT);

	for (uint i = 0; i < registry.pendulums.size(); i++) {
		Entity entity = registry.pendulums.entities[i];
		update_pendulum(entity, step_seconds);
	}

	update_pendulum_rods();

	for(uint i = 0; i< motion_registry.size(); i++)
	{

		Motion& motion = motion_registry.components[i];
		Entity& entity = motion_registry.entities[i];

		// we handle the pendulum motion separately
		if (registry.pendulums.has(entity)) {
			continue;
		}

		if (registry.physicsObjects.has(entity)) {

			PhysicsObject& phys = registry.physicsObjects.get(entity);
			if (phys.apply_gravity) apply_gravity(entity, motion, step_seconds);

			if (phys.apply_rotation) {
                // Update angle
                motion.angle += degrees(phys.angular_velocity * step_seconds);
                motion.cache_invalidated = true;
				vec2 tangent = { -sin(motion.angle), cos(motion.angle) };
				float rotationFrictionFactor = 1.0f;
				vec2 angular_push = tangent * fabs(phys.angular_velocity) * rotationFrictionFactor;
				motion.position += angular_push * step_seconds;
                phys.angular_velocity *= (1.0f - 0.95f * step_seconds); // Damping factor
			}

		}



		if (registry.players.has(entity)	) {
			player_walk(entity, motion, step_seconds);
			player_climb(entity, motion, step_seconds);
		}

		if (registry.movementPaths.has(entity)) {
			move_object_along_path(entity, motion, step_seconds);
		}

		vec2 oldMotion = motion.position;
		motion.position += (motion.velocity * motion.velocityModifier) * step_seconds;

		// invalidate the cached vertices of the motion moved
		if (oldMotion != motion.position) {
			motion.cache_invalidated = true;
		}
	}

	// clear blocked...
	for (Entity& entity : registry.physicsObjects.entities) {
		if (registry.blocked.has(entity)) {
			Blocked& blocked = registry.blocked.get(entity);
			blocked.normal = vec2{0, 0};
		}
	}

	for (Entity entity : registry.motions.entities) {
		Motion& motion = registry.motions.get(entity);
		if (motion.cache_invalidated) {
			compute_vertices(motion, entity);
			compute_axes(motion, motion.cached_vertices);
			motion.cache_invalidated = false; // Reset flag
		}
	}

	detect_collisions();
	handle_collisions(elapsed_ms);
}

// Handles all collisions detected in PhysicsSystem::step
void PhysicsSystem::late_step(float elapsed_ms) {

}




// returns the mesh verticies if a mesh exists, otherwise, returns verticies of a square defined by the scale of the object (BB)
std::vector<vec2>& get_vertices(Entity& e) {
	Motion& motion = registry.motions.get(e);
	return motion.cached_vertices;
}


std::vector<vec2>& get_axes(Entity& e)
{
	Motion& motion = registry.motions.get(e);
	return motion.cached_axes;
}

// projects the verticies onto the axis
// only return the min and max since the object is convex and the axis is 1 dimensional
std::pair<float, float> project(const std::vector<vec2>& verts, const vec2& axis) {
	const size_t num_verts = verts.size();
	if (num_verts == 0) {
		return {0.0f, 0.0f};
	}

	const float ax = axis.x;
	const float ay = axis.y;
	const vec2* verts_ptr = verts.data();

	float min_proj = ax * verts_ptr[0].x + ay * verts_ptr[0].y;
	float max_proj = min_proj;

	for (size_t i = 1; i < num_verts; ++i) {
		const vec2& v = verts_ptr[i];
		const float proj = ax * v.x + ay * v.y;
		if (proj < min_proj) {
			min_proj = proj;
		} else if (proj > max_proj) {
			max_proj = proj;
		}
	}

	return {min_proj, max_proj};
}

// see here for details https://dyn4j.org/2010/01/sat/
// computes collision between any two shapes, must be convex!
Collision compute_convex_collision(const std::vector<vec2>& a_verts, const std::vector<vec2>& a_axes, const std::vector<vec2>& b_verts, const std::vector<vec2>& b_axes, const vec2& a_position, const vec2& b_position, Entity a, Entity b) {
	float min_overlap = FLT_MAX;
	vec2 smallest_axis;


	// SAT time!! (perform the main SAT check)
	// For every axis:
	//	1. project each shape onto the axis
	//  2. check overlap on the axis
	for (const auto& axis : a_axes)
	{
		auto [a_min, a_max] = project(a_verts, axis);
		auto [b_min, b_max] = project(b_verts, axis);

		// by the theorem:
		// if there is any axis where the projections do not overlap, there cannot be a collision. :O
		if (a_max < b_min || b_max < a_min)
		{
			return Collision{b.id(), vec2{0,0}, vec2{0,0}};
		}

		// min and max so that we handle the case where there one is fully contained in the other
		float overlap = std::min(a_max, b_max) - std::max(a_min, b_min);

		if (overlap < min_overlap)
		{
			min_overlap = overlap;
			smallest_axis = axis;
		}
	}

	for (const auto& axis : b_axes)
	{
		auto [a_min, a_max] = project(a_verts, axis);
		auto [b_min, b_max] = project(b_verts, axis);

		// by the theorem:
		// if there is any axis where the projections do not overlap, there cannot be a collision. :O
		if (a_max < b_min || b_max < a_min)
		{
			return Collision{b.id(), vec2{0,0}, vec2{0,0}};
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
	vec2 direction = a_position - b_position;
	if (dot(direction, smallest_axis) < 0) {
		smallest_axis *= -1;
	}

	// the smallest axis is where the shapes overlap the least ->
	// the minimal amount we need to move one shape to resolve the collision
	// (the axis are represented by the normals)
	return Collision{ b.id(), min_overlap * smallest_axis, smallest_axis };
}

// convex decomposition for SAT -> basically just check collision between each submesh
Collision compute_sat_collision(Motion& a_motion, Motion& b_motion, Entity a, Entity b) {
	Collision strongest_collision{b.id(), vec2{0,0}, vec2{0,0}};
	float max_overlap_length = 0.0f;

	// small helper method that just checks the collision between two meshes
	auto check_submesh_collision = [&](
		const std::vector<vec2>& verts_a, const std::vector<vec2>& axes_a, vec2 pos_a,
		const std::vector<vec2>& verts_b, const std::vector<vec2>& axes_b, vec2 pos_b
	) {
		Collision collision = compute_convex_collision(verts_a, axes_a, verts_b, axes_b, pos_a, pos_b, a, b);

		if (collision.normal != vec2{0, 0}) {
			float overlap = length(collision.overlap);
			if (overlap > max_overlap_length) {
				max_overlap_length = overlap;
				strongest_collision = collision;
			}
		}
	};

	bool a_is_composite = registry.compositeMeshes.has(a);
	bool b_is_composite = registry.compositeMeshes.has(b);

	// Both are composite meshes
	if (a_is_composite && b_is_composite) {
		CompositeMesh& a_composite = registry.compositeMeshes.get(a);
		CompositeMesh& b_composite = registry.compositeMeshes.get(b);

		// here we must loop through each submesh of A and check for collision between each submesh of B
		for (const SubMesh& a_sub_mesh : a_composite.meshes) {
			for (const SubMesh& b_sub_mesh : b_composite.meshes) {
				check_submesh_collision(
					a_sub_mesh.cached_vertices, a_sub_mesh.cached_axes, a_sub_mesh.world_pos,
					b_sub_mesh.cached_vertices, b_sub_mesh.cached_axes, b_sub_mesh.world_pos);
			}
		}
	}

	// A is composite, B is not
	else if (a_is_composite) {
		CompositeMesh& a_composite = registry.compositeMeshes.get(a);

		// now we just have to check all of the submeshes of A against B
		for (const SubMesh& a_submesh : a_composite.meshes) {
			check_submesh_collision(
				a_submesh.cached_vertices, a_submesh.cached_axes, a_submesh.world_pos,
				b_motion.cached_vertices, b_motion.cached_axes, b_motion.position
			);
		}
	}

	// B is composite, A is not
	else if (b_is_composite) {
		CompositeMesh& b_composite = registry.compositeMeshes.get(b);

		// check sub meshes of B against A
		for (const SubMesh& b_submesh : b_composite.meshes) {
			check_submesh_collision(
				a_motion.cached_vertices, a_motion.cached_axes, a_motion.position,
				b_submesh.cached_vertices, b_submesh.cached_axes, b_submesh.world_pos
			);
		}
	}

	// otheriwse, just a normal convex collision
	else {
		check_submesh_collision(
		 a_motion.cached_vertices, a_motion.cached_axes, a_motion.position,
		 b_motion.cached_vertices, b_motion.cached_axes, b_motion.position
		 );
	}

	return strongest_collision;
}

bool compute_AABB_collision(const Motion& a_motion, const Motion& b_motion)
{
	vec2 a_half = a_motion.scale * 0.5f;
	vec2 b_half = b_motion.scale * 0.5f;

	// A's BB
	float a_min_x = a_motion.position.x - a_half.x;
	float a_max_x = a_motion.position.x + a_half.x;
	float a_min_y = a_motion.position.y - a_half.y;
	float a_max_y = a_motion.position.y + a_half.y;

	// B's BB
	float b_min_x = b_motion.position.x - b_half.x;
	float b_max_x = b_motion.position.x + b_half.x;
	float b_min_y = b_motion.position.y - b_half.y;
	float b_max_y = b_motion.position.y + b_half.y;

	if (a_max_x < b_min_x || a_min_x > b_max_x)
		return false;
	if (a_max_y < b_min_y || a_min_y > b_max_y)
		return false;

	return true;
}

void collision_check(Entity& entity_i, Motion& motion_i, Entity& entity_j) {
	if (entity_i.id() == entity_j.id()) return;

	Motion& motion_j = registry.motions.get(entity_j);
	if (!compute_AABB_collision(motion_i, motion_j)) return;

	Collision result = compute_sat_collision(motion_i, motion_j, entity_i, entity_j);

	if (result.normal != vec2{0, 0}) {
		registry.collisions.emplace_with_duplicates(entity_i, entity_j, result.overlap, result.normal);
	}
}

// detect collisions between all moving entities.
void PhysicsSystem::detect_collisions() {
	auto& physics_objects = registry.physicsObjects;
	auto& platform_container = registry.platforms;

	for (uint i = 0; i < physics_objects.size(); ++i) {
		Entity entity_i = physics_objects.entities[i];
		Motion& motion_i = registry.motions.get(entity_i);

		// check between each object and each platform
		for (uint j = 0; j < platform_container.size(); ++j) {
			Entity entity_j = platform_container.entities[j];
			collision_check(entity_i, motion_i, entity_j);
		}

		// check between each physics object and each other
		for (uint j = i+1; j < physics_objects.size(); ++j) {
			Entity entity_j = physics_objects.entities[j];
			collision_check(entity_i, motion_i, entity_j);
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
	motion.cache_invalidated = true;
}

// pendulum physics based off of this https://www.acs.psu.edu/drussell/Demos/Pendulum/Pendulum.html
//									  https://stackoverflow.com/questions/34819949/modelling-a-pendulum-in-c
// basically, just the equation: d²θ/dt² = -(g/L)sin(θ))
void PhysicsSystem::update_pendulum(Entity& entity, float step_seconds) {
	Pendulum& pendulum = registry.pendulums.get(entity);
	Motion& motion = registry.motions.get(entity);

	float g = (GRAVITY) / pendulum.length;
	float angular_accel = -g * sin(pendulum.current_angle);

	float modified_step_seconds = step_seconds * motion.velocityModifier;

	pendulum.angular_velocity += angular_accel * modified_step_seconds;
	pendulum.angular_velocity *= (1.0f - pendulum.damping * modified_step_seconds); // will slow the pendulum down

	// angle is tracked separately so the sprite does not rotate
	pendulum.current_angle += pendulum.angular_velocity * modified_step_seconds;

	motion.position.x = pendulum.pivot_point.x + pendulum.length * sin(pendulum.current_angle);
	motion.position.y = pendulum.pivot_point.y + pendulum.length * cos(pendulum.current_angle);

	// need the x and y velocity for collision handling
	motion.velocity.x = pendulum.angular_velocity * pendulum.length * cos(pendulum.current_angle);
	motion.velocity.y = -pendulum.angular_velocity * pendulum.length * sin(pendulum.current_angle);
	motion.cache_invalidated = true;
}

// after updating all of the pendulum bobs, also update their rods
void PhysicsSystem::update_pendulum_rods() {
	for (uint i = 0; i < registry.pendulumRods.size(); i++) {
		Entity rod_entity = registry.pendulumRods.entities[i];
		PendulumRod& rod = registry.pendulumRods.components[i];

		Entity bob_entity = Entity(rod.bob_id);
		if (!registry.pendulums.has(bob_entity))
			continue;

		Pendulum& pendulum = registry.pendulums.get(bob_entity);
		Motion& bob_motion = registry.motions.get(bob_entity);
		Motion& rod_motion = registry.motions.get(rod_entity);

		//rod_motion.position = pendulum.pivot_point;

		float length = glm::length(bob_motion.position - pendulum.pivot_point);
		rod_motion.scale.y = length;

		float angle = atan2(bob_motion.position.y - pendulum.pivot_point.y,
						   bob_motion.position.x - pendulum.pivot_point.x);

		rod_motion.angle = angle * (180 / M_PI) - 90.0f;

		rod_motion.cache_invalidated = true;

	}
}

// accelerates the entity by GRAVITY until it reaches the max_speed (terminal velocity)
// TODO:: more terminal velocities
void PhysicsSystem::apply_gravity(Entity& entity, Motion& motion, float step_seconds) {
	if (registry.climbing.has(entity)) return;

	float max_fall_speed = OBJECT_MAX_FALLING_SPEED;
	float gravity = GRAVITY;

	if (registry.players.has(entity)) {
		max_fall_speed = PLAYER_MAX_FALLING_SPEED;

		if (motion.velocity.y < 0.0f) gravity = GRAVITY_JUMP_ASCENT;
	}

	motion.velocity.y = clampToTarget(motion.velocity.y, gravity * step_seconds, max_fall_speed);
}

float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

void PhysicsSystem::player_climb(Entity& entity, Motion& motion, float step_seconds) {
	if (!registry.climbing.has(entity)) return;

	Climbing& climbing = registry.climbing.get(entity);

	if (climbing.is_up) {
		motion.velocity.y = lerp(motion.velocity.y, -PLAYER_CLIMBING_SPEED, step_seconds * 1.5f);

		if (motion.velocity.y < -PLAYER_CLIMBING_SPEED) {
			motion.velocity.y = -PLAYER_CLIMBING_SPEED;
		}

	} else {
		motion.velocity.y = lerp(motion.velocity.y, PLAYER_CLIMBING_SPEED, step_seconds * 1.5f);

		if (motion.velocity.y > PLAYER_CLIMBING_SPEED) {
			motion.velocity.y = PLAYER_CLIMBING_SPEED;
		}
	}
}

// Moves the player left or right depending on the direction specified in the walking component
// will not move if blocked
void PhysicsSystem::player_walk(Entity& entity, Motion& motion, float step_seconds)
{
	if (registry.walking.has(entity))
	{

		float acceleration = PLAYER_WALK_ACCELERATION;
		if (registry.climbing.has(entity)) {
			acceleration = PLAYER_WALK_LADDER_ACCELERATION;
		}


		Walking& walking = registry.walking.get(entity);
		vec2 desired_direction = {walking.is_left ? -1.0f : 1.0f, 0.0f};
		vec2 accel = desired_direction * acceleration;

		vec2 platform_velocity = {0.0f, 0.0f};
		if (registry.onGrounds.has(entity)) {
			unsigned int ground_id = registry.onGrounds.get(entity).other_id;
			Motion& platform_motion = registry.motions.get(ground_id);
			platform_velocity = get_modified_velocity(platform_motion);
		}




		vec2 rel_velocity = motion.velocity - platform_velocity;

		// accelerate on a curve of x^2 instead of linear
		float normalizedSpeed = fabs(rel_velocity.x) / PLAYER_MAX_WALKING_SPEED;
		float interpFactor = 1.0f - (normalizedSpeed * normalizedSpeed);
		rel_velocity += desired_direction * acceleration * interpFactor * step_seconds;

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

	bool player_ladder_collision = false;


	for (uint i = 0; i < collision_container.components.size(); i++) {
		Entity& one = collision_container.entities[i];
		Collision& collision = collision_container.components[i];
		Entity other = Entity(collision.other_id);

		// if player hits a breakable platform
		if (registry.players.has(one) && registry.breakables.has(other)) {
			handle_player_breakable_collision(one, other, collision);
		} else if (registry.players.has(other) && registry.breakables.has(one)) {
			handle_player_breakable_collision(other, one, collision);
		}

		// order here is important so handle both cases sep
		if (registry.physicsObjects.has(one) && registry.platforms.has(other)) {
			handle_object_rigid_collision(one, other, collision, step_seconds, groundedEntities);
		} else if (registry.physicsObjects.has(other) && registry.platforms.has(one)) {
			collision.normal *= -1;
			handle_object_rigid_collision(other, one, collision, step_seconds, groundedEntities);
		}


		if (registry.players.has(one) && registry.projectiles.has(other)) {
			// TODO: should handle_player_projectile_collision() be handle_player_attack_collision() ?
			// TODO: should leave all events that kill player to collision with harmful entities

			handle_player_attack_collision(one, other, collision);
		} else if (registry.players.has(other) && registry.projectiles.has(one)) {
			handle_player_attack_collision(other, one, collision);
		}

		GameState& gameState = registry.gameStates.components[0];
		// Very coarse method of eliminating projectiles;
		// Should consider:
		// - Bouncing projectiles (if exists);
		// - Projectile dying effect (need particle system);
		if (registry.projectiles.has(one)) {
			handle_projectile_collision(one, other);
		}
		else if (registry.projectiles.has(other)) {
			handle_projectile_collision(other, one);
		}

		if (registry.players.has(one) && registry.bosses.has(other)) {
			handle_player_boss_collision(one, other, collision);
		} else if (registry.players.has(other) && registry.bosses.has(one)) {
			handle_player_boss_collision(other, one, collision);
		}

		// if player touches boundary or spike, reset the game
		if (is_collision_between_player_and_boundary(one, other) || is_collision_between_player_and_spike(one, other)) {
			PlayerSystem::kill();
		}

		// if objects touch the boundary, remove them
		// if (registry.physicsObjects.has(one) && registry.boundaries.has(other)) {
		// 	registry.remove_all_components_of(one);
		// } else if (registry.physicsObjects.has(other) && registry.boundaries.has(one)) {
		// 	registry.remove_all_components_of(other);
		// }

		if (registry.players.has(one) && registry.ladders.has(other)) {
			handle_player_ladder_collision(one, other, step_seconds);
			player_ladder_collision = true;
		} else if (registry.players.has(other) && registry.ladders.has(one)) {
			handle_player_ladder_collision(other, one, step_seconds);
			player_ladder_collision = true;
		}

		if (registry.physicsObjects.has(one) && registry.physicsObjects.has(other)) {
			handle_physics_collision(step_seconds, one, other, collision, groundedEntities);
		}

	}
	for (int i = 0; i < registry.physicsObjects.entities.size(); i++){
		Entity& entity = registry.physicsObjects.entities[i];


		if(!in(groundedEntities, entity.id())) {
			registry.onGrounds.remove(entity);
			Motion& motion = registry.motions.get(entity);
			apply_air_resistance(entity, motion, step_seconds);
		}
	}

	if (!player_ladder_collision) {
		registry.climbing.remove(registry.players.entities[0]);
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

void PhysicsSystem::handle_projectile_collision(Entity proj_entity, Entity other_entity) {
	// If not harmful, don't remove projectile upon player collision
	if (!registry.harmfuls.has(proj_entity) /* && registry.players.has(other_entity)*/) {
		return;
	}

	// Upon colliding with physicsObjects (other projectiles, player) / platforms
	if (registry.physicsObjects.has(other_entity) || (registry.platforms.has(other_entity))) {
		// TODO: add more effects to killing projectiles, likely based on types
		registry.remove_all_components_of(proj_entity);
	}
}

void PhysicsSystem::handle_player_ladder_collision(Entity& player_entity, Entity& ladder_entity, int step_seconds) {
	Motion& player_motion = registry.motions.get(player_entity);
	Motion& ladder_motion = registry.motions.get(ladder_entity);

	float ladder_top = ladder_motion.position.y - ladder_motion.scale.y/2;
	float player_bottom = player_motion.position.y + player_motion.scale.y/2;

	if (player_bottom > ladder_top) {
		if (!registry.climbing.has(player_entity)) {
			registry.climbing.emplace(player_entity);
		}

		player_motion.velocity.x *= 0.99f;

		if (player_bottom < ladder_top + TILE_TO_PIXELS/2.0f) {
			player_motion.velocity.y = std::max(player_motion.velocity.y, 0.0f);
		}

	} else {
		if (registry.climbing.has(player_entity)) {
			if (player_motion.velocity.y < 0) {
				player_motion.velocity.y = 0;
			}
			registry.climbing.remove(player_entity);
		}
	}
}

// Handles collision between a PhysicsObject entity and a Platform entity.
void PhysicsSystem::handle_object_rigid_collision(Entity& object_entity, Entity& platform_entity, Collision collision, float step_seconds, std::vector<unsigned int>& groundedEntities)
{

	// Blocked& blocked = registry.blocked.get(object_entity);
	Motion& obj_motion = registry.motions.get(object_entity);

	vec2 normal = collision.normal;
	// blocked.normal = normal;

	resolve_collision_position(object_entity, platform_entity, collision);
	handle_rotational_dynamics(object_entity, platform_entity, collision.normal, step_seconds);

	adjust_velocity_along_normal(obj_motion, normal);

	float mass = DEFAULT_MASS;
	if (registry.physicsObjects.has(object_entity)) {
		mass = registry.physicsObjects.get(object_entity).mass;
	}

	Motion& platform_motion = registry.motions.get(platform_entity);
	vec2 platform_velocity = get_modified_velocity(platform_motion);


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
			//vec2 tangent = normalize(vec2(-normal.y, normal.x));

			// Fling in platform's movement direction scaled by surface alignment
		//	float surface_alignment = abs(dot(normalize(platform_velocity), tangent));
			obj_motion.velocity += vec2{platform_velocity.x * 0.05f, platform_velocity.y * 0.005f};


			// TODO I wanted to avoid explicitly doing this, but it works
			float relative_y = obj_motion.velocity.y - platform_velocity.y;
			if (fabs(relative_y) > 5.0f) {
				obj_motion.velocity.y = platform_velocity.y;
			}
		}


	}

	if (is_on_ground(-normal.y)) {
		groundedEntities.push_back(object_entity);
		if (registry.onGrounds.has(object_entity)) {
			registry.onGrounds.get(object_entity).other_id = platform_entity.id();
		} else {
			registry.onGrounds.emplace(object_entity, platform_entity.id());
		}

		if (registry.players.has(object_entity)) {
			PlayerSystem::set_jumping_validity(true);
		}


	} else {

	}
}

// TODO: prob should invest in a more robust method here, currently just approximating everything as a rectangle.
float calculate_moment_of_inertia(Entity entity) {
	Motion& motion = registry.motions.get(entity);
	PhysicsObject& phys = registry.physicsObjects.get(entity);

	if (phys.moment_of_inertia == 0.0f) {
		if (registry.compositeMeshes.has(entity)) {
			float total = 0.0f;
			CompositeMesh& composite = registry.compositeMeshes.get(entity);
			for (SubMesh& submesh : composite.meshes) {
				vec2 size = motion.scale * submesh.scale_ratio;
				// Distance from submesh's center to composite's center of mass
				vec2 offset = submesh.world_pos - motion.position;
				float distance_sq = dot(offset, offset);
				// Moment of inertia for this submesh (rectangle) + parallel axis term
				float submesh_mass = phys.mass;
				total += (1.0f/12.0f) * submesh_mass * (size.x*size.x + size.y*size.y)
					   + submesh_mass * distance_sq;
			}
			phys.moment_of_inertia = total;
			return total;
		} else {
			vec2 size = motion.scale;
			float total =  (1.0f/12.0f) * phys.mass * (size.x*size.x + size.y*size.y);
			phys.moment_of_inertia = total;
			return total;
		}
	}
	return phys.moment_of_inertia;
}

// helper function that returns the closest point to p along the segment a<->b
vec2 closest_point_on_segment(const vec2& p, const vec2& a, const vec2& b) {
	vec2 ab = b - a;
	float t = dot(p - a, ab) / dot(ab, ab);
	t = clamp(t, 0.0f, 1.0f);
	return a + t * ab;
}

// process rotational dynamics!
void PhysicsSystem::handle_rotational_dynamics(Entity& object_entity, Entity& platform_entity, const vec2& collision_normal, float step_seconds) {
	Motion& obj_motion = registry.motions.get(object_entity);
	PhysicsObject& phys = registry.physicsObjects.get(object_entity);

	if (!phys.apply_rotation) return;

	Motion& platform_motion = registry.motions.get(platform_entity);
	vec2 platform_normal = collision_normal;
	vec2 tangent = normalize(vec2(platform_normal.y, -platform_normal.x));
	vec2 platform_pos = platform_motion.position;

	// get all the verticies (including for composite meshes)
	std::vector<vec2> object_vertices;
	if (registry.compositeMeshes.has(object_entity)) {
		CompositeMesh& composite = registry.compositeMeshes.get(object_entity);
		for (SubMesh& submesh : composite.meshes) {
			for (const vec2& v : submesh.cached_vertices) {
				object_vertices.push_back(v);
			}
		}
	} else {
		object_vertices = obj_motion.cached_vertices;
	}


	// determine candidate contact points
	const float contact_threshold = 3.0f; // pixel threshold to find more ontacts
	std::vector<vec2> contact_points;
	for (const vec2& obj_v : object_vertices) {
		float min_dist = FLT_MAX;
		const std::vector<vec2>& platform_verts = platform_motion.cached_vertices;
		platform_motion.cache_invalidated = true;

		// finding the closest platform edge to the vertex
		for (size_t i = 0; i < platform_verts.size(); i++) {
			vec2 edge_start = platform_verts[i];
			vec2 edge_end = platform_verts[(i+1)%platform_verts.size()];
			vec2 edge_closest = closest_point_on_segment(obj_v, edge_start, edge_end);
			float dist = distance(obj_v, edge_closest);
			if (dist < min_dist) min_dist = dist;
		}


		if (min_dist < contact_threshold) contact_points.push_back(obj_v);
	}

	if (contact_points.empty()) return;

	// since we used a generous threshold, we often end up with too many contact points
	// so we filter for the deepest two contact points

	// project the contacts onto the platform's normal to find the deepest points
	std::vector<std::pair<float, vec2>> projected_contacts;
	for (const vec2& p : contact_points) {
		float proj = dot(p - platform_pos, platform_normal);
		projected_contacts.emplace_back(proj, p);
	}
	std::sort(projected_contacts.begin(), projected_contacts.end(),
			  [](const auto& a, const auto& b) { return a.first < b.first; });

	std::vector<vec2> selected_contacts;

	// take up to two deepest contact points
	for (size_t i = 0; i < std::min(projected_contacts.size(), size_t(2)); i++) {
		selected_contacts.push_back(projected_contacts[i].second);
	}

	if (selected_contacts.empty())
		return;

	// project the contact points to determine the support area of the object
	vec2 min_pivot_point = selected_contacts[0];
	vec2 max_pivot_point = selected_contacts[0];
	float min_contact_x = FLT_MAX;
	float max_contact_x = -FLT_MAX;

	for (size_t i = 1; i < selected_contacts.size(); i++) {
		vec2 p = selected_contacts[i];
		float projected = dot(p - platform_pos, tangent);

		if (projected < min_contact_x) {
			min_contact_x = projected;
			min_pivot_point = p;
		}
		if (projected > max_contact_x) {
			max_contact_x = projected;
			max_pivot_point = p;
		}
	}


	vec2 com = obj_motion.position;
	vec2 rel_com = com - platform_pos;
	float projected_com = dot(rel_com, tangent);
	vec2 pivot;

	// check if the com falls within the support pivot points
	if (projected_com > min_contact_x && projected_com < max_contact_x) {
		// damp motion if stable
		phys.angular_velocity *= 0.02;
		return;
	} else if (projected_com >= max_contact_x) {
		pivot = max_pivot_point;
	} else {
		pivot = min_pivot_point;
	}


	// otherwise, if unstable, compute torque based on lever arm defined by pivot and center of mass
	vec2 lever_arm_vec = com - pivot;
	vec2 gravity_force = vec2(0, phys.mass * -GRAVITY);


	// (cross product can be simplified in 2D) torque = r x F, since Fx = 0 we can ignore that term
	float torque = -(lever_arm_vec.x * gravity_force.y);


	float moment_of_inertia = calculate_moment_of_inertia(object_entity);


	if (moment_of_inertia > 0) {
		float angular_acceleration = torque / moment_of_inertia;
		phys.angular_velocity += angular_acceleration * step_seconds;
	}
}

void PhysicsSystem::handle_player_attack_collision(Entity& player_entity, Entity& attack_entity, Collision collision) {
	GameState& gameState = registry.gameStates.components[0];

	// TODO: make this part of logic consistent with WorldSystem::control_time
	if ((registry.harmfuls.has(attack_entity))) {
		assert(registry.gameStates.components.size() <= 1);
		GameState& gameState = registry.gameStates.components[0];
		gameState.game_running_state = GAME_RUNNING_STATE::OVER; // might not be necessary

		PlayerSystem::kill();
	}
}

void PhysicsSystem::handle_player_boss_collision(Entity& player_entity, Entity& boss_entity, Collision collision) {
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
void PhysicsSystem::handle_physics_collision(float step_seconds, Entity& entityA, Entity& entityB, Collision collision, std::vector<unsigned int>& grounded)
{
	Motion& motionA = registry.motions.get(entityA);
	Motion& motionB = registry.motions.get(entityB);

	PhysicsObject& physA = registry.physicsObjects.get(entityA);
	PhysicsObject& physB = registry.physicsObjects.get(entityB);

	vec2 normal = collision.normal;

	vec2 posDiff = motionB.position - motionA.position;
	if (dot(normal, posDiff) < 0) {
		normal = -normal;
	}


	// prepare inverse masses
	float a_inv_mass = (1.0f / physA.mass);
	float b_inv_mass = (1.0f / physB.mass);
	float total_inv_mass = a_inv_mass + b_inv_mass;
	resolve_collision_position(entityA, entityB, collision);

	// detect if they are on the ground! (or an angled platform)
	if (is_on_ground(normal.y))
	{
		grounded.push_back(entityA.id());
		if (!registry.onGrounds.has(entityA)) {
			registry.onGrounds.emplace(entityA, entityB.id());
		}

		if (registry.players.has(entityA)) {
			PlayerSystem::set_jumping_validity(true);
		}
	}

	if (is_on_ground(-normal.y))
	{
		grounded.push_back(entityB.id());
		if (!registry.onGrounds.has(entityB)) {
			registry.onGrounds.emplace(entityB, entityA.id());
		}

		if (registry.players.has(entityB)) {
			PlayerSystem::set_jumping_validity(true);
		}
	}

	// approximate contact as midpoint between two motions TODO: more robust?
	vec2 contact_point = 0.5f * (motionA.position + motionB.position);

	// vectors from center of mass (assuming center for all objects)
	vec2 contact_offset_a = contact_point - motionA.position;
	vec2 contact_offset_b = contact_point - motionB.position;

	//
	float lever_arm_A_perp = contact_offset_a.x * normal.y - contact_offset_a.y * normal.x;
	float lever_arm_B_perp = contact_offset_b.x * normal.y - contact_offset_b.y * normal.x;

	float A_ang_term = physA.apply_rotation ? physA.angular_velocity * lever_arm_A_perp : 0.0f;
	float B_ang_term = physB.apply_rotation ? physB.angular_velocity * lever_arm_B_perp : 0.0f;
	float A_vel_towards_collision = dot(motionA.velocity, normal) + A_ang_term;
	float B_vel_towards_collision = dot(motionB.velocity, normal) + B_ang_term;
	float vel_along_normal = B_vel_towards_collision - A_vel_towards_collision;

	if (vel_along_normal > 0.0f) return;

	// compute the impulse from the collision (relative to center of mass)
	float impulse_magnitude = -(1.0f + PHYSICS_OBJECT_BOUNCE) * vel_along_normal;
	float a_inv_inertia = (physA.apply_rotation) ? 1.0f / calculate_moment_of_inertia(entityA) : 0.0f;
	float b_inv_inertia = (physB.apply_rotation) ? 1.0f / calculate_moment_of_inertia(entityB) : 0.0f;
	float impulse_denom = total_inv_mass + (lever_arm_A_perp * lever_arm_A_perp) * a_inv_inertia + (lever_arm_B_perp * lever_arm_B_perp) * b_inv_inertia;

	if (impulse_denom == 0.0f) return;

	impulse_magnitude /= impulse_denom;
	vec2 impulse = impulse_magnitude * normal;

	// update velocity with the impulse and angular velocities
	motionA.velocity -= impulse * a_inv_mass; // because normal points A->B but A is moving towards B
	motionB.velocity += impulse * b_inv_mass;


	if (physA.apply_rotation) {
		physA.angular_velocity -= impulse_magnitude * lever_arm_A_perp * a_inv_inertia;
	}
	if (physB.apply_rotation) {
		physB.angular_velocity += impulse_magnitude * lever_arm_B_perp * b_inv_inertia;
	}

	// friction
	vec2 rel_velocity = motionB.velocity - motionA.velocity;
	vec2 tangent = rel_velocity - dot(rel_velocity, normal) * normal;
	float tan_len = length(tangent);

	if (tan_len < 0.001f) return;

	vec2 friction_dir = tangent / tan_len;

	float tangent_vel_A = dot(motionA.velocity, friction_dir);
	if (physA.apply_rotation) {
		tangent_vel_A += physA.angular_velocity * (contact_offset_a.x * friction_dir.y - contact_offset_a.y * friction_dir.x);
	}
	float tangent_vel_B = dot(motionB.velocity, friction_dir);
	if (physB.apply_rotation) {
		tangent_vel_B += physB.angular_velocity * (contact_offset_b.x * friction_dir.y - contact_offset_b.y * friction_dir.x);
	}
	float rel_tan_velocity = tangent_vel_B - tangent_vel_A;

	float friction = sqrt(physA.friction * physB.friction);
	float tangent_impulse_scalar = -rel_tan_velocity / impulse_denom;
	tangent_impulse_scalar = clamp(tangent_impulse_scalar, -impulse_magnitude * friction, impulse_magnitude * friction); // clamp to coloumb's law

	vec2 tangent_impulse = friction_dir * tangent_impulse_scalar;
	motionA.velocity -= tangent_impulse * a_inv_mass;
	motionB.velocity += tangent_impulse * b_inv_mass;


	// angular friction
	if (physA.apply_rotation) {
		float tangential_lever_a = contact_offset_a.x * friction_dir.y - contact_offset_a.y * friction_dir.x;
		physA.angular_velocity -= tangent_impulse_scalar * tangential_lever_a * a_inv_inertia;
		handle_rotational_dynamics(entityA, entityB, collision.normal, step_seconds);
	}
	if (physB.apply_rotation) {
		float tangential_lever_b = contact_offset_b.x * friction_dir.y - contact_offset_b.y * friction_dir.x;
		physB.angular_velocity += tangent_impulse_scalar * tangential_lever_b * b_inv_inertia;
		handle_rotational_dynamics(entityB, entityA, -collision.normal, step_seconds);
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

	float friction = registry.physicsObjects.get(e).friction;
	if (registry.walking.has(e)) {
		Walking& walking = registry.walking.get(e);
		if ((walking.is_left && velocity.x < 0.0f) || (!walking.is_left && velocity.x > 0.0f)) {
			friction = DYNAMIC_FRICTION;
		}
	}

	if (is_moving_platform) {
		friction *= 3.0f;
	}

	// friction normal force is determined by mass * gravity
	float impulse = friction * (GRAVITY * 10.0f) * step_seconds;
	impulse = std::min(impulse, tangent_speed);

	vec2 tangent_dir = normalize(velocity_tangent);
	return velocity - tangent_dir * impulse;
}

void PhysicsSystem::apply_air_resistance(Entity entity, Motion& motion, float step_seconds)
{
	if (!registry.physicsObjects.has(entity)) return;

	PhysicsObject& physics = registry.physicsObjects.get(entity);
	vec2 velocity = motion.velocity;
	float speed_squared = dot(velocity, velocity);
	if (speed_squared <= 0.0f) return;

	float drag = physics.drag_coefficient;
	float speed = sqrt(speed_squared);

	// compute area relative to direciton we are moving (ie. falling down only care about y scale)
	float effective_area = (fabs(velocity.x) / speed) * motion.scale.y +
						   (fabs(velocity.y) / speed) * motion.scale.x;

	float magnitude = 0.5 * AIR_DENSITY * speed_squared * drag * effective_area;

	vec2 direction = -normalize(velocity);
	vec2 acceleration = (direction * magnitude) / physics.mass; // divide by mass since F=ma

	motion.velocity += acceleration * step_seconds;
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
	const float epsilon = 0.0001f * sign(collision.normal.y);
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


// apply gravity to any bolt that is within dist_threshold
void PhysicsSystem::drop_bolt_when_player_near(float dist_threshold) {
	Entity& player = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player);

	for (Entity& bolt : registry.bolts.entities)
	{
		Motion& bolt_motion = registry.motions.get(bolt);
		vec2 dist_to_bolt = abs(bolt_motion.position - player_motion.position);

		if (length(dist_to_bolt) < dist_threshold) {
			PhysicsObject& bolt_physics_object = registry.physicsObjects.get(bolt);
			bolt_physics_object.apply_gravity = true;
		}
	}
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
bool PhysicsSystem::in(std::vector<unsigned int>& vec, unsigned int id) {
	return std::find(vec.begin(), vec.end(), id) != vec.end();
}

bool PhysicsSystem::is_collision_between_player_and_boundary(Entity& one, Entity& other) {
	return (registry.players.has(one) && registry.boundaries.has(other)) ||
		(registry.players.has(other) && registry.boundaries.has(one));
}

bool PhysicsSystem::is_collision_between_player_and_spike(Entity& one, Entity& other) {
	return (registry.players.has(one) && registry.spikes.has(other)) ||
	(registry.players.has(other) && registry.spikes.has(one));
}

void PhysicsSystem::handle_player_breakable_collision(Entity& player_entity, Entity& breakable_entity, Collision collision) {
	Breakable& breakable = registry.breakables.get(breakable_entity);

	if (breakable.should_break_instantly) {

		assert(registry.gameStates.components.size() <= 1);
		GameState& gameState = registry.gameStates.components[0];

		if (gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED) {
			registry.remove_all_components_of(breakable_entity);
		}
	}
}
