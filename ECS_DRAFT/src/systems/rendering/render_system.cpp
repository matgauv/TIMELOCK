#include <SDL.h>
#include <iostream>

// internal
#include "render_system.hpp"
#include "../../tinyECS/registry.hpp"
#include "systems/world/world_init.hpp"

void RenderSystem::drawTexturedMesh(Entity entity, const mat3& projection) {
	assert(registry.renderRequests.has(entity));
	const RenderRequest& render_request = registry.renderRequests.get(entity);
	RenderSystem::drawTexturedMesh(entity, projection, render_request);
}

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection,
									const RenderRequest& render_request)
{
	assert(render_request.used_effect != EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[(GLuint)render_request.used_effect];

	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;

	if (!registry.tiles.has(entity)) {
		Motion& motion = registry.motions.get(entity);

		// No need to render mesh with 0 dimension
		if (abs(motion.scale.x) < 1e-8 || abs(motion.scale.y) < 1e-8) {
			return;
		}

		// if pivot point exists, translate to offset, rotate, translate back, scale (hack for pendulums)
		if (registry.pivotPoints.has(entity)) {
			vec2 pivot_offset = registry.pivotPoints.get(entity).offset;

			transform.translate(motion.position);
			transform.translate(pivot_offset);
			transform.rotate(radians(motion.angle));
			transform.translate(-pivot_offset);
			transform.scale(motion.scale);
		} else {
			transform.translate(motion.position);
			transform.rotate(radians(motion.angle));
			transform.scale(motion.scale);
		}
	}

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();


	// texture-mapped entities - use data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED ||
		render_request.used_effect == EFFECT_ASSET_ID::FILL)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED) {
			GLint color_uloc = glGetUniformLocation(program, "silhouette_color");
			vec4 color;
			setSilhouetteColor(entity, color);

			glUniform4fv(color_uloc, 1, (float*)&color);
		}
		else if (render_request.used_effect == EFFECT_ASSET_ID::FILL) {
			GLint fill_color_uloc = glGetUniformLocation(program, "fill_color");
			
			// TODO
			if (registry.haloRequests.has(entity)) {
				vec4 fill_color = registry.haloRequests.get(entity).halo_color;
				glUniform4fv(fill_color_uloc, 1, (float*)&fill_color);
			}
		}
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::TILE)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();


		GLint color_uloc = glGetUniformLocation(program, "silhouette_color");
		vec4 color = vec4(-1.0f);
		const GameState& gameState = registry.gameStates.components[0];

		glUniform4fv(color_uloc, 1, (float*)&color);
		gl_has_errors();

		GLuint tile_id_uloc = glGetUniformLocation(program, "tile_id");
		GLuint tile_pos_uloc = glGetUniformLocation(program, "tile_pos");
		GLuint tile_offset_uloc = glGetUniformLocation(program, "t_offset");

		Tile& tile_info = registry.tiles.get(entity);
		Motion& motion = registry.motions.get(tile_info.parent_id);

		// starts from the top left tile of an object.
		int tile_start_x = motion.position.x - (motion.scale.x / 2) + (0.5 * TILE_TO_PIXELS);
		int tile_start_y = motion.position.y - (motion.scale.y / 2) + (0.5 * TILE_TO_PIXELS);

		glUniform1i(tile_id_uloc, tile_info.id);
		glUniform2f(tile_pos_uloc, (float)tile_start_x, (float)tile_start_y);
		glUniform2f(tile_offset_uloc, (float)(tile_info.offset.x* TILE_TO_PIXELS), (float)(tile_info.offset.y* TILE_TO_PIXELS));
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::HEX)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");

		gl_has_errors();

		GLint check;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &check);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)offsetof(ColoredVertex, position));
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)offsetof(ColoredVertex, color));
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)offsetof(ColoredVertex, uv));
		gl_has_errors();


		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id = texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();


		GLint color_uloc = glGetUniformLocation(program, "silhouette_color");

		vec4 color;
		setSilhouetteColor(entity, color);

		glUniform4fv(color_uloc, 1, (float*)&color);
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();


	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	// Setting uniform values to the currently bound program
	GLint depth_uloc = glGetUniformLocation(program, "depth");
	LAYER_ID layer = registry.layers.get(entity).layer;
	float depth = (
		layer == LAYER_ID::PARALLAXBACKGROUND ? PARALLAXBACKGROUND_DEPTH : (
		layer == LAYER_ID::BACKGROUND ? BACKGROUND_DEPTH : (
		layer == LAYER_ID::MIDGROUND ? MIDGROUND_DEPTH :
			FOREGROUND_DEPTH)));
	glUniform1fv(depth_uloc, 1, (float*)&depth);
	gl_has_errors();

	if (render_request.used_effect != EFFECT_ASSET_ID::HEX)
	{
		GLuint tex_u_range_loc = glGetUniformLocation(program, "tex_u_range");
		vec2 tex_u_range;

		setURange(entity, tex_u_range);

		glUniform2fv(tex_u_range_loc, 1, (float*)&tex_u_range);
		gl_has_errors();
	}


	GLuint transform_loc = glGetUniformLocation(program, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(program, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// first draw to an intermediate texture
// then draw the intermediate texture
// TODO: do we still need intermediate texture here? (used to be for vignette, could be helpful for full-screen effects)
void RenderSystem::drawToScreen()
{
  	// Setting shaders
	// get the vignette texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::SCREEN]);
	gl_has_errors();

	bindFrameBuffer(FRAME_BUFFER_ID::SCREEN_BUFFER);
	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
	gl_has_errors();

    // add the "vignette" effect
	const GLuint screen_shader_program = effects[(GLuint)EFFECT_ASSET_ID::SCREEN];

	// set acceleration/deceleration factors
	GLuint acc_act_fac_uloc = glGetUniformLocation(screen_shader_program, "acc_act_factor");
	GLuint dec_act_fac_uloc = glGetUniformLocation(screen_shader_program, "dec_act_factor");

	GLuint acc_emerge_fac_uloc = glGetUniformLocation(screen_shader_program, "acc_emerge_factor");
	GLuint dec_emerge_fac_uloc = glGetUniformLocation(screen_shader_program, "dec_emerge_factor");

	GLuint transition_fac_uloc = glGetUniformLocation(screen_shader_program, "transition_factor");
	GLuint focal_point_uloc = glGetUniformLocation(screen_shader_program, "focal_point");

	GLuint grid_wide_count_uloc = glGetUniformLocation(screen_shader_program, "GRID_WIDE_COUNT");
	glUniform1i(grid_wide_count_uloc, GRID_WIDE_COUNT);

	GLuint grid_high_count_uloc = glGetUniformLocation(screen_shader_program, "GRID_HIGH_COUNT");
	glUniform1i(grid_high_count_uloc, GRID_HIGH_COUNT);

	GLuint boundary_wide_uloc = glGetUniformLocation(screen_shader_program, "BOUNDARY_WIDE_COUNT");
	glUniform1i(boundary_wide_uloc, BOUNDARY_WIDE_COUNT);

	GLuint boundary_high_uloc = glGetUniformLocation(screen_shader_program, "BOUNDARY_HIGH_COUNT");
	glUniform1i(boundary_high_uloc, BOUNDARY_HIGH_COUNT);

	GLuint vignette_width_uloc = glGetUniformLocation(screen_shader_program, "VIGNETTE_WIDTH");
	glUniform1f(vignette_width_uloc, VIGNETTE_WIDTH);

	GLuint paled_blue_tone_uloc = glGetUniformLocation(screen_shader_program, "PALED_BLUE_TONE");
	glUniform3fv(paled_blue_tone_uloc, 1, (float*)&PALED_BLUE_TONE);

	GLuint shard_color_1_uloc = glGetUniformLocation(screen_shader_program, "SHARD_COLOR_1");
	glUniform3fv(shard_color_1_uloc, 1, (float*)&SHARD_COLOR_1);

	GLuint shard_color_2_uloc = glGetUniformLocation(screen_shader_program, "SHARD_COLOR_2");
	glUniform3fv(shard_color_2_uloc, 1, (float*)&SHARD_COLOR_2);

	GLuint shard_silhouette_uloc = glGetUniformLocation(screen_shader_program, "SHARD_SILHOUETTE_COLOR");
	glUniform3fv(shard_silhouette_uloc, 1, (float*)&SHARD_SILHOUETTE_COLOR);

	GLuint shard_speed_uloc = glGetUniformLocation(screen_shader_program, "SHARD_EVOLVING_SPEED");
	glUniform1f(shard_speed_uloc, SHARD_EVOLVING_SPEED);

	GLuint time_uloc = glGetUniformLocation(screen_shader_program, "time");

	glUniform1f(time_uloc, (float)(glfwGetTime() * 1000.0f)); // May need to adjust this if pauses the game when decelerated

	ScreenState &screen = registry.screenStates.get(screen_state_entity);

	glUniform1f(dec_act_fac_uloc, screen.deceleration_factor);
	glUniform1f(acc_act_fac_uloc, screen.acceleration_factor);
	gl_has_errors();

	glUniform1f(acc_emerge_fac_uloc, ACCELERATION_EMERGE_MS/ACCELERATION_DURATION_MS);
	glUniform1f(dec_emerge_fac_uloc, DECELERATION_EMERGE_MS/DECELERATION_DURATION_MS);
	gl_has_errors();

	// TODO
	glUniform1f(transition_fac_uloc, std::min(1.0f, screen.scene_transition_factor));


	vec3 augmented_default_pos = vec3{WINDOW_WIDTH_PX / 2.0f, WINDOW_HEIGHT_PX / 2.0f, 1.0f};
	vec3 canonical_default_pos = this->projection_matrix * augmented_default_pos;
	float focal_point[2] = {
		(canonical_default_pos[0] + 1.0f) / 2.0f,
		(canonical_default_pos[1] + 1.0f) / 2.0f
	};
	if (registry.players.size() > 0) {
		const Entity player_entity = registry.players.entities[0];
		const Motion& motion = registry.motions.get(player_entity);
		vec3 augmented_player_pos = vec3{motion.position.x, motion.position.y, 1.0f};
		vec3 canonical_player_pos = this->projection_matrix * augmented_player_pos;

		focal_point[0] = (canonical_player_pos[0] + 1.0f) / 2.0f;
		focal_point[1] = (canonical_player_pos[1] + 1.0f) / 2.0f;
	}
	glUniform2fv(focal_point_uloc, 1, focal_point);
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(screen_shader_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();

	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

void RenderSystem::step(float elapsed_ms) {
	// anything to do here?

	// Update Screen factors
	// assert(registry.gameStates.components.size() <= 1);
	GameState& gameState = registry.gameStates.components[0];
	ScreenState& screen = registry.screenStates.get(screen_state_entity);

	// Acceleration & Deceleration
	updateDecelerationFactor(gameState, screen, elapsed_ms);
	updateAccelerationFactor(gameState, screen, elapsed_ms);


	// Scene Transition
	// Can extend to enter/exit scenes
	if (gameState.game_scene_transition_state == SCENE_TRANSITION_STATE::TRANSITION_OUT) {
		if (screen.scene_transition_factor < 1.0) {
			screen.scene_transition_factor = max(0.0f, screen.scene_transition_factor) + elapsed_ms/DEAD_REVIVE_TIME_MS;
		}
	} else {
		if (screen.scene_transition_factor > 0.0) {
			// Update tolerance
			if (screen.scene_transition_factor > 1.0) {
				screen.scene_transition_factor = max(1.0f, screen.scene_transition_factor - 1.0f);
				return;
			}
			screen.scene_transition_factor = min(1.0f, screen.scene_transition_factor) - elapsed_ms / DEAD_REVIVE_TIME_MS;
			//std::cout << screen.scene_transition_factor << ":" << elapsed_ms << std::endl;
			if (screen.scene_transition_factor <= 0.0) {
				screen.scene_transition_factor = -1.0;
			}
		}
	}

	// Halos
	for (HaloRequest& halo : registry.haloRequests.components) {
		if (glm::length(halo.halo_color - halo.target_color) < HALO_LERP_TOLERANCE) {
			halo.halo_color = halo.target_color;
		}
		else {
			halo.halo_color = halo.halo_color * HALO_LERP_FACTOR + halo.target_color * (1.0F - HALO_LERP_FACTOR);
		}
	}
}

void RenderSystem::updateDecelerationFactor(GameState& gameState, ScreenState& screen, float elapsed_ms)
{
	if (gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED) {
		screen.deceleration_factor = max(0.0f,
			screen.deceleration_factor + elapsed_ms / DECELERATION_DURATION_MS);
	}
	else if (screen.deceleration_factor >= 0) {
		screen.deceleration_factor = min(
			screen.deceleration_factor - elapsed_ms / DECELERATION_DURATION_MS,
			DECELERATION_EMERGE_MS / DECELERATION_DURATION_MS);

		if (screen.deceleration_factor < 0) {
			screen.deceleration_factor = -1.0;
		}
	}
}

void RenderSystem::updateAccelerationFactor(GameState& gameState, ScreenState& screen, float elapsed_ms)
{
	if (gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED) {
		screen.acceleration_factor = max(0.0f,
			screen.acceleration_factor + elapsed_ms / ACCELERATION_DURATION_MS);
	}
	else if (screen.acceleration_factor >= 0) {
		screen.acceleration_factor = min(
			screen.acceleration_factor - elapsed_ms / ACCELERATION_DURATION_MS,
			ACCELERATION_EMERGE_MS / ACCELERATION_DURATION_MS);

		if (screen.acceleration_factor < 0) {
			screen.acceleration_factor = -1.0;
		}
	}
}

void RenderSystem::late_step(float elapsed_ms) {
	this->projection_matrix = createProjectionMatrix();
	draw();
};

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	GameState& gs = registry.gameStates.components[0];
	if (gs.game_running_state == GAME_RUNNING_STATE::LOADING) {
		glBindVertexArray(vao_general);
		std::cout << "DRAWING LOADING SCREEN" << std::endl;
		Entity entity = create_loading_screen();
		drawTexturedMesh(entity, this->projection_matrix);
		drawToScreen();
		glfwSwapBuffers(window);
		return;
	}

	// handle meshes
	// TODO prob handle this somewhere better...
	for (Entity entity : registry.players.entities) {
		if (!registry.meshPtrs.has(entity)) {
			Mesh& player = getMesh(GEOMETRY_BUFFER_ID::PLAYER);
			registry.meshPtrs.emplace(entity, &player);
		}
	}

	for (Entity entity : registry.platformGeometries.entities) {
		if (!registry.meshPtrs.has(entity)) {
			Mesh& platform = getMesh(GEOMETRY_BUFFER_ID::PLATFORM);
			registry.meshPtrs.emplace(entity, &platform);
		}
	}

	// Assort rendering tasks according to layers

	std::vector<Entity> parallaxbackgrounds;

	std::vector<Entity> backgrounds;

	std::vector<Entity> midgrounds;

	std::vector<Entity> foregrounds;

	for (Entity entity : registry.layers.entities)
	{
		// Check for rendering necessity
		if (!registry.renderRequests.has(entity) || (!registry.motions.has(entity) && !registry.tiles.has(entity)))
			continue;

		// TODO: this could be somewhere else, but idk how to get the mesh pointer without increased system coupling...
		// Keep track of pointer to any custom mesh in the registry for use in other systems
		if (!registry.meshPtrs.has(entity)) {
			RenderRequest request = registry.renderRequests.get(entity);
			if (request.used_geometry == GEOMETRY_BUFFER_ID::HEX) {
				Mesh& mesh = getMesh(request.used_geometry);
				registry.meshPtrs.emplace(entity, &mesh);
			}
		}

		switch (registry.layers.get(entity).layer)
		{
			case LAYER_ID::FOREGROUND:
				foregrounds.push_back(entity);
				break;
			case LAYER_ID::MIDGROUND:
				// Render Player last?
				if (registry.haloRequests.has(entity)) {
					continue;
				}
				midgrounds.push_back(entity);
				break;
			case LAYER_ID::PARALLAXBACKGROUND:
				parallaxbackgrounds.push_back(entity);
				break;
			case LAYER_ID::BACKGROUND:
				backgrounds.push_back(entity);
				break;
			default:
				break;
		}
	}

	glBindVertexArray(vao_general);

	// Render Halo
	bindFrameBuffer(FRAME_BUFFER_ID::BLUR_BUFFER_1);

	std::vector<Entity> halo_entities;
	if (registry.bosses.size() > 0) {
		halo_entities.push_back(registry.bosses.entities[0]);
	}

	if (registry.snoozeButtons.size() > 0) {
		halo_entities.push_back(registry.snoozeButtons.entities[0]);
	}
	// TODO: add decel bar after merge
	if (registry.players.size() > 0) {
		halo_entities.push_back(registry.players.entities[0]);
	}

	if (registry.decelerationBars.size() > 0) {
		halo_entities.push_back(registry.decelerationBars.entities[0]);
	}

	for (const Entity halo_entity : halo_entities) {
		drawFilledMesh(halo_entity, this->projection_matrix);
	}

	// Prepare halo effect
	for (int i = 6; i >= 0; i--) {
		// Pass-catch

		// Render to blur 2
		bindFrameBuffer(FRAME_BUFFER_ID::BLUR_BUFFER_2);
		drawBlurredLayer(blur_buffer_color_1, BLUR_MODE::HORIZONTAL, 1.75f, 1.45f);

		// Render to blur 1
		bindFrameBuffer(FRAME_BUFFER_ID::BLUR_BUFFER_1);
		drawBlurredLayer(blur_buffer_color_2, BLUR_MODE::VERTICAL, 1.75f, 1.45f);
	}

	// Halo effects in blur 1

	// Render foreground to blur 2
	bindFrameBuffer(FRAME_BUFFER_ID::BLUR_BUFFER_2);
	for (Entity entity : foregrounds)
	{
		drawTexturedMesh(entity, this->projection_matrix);
	}

	// Start rendering to intermediate buffer
	bindFrameBuffer(FRAME_BUFFER_ID::INTERMEDIATE_BUFFER);


	// draw all entities with a render request to the frame buffer
	for (Entity entity : parallaxbackgrounds)
	{
		drawTexturedMesh(entity, this->projection_matrix);
	}


	for (Entity entity : backgrounds)
	{
		drawTexturedMesh(entity, this->projection_matrix);
	}



	for (Entity entity : midgrounds)
	{
		drawTexturedMesh(entity, this->projection_matrix);
	}

	drawBlurredLayer(blur_buffer_color_1, BLUR_MODE::TWO_D, 1.5f, 1.2f);

	for (Entity entity : halo_entities)
	{
		drawTexturedMesh(entity, this->projection_matrix);
	}
	glBindVertexArray(0);

	glBindVertexArray(vao_particles);
	// Potentially aim for multi-layers
	instancedRenderParticles(registry.particles.entities, MIDGROUND_DEPTH);

	glBindVertexArray(0);

	glBindVertexArray(vao_general);
	// Render foreground
	drawBlurredLayer(blur_buffer_color_2, BLUR_MODE::TWO_D, 1.5f, 1.2f);


	// draw framebuffer to screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	//gl_has_errors();
}


mat3 RenderSystem::createProjectionMatrix()
{
	// fake projection matrix, scaled to window coordinates


	// assert(registry.cameras.entities.size() == 1);
	if (registry.cameras.entities.size() < 1) {
		std::cout << "NO CAMERA" << std::endl;
		float left = 0.f;
		float top = 0.f;
		float right = (float)WINDOW_WIDTH_PX;
		float bottom = (float)WINDOW_HEIGHT_PX;

		float sx = 2.f / (right - left);
		float sy = 2.f / (top - bottom);
		float tx = -(right + left) / (right - left);
		float ty = -(top + bottom) / (top - bottom);

		return {
		{ sx, 0.f, 0.f},
		{0.f,  sy, 0.f},
		{ tx,  ty, 1.f}
		};
	}

	Entity camera_entity = registry.cameras.entities[0];
	const vec2 camera_pos = registry.motions.get(camera_entity).position;
	const vec2 camera_scale = registry.motions.get(camera_entity).scale;

	const vec2 camera_offsets = CameraSystem::get_camera_offsets(camera_scale);

	float left = camera_pos.x - camera_offsets[0];
	float top = camera_pos.y - camera_offsets[1];
	float right = camera_pos.x + camera_offsets[0];
	float bottom = camera_pos.y + camera_offsets[1];
	
	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);

	return {
		{ sx, 0.f, 0.f},
		{0.f,  sy, 0.f},
		{ tx,  ty, 1.f}
	};
}