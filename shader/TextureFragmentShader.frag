#version 330 core

// Interpolated values from the vertex shaders
in vec3 normal;
in vec2 UV;

// Ouput data
layout(location = 0) out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform samplerCube textureCube;

void main(){

	// Output color = color of the texture at the specified UV
	color = texture2D( myTextureSampler, UV ).rgb;
	//color = texture( textureCube, normal).rgb;
	// float ndcDepth = 
 //    (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) /
 //    (gl_DepthRange.far - gl_DepthRange.near);
	// float clipDepth = ndcDepth / gl_FragCoord.w;
	// color = vec3((clipDepth * 0.5) + 0.5); 
}