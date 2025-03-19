#include <iostream>
#include "render_system.hpp"
#include "../../tinyECS/registry.hpp"

void RenderSystem::setTransform(Entity entity, glm::mat3& transform) {
	const Motion& motion = registry.motions.get(entity);

	Transform t;
	t.translate(motion.position);
	t.rotate(radians(motion.angle));
	t.scale(motion.scale);

	transform = t.mat;
}

void  RenderSystem::setFColor(Entity entity, vec3& fcolor) {
	fcolor = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
}

void RenderSystem::setURange(Entity entity, vec2& uRange) {
	const RenderRequest& renderRequest = registry.renderRequests.get(entity);

	uRange = (registry.animateRequests.has(entity) ?
		registry.animateRequests.get(entity).tex_u_range : vec2{ 0.0f, 1.0f });

	// Flip uv if sprite is horizontally flipped
	if (renderRequest.flipped) {
		float temp = uRange[0];
		uRange[0] = uRange[1];
		uRange[1] = temp;
	}
}

void RenderSystem::setSilhouetteColor(Entity entity, vec4& silhouette_color) {
	silhouette_color = vec4(-1.0f);
	const GameState& gameState = registry.gameStates.components[0];

	if (registry.timeControllables.has(entity)) {
		const TimeControllable& tc = registry.timeControllables.get(entity);
		if (
			(gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED && tc.can_become_harmless) ||
			(gameState.game_time_control_state != TIME_CONTROL_STATE::ACCELERATED && tc.can_become_harmful)) {
			// Green silhouette if (become harmless + decel) OR (become harmful + !accel)
			silhouette_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if (
			(gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED && tc.can_become_harmful) ||
			(gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED && tc.can_become_harmless)) {
			// Red silhouette if (become harmful + accel) OR (become harmless + !decel)
			silhouette_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
	}
}

/*
// The following are abandoned due to the inefficiency of instanced rendering for every object

// Textured
struct TexturedInstancedNode {
	glm::mat3 transform;
	vec2 tex_u;
	vec3 fcolor;
	vec4 silhouette_color;
};

void RenderSystem::setupTextured(const std::vector<Entity>& entities, GLuint program) {
	if (entities.size() <= 0) {
		return;
	}

	int instance_count = entities.size();

	// Set Vertex Attributes
	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	//gl_has_errors();
	assert(in_texcoord_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
		sizeof(TexturedVertex), (void*)0);
	//gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void*)sizeof(
			vec3)); // note the stride to skip the preceeding vertex position

	// Set instanced properties
	TexturedInstancedNode *nodes = new TexturedInstancedNode[instance_count];
	for (int i = 0; i < instance_count; i++) {
		assert(registry.motions.has(entities[i]));
		Entity entity = entities[i];

		setTransform(entity, nodes[i].transform);

		setFColor(entity, nodes[i].fcolor);
		
		setURange(entity, nodes[i].tex_u);

		setSilhouetteColor(entity, nodes[i].silhouette_color);
	}

	glm::size_t NODE_SIZE = sizeof(TexturedInstancedNode);

	glm::size_t new_size = instance_count * NODE_SIZE;
	glBindBuffer(GL_ARRAY_BUFFER, instanced_vbo_static_tiles);
	void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, new_size,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	if (ptr) {
		memcpy(ptr, nodes, new_size);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	GLint transform_loc = glGetAttribLocation(program, "transform");
	GLint urange_loc = glGetAttribLocation(program, "tex_u_range");
	GLint fcolor_loc = glGetAttribLocation(program, "v_fcolor");
	GLint silhouette_color_loc = glGetAttribLocation(program, "v_silhouette_color");
	//gl_has_errors();

	glEnableVertexAttribArray(transform_loc);
	glVertexAttribPointer(transform_loc, 3, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)0);
	glEnableVertexAttribArray(transform_loc + 1);
	glVertexAttribPointer(transform_loc + 1, 3, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)(sizeof(vec3)));
	glEnableVertexAttribArray(transform_loc + 2);
	glVertexAttribPointer(transform_loc + 2, 3, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)(2 * sizeof(vec3)));

	glEnableVertexAttribArray(urange_loc);
	glVertexAttribPointer(urange_loc, 2, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)offsetof(TexturedInstancedNode, tex_u));

	glEnableVertexAttribArray(fcolor_loc);
	glVertexAttribPointer(fcolor_loc, 3, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)offsetof(TexturedInstancedNode, fcolor));

	glEnableVertexAttribArray(silhouette_color_loc);
	glVertexAttribPointer(silhouette_color_loc, 4, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)offsetof(TexturedInstancedNode, silhouette_color));

	glVertexAttribDivisor(transform_loc, 1);
	glVertexAttribDivisor(transform_loc + 1, 1);
	glVertexAttribDivisor(transform_loc + 2, 1);

	glVertexAttribDivisor(urange_loc, 1);
	glVertexAttribDivisor(fcolor_loc, 1);
	glVertexAttribDivisor(silhouette_color_loc, 1);
	//gl_has_errors();

	delete[] nodes;
}


// Tile
struct TileInstancedNode {
	//glm::mat3 transform;
	vec2 tex_u;
	vec2 tile_pos;
	int tile_id;
	vec2 t_offset;
};

void RenderSystem::setupTile(const std::vector<Entity>& entities, GLuint program) {
	if (entities.size() <= 0) {
		return;
	}

	int instance_count = entities.size();

	// Set Vertex Attributes
	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	//gl_has_errors();
	assert(in_texcoord_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
		sizeof(TexturedVertex), (void*)0);
	//gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void*)sizeof(
			vec3)); // note the stride to skip the preceeding vertex position

	// Set instanced properties
	TileInstancedNode* nodes = new TileInstancedNode[instance_count];
	for (int i = 0; i < instance_count; i++) {
		Entity entity = entities[i];

		//setTransform(entity, nodes[i].transform);

		setURange(entity, nodes[i].tex_u);

		const Tile& tile_info = registry.tiles.get(entity);
		const Motion& motion = registry.motions.get(tile_info.parent_id);
		const int tile_start_x = motion.position.x - (motion.scale.x / 2) + (0.5 * TILE_TO_PIXELS);

		nodes[i].tile_pos = { (float)tile_start_x, motion.position.y};
		nodes[i].tile_id = tile_info.id;
		nodes[i].t_offset = {(float)(tile_info.offset * TILE_TO_PIXELS), 0.0f};
	}

	glm::size_t NODE_SIZE = sizeof(TileInstancedNode);


	glm::size_t new_size = instance_count * NODE_SIZE;
	glBindBuffer(GL_ARRAY_BUFFER, instanced_vbo_particles);
	void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, new_size,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	if (ptr) {
		memcpy(ptr, nodes, new_size);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	//GLint transform_loc = glGetAttribLocation(program, "transform");
	GLint urange_loc = glGetAttribLocation(program, "tex_u_range");
	GLint tile_pos_loc = glGetAttribLocation(program, "tile_pos");
	GLint tile_id_loc = glGetAttribLocation(program, "tile_id");
	GLint t_offset_loc = glGetAttribLocation(program, "t_offset");
	//gl_has_errors();

	glEnableVertexAttribArray(urange_loc);
	glVertexAttribPointer(urange_loc, 2, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)offsetof(TileInstancedNode, tex_u));

	glEnableVertexAttribArray(tile_pos_loc);
	glVertexAttribPointer(tile_pos_loc, 3, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)offsetof(TileInstancedNode, tile_pos));

	glEnableVertexAttribArray(tile_id_loc);
	glVertexAttribPointer(tile_id_loc, 4, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)offsetof(TileInstancedNode, tile_id));

	glEnableVertexAttribArray(t_offset_loc);
	glVertexAttribPointer(t_offset_loc, 4, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)offsetof(TileInstancedNode, t_offset));


	glVertexAttribDivisor(urange_loc, 1);
	glVertexAttribDivisor(tile_pos_loc, 1);
	glVertexAttribDivisor(tile_id_loc, 1);
	glVertexAttribDivisor(t_offset_loc, 1);
	//gl_has_errors();

	delete[] nodes;
}
*/