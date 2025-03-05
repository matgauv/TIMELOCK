#pragma once

#include <array>
#include <utility>

#include "../../common.hpp"
#include "../../tinyECS/components.hpp"
#include "../../tinyECS/component_container.hpp"
#include "systems/ISystem.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem : public ISystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count>  texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::HEX, mesh_path("hex.obj")),
		// specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
	const std::array<std::string, texture_count> texture_paths = {
        textures_path("black.png"),
		textures_path("greyCircle.png"),
		textures_path("backgrounds/SampleBackground.png"),
		textures_path("player/SamplePlayerWalking.png"),
		textures_path("player/SamplePlayerStanding.png"),
		textures_path("player/PlayerWalking_v1.png"),
		textures_path("player/PlayerStanding_v1.png"),
		textures_path("white_bubble.png"),
		textures_path("greenbox.png"),
		textures_path("transparent1px.png"),
		textures_path("backgrounds/gears.png"),
		textures_path("backgrounds/metal.png"),
		textures_path("backgrounds/chains.png"),
		textures_path("hex.png"),
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("textured"),
        shader_path("line"),
        shader_path("screen"),
		shader_path("hex"),
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	void init(GLFWwindow* window) override;
	void step(float elapsed_ms) override;
	void late_step(float elapsed_ms) override;

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();

	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();

	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the screen shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

	Entity get_screen_state_entity() { return screen_state_entity; }

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();

	// Update Screen shader factors
	void updateDecelerationFactor(GameState& gameState, ScreenState& screen, float elapsed_ms);
	void updateAccelerationFactor(GameState& gameState, ScreenState& screen, float elapsed_ms);

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
