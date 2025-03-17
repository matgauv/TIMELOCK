#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

in mat3 transform;
in vec2 tex_u_range;
in vec2 tile_pos;
in int tile_id;
in vec2 t_offset;

// Passed to fragment shader
out vec2 texcoord;
out vec2 tile_offset;

// Application data
//uniform mat3 transform;
uniform mat3 projection;
uniform float depth;
/*
uniform vec2 tex_u_range;
uniform vec2 tile_pos;
uniform int tile_id;
uniform vec2 t_offset;
*/

void main()
{
	texcoord = vec2(
			tex_u_range[0] + in_texcoord.x * (tex_u_range[1] - tex_u_range[0]),
			in_texcoord.y);

	vec2 currPixelPos = tile_pos + t_offset + (in_position.xy * 16);
	
	vec3 pos = projection * /*transform **/ vec3(currPixelPos.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, depth);

	// tilesheet curently 7x7
	const int tiles_per_row = 7;
    const float tile_size = 1.0 / float(tiles_per_row); // each tile occupies 1/7th of the texture

	int col = (tile_id - 1) % tiles_per_row;
    int row = (tile_id - 1) / tiles_per_row;

	tile_offset = vec2(float(col), float(row)) * tile_size;
}