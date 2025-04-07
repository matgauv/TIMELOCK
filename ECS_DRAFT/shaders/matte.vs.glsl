#version 330

// Input attributes
in vec3 in_position;

// Instanced properties

// Passed to fragment shader
out vec2 texcoord;


// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform float depth;
uniform vec2 uv_scale;

void main()
{
	// Map texcoord into given range
	vec3 world_pos = transform * vec3(in_position.xy, 1.0);
	texcoord = world_pos.xy* uv_scale;

	vec3 pos = projection * world_pos;
	gl_Position = vec4(pos.xy, in_position.z, depth);
}