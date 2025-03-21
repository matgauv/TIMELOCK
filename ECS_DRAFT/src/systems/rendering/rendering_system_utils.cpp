#include <iostream>
#include "render_system.hpp"
#include "../../tinyECS/registry.hpp"

GLuint RenderSystem::useShader(EFFECT_ASSET_ID shader_id) {
	assert(shader_id != EFFECT_ASSET_ID::EFFECT_COUNT);

	const GLuint used_effect_enum = (GLuint)shader_id;
	const GLuint program = (GLuint)effects[used_effect_enum];

	glUseProgram(program);
	gl_has_errors();

	return program;
}

void RenderSystem::bindGeometryBuffers(GEOMETRY_BUFFER_ID geo_id) {
	assert(geo_id != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)geo_id];
	const GLuint ibo = index_buffers[(GLuint)geo_id];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();
}

void RenderSystem::bindTexture(GLenum texture_unit, TEXTURE_ASSET_ID tex_id) {
	glActiveTexture(texture_unit);
	gl_has_errors();

	GLuint texture_id = texture_gl_handles[(GLuint)tex_id];

	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();
}

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

// Particles
struct ParticleInstancedNode {
	vec2 global_pos;
	float rotation;
	vec2 scale;
	vec4 color_info;
};

void RenderSystem::instancedRenderParticles(const std::vector<Entity> & particles, float depth) {
	auto particles_reg = registry.particles;
	int instance_count = particles.size();

	if (instance_count <= 0) {
		return;
	}

	// Use shader
	GLuint curr_program = useShader(EFFECT_ASSET_ID::PARTICLE_INSTANCED);

	// Bind all textures in order
	bindTexture(GL_TEXTURE0, TEXTURE_ASSET_ID::GREY_CIRCLE);

	// Set Vertex Attributes
	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(TexturedVertex), (void*)0);
	gl_has_errors();

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void*)sizeof(
			vec3)); // note the stride to skip the preceeding vertex position

	// Set instanced properties
	std::vector<ParticleInstancedNode> nodes(instance_count);
	for (int i = 0; i < instance_count; i++) {
		const Entity entity = particles[i];
		const Particle &particle = particles_reg.get(entity);

		// Transform info

		nodes[i].global_pos = particle.position;
		nodes[i].rotation = particle.angle * M_PI / 180.0;
		nodes[i].scale = particle.scale;

		// Fade in/out
		float fade_factor = 1.0f;
		if (particle.fade_in_out[0] > 1e-4 && particle.timer <= particle.fade_in_out[0]) {
			// Fade in
			fade_factor = particle.timer / particle.fade_in_out[0];
		}
		else if (particle.fade_in_out[1] > 1e-4 && (particle.life - particle.timer) <= particle.fade_in_out[1]) {
			// Fade out
			fade_factor = (particle.life - particle.timer) / particle.fade_in_out[1];
		}

		// Shrink in/out
		float shrink_factor = 1.0f;
		if (particle.shrink_in_out[0] > 1e-4 && particle.timer <= particle.shrink_in_out[0]) {
			// Shrink in
			shrink_factor = particle.timer / particle.shrink_in_out[0];
		}
		else if (particle.shrink_in_out[1] > 1e-4 && (particle.life - particle.timer) <= particle.shrink_in_out[1]) {
			// Shrink out
			shrink_factor = (particle.life - particle.timer) / particle.shrink_in_out[1];
		}

		nodes[i].scale *= shrink_factor;

		// Color info
		if (particle.particle_id == PARTICLE_ID::COLORED) {
			assert(registry.colors.has(entity));
			nodes[i].color_info = vec4(registry.colors.get(entity), cubic_interpolation(0.0, particle.alpha, fade_factor));
		}
		else {
			vec2 tex_u;
			setURange(entity, tex_u);
			nodes[i].color_info = vec4(tex_u, cubic_interpolation(0.0, particle.alpha, fade_factor), - (int)particle.particle_id);
		}
	}

	glm::size_t NODE_SIZE = sizeof(ParticleInstancedNode);

	glm::size_t new_size = instance_count * NODE_SIZE;
	glBindBuffer(GL_ARRAY_BUFFER, instanced_vbo_particles);
	glBufferData(GL_ARRAY_BUFFER, new_size, nodes.data(), GL_DYNAMIC_DRAW);

	// global_pos
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)0);

	// rotation
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)offsetof(ParticleInstancedNode, rotation));

	// scale
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)offsetof(ParticleInstancedNode, scale));

	// color_info
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, NODE_SIZE, (void*)offsetof(ParticleInstancedNode, color_info));

	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	gl_has_errors();

	// Uniforms
	GLuint projection_loc = glGetUniformLocation(curr_program, "projection");
	if (projection_loc >= 0) {
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&(this->projection_matrix));
	}

	GLuint depth_loc = glGetUniformLocation(curr_program, "depth");
	if (depth_loc >= 0) {
		glUniform1fv(depth_loc, 1, (float*)&depth);
	}

	// 6 indices for sprite
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, instance_count);
}

void RenderSystem::drawLayer(const std::vector<Entity>& entities) {
	if (entities.size() <= 0) {
		return;
	}

	LAYER_ID layer = registry.layers.get(entities[0]).layer;
	float depth = (
		layer == LAYER_ID::PARALLAXBACKGROUND ? PARALLAXBACKGROUND_DEPTH : (
			layer == LAYER_ID::BACKGROUND ? BACKGROUND_DEPTH : (
				layer == LAYER_ID::MIDGROUND ? MIDGROUND_DEPTH :
				FOREGROUND_DEPTH)));

	// Categorize all entities within layer by:
	// 1. Shader;
	// 2. Within each shader, geometry;
	// 3. Within each geometry, texture (regardless if exists);

	std::unordered_map<EFFECT_ASSET_ID, std::unordered_map<GEOMETRY_BUFFER_ID, std::unordered_map<TEXTURE_ASSET_ID, std::vector<Entity>>>> grouped_entities;

	for (const Entity e : entities) {
		if (!registry.renderRequests.has(e)) {
			continue;
		}

		const RenderRequest& render_request = registry.renderRequests.get(e);

		// if (registry.spawnPoints.has(e)) // insert to front

		// C++ automatically creates new object if key does not exist; beware of potential risks
		grouped_entities[render_request.used_effect][render_request.used_geometry][render_request.used_texture].push_back(e);
	}


	// Handle each group to render
	for (auto effect_group = grouped_entities.begin(); effect_group != grouped_entities.end(); effect_group++) {
		EFFECT_ASSET_ID current_effect = effect_group->first;

		// Set shader
		GLint currProgram = useShader(current_effect);

		// Setting general uniform values to the currently bound program

		// Set Depth
		GLint depth_uloc = glGetUniformLocation(currProgram, "depth");
		if (depth_uloc >= 0) {
			glUniform1fv(depth_uloc, 1, (float*)&depth);
			gl_has_errors();
		}

		// Set Projection
		GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
		if (projection_loc >= 0) {
			glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&(this->projection_matrix));
		}

		for (auto geo_group = effect_group->second.begin(); geo_group != effect_group->second.end(); geo_group++) {
			GEOMETRY_BUFFER_ID current_geo = geo_group->first;

			for (auto tex_group = geo_group->second.begin(); tex_group != geo_group->second.end(); tex_group++) {
				TEXTURE_ASSET_ID current_tex = tex_group->first;

				bindTexture(GL_TEXTURE0, current_tex);
				// Set geometry buffer
				bindGeometryBuffers(current_geo);

				drawInstances(current_effect, current_geo, current_tex, tex_group->second);
			}
		}
	}
}


void RenderSystem::drawInstances(EFFECT_ASSET_ID effect_id, GEOMETRY_BUFFER_ID geo_id, TEXTURE_ASSET_ID tex_id, const std::vector<Entity>& entities) {
	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);

	GLsizei instance_count = entities.size();

	// Handle different cases
	switch (effect_id) {
	default:
		std::cout << "Invalid Shader for Instanced Rendering" << std::endl;
		break;
	}

	glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0, instance_count);
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