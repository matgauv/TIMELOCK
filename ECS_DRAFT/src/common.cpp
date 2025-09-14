#include "common.hpp"

// Note, we could also use the functions from GLM but we write the transformations here to show the uderlying math
void Transform::scale(vec2 scale)
{
	mat3 S = { { scale.x, 0.f, 0.f },{ 0.f, scale.y, 0.f },{ 0.f, 0.f, 1.f } };
	mat = mat * S;
}

void Transform::rotate(float radians)
{
	float c = cosf(radians);
	float s = sinf(radians);
	mat3 R = { { c, s, 0.f },{ -s, c, 0.f },{ 0.f, 0.f, 1.f } };
	mat = mat * R;
}

void Transform::translate(vec2 offset)
{
	mat3 T = { { 1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ offset.x, offset.y, 1.f } };
	mat = mat * T;
}

// M1 Interpolation implementation (lerp formula)
float lerpToTarget(float current, float target, float time) {
	return current * (1.0f - time) + target * time;
}

// Smooth cubic interpolation
// Referred from The Book of Shaders: https://thebookofshaders.com/11/
float cubic_interpolation(float source, float target, float t) {
	float refined_t = t * t * (3.0f - 2.0f * t);

	return source * (1 - refined_t) + target * refined_t;
}

// Randonly sample a float number
float rand_float(float min, float max) {
	assert(min <= max);
	return min + (max - min) * ((float)std::max(0, rand()-1) / (float)RAND_MAX);
}

vec2 rand_direction() {
	float angle = rand_float(0.0, M_PI * 2.0f);

	return vec2{cosf(angle), sinf(angle)};
}

vec2 rotate_2D(vec2 v, float angle_rad) {
	float c = cosf(angle_rad);
	float s = sinf(angle_rad);

	return {
		c * v[0] - s * v[1],
		s * v[0] + c * v[1]
	};
}

vec2 angle_to_direction(float angle_rad) {
	return vec2{ cosf(angle_rad), sinf(angle_rad) };
}

vec2 random_sample_rectangle(vec2 center, vec2 dimensions, float angle_radians) {
	vec2 offset = {
		dimensions[0] * rand_float(-0.5f, 0.5f),
		dimensions[1] * rand_float(-0.5f, 0.5f),
	};

	return center + rotate_2D(offset, angle_radians);
}


vec2 random_sample_ellipse(vec2 center, vec2 dimensions, float angle_radians) {
	vec2 direction = rand_direction();
	float distance = sqrt(rand_float(1e-4f, 1.0f));

	vec2 offset = direction * distance;
	offset[0] *= (dimensions[0] * 0.5f);
	offset[1] *= (dimensions[1] * 0.5f);

	return center + rotate_2D(offset, angle_radians);
}

vec2 safe_normalize(vec2 v) {
	// Handles the case where v is zero vector
	float length = glm::length(v);

	if (length == 0.0) {
		return v;
	}

	return v / length;
}

bool gl_has_errors()
{
	return true;

	GLenum error = glGetError();

	if (error == GL_NO_ERROR) return false;

	while (error != GL_NO_ERROR)
	{
		const char* error_str = "";
		switch (error)
		{
		case GL_INVALID_OPERATION:
			error_str = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error_str = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error_str = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error_str = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error_str = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		fprintf(stderr, "OpenGL: %s", error_str);
		error = glGetError();
		assert(false);
	}

	return true;
}