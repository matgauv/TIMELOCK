#version 330

uniform sampler2D screen_texture;

uniform float stride;
uniform float strength;
uniform mat3 kernel_2D;
uniform vec3 kernel_1D;
uniform int blur_mode;

in vec2 texcoord;

layout(location = 0) out vec4 color;


void main()
{
	vec4 in_color = vec4(0.0);
	vec2 tex_size = textureSize(screen_texture, 0);

	if (blur_mode == 0) {
		// 2D blur
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				vec4 sampled_color = texture(screen_texture, texcoord + stride*vec2(float(i), float(j)) / tex_size);
				float weight = kernel_2D[i+1][j+1];
				in_color.rgb += (sampled_color.a * sampled_color.rgb * weight);
				in_color.a += (sampled_color.a * weight);
			}
		}
	} else if (blur_mode == 1) {
		// Horizontal Blur
		for (int i = -2; i <= 2; i++) {
			vec4 sampled_color = texture(screen_texture, texcoord + stride*vec2(float(i), 0.0) / tex_size);
			float weight = kernel_1D[abs(i)];
			in_color.rgb += (sampled_color.a * sampled_color.rgb * weight);
			in_color.a += (sampled_color.a * weight);
		}
	} else {
		// Vertical Blur
		for (int j = -2; j <= 2; j++) {
			vec4 sampled_color = texture(screen_texture, texcoord + stride*vec2(0.0, float(j)) / tex_size);
			float weight = kernel_1D[abs(j)];
			in_color.rgb += (sampled_color.a * sampled_color.rgb * weight);
			in_color.a += (sampled_color.a * weight);
		}
	}

	if (in_color.a > 0.001) {
		in_color.rgb /= in_color.a;
	}

	color = clamp(strength* in_color, 0.0, 1.0);
}