#version 330 core
#pragma optionNV(unroll all)

#define N_LIGHTS 5

in vec2 frag_uv;
in vec3 frag_curr_position;
in vec3 frag_prev_position;
in vec3 frag_world_position;
in vec3 frag_view;
in vec3 frag_normal;
in vec3 frag_tangent;
in vec3 frag_bitangent;

layout(location = 0) out vec4 out_color;
layout(location = 1) out float out_depth;
layout(location = 2) out vec2 out_velocity;
layout(location = 3) out vec4 out_specular_color;
layout(location = 4) out vec4 out_shadow;

struct Light {
    vec3 position;
    vec3 direction;
    float falloffStart;
    float falloffWidth;
    vec3 color;
    float attenuation;
    float farPlane;
    float bias;
    mat4 viewProjection;
};

uniform sampler2D diffuse_tex;
uniform sampler2D specularAO_tex;
uniform sampler2D normal_map_tex;
uniform sampler2D beckmann_tex;
uniform samplerCube irradiance_tex;
uniform sampler2DShadow shadow_maps[N_LIGHTS];
uniform sampler2D depth_textures[N_LIGHTS]; // the same textures with shadow_maps

uniform float bumpiness;	// for normal map
uniform float specularIntensity;
uniform float specularRoughness;
uniform float specularFresnel;
uniform float translucency;
uniform float sssWidth;
uniform float ambient;

uniform bool sssEnabled;
uniform bool sssTranslucencyEnabled;
uniform bool separate_speculars;

uniform Light lights[N_LIGHTS];

#define SSSSTexture2D sampler2D
#define SSSSSampleLevelZero(tex, coord) textureLod(tex, coord, 0.0)
#define SSSSSampleLevelZeroPoint(tex, coord) textureLod(tex, coord, 0.0)
#define SSSSSample(tex, coord) texture(tex, coord)
#define SSSSSamplePoint(tex, coord) texture(tex, coord)
#define SSSSSampleLevelZeroOffset(tex, coord, offset) textureLodOffset(tex, coord, 0.0, offset)
#define SSSSSampleOffset(tex, coord, offset) texture(tex, coord, offset)
#define SSSSLerp(a, b, t) mix(a, b, t)
#define SSSSSaturate(a) clamp(a, 0.0, 1.0)
#define SSSSMad(a, b, c) (a * b + c)
#define SSSSMul(v, m) (m * v)
#define SSSS_FLATTEN
#define SSSS_BRANCH
#define SSSS_UNROLL
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define float4x4 mat4x4

// get original z [(mvp * v).z] from z in shadowMap
float to_frag_z(float z)
{
	float far = 10.0f;
	float near = 0.1f;
	float frag_z = far*near / (far-z*(far-near));
	return (frag_z-near)*far / (far-near);
}

//-----------------------------------------------------------------------------
// Separable SSS Transmittance Function

float3 SSSSTransmittance(
        /**
         * This parameter allows to control the transmittance effect. Its range
         * should be 0..1. Higher values translate to a stronger effect.
         */
        float translucency,

        /**
         * This parameter should be the same as the 'SSSSBlurPS' one. See below
         * for more details.
         */
        float sssWidth,

        /**
         * Position in world space.
         */
        float3 worldPosition,

        /**
         * Normal in world space.
         */
        float3 worldNormal,

        /**
         * Light vector: lightWorldPosition - worldPosition.
         */
        float3 light,

        /**
         * Linear 0..1 shadow map.
         */
        SSSSTexture2D shadowMap,

        /**
         * Regular world to light space matrix.
         */
        float4x4 lightViewProjection,

        /**
         * Far plane distance used in the light projection matrix.
         */
        float lightFarPlane) {
    /**
     * Calculate the scale of the effect.
     */
    float scale = 8.25 * (1.0 - translucency) / sssWidth;
       
    /**
     * First we shrink the position inwards the surface to avoid artifacts:
     * (Note that this can be done once for all the lights)
     */
    float4 shrinkedPos = float4(worldPosition - 0.005 * worldNormal, 1.0);

    /**
     * Now we calculate the thickness from the light point of view:
     */
    float4 shadowPosition = SSSSMul(shrinkedPos, lightViewProjection);


 
	shadowPosition.xy /= shadowPosition.w;
	float d1 = SSSSSample(shadowMap, shadowPosition.xy).r;
	d1 = to_frag_z(d1);
	float d2 = shadowPosition.z;
	float d = scale * abs(d1 - d2);

    //float d1 = SSSSSample(shadowMap, shadowPosition.xy / shadowPosition.w).r; // 'd1' has a range of 0..1
    //float d2 = shadowPosition.z; // 'd2' has a range of 0..'lightFarPlane'
    //d1 *= lightFarPlane; // So we scale 'd1' accordingly:
    //float d = scale * abs(d1 - d2);

    /**
     * Armed with the thickness, we can now calculate the color by means of the
     * precalculated transmittance profile.
     * (It can be precomputed into a texture, for maximum performance):
     */
    float dd = -d * d;
    float3 profile = float3(0.233, 0.455, 0.649) * exp(dd / 0.0064) +
                     float3(0.1,   0.336, 0.344) * exp(dd / 0.0484) +
                     float3(0.118, 0.198, 0.0)   * exp(dd / 0.187)  +
                     float3(0.113, 0.007, 0.007) * exp(dd / 0.567)  +
                     float3(0.358, 0.004, 0.0)   * exp(dd / 1.99)   +
                     float3(0.078, 0.0,   0.0)   * exp(dd / 7.41);

    /** 
     * Using the profile, we finally approximate the transmitted lighting from
     * the back of the object:
     */
    return profile * SSSSSaturate(0.3 + dot(light, -worldNormal));
    //return vec3(1);
    //return vec3(textureProj(shadowMap, shadowPosition));
}


vec3 BumpMap(sampler2D normal_tex, vec2 uv)
{
	vec3 bump;
	bump.xy = -1.0 + 2.0 * texture2D(normal_tex, uv).gr;
	bump.z = sqrt(1.0 - bump.x * bump.x - bump.y * bump.y);
	//bump = -1.0 + 2.0 * texture2D(normal_tex, uv).rgb;
	return normalize(bump);
}

// H: half
float Fresnel(vec3 H, vec3 View, float f0)
{
	float base = 1.0 - dot(View, H);
	float exponential = pow(base, 5.0);
	return exponential + f0 * (1.0 - exponential);
}

float SpecularKSK(sampler2D beckmann_tex, vec3 normal, vec3 light, vec3 view, float roughness)
{
	vec3 H = view + light;
	vec3 HN = normalize(H);

	float NdotL = max(dot(normal, light), 0.0);
	float NdotH = max(dot(normal, HN), 0.0);

	float ph = pow(2.0 * texture2D(beckmann_tex, vec2(NdotH, roughness), 0).r, 10.0f);
	float f = mix(0.25, Fresnel(HN, view, 0.028), specularFresnel);
	float ksk = max(ph * f / dot(H, H), 0.0);

	return NdotL * ksk;
}


#define EPSILON -0.0000001

// TODO
float ShadowPCF(vec3 world_pos, int i, int samples, float width)
{
	// vec4 ShadowCoord = lights[i].viewProjection * world_pos;
	// ShadowCoord.xyz /= ShadowCoord.w;
	// //ShadowCoord.xyz *= 0.5f;
	// //ShadowCoord.xyz += 0.5f * ShadowCoord.w;
	// //float z = ShadowCoord.z / lights[i].farPlane;
	// float z = ShadowCoord.z;
	// //z += lights[i].bias;
	// //z = (z+1)/2;
	// //z += lights[i].bias;

	// float visibility = 0.0f;
	// int offset = (samples - 1) / 2;
	// for (int x = -offset; x <= offset; x += 1)
	// {
	// 	for (int y = -offset; y <= offset; y += 1)
	// 	{
	// 		vec2 pos = ShadowCoord.xy + width * vec2(x, y) / 2048.0f;
	// 		//pos = ShadowCoord.xy / ShadowCoord.w;
	// 		visibility += texture(shadow_maps[i], vec3(pos, z+EPSILON));
	// 	}
	// }
	// visibility /= samples * samples;
	// visibility = texture(shadow_maps[i], vec3(ShadowCoord.xy, ShadowCoord.z));
	// return visibility;
	return textureProj(shadow_maps[i], lights[i].viewProjection * vec4(world_pos, 1));
}

// float shadow_map_factor(int light_id)
// {
// 	vec4 ShadowCoord = lights[light_id].viewProjection * vec4(frag_world_position, 1.0f);
// 	float visibility = textureProj(shadow_maps[light_id], ShadowCoord);
// 	return visibility;
// }

vec3 degamma(vec3 color)
{
	//return pow(color, vec3(1.0 / 1.8));
	return color;
}

vec3 gamma(vec3 color)
{
	return pow(color, vec3(1.8));
}

void main()
{
	vec3 in_normal = normalize(frag_normal);
	vec3 tangent = normalize(frag_tangent);
	vec3 bitangent = normalize(frag_bitangent);
	mat3 tbn = mat3(tangent, bitangent, in_normal);

	vec2 uv_for_dds = vec2(frag_uv.x, 1- frag_uv.y);
	vec3 bump_normal = BumpMap(normal_map_tex, uv_for_dds);
	vec3 tangent_normal = mix(vec3(0, 0, 1), BumpMap(normal_map_tex, uv_for_dds), bumpiness);
	vec3 normal = tbn * tangent_normal;
	vec3 view = normalize(frag_view);

	vec4 albedo = texture2D( diffuse_tex, frag_uv );
	vec3 specularAO = texture2D( specularAO_tex, uv_for_dds).bgr;
	//specularAO = degamma(specularAO);

	float occlusion = specularAO.b;
	float intensity = specularAO.r * specularIntensity;
	float roughness = (specularAO.g / 0.3) * specularRoughness;

	out_color = vec4(0, 0, 0, 0);
	out_specular_color = vec4(0, 0, 0, 0);

	float temp_shadow = 0;
	vec3 temp_translucency = vec3(0);
	out_shadow = vec4(0);
	for (int i = 0; i < N_LIGHTS; i++)
	{
		vec3 L = lights[i].position - frag_world_position.xyz;
		float dist = length(L);
		L /= dist;

		float spot = dot(lights[i].direction, -L);
		if (spot > lights[i].falloffStart)
		//if (true)
		{
			float curve = min(pow(dist / lights[i].farPlane, 6.0), 1.0);
			float attenuation = mix(1.0 / (1.0 + lights[i].attenuation * dist * dist), 0, curve);

			spot = clamp((spot - lights[i].falloffStart) / lights[i].falloffWidth , 0, 1);

			vec3 f1 = lights[i].color * attenuation * spot;
			vec3 f2 = albedo.rgb * f1;

			vec3 diffuse = vec3(clamp(dot(L, normal), 0, 1));
			float specular = intensity * SpecularKSK(beckmann_tex, normal, L, view, roughness);

			float shadow;
			//shadow = shadow_map_factor(i);
			shadow = ShadowPCF( frag_world_position, i, 3, 1.0);
			temp_shadow += shadow * lights[i].color.r;

			out_specular_color.rgb += shadow * f1 * specular;
			out_color.rgb += shadow * f2 * diffuse;
			if (!separate_speculars)
			{
				out_color.rgb += shadow * f1 * specular;
			}

			temp_translucency = f2 * SSSSTransmittance(translucency, sssWidth, frag_world_position.xyz, normalize(frag_normal), L, depth_textures[i], lights[i].viewProjection, lights[i].farPlane);
			out_shadow.rgb += temp_translucency;
			// TODO ssss
			if (sssTranslucencyEnabled)
			{
				out_color.rgb += temp_translucency;
			}
		}
	}

	//out_shadow = vec4(temp_shadow / N_LIGHTS);
	//out_shadow = vec4(temp_translucency, 1);

	// Add the ambient component:
	vec3 env_irradiance = texture( irradiance_tex, normal ).rgb;
	env_irradiance = degamma(env_irradiance);
	out_color.rgb += occlusion * ambient * albedo.rgb * env_irradiance;

	// Store the SSS strength:
	out_specular_color.a = albedo.a;

	// Store the depth value:
	out_depth = 1.0 / gl_FragCoord.w;	//!

	// Convert to non-homogeneous points by dividing by w:
	out_velocity = frag_curr_position.xy / frag_curr_position.z -
					 frag_prev_position.xy / frag_prev_position.z;	// w is stored in z

	// Compress the velocity for storing it in a 8-bit render target:
	out_color.a = sqrt(5.0 * length(out_velocity));
}