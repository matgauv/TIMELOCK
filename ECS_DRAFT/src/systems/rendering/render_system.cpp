
#include <SDL.h>
#include <glm/trigonometric.hpp>
#include <iostream>

// internal
#include "render_system.hpp"
#include "../../tinyECS/registry.hpp"

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{


	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT

	Transform transform;

	if (!registry.tiles.has(entity)) {
		Motion& motion = registry.motions.get(entity);
		transform.translate(motion.position);
		transform.rotate(radians(motion.angle));
		transform.scale(motion.scale);
	}

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

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
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
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

		if (registry.timeControllables.has(entity)) {
			const TimeControllable& tc = registry.timeControllables.get(entity);
			if (
				(gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED && tc.can_become_harmless) ||
				(gameState.game_time_control_state != TIME_CONTROL_STATE::ACCELERATED && tc.can_become_harmful)) {
				// Green silhouette if (become harmless + decel) OR (become harmful + !accel)
				color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
			}
			else if (
				(gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED && tc.can_become_harmful) ||
				(gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED && tc.can_become_harmless)) {
				// Red silhouette if (become harmful + accel) OR (become harmless + !decel)
				color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
			}
		}
		glUniform4fv(color_uloc, 1, (float*)&color);
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
		vec4 color = vec4(-1.0f);
		const GameState& gameState = registry.gameStates.components[0];


		if (registry.timeControllables.has(entity)) {
			const TimeControllable& tc = registry.timeControllables.get(entity);
			if (
				(gameState.game_time_control_state == TIME_CONTROL_STATE::DECELERATED && tc.can_become_harmless) ||
				(gameState.game_time_control_state != TIME_CONTROL_STATE::ACCELERATED && tc.can_become_harmful)) {
				// Green silhouette if (become harmless + decel) OR (become harmful + !accel)
				color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
			}
			else if (
				(gameState.game_time_control_state == TIME_CONTROL_STATE::ACCELERATED && tc.can_become_harmful) ||
				(gameState.game_time_control_state != TIME_CONTROL_STATE::DECELERATED && tc.can_become_harmless)) {
				// Red silhouette if (become harmful + accel) OR (become harmless + !decel)
				color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
			}
		}

		glUniform4fv(color_uloc, 1, (float*)&color);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::LINE)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
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

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLint depth_uloc = glGetUniformLocation(currProgram, "depth");
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
		GLuint tex_u_range_loc = glGetUniformLocation(currProgram, "tex_u_range");
		vec2 tex_u_range = (registry.animateRequests.has(entity) ?
			registry.animateRequests.get(entity).tex_u_range : vec2{ 0.0f, 1.0f });

		// Flip uv if sprite is horizontally flipped
		if (render_request.flipped) {
			float temp = tex_u_range[0];
			tex_u_range[0] = tex_u_range[1];
			tex_u_range[1] = temp;
		}

		glUniform2fv(tex_u_range_loc, 1, (float*)&tex_u_range);
		gl_has_errors();
	}
	if (render_request.used_effect == EFFECT_ASSET_ID::TILE)
	{
		GLuint tile_id_uloc = glGetUniformLocation(currProgram, "tile_id");
		GLuint tile_pos_uloc = glGetUniformLocation(currProgram, "tile_pos");
		GLuint tile_offset_uloc = glGetUniformLocation(currProgram, "t_offset");

		Tile& tile_info = registry.tiles.get(entity);
		Motion& motion = registry.motions.get(tile_info.parent_id);
		int tile_start_x = motion.position.x - (motion.scale.x / 2) + (0.5 * TILE_TO_PIXELS);

		glUniform1i(tile_id_uloc, tile_info.id);
		glUniform2f(tile_pos_uloc, (float)tile_start_x, motion.position.y);
		glUniform2f(tile_offset_uloc, (float)(tile_info.offset * TILE_TO_PIXELS),0.0f);
		gl_has_errors();
	}


	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
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

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
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
	GLuint aspect_ratio_uloc = glGetUniformLocation(screen_shader_program, "aspect_ratio");

	GLuint time_uloc = glGetUniformLocation(screen_shader_program, "time");

	glUniform1f(time_uloc, (float)(glfwGetTime() * 1000.0f));

	ScreenState &screen = registry.screenStates.get(screen_state_entity);

	glUniform1f(dec_act_fac_uloc, screen.deceleration_factor);
	glUniform1f(acc_act_fac_uloc, screen.acceleration_factor);
	gl_has_errors();

	glUniform1f(acc_emerge_fac_uloc, ACCELERATION_EMERGE_MS/ACCELERATION_DURATION_MS);
	glUniform1f(dec_emerge_fac_uloc, DECELERATION_EMERGE_MS/DECELERATION_DURATION_MS);
	gl_has_errors();

	// TODO
	glUniform1f(transition_fac_uloc, screen.scene_transition_factor);
	
	const Entity player_entity = registry.players.entities[0];
	const Motion& motion = registry.motions.get(player_entity);
	vec3 augmeted_player_pos = vec3{motion.position.x, motion.position.y, 1.0f};
	vec3 canonical_player_pos = this->projection_matrix * augmeted_player_pos;
	
	float focal_point[2] = {
		(canonical_player_pos[0] + 1.0f) / 2.0f,
		(canonical_player_pos[1] + 1.0f) / 2.0f
	};
	glUniform2fv(focal_point_uloc, 1, focal_point);

	glUniform1f(aspect_ratio_uloc, ((float)WINDOW_WIDTH_PX) / WINDOW_HEIGHT_PX);
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
			screen.scene_transition_factor = min(1.0f, screen.scene_transition_factor) - elapsed_ms / DEAD_REVIVE_TIME_MS;

			if (screen.scene_transition_factor <= 0.0) {
				screen.scene_transition_factor = -1.0;
			}
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
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	
	// clear backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	
	// white background
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();


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


	// draw all entities with a render request to the frame buffer
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
				// TODO: may need to adjust rendering order for spawn points and interactive objects as well?
				if (registry.spawnPoints.has(entity)) {
					midgrounds.insert(midgrounds.begin(), entity);
				}
				else {
					midgrounds.push_back(entity);
				}
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

	for (Entity entity : foregrounds)
	{
		drawTexturedMesh(entity, this->projection_matrix);
	}

	/*
	for (Entity entity : registry.renderRequests.entities)
	{
		// filter to entities that have a motion component
		if (registry.motions.has(entity)) {
			// Note, its not very efficient to access elements indirectly via the entity
			// albeit iterating through all Sprites in sequence. A good point to optimize
			drawTexturedMesh(entity, this->projection_matrix );
		}
	}
	*/

	// draw framebuffer to screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// fake projection matrix, scaled to window coordinates


	// assert(registry.cameras.entities.size() == 1);
	if (registry.cameras.entities.size() < 1) {

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
	vec2 camera_pos = registry.motions.get(camera_entity).position;

	
	float left = camera_pos.x -0.35f * WINDOW_WIDTH_PX;
	float top = camera_pos.y -0.35f * WINDOW_HEIGHT_PX;
	float right = camera_pos.x + 0.35f * WINDOW_WIDTH_PX;
	float bottom = camera_pos.y + 0.35f * WINDOW_HEIGHT_PX;
	
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