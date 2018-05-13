//=================================================================================================
//
//  Specular AA Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

//=================================================================================================
// Includes
//=================================================================================================
#include "SharedConstants.h"
#include "SampleFramework11\\Shaders\\SH.hlsl"

//=================================================================================================
// Constants
//=================================================================================================
static const uint NumCascades = 4;

#if ShaderSS_ == 1
    #define ShaderSupersampling_ 1
#elif ShaderSS_ == 2
    #define TextureSpaceLighting_ 1
#endif

#if ShaderAAMode_ == 1
    #define UseVMF_ 1
#elif ShaderAAMode_ == 2
    #define UseLEAN_ 1
#elif ShaderAAMode_ == 3
    #define UseLEAN_ 1
    #define UseCLEAN_ 1
#elif ShaderAAMode_ == 4
    #define UseToksvig_ 1
#elif ShaderAAMode_ == 5
    #define UsePrecomputedVMF_ 1
#elif ShaderAAMode_ == 6
    #define UsePrecomputedToksvig_ 1
#endif

//=================================================================================================
// Constant buffers
//=================================================================================================
cbuffer VSConstants : register(b0)
{
    float4x4 World;
	float4x4 View;
    float4x4 WorldViewProjection;
}

cbuffer PSConstants : register(b0)
{
    float3 LightDirWS;
    float3 LightColor;
    float3 CameraPosWS;
    float Roughness;
    uint ShaderSSSamples;
    uint2 RenderTargetSize;
    float SampleRadius;
    float ScaleFactor;
    bool EnableDiffuse;
    bool EnableSpecular;
    bool VMFDiffuseAA;
}

//=================================================================================================
// Resources
//=================================================================================================
Texture2D NormalMap : register(t0);
Texture2DArray<float4> LEANMap : register(t1);

#if NumVMFs > 1
    Texture2DArray<float4> VMFMap : register(t2);
#else
    Texture2D<float4> VMFMap : register(t2);
#endif

Texture2D<float2> RoughnessMap : register(t3);

StructuredBuffer<float2> SampleOffsets : register(t4);

Texture2D LightingMap : register(t0);

SamplerState AnisoSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);
SamplerState LinearSampler : register(s2);

//=================================================================================================
// Input/Output structs
//=================================================================================================
struct VSInput
{
    float4 PositionOS 		: POSITION;
    float3 NormalOS 		: NORMAL;
    float2 TexCoord 		: TEXCOORD0;
	float3 TangentOS 		: TANGENT;
	float3 BitangentOS		: BITANGENT;
};

struct VSOutput
{
    #if ShaderSupersampling_
        float4 PositionCS 		: POSITIONCS;
    #else
        float4 PositionCS 		: SV_Position;
    #endif

    float3 NormalWS 		: NORMALWS;
	float3 TangentWS 		: TANGENTWS;
	float3 BitangentWS 		: BITANGENTWS;
    float DepthVS			: DEPTHVS;

    float3 PositionWS 		: POSITIONWS;
	float2 TexCoord 		: TEXCOORD;
};

struct PSInput
{
    float4 PositionSS 		: SV_Position;
    float3 NormalWS 		: NORMALWS;
    float3 TangentWS 		: TANGENTWS;
	float3 BitangentWS 		: BITANGENTWS;
    float DepthVS			: DEPTHVS;

    #if ShaderSupersampling_
        nointerpolation float2 TexCoord0 		: TEXCOORD0;
        nointerpolation float2 TexCoord1 		: TEXCOORD1;
        nointerpolation float2 TexCoord2 		: TEXCOORD2;
        nointerpolation float3 PositionWS0 		: POSITIONWS0;
        nointerpolation float3 PositionWS1 		: POSITIONWS1;
        nointerpolation float3 PositionWS2 		: POSITIONWS2;
        float3 Barycentrics                     : BARYCENTRICS;
    #else
        float3 PositionWS 		                : POSITIONWS;
        float2 TexCoord 		                : TEXCOORD;
    #endif
};

//=================================================================================================
// Vertex Shader
//=================================================================================================
VSOutput VS(in VSInput input, in uint VertexID : SV_VertexID)
{
    VSOutput output;

    // Calc the world-space position
    output.PositionWS = mul(input.PositionOS, World).xyz;

	// Calc the view-space depth
	output.DepthVS = mul(float4(output.PositionWS, 1.0f), View).z;

    #if TextureSpaceLighting_
        output.PositionCS = float4(input.TexCoord * 2.0f - 1.0f, 1.0f, 1.0f);
        output.PositionCS.y *= -1.0f;
    #else
        // Calc the clip-space position
        output.PositionCS = mul(input.PositionOS, WorldViewProjection);
    #endif

	// Rotate the normal into world space
    output.NormalWS = normalize(mul(input.NormalOS, (float3x3)World));

	// Rotate the rest of the tangent frame into world space
	output.TangentWS = normalize(mul(input.TangentOS, (float3x3)World));
	output.BitangentWS = normalize(mul(input.BitangentOS, (float3x3)World));

    // Pass along the texture coordinate
    output.TexCoord = input.TexCoord;

    return output;
}

#if ShaderSupersampling_

//=================================================================================================
// Geometry Shader, only uses for shader supersampling mode
//=================================================================================================
[maxvertexcount(3)]
void GS(in triangle VSOutput input[3], inout TriangleStream<PSInput> TriStream)
{
    PSInput output;
    output.TexCoord0 = input[0].TexCoord;
    output.TexCoord1 = input[1].TexCoord;
    output.TexCoord2 = input[2].TexCoord;

    output.PositionWS0 = input[0].PositionWS;
    output.PositionWS1 = input[1].PositionWS;
    output.PositionWS2 = input[2].PositionWS;

    // Emit 3 new verts, and 1 new triangle
	[unroll]
	for(uint i = 0; i < 3; i++)
	{
		output.PositionSS = input[i].PositionCS;
        output.NormalWS = input[i].NormalWS;
        output.TangentWS = input[i].TangentWS;
        output.BitangentWS = input[i].BitangentWS;
        output.DepthVS = input[i].DepthVS;

        // Output barycentric coordinates for each vert, so that we can do manual interpolation
        // in the pixel shader
        if(i == 0)
            output.Barycentrics = float3(1, 0, 0);
        else if(i == 1)
            output.Barycentrics = float3(0, 1, 0);
        else
            output.Barycentrics = float3(0, 0, 1);

		TriStream.Append(output);
	}

	TriStream.RestartStrip();
}

//=================================================================================================
// Interpolates texture coordinates using barycentric coordinates
//=================================================================================================
float2 InterpolateUV(in PSInput input, in float3 barycentrics)
{
    return input.TexCoord0 * barycentrics.x +
           input.TexCoord1 * barycentrics.y +
           input.TexCoord2 * barycentrics.z;
}

//=================================================================================================
// Interpolates vertex positions using barycentric coordinates
//=================================================================================================
float3 InterpolatePosition(in PSInput input, in float3 barycentrics)
{
    return input.PositionWS0 * barycentrics.x +
           input.PositionWS1 * barycentrics.y +
           input.PositionWS2 * barycentrics.z;
}

#endif // ShaderSupersampling_

// ================================================================================================
// Calculates the Fresnel factor using Schlick's approximation
// ================================================================================================
float3 Fresnel(in float3 specAlbedo, in float3 h, in float3 l)
{
    float lDotH = saturate(dot(l, h));
    float3 fresnel = specAlbedo + (1.0f - specAlbedo) * pow((1.0f - lDotH), 5.0f);

    // Disable specular entirely if the albedo is set to 0.0
    fresnel *= dot(specAlbedo, 1.0f) > 0.0f;

    return fresnel;
}

// ===============================================================================================
// Helper for computing the Beckmann geometry term
// ===============================================================================================
float Beckmann_G1(float m, float nDotX)
{
    float nDotX2 = nDotX * nDotX;
    float tanTheta = sqrt((1 - nDotX2) / nDotX2);
    float a = 1.0f / (m * tanTheta);
    float a2 = a * a;

    float g = 1.0f;
    if(a < 1.6f)
        g *= (3.535f * a + 2.181f * a2) / (1.0f + 2.276f * a + 2.577f * a2);

    return g;
}

// ===============================================================================================
// Computes the specular term using a Beckmann microfacet distribution, with a matching
// geometry factor and visibility term. Based on "Microfacet Models for Refraction Through
// Rough Surfaces" [Walter 07]. m is roughness, n is the surface normal, h is the half vector,
// l is the direction to the light source, and specAlbedo is the RGB specular albedo
// ===============================================================================================
float Beckmann_Specular(in float m, in float3 n, in float3 h, in float3 v, in float3 l)
{
    float nDotH = max(dot(n, h), 0.0001f);
    float nDotL = saturate(dot(n, l));
    float nDotV = max(dot(n, v), 0.0001f);

    float nDotH2 = nDotH * nDotH;
    float nDotH4 = nDotH2 * nDotH2;
    float m2 = m * m;

    // Calculate the distribution term
    float tanTheta2 = (1 - nDotH2) / nDotH2;
    float expTerm = exp(-tanTheta2 / m2);
    float d = expTerm / (Pi * m2 * nDotH4);

    // Calculate the matching geometric term
    float g1i = Beckmann_G1(m, nDotL);
    float g1o = Beckmann_G1(m, nDotV);
    float g = g1i * g1o;

    return d * g * (1.0f / (4.0f * nDotL * nDotV));
}

// ===============================================================================================
// Helper for computing the GGX visibility term
// ===============================================================================================
float GGX_V1(in float m2, in float nDotX)
{
    return 1.0f / (nDotX + sqrt(m2 + (1 - m2) * nDotX * nDotX));
}

// ===============================================================================================
// Computes the specular term using a GGX microfacet distribution, with a matching
// geometry factor and visibility term. Based on "Microfacet Models for Refraction Through
// Rough Surfaces" [Walter 07]. m is roughness, n is the surface normal, h is the half vector,
// l is the direction to the light source, and specAlbedo is the RGB specular albedo
// ===============================================================================================
float GGX_Specular(in float m, in float3 n, in float3 h, in float3 v, in float3 l)
{
    float nDotH = saturate(dot(n, h));
    float nDotL = saturate(dot(n, l));
    float nDotV = saturate(dot(n, v));

    float nDotH2 = nDotH * nDotH;
    float m2 = m * m;

    // Calculate the distribution term
    float d = m2 / (Pi * pow(nDotH * nDotH * (m2 - 1) + 1, 2.0f));

    // Calculate the matching visibility term
    float v1i = GGX_V1(m2, nDotL);
    float v1o = GGX_V1(m2, nDotV);
    float vis = v1i * v1o;

    return d * vis;
}

// ================================================================================================
// Converts a Beckmann roughness parameter to a Phong specular power
// ================================================================================================
float RoughnessToSpecPower(in float m) {
    return 2.0f / (m * m) - 2.0f;
}

// ================================================================================================
// Converts a Blinn-Phong specular power to a Beckmann roughness parameter
// ================================================================================================
float SpecPowerToRoughness(in float s) {
    return sqrt(2.0f / (s + 2.0f));
}

// ================================================================================================
// Calculates lighting for a directional light
// ================================================================================================
float3 CalcLighting(in float3 normal, in float3 lightDir, in float3 lightColor,
					in float3 diffuseAlbedo, in float3 specularAlbedo, in float roughness,
					in float3 positionWS)
{
    float3 lighting = 0.0f;
    float nDotL = saturate(dot(normal, lightDir));
    if(nDotL > 0.0f)
    {
        float3 view = normalize(CameraPosWS - positionWS);
        float3 h = normalize(view + lightDir);

        #if UseGGX_
            float specular = GGX_Specular(roughness, normal, h, view, lightDir);
        #else
            float specular = Beckmann_Specular(roughness, normal, h, view, lightDir);
        #endif

        float3 fresnel = Fresnel(specularAlbedo, h, lightDir);

        lighting = (diffuseAlbedo * InvPi + specular * fresnel) * nDotL * lightColor;
    }

    return max(lighting, 0.0f);
}

// ================================================================================================
// Calculates lighting for a point light
// ================================================================================================
float3 CalcPointLight(in float3 normal, in float3 lightColor, in float3 diffuseAlbedo,
					  in float3 specularAlbedo, in float roughness, in float3 positionWS,
					  in float3 lightPos, in float lightFalloff)
{
	float3 pixelToLight = lightPos - positionWS;
	float lightDist = length(pixelToLight);
	float3 lightDir = pixelToLight / lightDist;
	float attenuation = 1.0f / pow(lightDist, lightFalloff);
	return CalcLighting(normal, lightDir, lightColor, diffuseAlbedo, specularAlbedo,
						roughness, positionWS) * attenuation;
}

#if UseVMF_

// ================================================================================================
// Computes the SH coefficient of a vMF lobe for a given order (l)
// ================================================================================================
float VMFSHCoefficient(in float kappa, in float l)
{
    return exp(-(l * l) / (2.0f * kappa));
}

// ================================================================================================
// Calculates lighting for a directional light, taking into account an NDF represented as
// vMF lobes
// ================================================================================================
float3 CalcLightingVMF(in VMF vmfs[NumVMFs], in float3 lightDir, in float3 lightColor,
					   in float3 diffuseAlbedo, in float3 specularAlbedo, in float roughness,
					   in float3 positionWS)
{
    float3 lighting = 0.0f;

    [unroll]
    for(uint i = 0; i < NumVMFs; ++i)
    {
        float3 mu = vmfs[i].mu;
        float kappa = vmfs[i].kappa;
        float alpha = vmfs[i].alpha;
        float3 normal = mu;

        // Calculate a new roughness value
        // (equation 21 in "Frequency Domain Normal Map Filtering")
        float lobeRoughness = sqrt(roughness * roughness + 1.0f / (2.0f * kappa));
        lighting += CalcLighting(normal, lightDir, lightColor, 0.0f,
                                 specularAlbedo, lobeRoughness, positionWS) * alpha;

        // Calculate the diffuse by converting the NDF vMF lobe to SH coefficients,
        // convolving with the diffuse BRDF to get an effective BRDF, and convolving
        // that with the lighting environment
        float ndfA0 = VMFSHCoefficient(kappa, 0);
        float ndfA1 = VMFSHCoefficient(kappa, 1);
        float ndfA2 = VMFSHCoefficient(kappa, 2);

        if(VMFDiffuseAA)
        {
            SH9Color lightingEnv = ProjectOntoSH9(lightDir, lightColor, ndfA0, ndfA1, ndfA2);
            lighting += EvalSH9Cosine(normal, lightingEnv) * diffuseAlbedo;
        }
        else
        {
            lighting += saturate(dot(normal, lightDir)) * diffuseAlbedo * InvPi * lightColor;
        }
    }

    return lighting;
}

// ================================================================================================
// Calculates lighting for a point light, taking into account an NDF represented as
// vMF lobes
// ================================================================================================
float3 CalcPointLightVMF(in VMF vmfs[NumVMFs], in float3 lightColor, in float3 diffuseAlbedo,
					     in float3 specularAlbedo, in float roughness, in float3 positionWS,
					     in float3 lightPos, in float lightFalloff)
{
	float3 pixelToLight = lightPos - positionWS;
	float lightDist = length(pixelToLight);
	float3 lightDir = pixelToLight / lightDist;
	float attenuation = 1.0f / pow(lightDist, lightFalloff);
	return CalcLightingVMF(vmfs, lightDir, lightColor, diffuseAlbedo, specularAlbedo,
						   roughness, positionWS) * attenuation;
}

#elif UseLEAN_


// ================================================================================================
// Calculates lighting for a directional light using LEAN or CLEAN mapping
// ================================================================================================
float3 CalcLightingLEAN(in float3 normal, in float3 lightDir, in float3 lightColor,
                        in float3 diffuseAlbedo, in float3 specularAlbedo, in float roughness,
                        in float3 positionWS, in float2 leanB, in float4 leanM,
                        in float3x3 worldToTangent)
{
    float3 lighting = 0.0f;

    float nDotL = saturate(dot(normal, lightDir));
    if(nDotL > 0.0f)
    {
        float3 diffuse = nDotL * lightColor;
        float3 view = normalize(CameraPosWS - positionWS);

        float s = RoughnessToSpecPower(roughness);
        float invS = 1.0f / s;

        float3 ht = normalize(mul(normalize(view + lightDir), worldToTangent));

        #if UseCLEAN_
            // CLEAN mapping
            float2 B = leanB * ScaleFactor;
            float M = leanM.w * ScaleFactor * ScaleFactor;

            float2 h = ht.xy / ht.z;

            float variance = M - (B.x * B.x + B.y * B.y);
            variance += invS;

            float d = 0.0f;
            float e = ((h.x - B.x) * (h.x - B.x)) + ((h.y - B.y) * (h.y - B.y));
            if(ht.z > 0.0f && variance > 0.0f)
                d = exp(-0.5f * e / variance) / (variance * Pi * 2);
        #else
            // LEAN mapping
            float2 B  = leanB * ScaleFactor;
            float3 M = leanM.xyz * ScaleFactor * ScaleFactor;
            M.xy += invS;

            float3 sigma = M - float3(B * B, B.x * B.y);
            float det = sigma.x * sigma.y - sigma.z * sigma.z;

            float d = 0.0f;
            float2 h = ht.xy / ht.z - B;
            float e = (h.x * h.x * sigma.y + h.y * h.y * sigma.x - 2 * h.x * h.y * sigma.z);
            if(ht.z > 0.0f && det > 0.0f)
                d = exp(-0.5f * e / det) / (sqrt(det) * Pi * 2);
        #endif

        // -- geometric term
        float m = roughness;

        float3 n = normal;
        float3 l = lightDir;
        float3 v = view;

        float nDotV = max(dot(n, v), 0.0001f);

        float g1i = Beckmann_G1(m, nDotL);
        float g1o = Beckmann_G1(m, nDotV);
        float g = g1i * g1o;

        float specular = d * g * (1.0f / (4.0f * nDotL * nDotV));

        float3 fresnel = Fresnel(specularAlbedo, normalize(view + lightDir), lightDir);

        lighting = diffuse * diffuseAlbedo * InvPi + specular * fresnel * diffuse;
    }

    return max(lighting, 0.0f);
}

// ================================================================================================
// Calculates lighting for a point light using LEAN or CLEAN mapping
// ================================================================================================
float3 CalcPointLightLEAN(in float3 normal, in float3 lightColor, in float3 diffuseAlbedo,
                          in float3 specularAlbedo, in float roughness, in float3 positionWS,
                          in float3 lightPos, in float lightFalloff, in float2 leanB, in float4 leanM,
                          in float3x3 worldToTangent)
{
	float3 pixelToLight = lightPos - positionWS;
	float lightDist = length(pixelToLight);
	float3 lightDir = pixelToLight / lightDist;
	float attenuation = 1.0f / pow(lightDist, lightFalloff);
	return CalcLightingLEAN(normal, lightDir, lightColor, diffuseAlbedo, specularAlbedo,
                            roughness, positionWS, leanB, leanM, worldToTangent) * attenuation;
}

#endif

// ================================================================================================
// Applies a generalized cubic filter given a distance x
// ================================================================================================
float FilterCubic(in float x, in float B, in float C)
{
    // Rescale from [-2, 2] range to [-SampleRadius, SampleRadius]
    x *= 2.0f;

    float y = 0.0f;
    float x2 = x * x;
    float x3 = x * x * x;
    if(x < 1)
        y = (12 - 9 * B - 6 * C) * x3 + (-18 + 12 * B + 6 * C) * x2 + (6 - 2 * B);
    else if (x <= 2)
        y = (-B - 6 * C) * x3 + (6 * B + 30 * C) * x2 + (-12 * B - 48 * C) * x + (8 * B + 24 * C);

    return y / 6.0f;
}

//=================================================================================================
// Pixel Shader
//=================================================================================================
float4 PS(in PSInput input) : SV_Target0
{
	float3 vtxNormal = normalize(input.NormalWS);

    #if ShaderSupersampling_
        float2 uv  = InterpolateUV(input, input.Barycentrics);
        float3 positionWS = InterpolatePosition(input, input.Barycentrics);
    #else
        float2 uv  = input.TexCoord;
        float3 positionWS = input.PositionWS;
    #endif

    // Normalize after interpolation
    float3 normalWS = vtxNormal;
	float3 normalTS = float3(0, 0, 1);

	float3 tangentWS = normalize(input.TangentWS);
	float3 binormalWS = normalize(input.BitangentWS);
	float3x3 tangentToWorld = float3x3(tangentWS, binormalWS, normalWS);
    float3x3 worldToTangent = transpose(tangentToWorld);

	float3 diffuseAlbedo = EnableDiffuse ? 0.5f : 0.0f;

	float SpecularAlbedo = EnableSpecular ? 0.05f : 0.0f;

    #if UseLEAN_
        // unpack B and M
        float2 leanB = LEANMap.Sample(AnisoSampler, float3(uv, 0.0f)).xy;
        float4 leanM = LEANMap.Sample(AnisoSampler, float3(uv, 1.0f)).xyzw;
    #endif

    #if UseVMF_
        VMF vmfs[NumVMFs];

        [unroll]
        for(uint v = 0; v < NumVMFs; ++v)
        {
            #if NumVMFs > 1
                float4 vmfSample = VMFMap.Sample(AnisoSampler, float3(uv, v));
            #else
                float4 vmfSample = VMFMap.Sample(AnisoSampler, uv);

                vmfSample.zw = VMFMap.Sample(LinearSampler, uv).zw;
            #endif

            vmfs[v].mu.xy = vmfSample.xy;
            vmfs[v].mu.z = sqrt(saturate(1.0f - (vmfSample.x * vmfSample.x + vmfSample.y * vmfSample.y)));
            vmfs[v].mu = normalize(vmfs[v].mu);
            vmfs[v].mu = normalize(mul(vmfs[v].mu, tangentToWorld));
            vmfs[v].alpha = vmfSample.z;
            vmfs[v].kappa = 1.0f / vmfSample.w;
        }
    #endif

    float3 lighting = 0.0f;
    float3 weightSum = 0.0f;

    #if ShaderSupersampling_
        const uint NumSamples = ShaderSSSamples * ShaderSSSamples;

        const float3 barycentricsDX = ddx_fine(input.Barycentrics);
        const float3 barycentricsDY = ddy_fine(input.Barycentrics);

        [loop]
    #else
        const uint NumSamples = 1;
        [unroll]
    #endif
    for(uint i = 0; i < NumSamples; ++i)
    {
        #if ShaderSupersampling_
            // Compute a randomized sample position based on the pixel center, and interpolate the UV's
            // and position to that point
            const uint idxOffset = uint(input.PositionSS.y) * RenderTargetSize.x + uint(input.PositionSS.x);
            const float2 sampleOffset = SampleOffsets[(i + idxOffset) % NumSampleOffsets] * SampleRadius;
            const float3 sampleBarycentrics = input.Barycentrics + sampleOffset.x * barycentricsDX
                                                                 + sampleOffset.y * barycentricsDY;
            uv = InterpolateUV(input, sampleBarycentrics);
            positionWS = InterpolatePosition(input, sampleBarycentrics);

            const float sampleDist = length(sampleOffset) / (SampleRadius / 2.0f);
            const float filterWeight = NumSamples > 1 ? FilterCubic(sampleDist, 1.0, 0.0f) : 1.0f;
        #else
            const float filterWeight = 1.0f;
        #endif

        // Sample the normal map, and convert the normal to world space
        #if ShaderSupersampling_
            normalTS.xyz = NormalMap.SampleLevel(AnisoSampler, uv, 0.0f).xyz * 2.0f - 1.0f;
        #else
            normalTS.xyz = NormalMap.Sample(AnisoSampler, uv).xyz * 2.0f - 1.0f;
        #endif

        float normalMapLen = length(normalTS);

        normalWS = normalize(mul(normalTS, tangentToWorld));

        float3 sampleLighting = 0.0f;

        // Add in the point Lights
        const uint NumPointLights = 2;
        const float3 PointLightColor = 5.0f;
        const float PointLightFalloff = 2.0f;
        const float PointLightIntensity = 15.0f;
        const float3 PointLightColors[NumPointLights] =
        {
            float3(0.5f, 0.5f, 0.5f) * PointLightIntensity,
            float3(0.5f, 0.5f, 0.5f) * PointLightIntensity,
        };
        const float3 PointLightPositions[NumPointLights] =
        {
            float3(0.0f, 2.5f, 0.0f),
            float3(0.0f, 0.5f, 2.5f),
        };

        #if UseVMF_
            [unroll]
            for(uint i = 0; i < NumPointLights; ++i)
                sampleLighting += CalcPointLightVMF(vmfs, PointLightColors[i], diffuseAlbedo, SpecularAlbedo,
                                                    Roughness, positionWS, PointLightPositions[i], PointLightFalloff);
        #elif UseLEAN_
            [unroll]
            for(uint i = 0; i < NumPointLights; ++i)
                sampleLighting += CalcPointLightLEAN(normalWS, PointLightColors[i], diffuseAlbedo, SpecularAlbedo,
                                                     Roughness, positionWS, PointLightPositions[i], PointLightFalloff,
                                                     leanB, leanM, worldToTangent);
        #else
            float roughness = Roughness;

            #if UseToksvig_
                float s = RoughnessToSpecPower(roughness);
                float ft = normalMapLen / lerp(s, 1.0f, normalMapLen);
                ft = max(ft, 0.01f);
                roughness = SpecPowerToRoughness(ft * s);
            #elif UsePrecomputedVMF_
                roughness = RoughnessMap.Sample(LinearSampler, uv).x;
                roughness *= roughness;
            #elif UsePrecomputedToksvig_
                roughness = RoughnessMap.Sample(LinearSampler, uv).y;
                roughness *= roughness;
            #endif

            [unroll]
            for(uint i = 0; i < NumPointLights; ++i)
                sampleLighting += CalcPointLight(normalWS, PointLightColors[i], diffuseAlbedo, SpecularAlbedo,
                                            roughness, positionWS, PointLightPositions[i], PointLightFalloff);
        #endif

        lighting += sampleLighting * filterWeight;
        weightSum += filterWeight;
    }

    lighting /= weightSum;

    return float4(lighting, 1.0f);
}

//=================================================================================================
// Simple vertex shader that's used when rendering with texture-space lighting
//=================================================================================================
void VSTextureLighting(in float3 Position : POSITION, in float2 TexCoord : TEXCOORD,
                       out float4 OutPosition : SV_Position, out float2 OutTexCoord : TEXCOORD)
{
    OutPosition = mul(float4(Position, 1.0f), WorldViewProjection);
    OutTexCoord = TexCoord;
}

//=================================================================================================
// Simple pixel shader that's used when rendering with texture-space lighting
//=================================================================================================
float4 PSTextureLighting(in float4 PositionSS : SV_Position, in float2 TexCoord : TEXCOORD) : SV_Target0
{
    return float4(LightingMap.Sample(AnisoSampler, TexCoord).xyz, 1.0f);
}