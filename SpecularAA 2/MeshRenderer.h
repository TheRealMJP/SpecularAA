//=================================================================================================
//
//  Specular AA Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once

#include "SampleFramework11/PCH.h"

#include "SampleFramework11/Model.h"
#include "SampleFramework11/GraphicsTypes.h"
#include "SampleFramework11/DeviceStates.h"
#include "SampleFramework11/Camera.h"
#include "SampleFramework11/Math.h"

#include "AppSettings.h"

using namespace SampleFramework11;

// Represents a bouncing sphere for a MeshPart
struct Sphere
{
    Float3 Center;
    float Radius;
};

// Represents the 6 planes of a frustum
Float4Align struct Frustum
{
    XMVECTOR Planes[6];
};

class MeshRenderer
{

public:

    MeshRenderer();

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, Model* model, const Float3& lightDir,
        const Float3& lightColor, const Float4x4& world);
    void RenderDepth(ID3D11DeviceContext* context, const Camera& camera, const Float4x4& world);
    void Render(ID3D11DeviceContext* context, const Camera& camera, const Float4x4& world,
        const Uint2& renderTargetSize);
    void CreateMaps();
    void GenerateMaps(ID3D11DeviceContext* context);
    void GenerateLEANMap(ID3D11DeviceContext* context);

protected:

    static const UINT NumCascades = 4;

    ID3D11DevicePtr device;

    BlendStates blendStates;
    RasterizerStates rasterizerStates;
    DepthStencilStates depthStencilStates;
    SamplerStates samplerStates;

    Model* model;
    Float3 lightDir;

    std::vector<ID3D11InputLayoutPtr> meshInputLayouts;
    ID3D10BlobPtr compiledMeshVS;
    ID3D11VertexShaderPtr meshVS[SuperSamplingModeGUI::NumValues];
    ID3D11GeometryShaderPtr meshGS[2];
    ID3D11PixelShaderPtr meshPS[SuperSamplingModeGUI::NumValues][SpecularAAModeGUI::NumValues][SpecularBRDFGUI::NumValues];

    std::vector<ID3D11InputLayoutPtr> meshDepthInputLayouts;
    ID3D11VertexShaderPtr meshDepthVS;
    ID3D10BlobPtr compiledMeshDepthVS;

    std::vector<ID3D11InputLayoutPtr> meshTLInputLayouts;
    ID3D11VertexShaderPtr meshTLVS;
    ID3D11PixelShaderPtr meshTLPS;
    ID3D10BlobPtr compiledMeshTLVS;

    ID3D11ComputeShaderPtr generateLEANMap;
    ID3D11ComputeShaderPtr generateVMFMap;

    ID3D11ShaderResourceViewPtr normalMaps[NormalMapGUI::NumValues];
    RenderTarget2D leanMap;
    RenderTarget2D vmfMap;
    RenderTarget2D roughnessMap;

    StructuredBuffer sampleOffsetsBuffer;

    RenderTarget2D lightingTexture;

    // Constant buffers
    struct MeshVSConstants
    {
        Float4Align Float4x4 World;
        Float4Align Float4x4 View;
        Float4Align Float4x4 WorldViewProjection;
    };

    struct MeshPSConstants
    {
        Float4Align Float3 LightDirWS;
        Float4Align Float3 LightColor;
        Float4Align Float3 CameraPosWS;
        float Roughness;
        uint32 ShaderSSSamples;
        Uint2 RenderTargetSize;
        float SampleRadius;
        float ScaleFactor;
        bool32 EnableDiffuse;
        bool32 EnableSpecular;
        bool32 VMFDiffuseAA;
    };

    struct CSConstants
    {
        float TextureSizeX;
        float TextureSizeY;
        float OutputSizeX;
        float OutputSizeY;
        uint32 MipLevel;
        float Roughness;
        float ScaleFactor;
    };

    ConstantBuffer<MeshVSConstants> meshVSConstants;
    ConstantBuffer<MeshPSConstants> meshPSConstants;
    ConstantBuffer<CSConstants> csConstants;
};