#version 300 es
precision mediump float;
in vec3 uv;

layout(location = 0) out vec3 color;

uniform float intensity;

uniform samplerCube skyTex;


void main()
{
	color = texture(skyTex, normalize(uv)).rgb;
}
