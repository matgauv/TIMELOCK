#include "collision_detection.h"
//#include <glm/detail/func_trigonometric.inl>

// caches the vertices for the platform at the current position (in world space)
// includes the rounded edge mesh
void compute_platform_verticies(Motion& motion, Entity& e, float& angle_cos, float& angle_sin) {
	PlatformGeometry& geo = registry.platformGeometries.get(e);
	Mesh* edge_mesh = registry.meshPtrs.get(e);

	const float total_tiles = geo.num_tiles;
	const float platform_center_offset = total_tiles * 0.5f * TILE_TO_PIXELS;

	std::vector<vec2> platform_vertices;

	// left edge
	for (auto& v : edge_mesh->vertices) {
		vec2 pos = v.position;
		platform_vertices.emplace_back(pos / PLATFORM_EDGE_MESH_SIZE);
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

// caches the vertices for each submesh.
// NOTE: submesh verticies are stored in the submesh component, not the main motion. TODO: fix this...
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

                vec2 normal = vec2{-edge.y, edge.x};
				float normal_len = length(normal);
            	if (normal_len < 0.001f) continue;
            	normal /= normal_len;

                sub_mesh.cached_axes.push_back(normal);
            }

            sub_mesh.cache_invalidated = false;
        }
    }
}

// Used to compute the vertices for the entity e at the current position (in world space), based on the components present (mesh, composite mesh, or no mesh)
// vertices are store in the motion.cached_vertices or the submesh.cached_vertices depending on type
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
		vec2 normal = vec2{-edge.y, edge.x};

		// ignore these cases TODO: what is best solution here
		float len = length(normal);
		if (len < 0.001f) continue;

		normal /= len;

		motion.cached_axes.push_back(normal);
	}
}

// see here for details https://dyn4j.org/2010/01/sat/
// computes collision between any two shapes, must be convex!
Collision compute_convex_collision(const std::vector<vec2>& a_verts, const std::vector<vec2>& a_axes, const std::vector<vec2>& b_verts, const std::vector<vec2>& b_axes, const vec2& a_position, const vec2& b_position, Entity& a, Entity& b) {
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
Collision compute_sat_collision(Motion& a_motion, Motion& b_motion, Entity& a, Entity& b) {
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

	// before expensive SAT collision check, cheaply verify that the objects are even overlapping first...
	if (!compute_AABB_collision(motion_i, motion_j)) return;

	Collision result = compute_sat_collision(motion_i, motion_j, entity_i, entity_j);

	if (result.normal != vec2{0, 0}) {
		registry.collisions.emplace_with_duplicates(entity_i, entity_j, result.overlap, result.normal);
	}
}

