#include <iostream>
#include "render_system.hpp"
#include "../../tinyECS/registry.hpp"

void RenderSystem::bindFrameBuffer(FRAME_BUFFER_ID frame_buffer_id) {
	// Clearing backbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	switch (frame_buffer_id)
	{
		case FRAME_BUFFER_ID::SCREEN_BUFFER: {
			glViewport(0, 0, w, h);
			glDepthRange(0, 10);
			glClearColor(1.f, 0, 0, 1.0);
			glClearDepth(1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			gl_has_errors();
			// Enabling alpha channel for textures
			glDisable(GL_BLEND);
			// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_DEPTH_TEST);
			// indices to the bound GL_ARRAY_BUFFER
			break;
		}
		case FRAME_BUFFER_ID::INTERMEDIATE_BUFFER: {
			// First render to the custom framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
			gl_has_errors();

			// clear backbuffer
			glViewport(0, 0, w, h);
			glDepthRange(0.00001, 10);

			// white background -> black background
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			glClearDepth(10.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
			// and alpha blending, one would have to sort
			// sprites back to front
			break;
		}
		case FRAME_BUFFER_ID::BLUR_BUFFER_1: {
			// Bind blur buffer
			glBindFramebuffer(GL_FRAMEBUFFER, blur_buffer_1);
			gl_has_errors();

			// clear blur buffer
			// down sample
			glViewport(0, 0, w/ BLUR_FACTOR, h/ BLUR_FACTOR);
			glDepthRange(0.00001, 10);

			// transparent background
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

			glClearDepth(10.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_DEPTH_TEST);
			break;
		}
		case FRAME_BUFFER_ID::BLUR_BUFFER_2: {
			// Bind blur buffer
			glBindFramebuffer(GL_FRAMEBUFFER, blur_buffer_2);
			gl_has_errors();

			// clear blur buffer
			// down sample
			glViewport(0, 0, w / BLUR_FACTOR, h / BLUR_FACTOR);
			glDepthRange(0.00001, 10);

			// transparent background
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

			glClearDepth(10.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_DEPTH_TEST);
			break;
		}
		default:
			break;
	}
	gl_has_errors();
}

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
	bindTexture(GL_TEXTURE0 + 1, TEXTURE_ASSET_ID::BREAKABLE_FRAGMENTS);
	bindTexture(GL_TEXTURE0 + 2, TEXTURE_ASSET_ID::COYOTE_PARTICLES);
	bindTexture(GL_TEXTURE0 + 3, TEXTURE_ASSET_ID::SCREW_FRAGMENTS);
	bindTexture(GL_TEXTURE0 + 4, TEXTURE_ASSET_ID::HEX_FRAGMENTS);
	bindTexture(GL_TEXTURE0 + 5, TEXTURE_ASSET_ID::CRACKING_RADIAL);
	bindTexture(GL_TEXTURE0 + 6, TEXTURE_ASSET_ID::CRACKING_DOWNWARD);
	bindTexture(GL_TEXTURE0 + 7, TEXTURE_ASSET_ID::EXHALE);
	bindTexture(GL_TEXTURE0 + 8, TEXTURE_ASSET_ID::BROKEN_PARTS);
	bindTexture(GL_TEXTURE0 + 9, TEXTURE_ASSET_ID::CROSS_STAR);
	glUniform1i(glGetUniformLocation(curr_program, "texture1"), 0);
	glUniform1i(glGetUniformLocation(curr_program, "texture2"), 1);
	glUniform1i(glGetUniformLocation(curr_program, "texture3"), 2);
	glUniform1i(glGetUniformLocation(curr_program, "texture4"), 3);
	glUniform1i(glGetUniformLocation(curr_program, "texture5"), 4);
	glUniform1i(glGetUniformLocation(curr_program, "texture6"), 5);
	glUniform1i(glGetUniformLocation(curr_program, "texture7"), 6);
	glUniform1i(glGetUniformLocation(curr_program, "texture8"), 7);
	glUniform1i(glGetUniformLocation(curr_program, "texture9"), 8);
	glUniform1i(glGetUniformLocation(curr_program, "texture10"), 9);

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


// Can safely ignore the following; meant for instanced rendering of every object
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

void RenderSystem::drawFilledMesh(Entity entity, const mat3& projection) {
	assert(registry.renderRequests.has(entity));
	const RenderRequest& render_request = registry.renderRequests.get(entity);

	RenderRequest fill_request;
	fill_request.used_effect = EFFECT_ASSET_ID::FILL;
	fill_request.used_texture = render_request.used_texture;
	fill_request.used_geometry = render_request.used_geometry;
	fill_request.flipped = render_request.flipped;

	RenderSystem::drawTexturedMesh(entity, projection, fill_request);
}

void RenderSystem::drawBlurredLayer(GLuint source_texture, BLUR_MODE mode, float width_factor, float strength) {

	// Setting shaders
	const GLuint blur_shader_program = effects[(GLuint)EFFECT_ASSET_ID::GAUSSIAN_BLUR];

	glUseProgram(blur_shader_program);
	gl_has_errors();

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(blur_shader_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, source_texture);
	gl_has_errors();

	// Set uniforms
	GLuint stride_uloc = glGetUniformLocation(blur_shader_program, "stride");
	glUniform1f(stride_uloc, width_factor);

	GLuint strength_uloc = glGetUniformLocation(blur_shader_program, "strength");
	glUniform1f(strength_uloc, strength);

	GLuint mode_uloc = glGetUniformLocation(blur_shader_program, "blur_mode");
	glUniform1i(mode_uloc, (int)mode);

	if (mode == BLUR_MODE::TWO_D) {
		GLuint kernel_loc = glGetUniformLocation(blur_shader_program, "kernel_2D");
		glUniformMatrix3fv(kernel_loc, 1, GL_FALSE, (float*)&(RenderSystem::gaussian_blur_kernel_2D));
	}
	else {
		GLuint kernel_loc = glGetUniformLocation(blur_shader_program, "kernel_1D");
		glUniform4fv(kernel_loc, 1, (float*)&(RenderSystem::gaussian_blur_kernel_1D));
	}
	gl_has_errors();

	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
	// no offset from the bound index buffer
	gl_has_errors();
}
