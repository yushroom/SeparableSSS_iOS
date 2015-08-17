#version 330 core

in vec2 uv;

layout(location = 0)out vec4 color;

uniform sampler2D tex;

void main()
{
	color = vec4(texture2D( tex, uv).xyz, 1.0f);
}