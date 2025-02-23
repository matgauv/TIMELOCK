#version 330

uniform sampler2D screen_texture;
uniform float acc_act_factor;
uniform float acc_emerge_factor;
uniform float dec_act_factor;
uniform float dec_emerge_factor;
in vec2 texcoord;

uniform float time;

layout(location = 0) out vec4 color;

vec4 apply_dec_effect(vec4 in_color)
{
	/* These constants should be defined as global uniforms and be assigned from C++ end,
	 * But since these effects are subject to complete changes in later milestones,
	 * We will leave them hardcoded for M1
	 */
	float vignette_width = 0.1;
	float twinkle_period = 3500.0 / 6.2831853;
	vec3 dec_color = vec3(0.5, 0.9, 1.0);

	float curr_width = vignette_width * min(1.0, dec_act_factor / dec_emerge_factor);

	float transparency_factor = 
		1.0 - min(1.0, 
			min(min(texcoord.x, 1.0 - texcoord.x), min(texcoord.y, 1.0 - texcoord.y)) / curr_width);
	transparency_factor *= (0.5 + 0.3 * abs(sin(time/ twinkle_period)));

	return vec4(
		in_color.rgb * (1.0 - transparency_factor) + dec_color * transparency_factor,
		min(1.0, in_color.a + transparency_factor));
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

}