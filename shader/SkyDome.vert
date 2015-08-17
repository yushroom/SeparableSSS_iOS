#version 300 es
layout(location = 0) in vec3 position;

out vec3 uv;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(position, 1);
	uv = position.xyz;
	uv.z = -uv.z;
}
