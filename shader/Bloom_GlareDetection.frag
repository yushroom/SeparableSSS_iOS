#version 330 core

#define float2 vec2
#define float3 vec3
#define float4 vec4

uniform float exposure;
uniform float bloomThreshold;

uniform float2 pixelSize;

uniform sampler2D finalTex;

in vec2 uv;

out vec4 out_color;

void main()
{
    float2 offsets[] = vec2[] ( 
        float2( 0.0,  0.0), 
        float2(-1.0,  0.0), 
        float2( 1.0,  0.0), 
        float2( 0.0, -1.0),
        float2( 0.0,  1.0)
    );

    float4 color = texture(finalTex, uv + offsets[0] * pixelSize);
    for (int i = 1; i < 5; i++)
    {
        color = min(texture(finalTex, uv + offsets[i] * pixelSize), color);
    }
    color.rgb *= exposure;

    out_color = float4(max(color.rgb - bloomThreshold / (1.0 - bloomThreshold), 0.0), color.a);
}