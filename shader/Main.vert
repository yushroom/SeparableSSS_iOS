#version 330 core

in vec3 in_position;
in vec3 in_normal;
in vec3 in_tangent;
in vec2 in_uv;
in vec3 in_bitangent;

out vec2 frag_uv;
out vec3 frag_curr_position;
out vec3 frag_prev_position;
out vec3 frag_world_position;
out vec3 frag_view;
out vec3 frag_normal;
out vec3 frag_tangent;
out vec3 frag_bitangent;
//out vec3 frag_curr_position;
//out vec3 frag_prev_position;

uniform mat4 curr_model_view_proj;
uniform mat4 prev_model_view_proj;
uniform mat4 Model;
uniform mat4 ModelInverseTranspose;
uniform vec3 camera_position;

uniform vec2 jitter;

void main()
{
	gl_Position =  curr_model_view_proj * vec4(in_position, 1);

	frag_curr_position = gl_Position.xyw;
	frag_prev_position = (prev_model_view_proj * vec4(in_position, 1)).xyw;
	frag_curr_position.xy /= 2.0f;
	frag_prev_position.xy /= 2.0f;
	//frag_curr_position.y = -frag_curr_position.y;
	//frag_prev_position.y = -frag_prev_position.y;

	frag_uv = in_uv;
	frag_world_position = (Model * vec4(in_position, 1)).xyz;
	frag_view = camera_position - frag_world_position;
	frag_normal = mat3(ModelInverseTranspose) * in_normal;
	frag_tangent = mat3(ModelInverseTranspose) * in_tangent;
	frag_bitangent = mat3(ModelInverseTranspose) * in_bitangent;
}