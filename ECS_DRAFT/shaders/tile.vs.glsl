#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;


// Passed to fragment shader
out vec2 texcoord;

// Application data
//uniform mat3 transform;
uniform mat3 projection;
uniform float depth;

uniform vec2 tex_u_range;
uniform vec2 tile_pos;
uniform vec2 t_offset;


void main()
{
	texcoord = vec2(
			tex_u_range[0] + in_texcoord.x * (tex_u_range[1] - tex_u_range[0]),
			in_texcoord.y);

	vec2 currPixelPos = tile_pos + t_offset + (in_position.xy * 16);
	
	vec3 pos = projection * /*transform **/ vec3(currPixelPos.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, depth);
}