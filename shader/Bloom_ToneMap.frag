#version 330 core

#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float3x3 mat3
#define float4x4 mat4
#define mul(v, m) (m) * (v)

#define N_PASSES 6

// Can be 13, 11, 9, 7 or 5
#define N_SAMPLES 13

#ifndef TONEMAP_OPERATOR
#define TONEMAP_OPERATOR 0
#endif

#define TONEMAP_LINEAR 0
#define TONEMAP_EXPONENTIAL 1
#define TONEMAP_EXPONENTIAL_HSV 2
#define TONEMAP_REINHARD 3
#define TONEMAP_FILMIC 4

uniform float exposure;
uniform float burnout;
uniform float bloomWidth;
uniform float bloomIntensity;
uniform float bloomThreshold;
uniform float defocus;

uniform float2 pixelSize;
uniform float2 direction;

uniform sampler2D finalTex;
uniform sampler2D srcTex[N_PASSES];

float3 rgb2hsv(float3 rgb)
{
    float minValue = min(min(rgb.r, rgb.g), rgb.b);
    float maxValue = max(max(rgb.r, rgb.g), rgb.b);
    float d = maxValue - minValue;

    float3 hsv = float3(0.0);
    hsv.b = maxValue;
    if (d != 0) {
        hsv.g = d / maxValue;

        float3 delrgb = (((vec3(maxValue) - rgb) / 6.0) + vec3(d) / 2.0) / d;
        if      (maxValue == rgb.r) { hsv.r = delrgb.b - delrgb.g; }
        else if (maxValue == rgb.g) { hsv.r = 1.0 / 3.0 + delrgb.r - delrgb.b; }
        else if (maxValue == rgb.b) { hsv.r = 2.0 / 3.0 + delrgb.g - delrgb.r; }

        if (hsv.r < 0.0) { hsv.r += 1.0; }
        if (hsv.r > 1.0) { hsv.r -= 1.0; }
    }
    return hsv;
}

float3 rgb2xyz(float3 rgb)
{
    const float3x3 m = float3x3 ( 0.5141364, 0.3238786,  0.16036376,
                         0.265068,  0.67023428, 0.06409157,
                         0.0241188, 0.1228178,  0.84442666 );
    return mul(rgb, m);
}

float3 xyz2Yxy(float3 xyz) {
    float w = xyz.r + xyz.g + xyz.b;
    if (w > 0.0) {
        float3 Yxy;
        Yxy.r = xyz.g;
        Yxy.g = xyz.r / w;
        Yxy.b = xyz.g / w;
        return Yxy;
    } else {
        return vec3(0.0);
    }
}

float3 Yxy2xyz(float3 Yxy) {
    float3 xyz;
    xyz.g = Yxy.r;
    if (Yxy.b > 0.0) {
        xyz.r = Yxy.r * Yxy.g / Yxy.b;
        xyz.b = Yxy.r * (1 - Yxy.g - Yxy.b) / Yxy.b;
    } else {
        xyz.r = 0.0;
        xyz.b = 0.0;
    }
    return xyz;
}

float3 xyz2rgb(float3 xyz) {
    const float3x3 m  = mat3 ( 2.5651, -1.1665, -0.3986,
                          -1.0217,  1.9777,  0.0439, 
                           0.0753, -0.2543,  1.1892 );
    return mul(xyz, m);
}

float3 hsv2rgb(float3 hsv) {
    float h = hsv.r;
    float s = hsv.g;
    float v = hsv.b;

    float3 rgb = vec3(v, v, v);
    if (hsv.g != 0.0) {
        float h_i = floor(6 * h);
        float f = 6 * h - h_i;

        float p = v * (1.0 - s);
        float q = v * (1.0 - f * s);
        float t = v * (1.0 - (1.0 - f) * s);

        if      (h_i == 0) { rgb = float3(v, t, p); }
        else if (h_i == 1) { rgb = float3(q, v, p); }
        else if (h_i == 2) { rgb = float3(p, v, t); }
        else if (h_i == 3) { rgb = float3(p, q, v); }
        else if (h_i == 4) { rgb = float3(t, p, v); }
        else               { rgb = float3(v, p, q); }
    }
    return rgb;
}

float3 FilmicTonemap(float3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((x*(A*x+C*B)+D*E) / (x*(A*x+B)+D*F))- E / F;
}

float3 DoToneMap(float3 color) {
    #if TONEMAP_OPERATOR == TONEMAP_LINEAR
    return exposure * color;
    #elif TONEMAP_OPERATOR == TONEMAP_EXPONENTIAL
    color = 1.0 - exp2(-exposure * color);
    return color;
    #elif TONEMAP_OPERATOR == TONEMAP_EXPONENTIAL_HSV
    color = rgb2hsv(color);
    color.b = 1.0 - exp2(-exposure * color.b);
    color = hsv2rgb(color);
    return color;
    #elif TONEMAP_OPERATOR == TONEMAP_REINHARD
    color = xyz2Yxy(rgb2xyz(color));
    float L = color.r;
    L *= exposure;
    float LL = 1 + L / (burnout * burnout);
    float L_d = L * LL / (1 + L);
    color.r = L_d;
    color = xyz2rgb(Yxy2xyz(color));
    return color;
    #else // TONEMAP_FILMIC
    color = 2.0f * FilmicTonemap(exposure * color);
    float3 whiteScale = 1.0f / FilmicTonemap(11.2);
    color *= whiteScale;
    return color;
    #endif
}

in vec2 uv;
out vec4 out_color;

void main()
{
    out_color = texture(finalTex, uv);
    out_color.rgb = DoToneMap(out_color.rgb);
}