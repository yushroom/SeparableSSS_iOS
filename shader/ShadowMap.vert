#version 330 core

in vec3 in_position;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(in_position, 1.0f);
	//gl_Position.z *= gl_Position.w / 10.0f; // 100 far plane
	//gl_Position.z *= gl_Position.w; // We want linear positions
}