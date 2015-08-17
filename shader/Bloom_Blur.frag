#version 330 core

#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float3x3 mat3
#define float4x4 mat4

#define N_PASSES 6

// Can be 13, 11, 9, 7 or 5
#define N_SAMPLES 13

uniform sampler2D srcTex;

in vec2 uv;
out vec4 out_color;

uniform vec2 step;

void main()
{
    #if N_SAMPLES == 13
    float offsets[] = float[] ( -1.7688, -1.1984, -0.8694, -0.6151, -0.3957, -0.1940, 0, 0.1940, 0.3957, 0.6151, 0.8694, 1.1984, 1.7688 );
    const int n = 13;
    #elif N_SAMPLES == 11
    float offsets[] = float[] { -1.6906, -1.0968, -0.7479, -0.4728, -0.2299, 0, 0.2299, 0.4728, 0.7479, 1.0968, 1.6906 );
    const float n = 11.0;
    #elif N_SAMPLES == 9
    float offsets[] = float[] { -1.5932, -0.9674, -0.5895, -0.2822, 0, 0.2822, 0.5895, 0.9674, 1.5932 );
    const float n = 9.0;
    #elif N_SAMPLES == 7
    float offsets[] = float[] { -1.4652, -0.7916, -0.3661, 0, 0.3661, 0.7916, 1.4652 );
    const float n = 7.0;
    #else
    float offsets[] = float[] { -1.282, -0.524, 0.0, 0.524, 1.282 );
    const float n = 5.0;
    #endif

    out_color = float4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < n; i++)
    {
        out_color += texture(srcTex, uv + step * offsets[i]);
    }
    out_color /=  n;
}