//
// Created by d on 2025-03-23.
//

#include "physics_utils.h"
float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

// TODO: prob should invest in a more robust method here, currently just approximating everything as a rectangle.
float calculate_moment_of_inertia(Entity& entity) {
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

	float dp = dot(ab, ab);
	if (abs(dp) <= 0.01f) dp = 0.1f;

	float t = dot(p - a, ab) / dp;
	t = clamp(t, 0.0f, 1.0f);
	return a + t * ab;
}



bool is_on_ground(float normal_y) {
	float ground_angle_rad = PLAYER_MAX_WALK_ANGLE * (M_PI / 180.0f);
	float threshold = cos(ground_angle_rad);
	return normal_y >= threshold;
}

// function for interpolating object velocity, specifically when an object is on a moving platforms.
float clampToTarget(float value, float change, float target) {
	change = abs(change);
	if (value > target) {
		return std::max(value - change, target);
	} else if (value < target) {
		return std::min(value + change, target);
	}
	return target;
}

// Helper function to check if an entity id is within a vector.
bool in(std::vector<unsigned int>& vec, unsigned int id) {
	return std::find(vec.begin(), vec.end(), id) != vec.end();
}

bool is_collision_between_player_and_boundary(Entity& one, Entity& other) {
	return (registry.players.has(one) && registry.boundaries.has(other)) ||
		(registry.players.has(other) && registry.boundaries.has(one));
}

bool is_collision_between_player_and_spike(Entity& one, Entity& other) {
	return (registry.players.has(one) && registry.spikes.has(other)) ||
	(registry.players.has(other) && registry.spikes.has(one));
}

bool player_harmful_collision(Entity& one, Entity& other) {
	return (registry.players.has(one) && registry.harmfuls.has(other))
			|| (registry.players.has(other) && registry.harmfuls.has(one));
}

void handle_player_breakable_collision(Entity& player_entity, Entity& breakable_entity, Collision collision) {
	Breakable& breakable = registry.breakables.get(breakable_entity);

	if (breakable.should_break_instantly) {

		assert(registry.gameStates.components.size() <= 1);
		GameState& gameState = registry.gameStates.components[0];

		if (gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED) {
			registry.remove_all_components_of(breakable_entity);
		}
	}
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

// returns the mesh verticies if a mesh exists, otherwise, returns verticies of a square defined by the scale of the object (BB)
std::vector<vec2>& get_vertices(Entity& e) {
	Motion& motion = registry.motions.get(e);
	return motion.cached_vertices;
}


vec2 get_modified_velocity(Motion& m)
{
	return {m.velocity.x * m.velocityModifier, m.velocity.y * m.velocityModifier};
}

float get_modified_angular_velocity(Motion& m, PhysicsObject& physics)
{
  return physics.angular_velocity * m.velocityModifier;
}

std::vector<vec2>& get_axes(Entity& e)
{
	Motion& motion = registry.motions.get(e);
	return motion.cached_axes;
}

