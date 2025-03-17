#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Instanced properties
in mat3 transform;
in vec2 tex_u_range;
in vec3 v_fcolor;
in vec4 v_silhouette_color;

// Passed to fragment shader
out vec2 texcoord;
out vec3 fcolor;
out vec4 silhouette_color;

// Application data
//uniform mat3 transform;
uniform mat3 projection;
uniform float depth;
//uniform vec2 tex_u_range;

void main()
{
	// Map texcoord into given range
	texcoord = vec2(
		tex_u_range[0] + in_texcoord.x * (tex_u_range[1] - tex_u_range[0]),
		in_texcoord.y);
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, depth);

	fcolor = v_fcolor;
	silhouette_color = v_silhouette_color;
}