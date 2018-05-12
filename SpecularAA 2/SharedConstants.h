//=================================================================================================
//
//  Specular AA Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#if _WINDOWS

#pragma once

typedef Float2 float2;
typedef Float3 float3;
typedef Float4 float4;

typedef uint32 uint;
typedef Uint2 uint2;
typedef Uint3 uint3;
typedef Uint4 uint4;

#endif

// Constants
static const float Pi = 3.141592654f;
static const float Pi2 = 6.283185307f;
static const float Pi_2 = 1.570796327f;
static const float Pi_4 = 0.7853981635f;
static const float InvPi = 0.318309886f;
static const float InvPi2 = 0.159154943f;

static const uint NumSampleOffsets = 256 * 1024;

#define NumVMFs_ 1
static const uint NumVMFs = NumVMFs_;

struct VMF
{
    float3 mu;
    float kappa;
    float alpha;
};