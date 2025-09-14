#version 330

// Input attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(location = 2) in vec2 global_pos;
layout(location = 3) in float rotation;
layout(location = 4) in vec2 scale;

// (x, y, z, a)
// if a > 0, xyza = rgba
// if a < 0, [x, y] = tex_u_range, z = alpha, -a = texture_id
layout(location = 5) in vec4 in_color_info;

// Passed to fragment shader
out vec2 texcoord;
out vec4 color_info;

// Application data
uniform mat3 projection;
uniform float depth;

void main()
{
	texcoord = vec2(
			in_color_info.x + in_texcoord.x * (in_color_info.y - in_color_info.x),
			in_texcoord.y);


	color_info = in_color_info;

	mat2 transform = mat2(
		cos(rotation) * scale.x, - sin(rotation) * scale.y,
		sin(rotation) * scale.x,cos(rotation) * scale.y
	);

	vec2 vert_pos_2D = global_pos.xy + transform * in_position.xy;
	
	vec3 pos = projection * vec3(vert_pos_2D, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, depth);
}