#version 330 core

// Can be 13, 11, 9, 7 or 5
#define N_SAMPLES 13

uniform vec2 step;
//uniform vec2 pixelSize;
//uniform vec2 direction;
//uniform float blurWidth;
uniform float focusDistance;
uniform float focusRange;
uniform vec2 focusFalloff;

uniform sampler2D blurredTex;
uniform sampler2D depthTex;
uniform sampler2D cocTex;

in vec2 uv;

out vec4 out_color;

void main()
{
	//vec2 step = pixelSize * blurWidth * direction;

    #if N_SAMPLES == 13
    float offsets[] = float[]( -1.7688, -1.1984, -0.8694, -0.6151, -0.3957, -0.1940, 0.1940, 0.3957, 0.6151, 0.8694, 1.1984, 1.7688 );
    const int n = 12;
    #elif N_SAMPLES == 11
    float offsets[] = float[]( -1.6906, -1.0968, -0.7479, -0.4728, -0.2299, 0.2299, 0.4728, 0.7479, 1.0968, 1.6906 );
    const int n = 10.0;
    #elif N_SAMPLES == 9
    float offsets[] = float[]( -1.5932, -0.9674, -0.5895, -0.2822, 0.2822, 0.5895, 0.9674, 1.5932 );
    const float n = 8.0;
    #elif N_SAMPLES == 7
    float offsets[] = float[]( -1.4652, -0.7916, -0.3661, 0.3661, 0.7916, 1.4652 );
    const float n = 6.0;
    #else
    float offsets[] = float[]( -1.282, -0.524, 0.524, 1.282 );
    const float n = 4.0;
    #endif

    float CoC = texture2D(cocTex, uv).r;

    vec4 color = texture2D(blurredTex, uv);
    float sum = 1.0f;
    for (int i = 0; i < n; i++)
    {
    	float tapCoC = texture2D(cocTex, uv + step * offsets[i] * CoC).r;
    	vec4 tap = texture2D(blurredTex, uv + step * offsets[i] * CoC);

    	float contribution = tapCoC > CoC ? 1.0f : tapCoC;
    	color += contribution * tap;
    	sum += contribution;
    }

    out_color = color / sum;
}
