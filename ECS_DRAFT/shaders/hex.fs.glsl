#version 330

// From vertex shader
in vec2 texcoord;
in vec3 vcolor;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = vec4(fcolor * vcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
//	color = vec4(fcolor * vcolor, 1.0);
}