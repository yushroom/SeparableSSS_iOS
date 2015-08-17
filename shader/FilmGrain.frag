#version 330 core

uniform vec2 pixel_size;
uniform float noise_intensity;
uniform float exposure;
uniform float t;

uniform sampler2D tex_src;
uniform sampler3D tex_noise;

in vec2 uv;

out vec4 out_color;

vec3 overlay(vec3 a, float b)
{
	return pow(abs(b), 2.2) < 0.5f ? 2*a*b : 1.0-2.0*(1.0-a)*(1.0-b);
}

vec3 add_noise(vec3 color, vec2 uv)
{
	vec2 coord = uv * 2.0f;
	coord.x *= pixel_size.y / pixel_size.x;
	float noise = texture(tex_noise, vec3(coord, t)).r;
	float exposure_factor = sqrt(exposure / 2.0f);
	float t = mix(3.5f * noise_intensity, 1.13 * noise_intensity, exposure_factor);
	return overlay(color, mix(0.5f, noise, t));
}

void main()
{
	vec3 color = texture(tex_src, uv).rgb;
	color = add_noise(color, uv);
	out_color = vec4(color, 1.0);
}