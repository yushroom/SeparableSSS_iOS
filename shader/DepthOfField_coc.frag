#version 330 core

// Can be 13, 11, 9, 7 or 5
#define N_SAMPLES 13

#define saturate(f) clamp((f), 0.0, 1.0)

uniform float focusDistance;
uniform float focusRange;
uniform vec2 focusFalloff;
uniform sampler2D depthTex;

in vec2 uv;
out vec4 out_color;


void main()
{
	float depth = texture2D(depthTex, uv).r;
	float d = abs(depth - focusDistance) - focusRange / 2.0f;
	if (d > 0.0)
	{
		float t = saturate(d);
		if (depth - focusDistance > 0.0)
			out_color = vec4(saturate( t * focusFalloff.x ));
		else
			out_color = vec4(saturate( t * focusFalloff.y ));
	}
	else
	{
		out_color = vec4(0);
	}
}
