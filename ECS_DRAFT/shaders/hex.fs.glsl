#version 330

// From vertex shader
in vec2 texcoord;
in vec3 vcolor;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

uniform vec4 silhouette_color;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	vec4 sampled_color = vec4(fcolor * vcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));

	color = sampled_color;


	if (silhouette_color.a > 0.0) {
		float thickness = 0.035;
		
		// Test Boundary: if this pixel opaque & close to transparent pixel
		if (sampled_color.a > 0.95) {
			// y coordinate of texture needs to be inverted for DDS Textured mesh
			float dist_to_edge = min(
				min(texcoord.x, 1.0 - texcoord.x), min(-texcoord.y, 1.0 + texcoord.y)
			);

			if (dist_to_edge <= thickness) {
				color = silhouette_color;
			} else {
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						if (i == 0 && j == 0) {
							continue;
						}
						vec4 neighbor_color = vec4(fcolor * vcolor, 1.0) * texture(sampler0, vec2(texcoord.x + i * thickness, texcoord.y + j * thickness));

						if (neighbor_color.a < 0.05) {
							color = silhouette_color;
							break;
						}
					}
				}
			}
		}
	}
}