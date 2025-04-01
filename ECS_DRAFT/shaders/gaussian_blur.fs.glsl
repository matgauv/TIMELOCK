#version 330

uniform sampler2D screen_texture;

uniform float stride;
uniform mat3 kernel;

in vec2 texcoord;

layout(location = 0) out vec4 color;


void main()
{
	vec4 in_color = vec4(0.0);

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			in_color += (texture(screen_texture, texcoord + vec2(float(i), float(j)) / textureSize(screen_texture, 0)) * kernel[i+1][j+1]);
		}
	}

	color = vec4(in_color.rgb, clamp(in_color.a * 1.3, 0.0, 1.0));
}