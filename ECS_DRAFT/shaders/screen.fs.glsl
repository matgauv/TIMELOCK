#version 330

uniform sampler2D screen_texture;
uniform float acc_act_factor;
uniform float acc_emerge_factor;
uniform float dec_act_factor;
uniform float dec_emerge_factor;

uniform float transition_factor;
uniform vec2 focal_point; // [0, 1]
uniform float aspect_ratio;

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

vec3 pale_filter(vec3 color, float factor) {
	float luminance = clamp(dot(vec3(0.299, 0.587, 0.114), color) * 1.4, 0.0, 1.0);
	vec3 paled_color = vec3(luminance) * vec3(0.77,0.91,0.96);
	return mix(color, paled_color, factor * 0.6);
}

vec4 apply_dec_effect(vec4 in_color)
{
	float effect_factor = min(1.0, dec_act_factor / dec_emerge_factor);

	// Ice shard effects
	int GRID_WIDE_COUNT = 16;
	int GRID_HIGH_COUNT = 9;
	int BOUNDARY_WIDE_COUNT = 2;
	int BOUNDARY_HIGH_COUNT = 1;

	vec2 texture_scale = textureSize(screen_texture, 0);
	float aspect_ratio = texture_scale.y/texture_scale.x;
	vec2 center = 0.5 * vec2(1.0, aspect_ratio);
	float HALF_AXIS = length(center);

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
			// TODO

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


	if (!render_effect) {
		return vec4(pale_filter(in_color.rgb, effect_factor), in_color.a);
	}

	float color_factor = clamp((length(min_pt - vec2(0.5, 0.5 * aspect_ratio)) - 0.3 * HALF_AXIS)/(0.7*HALF_AXIS), 0.0, 1.0) * 0.5 + 0.3;

	float silhouette_factor_raw = min_dist/GRID_WIDTH * 1.5;
	float silhouette_factor = clamp(silhouette_factor_raw, 0.0, 1.0);
	for (int i = 0; i < 2; i++) {
		silhouette_factor = silhouette_factor * silhouette_factor;
	}

	// Sample displaced texture
	vec4 displaced_color = texture(screen_texture, texcoord + deviation / aspect_ratio);

	// Apply border transparency
	float vignette_width = 0.1;

	float curr_width = vignette_width * min(1.0, dec_act_factor / dec_emerge_factor);

	float transparency_factor = 
		min(1.0, 
			min(min(texcoord.x, 1.0 - texcoord.x), min(texcoord.y, 1.0 - texcoord.y) * aspect_ratio) / curr_width);
	transparency_factor = clamp(transparency_factor * 2.0 - 1.0,0.0, 1.0);

	vec3 shard_color = mix(
		mix(pale_filter(displaced_color.rgb, effect_factor), vec3(0.56, 0.85, 1.0), color_factor), 
		vec3(0.87, 0.98, 0.98), silhouette_factor);

	return mix(
		vec4(shard_color, displaced_color.a), 
		vec4(pale_filter(in_color.rgb, effect_factor), in_color.a), transparency_factor);
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
	// float pixel_width = 
	vec2 focal_adjusted = vec2(aspect_ratio * focal_point.x, focal_point.y);
	vec2 uv_adjusted = vec2(aspect_ratio * texcoord.x, texcoord.y);
	vec2 disp = focal_adjusted - uv_adjusted;

	// Square of diagonal length
	float refined_factor = (1.0 - transition_factor) * (1.0 - transition_factor);
	float threshold_squared = refined_factor * refined_factor * (aspect_ratio * aspect_ratio + 1.0);
	
	if (disp.x*disp.x + disp.y * disp.y > threshold_squared) {
		return vec4(0.0, 0.0, 0.0, in_color.a);
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