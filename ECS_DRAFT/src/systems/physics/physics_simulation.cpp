//
// Created by d on 2025-03-23.
//

#include "physics_simulation.h"
#include "../player/player_system.hpp"

// process rotational dynamics!
void handle_rotational_dynamics(Entity& object_entity, Entity& other_entity, const vec2& collision_normal, float step_seconds) {
	Motion& obj_motion = registry.motions.get(object_entity);
	PhysicsObject& phys = registry.physicsObjects.get(object_entity);

	if (!phys.apply_rotation) return;

	Motion& platform_motion = registry.motions.get(other_entity);
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
	const float contact_threshold = 8.0f; // pixel threshold to find more ontacts
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
	auto cmp = [&](const vec2& a, const vec2& b) {
		return dot(a - platform_pos, platform_normal) < dot(b - platform_pos, platform_normal);
	};

	std::vector<vec2> sorted_contacts = contact_points;
	std::sort(sorted_contacts.begin(), sorted_contacts.end(), cmp);

	// Select up to two deepest contact points (those with the smallest projection values)
	std::vector<vec2> selected_contacts;
	for (size_t i = 0; i < std::min(sorted_contacts.size(), size_t(2)); i++) {
		selected_contacts.push_back(sorted_contacts[i]);
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
	const float tolerance = 1.0f;
	if (projected_com > min_contact_x - tolerance && projected_com < max_contact_x + tolerance) {
		// if stable, damp rotation a lot to prevent from tipping.
		if (phys.angular_damping > 0.0f) phys.angular_velocity *= 0.02;
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



// Handles collision between two PhysicsObject entities.
// TODO, this method is long. should split up and document better
void handle_physics_collision(float step_seconds, Entity& entityA, Entity& entityB, Collision collision, std::vector<unsigned int>& grounded)
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
	float a_inv_mass = physA.mass > 0 ? (1.0f / physA.mass) : 0.0f;
	float b_inv_mass = physB.mass > 0 ? (1.0f / physB.mass) : 0.0f;
	float total_inv_mass = a_inv_mass + b_inv_mass;
	resolve_collision_position(entityA, entityB, collision, a_inv_mass, b_inv_mass);

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

	float lever_arm_A_perp = contact_offset_a.x * normal.y - contact_offset_a.y * normal.x;
	float lever_arm_B_perp = contact_offset_b.x * normal.y - contact_offset_b.y * normal.x;

	float A_ang_term = physA.apply_rotation ? physA.angular_velocity * lever_arm_A_perp : 0.0f;
	float B_ang_term = physB.apply_rotation ? physB.angular_velocity * lever_arm_B_perp : 0.0f;
	float A_vel_towards_collision = dot(motionA.velocity, normal) + A_ang_term;
	float B_vel_towards_collision = dot(motionB.velocity, normal) + B_ang_term;
	float vel_along_normal = B_vel_towards_collision - A_vel_towards_collision;

	if (vel_along_normal > 0.0f) return;

	float bounce = min(physA.bounce, physB.bounce);

	// compute the impulse from the collision (relative to center of mass)
	float impulse_magnitude = -(1.0f + bounce) * vel_along_normal;
	float a_inv_inertia = (physA.apply_rotation) ? 1.0f / calculate_moment_of_inertia(entityA) : 0.0f;
	float b_inv_inertia = (physB.apply_rotation) ? 1.0f / calculate_moment_of_inertia(entityB) : 0.0f;
	float impulse_denom = total_inv_mass + (lever_arm_A_perp * lever_arm_A_perp) * a_inv_inertia + (lever_arm_B_perp * lever_arm_B_perp) * b_inv_inertia;

	if (impulse_denom == 0.0f) return;



	impulse_magnitude /= impulse_denom;
	vec2 impulse = impulse_magnitude * normal;

	// TODO: hacky, but we need to dampen the impulse since the platform has 0 mass but high velocity
	if (registry.movementPaths.has(entityA) || registry.movementPaths.has(entityB)) {
		impulse.y *= 0.1f;
	}

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

	// TODO: again hacky better solution prob exists for this
	if (registry.movementPaths.has(entityA)) {
		tangent_vel_A = 0.0f;
	}

	if (registry.movementPaths.has(entityB)) {
		tangent_vel_B = 0.0f;
	}

	float rel_tan_velocity = tangent_vel_B - tangent_vel_A;

	float friction = sqrt(physA.friction * physB.friction);

	if (registry.pendulums.has(entityA) || registry.pendulums.has(entityB)) friction = 1.0f;

	float tangent_impulse_scalar = -rel_tan_velocity / impulse_denom;
	tangent_impulse_scalar = clamp(tangent_impulse_scalar, -impulse_magnitude * friction, impulse_magnitude * friction); // clamp to coloumb's law

	vec2 tangent_impulse = friction_dir * tangent_impulse_scalar;

	if (physA.apply_friction) motionA.velocity -= tangent_impulse * a_inv_mass;
	if (physB.apply_friction) motionB.velocity += tangent_impulse * b_inv_mass;


	// angular friction
	if (physA.apply_rotation && physA.angular_damping > 0.0f) {
		float tangential_lever_a = contact_offset_a.x * friction_dir.y - contact_offset_a.y * friction_dir.x;
		physA.angular_velocity -= tangent_impulse_scalar * tangential_lever_a * a_inv_inertia;
		handle_rotational_dynamics(entityA, entityB, collision.normal, step_seconds);
	}
	if (physB.apply_rotation && physB.angular_damping > 0.0f) {
		float tangential_lever_b = contact_offset_b.x * friction_dir.y - contact_offset_b.y * friction_dir.x;
		physB.angular_velocity += tangent_impulse_scalar * tangential_lever_b * b_inv_inertia;
		handle_rotational_dynamics(entityB, entityA, -collision.normal, step_seconds);
	}
}

void apply_air_resistance(Entity entity, Motion& motion, float step_seconds)
{
	if (!registry.physicsObjects.has(entity)) return;

	PhysicsObject& physics = registry.physicsObjects.get(entity);

	if (!physics.apply_air_resistance) return;

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

// accelerates the entity by GRAVITY until it reaches the max_speed (terminal velocity)
// TODO:: more terminal velocities
void apply_gravity(Entity& entity, Motion& motion, float step_seconds) {
	if (registry.climbing.has(entity)) return;

	float max_fall_speed = OBJECT_MAX_FALLING_SPEED;
	float gravity = GRAVITY;

	if (registry.players.has(entity)) {
		max_fall_speed = PLAYER_MAX_FALLING_SPEED;

		if (motion.velocity.y < 0.0f) gravity = GRAVITY_JUMP_ASCENT;
	}

	motion.velocity.y = clampToTarget(motion.velocity.y, gravity * step_seconds, max_fall_speed);
}

// rotates the projectile based on its velocity
void rotate_projectile(Entity& entity, Motion& motion, float step_seconds) {
	float angularSpeed = 40.0f;
	motion.angle -= angularSpeed * step_seconds;
	motion.cache_invalidated = true;
}

// pendulum physics based off of this https://www.acs.psu.edu/drussell/Demos/Pendulum/Pendulum.html
//									  https://stackoverflow.com/questions/34819949/modelling-a-pendulum-in-c
// basically, just the equation: d²θ/dt² = -(g/L)sin(θ))
void update_pendulum(Entity& entity, float step_seconds) {
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
void update_pendulum_rods() {
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
void resolve_collision_position(Entity& entityA, Entity& entityB, Collision& collision, float inv_mass_a, float inv_mass_b)
{
	Motion& motionA = registry.motions.get(entityA);
	Motion& motionB = registry.motions.get(entityB);

	const float total_inv_mass = inv_mass_a + inv_mass_b;
	if (total_inv_mass <= 0.0f) return;

	// leave the objects slightly colliding so that the collision is still triggered
	const float epsilon = 0.0001f * sign(collision.normal.y);
	vec2 resolution = collision.normal * (length(collision.overlap) - epsilon);
	motionA.position += resolution * (inv_mass_a / total_inv_mass);
	motionB.position -= resolution * (inv_mass_b / total_inv_mass);
}

// will move an object along its movement path, rotating between the different paths specified in the component
void move_object_along_path(Entity& entity, Motion& motion, float step_seconds) {
	MovementPath& movement_path = registry.movementPaths.get(entity);
	Path currentPath = movement_path.paths[movement_path.currentPathIndex];

	vec2 displacement = currentPath.velocity * step_seconds;

	if (abs(motion.position.x - currentPath.end.x) <= abs(displacement.x) &&
		abs(motion.position.y - currentPath.end.y) <= abs(displacement.y)) {
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

