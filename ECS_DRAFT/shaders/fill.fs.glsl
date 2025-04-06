#version 330

// From vertex shader
in vec2 texcoord;


// Application data
uniform sampler2D sampler0;
uniform vec4 fill_color;
uniform vec3 fcolor;


// Output color
layout(location = 0) out  vec4 color;

void main()
{
	vec4 sampled_color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
	
	color = vec4(fill_color.rgb, sampled_color.a * fill_color.a);
}
