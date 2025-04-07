#version 330

uniform sampler2D screen_texture;
uniform sampler2D loading_texture;

uniform float acc_act_factor;
uniform float acc_emerge_factor;
uniform float dec_act_factor;
uniform float dec_emerge_factor;

uniform int GRID_WIDE_COUNT;// = 16;
uniform int GRID_HIGH_COUNT;// = 9;
uniform int BOUNDARY_WIDE_COUNT;// = 2;
uniform int BOUNDARY_HIGH_COUNT;// = 2;

uniform float VIGNETTE_WIDTH; //=1.25
uniform vec3 PALED_BLUE_TONE;//vec3(0.77,0.91,0.96);
uniform vec3 SHARD_COLOR_1;// vec3(0.573, 0.812, 1)
uniform vec3 SHARD_COLOR_2;// vec3(0.475, 0.745, 0.961)
uniform vec3 SHARD_SILHOUETTE_COLOR;//vec3(0.87, 0.98, 0.98)
uniform float SHARD_EVOLVING_SPEED;//0.001

uniform float transition_factor;
uniform vec2 focal_point; // [0, 1]

uniform float time;

in vec2 texcoord;

layout(location = 0) out vec4 color;

// Random value from 2d input
float random(vec2 uv) {
	return fract(651.464 * sin(dot(uv, vec2(61.5135, -2454.51))));
}

// Random vector from 2d input
vec2 random2D(vec2 uv) {
	return fract(5641.84 * sin(
		vec2(
			dot(uv, vec2(-352.36, 84.351)), 
			dot(uv, vec2(686.139, 9318.4)))));
}

// Reference: https://thebookofshaders.com/11/
float perlinNoise(vec2 uv) {
	float NOISE_SCALE = 16.0;
	vec2 uv_scaled = uv * NOISE_SCALE;

	vec2 uv_i = floor(uv_scaled);
	vec2 uv_f = fract(uv_scaled);

	vec2 refined_uv_f = uv_f * uv_f * (2.0 - uv_f);

	return mix(
		mix(random(mod(uv_i, NOISE_SCALE)), random(mod(uv_i + vec2(1.0, 0.0), NOISE_SCALE)), refined_uv_f.x),
		mix(random(mod(uv_i + vec2(0.0, 1.0), NOISE_SCALE)), random(mod(uv_i + vec2(1.0, 1.0), NOISE_SCALE)), refined_uv_f.x),
		refined_uv_f.y
	);
}

float toAngle(vec2 uv) {
	if (abs(uv.x) < 1e-8) {
		return uv.y > 0 ? 1.571 : 4.712;
	}

	float angle = atan(uv.y, uv.x);

	if (angle < 0) {
		angle += 6.283;
	}
	return angle;
}

vec3 pale_filter(vec3 color, float factor) {
	// Gray scale formula: https://en.wikipedia.org/wiki/Grayscale
	float luminance = clamp(dot(vec3(0.299, 0.587, 0.114), color) * 1.4, 0.0, 1.0);
	vec3 paled_color = vec3(luminance) * PALED_BLUE_TONE;
	return mix(color, paled_color, factor * 0.4);
}

vec4 apply_dec_effect(vec4 in_color)
{
	float effect_factor = min(1.0, dec_act_factor / dec_emerge_factor);

	// Ice shard effects
	vec2 texture_scale = textureSize(screen_texture, 0);
	float aspect_ratio = texture_scale.y/texture_scale.x;
	vec2 center = 0.5 * vec2(1.0, aspect_ratio);

	float GRID_WIDTH = 1.0/float(GRID_WIDE_COUNT);
	float GRID_HEIGHT = aspect_ratio/float(GRID_HIGH_COUNT);

	vec2 uv_refined = texcoord * vec2(1.0, aspect_ratio);

	float min_dist = length(uv_refined - center);
	vec2 min_pt = center;
	// Centre point
	int min_pt_index_x = -1;
	int min_pt_index_y = -1;
	vec2 deviation = vec2(0.0);

	int original_index_x = int(uv_refined.x/GRID_WIDTH);
	int original_index_y = int(uv_refined.y/GRID_HEIGHT);

	// Sample local group; adjust dimension if grid number changes
	// Cellular noise (Voronoi): https://thebookofshaders.com/12/
	for (int i = -2; i <= 2; i++) {
		for (int j = -1; j <=1; j++) {
			int grid_x = i + original_index_x;
			int grid_y = j + original_index_y;

			if (grid_x < 0 || grid_x >= GRID_WIDE_COUNT || grid_y < 0 || grid_y >= GRID_HIGH_COUNT ||
				((BOUNDARY_WIDE_COUNT < grid_x && grid_x < GRID_WIDE_COUNT - BOUNDARY_WIDE_COUNT - 1) && 
		  		 (BOUNDARY_HIGH_COUNT < grid_y && grid_y < GRID_HIGH_COUNT - BOUNDARY_HIGH_COUNT - 1))) {
					continue;
			}

			vec2 grid_center = vec2((float(grid_x) + 0.5) * GRID_WIDTH, (float(grid_y) + 0.5) * GRID_HEIGHT);

			vec2 rand_vec = random2D(grid_center);
			float rand_scalar = random(grid_center);
			vec2 point = grid_center + (sin(time* (0.001 + 0.0015 * rand_scalar) + rand_vec * 6.283) * GRID_WIDTH * (0.1 + 0.3 * (1.0 - rand_scalar)));

			vec2 inflated_point_pos = mix(
				1.3*(point - center) + center, 
				point, 
				effect_factor);

			float dist = length(inflated_point_pos - uv_refined);

			if (dist < min_dist) {
				min_dist = dist;
				min_pt = inflated_point_pos;
				min_pt_index_x = grid_x;
				min_pt_index_y = grid_y;
				deviation = point - grid_center;
			}
		}
	}

	bool render_effect = (
		(min_pt_index_x >= 0 && min_pt_index_y >= 0) && 
		!((BOUNDARY_WIDE_COUNT <= min_pt_index_x && min_pt_index_x <= GRID_WIDE_COUNT - BOUNDARY_WIDE_COUNT - 1) && 
		  (BOUNDARY_HIGH_COUNT <= min_pt_index_y && min_pt_index_y <= GRID_HIGH_COUNT - BOUNDARY_HIGH_COUNT - 1)));

	float curr_width = VIGNETTE_WIDTH * min(1.0, dec_act_factor / dec_emerge_factor);

	vec4 base_color = vec4(0.0);

	if (!render_effect) {
		base_color = vec4(pale_filter(in_color.rgb, effect_factor), in_color.a);
	} else {

		float color_factor = 2.0 * sin(random(vec2(float(min_pt_index_x), float(min_pt_index_y))) * 6.283 + time * SHARD_EVOLVING_SPEED) - 1.0;
		vec3 shard_tinted_color = mix(SHARD_COLOR_1, SHARD_COLOR_2, color_factor);

		float silhouette_factor_raw = min_dist/GRID_WIDTH * 1.5;
		float silhouette_factor = clamp(silhouette_factor_raw, 0.0, 1.0);
		for (int i = 0; i < 2; i++) {
			silhouette_factor = silhouette_factor * silhouette_factor;
		}

		// Apply border transparency
		// displace sample coordinate radially
		vec2 disp_from_center = texcoord - 0.5;
		if (length(disp_from_center) > 1e-8) {
			disp_from_center = normalize(disp_from_center);
		}
		float normalized_angle = toAngle(texcoord - 0.5)/6.283;
	
		vec2 radial_disp_coord = 0.04*(perlinNoise(vec2(normalized_angle, time*0.0001)) - 0.5) * disp_from_center + texcoord;

		float transparency_factor = 
			min(1.0, 
				min(min(radial_disp_coord.x, 1.0 - radial_disp_coord.x) * 1.2, 
					  min(radial_disp_coord.y, 1.0 - radial_disp_coord.y) * 1.5 * aspect_ratio) / curr_width);
		transparency_factor = clamp(transparency_factor * 1.5 - 0.50,0.0, 1.0);

		// Sample displaced texture
		vec4 displaced_color = texture(screen_texture, texcoord + (1.0 - transparency_factor) * deviation / aspect_ratio);

		vec3 shard_color = clamp(shard_tinted_color + SHARD_SILHOUETTE_COLOR * silhouette_factor, 0.0, 1.3);

		base_color = mix(
		vec4(shard_color, 1.0), 
		vec4(pale_filter(displaced_color.rgb, effect_factor), in_color.a), 
		transparency_factor);
	}

	float edge_brightness = 
		1.0 - min(1.0, 
			min(min(texcoord.x, 1.0 - texcoord.x), min(texcoord.y, 1.0 - texcoord.y) * aspect_ratio) / (0.05*curr_width));

	return vec4(base_color.rgb + vec3(0.5 * edge_brightness), base_color.a);
}

vec4 apply_acc_effect(vec4 in_color)
{
	float vignette_width = 0.15;
	float twinkle_period = 600.0 / 6.2831853;
	float spike_frequency = 20.0;
	vec3 acc_color = vec3(1.0, 0.25, 0.2);
	float PI = 3.1415927;

	float angle = atan(texcoord.x - 0.5, texcoord.y - 0.5);

	float polar_dist = ((-0.75 * PI < angle && angle < -0.25 * PI) || (0.25 * PI < angle && angle < 0.75 * PI) ?
		(0.5 / sin(abs(angle))) :
		(0.5 / abs(cos(angle))));

	float curr_width = vignette_width * min(1.0, acc_act_factor / acc_emerge_factor);
	curr_width *= (0.8 + 0.2 * sin(spike_frequency * angle - time/twinkle_period));

	float transparency_factor = min(1.0, max(0.0, 
		(length(texcoord - 0.5) - (polar_dist - curr_width)) / curr_width));

	return vec4(
		in_color.rgb * (1.0 - transparency_factor) + acc_color * transparency_factor,
		min(1.0, in_color.a + transparency_factor));
}


vec4 apply_transition_effect(vec4 in_color) {
	vec2 texture_scale = textureSize(screen_texture, 0);
	float aspect_ratio = texture_scale.y/texture_scale.x;

	vec2 focal_adjusted = vec2(focal_point.x, aspect_ratio * focal_point.y);
	vec2 uv_adjusted = vec2(texcoord.x, aspect_ratio * texcoord.y);
	vec2 disp = focal_adjusted - uv_adjusted;

	// Square of diagonal length
	float refined_factor = (1.0 - transition_factor) * (1.0 - transition_factor);
	float threshold_squared = refined_factor * refined_factor * (aspect_ratio * aspect_ratio + 1.0);
	
	if (disp.x*disp.x + disp.y * disp.y > threshold_squared) {
		return texture2D(loading_texture, vec2(texcoord.x, 1.0-texcoord.y));
	}
	else {
		return in_color;
	}
}


void main()
{
    vec4 in_color = texture(screen_texture, texcoord);
	color = in_color;

	if (acc_act_factor > 0) {
		color = apply_acc_effect(color);
	}

	if (dec_act_factor > 0) {
		color = apply_dec_effect(color);
	}

	if (transition_factor > 0) {
		color = apply_transition_effect(color);
	}
}