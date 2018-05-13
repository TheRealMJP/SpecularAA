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

//=================================================================================================
// Constants
//=================================================================================================
cbuffer Constants : register(b0)
{
    float2 TextureSize;
    float2 OutputSize;
    uint MipLevel;
    float Roughness;
    float ScaleFactor;
};

//=================================================================================================
// Resources
//=================================================================================================

// Inputs
Texture2D<float4> NormalMap : register(t0);

// Outputs
RWTexture2DArray<snorm float4> OutputLEANMap : register(u0);
RWTexture2D<float4> OutputVMFMap : register(u0);
RWTexture2DArray<float4> OutputVMFArrayMap : register(u0);
RWTexture2D<unorm float2> OutputRoughnessMap : register(u1);

float FilterBox(in float x)
{
    return 1.0f;
}

float FilterSmoothstep(in float x)
{
    return 1.0f - smoothstep(0.0f, 1.0f, x);
}

float3 FetchNormal(uint2 samplePos)
{
    float3 normal = normalize(NormalMap[samplePos].xyz * 2.0f - 1.0f);

    return normal;
}

// ================================================================================================
// Converts a Beckmann roughness parameter to a Blinn-Phong specular power
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
// Computes an optimal set of vMF lobes that represent the NDF of a texel
// ================================================================================================
void SolveVMF(float2 centerPos, uint numSamples, out VMF vmfs[NumVMFs], out float vmfRoughness,
              out float toksvigRoughness)
{
    if(MipLevel == 0)
    {
        vmfs[0].mu = FetchNormal(uint2(centerPos)).xyz,
        vmfs[0].alpha = 1.0f;
        vmfs[0].kappa = 10000.0f;

        [unroll]
        for(uint i = 1; i < NumVMFs; ++i)
        {
            vmfs[i].mu = 0.0f;
            vmfs[i].alpha = 0.0f;
            vmfs[i].kappa = 10000.0f;
        }

        vmfRoughness = Roughness;
        toksvigRoughness = Roughness;
    }
    else
    {
        float3 avgNormal = 0.0f;

        float2 topLeft = (-float(numSamples) / 2.0f) + 0.5f;

        for(uint y = 0; y < numSamples; ++y)
        {
            for(uint x = 0; x < numSamples; ++x)
            {
                float2 offset = topLeft + float2(x, y);
                float2 samplePos = floor(centerPos + offset) + 0.5f;
                float3 sampleNormal = FetchNormal(samplePos);

                avgNormal += sampleNormal;
            }
        }

        avgNormal /= (numSamples * numSamples);

        float r = length(avgNormal);
        float kappa = 10000.0f;
        if(r < 1.0f)
            kappa = (3 * r - r * r * r) / (1 - r * r);

        float3 mu = normalize(avgNormal);

        vmfs[0].mu = mu;
        vmfs[0].alpha = 1.0f;
        vmfs[0].kappa = kappa;

        [unroll]
        for(uint i = 1; i < NumVMFs; ++i)
        {
            vmfs[i].mu = 0.0f;
            vmfs[i].alpha = 0.0f;
            vmfs[i].kappa = 10000.0f;
        }

        // Pre-compute roughness map values
        vmfRoughness = sqrt(Roughness * Roughness + (1.0f / (2.0f * kappa)));

        float s = RoughnessToSpecPower(Roughness);
        float ft = r / lerp(s, 1.0f, r);
        toksvigRoughness = SpecPowerToRoughness(ft * s);
    }
}

//=================================================================================================
// Generates a LEAN map
//=================================================================================================
[numthreads(TGSize_, TGSize_, 1)]
void GenerateLEANMap(uint3 GroupID : SV_GroupID, uint3 DispatchThreadID : SV_DispatchThreadID,
                      uint3 GroupThreadID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    uint2 samplePos = GroupID.xy * uint2(TGSize_, TGSize_) + GroupThreadID.xy;

    float3 N = FetchNormal(samplePos);

    float2 B = N.xy / (ScaleFactor * N.z);
    float3 M = float3(B.x * B.x, B.y * B.y , B.x * B.y);
    float cleanM = B.x * B.x + B.y * B.y;

    OutputLEANMap[uint3(samplePos, 0)] = float4(B, 0.0f, 1.0f);
    OutputLEANMap[uint3(samplePos, 1)] = float4(M, cleanM);
}

//=================================================================================================
// vMF solver for normal map NDF
//=================================================================================================
[numthreads(TGSize_, TGSize_, 1)]
void SolveVMF(uint3 GroupID : SV_GroupID, uint3 DispatchThreadID : SV_DispatchThreadID,
              uint3 GroupThreadID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    uint2 outputPos = GroupID.xy * uint2(TGSize_, TGSize_) + GroupThreadID.xy;

    [branch]
    if(outputPos.x < uint(OutputSize.x) && outputPos.y < uint(OutputSize.y))
    {
        float2 uv = (outputPos + 0.5f) / OutputSize;
        uint sampleRadius = (1 << MipLevel);
        float2 samplePos = uv * TextureSize;

        VMF vmfs[NumVMFs];
        float vmfRoughness, toksvigRoughness;
        SolveVMF(samplePos, sampleRadius, vmfs, vmfRoughness, toksvigRoughness);

        #if NumVMFs_ > 1
            [unroll]
            for(uint i = 0; i < NumVMFs; ++i)
                OutputVMFArrayMap[uint3(outputPos, i)] = float4(vmfs[i].mu.xy, vmfs[i].alpha,
                                                                1.0f / vmfs[i].kappa);
        #else
            OutputVMFMap[outputPos] = float4(vmfs[0].mu.xy, 1.0f, 1.0f / vmfs[0].kappa);
        #endif

        OutputRoughnessMap[outputPos] = sqrt(float2(vmfRoughness, toksvigRoughness));
    }
}