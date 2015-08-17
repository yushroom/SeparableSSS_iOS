#version 330 core

out vec4 out_color;

void main()
{
	//out_color = gl_FragCoord;

	//float ndc_depth = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
	//float clip_depth = ndc_depth / gl_FragCoord.w;
	float ndc_depth = (2.0 * gl_DepthRange.near) / (gl_DepthRange.far - gl_DepthRange.near - gl_FragCoord.z * (gl_DepthRange.far - gl_DepthRange.near));
	//float clip_depth = ndc_depth / gl_FragCoord.w;
	out_color = vec4(gl_FragCoord.z / gl_FragCoord.w / 5);
}