#version 330

// Input attributes
in vec3 in_color;
in vec3 in_position;
in vec2 in_texcoord;

out vec3 vcolor;
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform float depth;

void main()
{
	texcoord = in_texcoord;
	vcolor = in_color;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, depth);
}