#version 300 es

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;

uniform mat4 mvp;

void main()
{
    //gl_Position = mvp * vec4(in_pos, 1.0);
    gl_Position = vec4(in_pos, 1.0f);
}
