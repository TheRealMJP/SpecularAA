//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "PCH.h"

#include "DeviceStates.h"
#include "Utility.h"

namespace SampleFramework11
{

void BlendStates::Initialize(ID3D11Device* device)
{
    D3D11_BLEND_DESC blendDisabledDesc = BlendDisabledDesc();
    DXCall(device->CreateBlendState(&blendDisabledDesc, &blendDisabled));
    D3D11_BLEND_DESC additiveBlendDesc = AdditiveBlendDesc();
    DXCall(device->CreateBlendState(&additiveBlendDesc, &additiveBlend));
    D3D11_BLEND_DESC alphaBlendDesc = AlphaBlendDesc();
    DXCall(device->CreateBlendState(&alphaBlendDesc, &alphaBlend));
    D3D11_BLEND_DESC preMultipliedAlphaBlendDesc = PreMultipliedAlphaBlendDesc();
    DXCall(device->CreateBlendState(&preMultipliedAlphaBlendDesc, &pmAlphaBlend));
    D3D11_BLEND_DESC colorWriteDisabledDesc = ColorWriteDisabledDesc();
    DXCall(device->CreateBlendState(&colorWriteDisabledDesc, &noColor));
    D3D11_BLEND_DESC alphaToCoverageDesc = AlphaToCoverageDesc();
    DXCall(device->CreateBlendState(&alphaToCoverageDesc, &alphaToCoverage));
    D3D11_BLEND_DESC opacityBlendDesc = OpacityBlendDesc();
    DXCall(device->CreateBlendState(&opacityBlendDesc, &opacityBlend));
}

D3D11_BLEND_DESC BlendStates::BlendDisabledDesc()
{
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    for(UINT i = 0; i < 8; ++i)
    {
        blendDesc.RenderTarget[i].BlendEnable = false;
        blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
    }

    return blendDesc;
}

D3D11_BLEND_DESC BlendStates::AdditiveBlendDesc()
{
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    for (uint32 i = 0; i < 8; ++i)
    {
        blendDesc.RenderTarget[i].BlendEnable = true;
        blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
    }

    return blendDesc;
}

D3D11_BLEND_DESC BlendStates::AlphaBlendDesc()
{
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = true;
    for (uint32 i = 0; i < 8; ++i)
    {
        blendDesc.RenderTarget[i].BlendEnable = true;
        blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
    }

    blendDesc.RenderTarget[0].BlendEnable = true;

    return blendDesc;
}

D3D11_BLEND_DESC BlendStates::PreMultipliedAlphaBlendDesc()
{
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    for (uint32 i = 0; i < 8; ++i)
    {
        blendDesc.RenderTarget[i].BlendEnable = false;
        blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
    }

    return blendDesc;
}

D3D11_BLEND_DESC BlendStates::ColorWriteDisabledDesc()
{
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    for (uint32 i = 0; i < 8; ++i)
    {
        blendDesc.RenderTarget[i].BlendEnable = false;
        blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].RenderTargetWriteMask = 0;
        blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
    }

    return blendDesc;
}

D3D11_BLEND_DESC BlendStates::AlphaToCoverageDesc()
{
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = true;
    blendDesc.IndependentBlendEnable = false;
    for (uint32 i = 0; i < 8; ++i)
    {
        blendDesc.RenderTarget[i].BlendEnable = false;
        blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
    }

    return blendDesc;
}

D3D11_BLEND_DESC BlendStates::OpacityBlendDesc()
{
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    for (uint32 i = 0; i < 8; ++i)
    {
        blendDesc.RenderTarget[i].BlendEnable = true;
        blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC1_COLOR;
        blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
    }

    return blendDesc;
}

void RasterizerStates::Initialize(ID3D11Device* device)
{
    D3D11_RASTERIZER_DESC noCullDesc = NoCullDesc();
    DXCall(device->CreateRasterizerState(&noCullDesc, &noCull));
    D3D11_RASTERIZER_DESC frontFaceCullDesc = FrontFaceCullDesc();
    DXCall(device->CreateRasterizerState(&frontFaceCullDesc, &cullFrontFaces));
    D3D11_RASTERIZER_DESC frontFaceCullScissorDesc = FrontFaceCullScissorDesc();
    DXCall(device->CreateRasterizerState(&frontFaceCullScissorDesc, &cullFrontFacesScissor));
    D3D11_RASTERIZER_DESC backFaceCullDesc = BackFaceCullDesc();
    DXCall(device->CreateRasterizerState(&backFaceCullDesc, &cullBackFaces));
    D3D11_RASTERIZER_DESC backFaceCullScissorDesc = BackFaceCullScissorDesc();
    DXCall(device->CreateRasterizerState(&backFaceCullScissorDesc, &cullBackFacesScissor));
    D3D11_RASTERIZER_DESC noCullNoMSDesc = NoCullNoMSDesc();
    DXCall(device->CreateRasterizerState(&noCullNoMSDesc, &noCullNoMS));
    D3D11_RASTERIZER_DESC noCullScissorDesc = NoCullScissorDesc();
    DXCall(device->CreateRasterizerState(&noCullScissorDesc, &noCullScissor));
    D3D11_RASTERIZER_DESC wireframeDesc = WireframeDesc();
    DXCall(device->CreateRasterizerState(&wireframeDesc, &wireframe));
}

D3D11_RASTERIZER_DESC RasterizerStates::NoCullDesc()
{
    D3D11_RASTERIZER_DESC rastDesc;

    rastDesc.AntialiasedLineEnable = false;
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = true;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.MultisampleEnable = true;
    rastDesc.ScissorEnable = false;
    rastDesc.SlopeScaledDepthBias = 0;

    return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStates::FrontFaceCullDesc()
{
    D3D11_RASTERIZER_DESC rastDesc;

    rastDesc.AntialiasedLineEnable = false;
    rastDesc.CullMode = D3D11_CULL_FRONT;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = true;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.MultisampleEnable = true;
    rastDesc.ScissorEnable = false;
    rastDesc.SlopeScaledDepthBias = 0;

    return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStates::FrontFaceCullScissorDesc()
{
    D3D11_RASTERIZER_DESC rastDesc;

    rastDesc.AntialiasedLineEnable = false;
    rastDesc.CullMode = D3D11_CULL_FRONT;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = true;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = true;
    rastDesc.MultisampleEnable = true;
    rastDesc.ScissorEnable = true;
    rastDesc.SlopeScaledDepthBias = 0;

    return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStates::BackFaceCullDesc()
{
    D3D11_RASTERIZER_DESC rastDesc;

    rastDesc.AntialiasedLineEnable = false;
    rastDesc.CullMode = D3D11_CULL_BACK;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = true;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.MultisampleEnable = true;
    rastDesc.ScissorEnable = false;
    rastDesc.SlopeScaledDepthBias = 0;

    return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStates::BackFaceCullScissorDesc()
{
    D3D11_RASTERIZER_DESC rastDesc;

    rastDesc.AntialiasedLineEnable = false;
    rastDesc.CullMode = D3D11_CULL_BACK;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = true;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.MultisampleEnable = true;
    rastDesc.ScissorEnable = true;
    rastDesc.SlopeScaledDepthBias = 0;

    return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStates::NoCullNoMSDesc()
{
    D3D11_RASTERIZER_DESC rastDesc;

    rastDesc.AntialiasedLineEnable = false;
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = true;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.MultisampleEnable = false;
    rastDesc.ScissorEnable = false;
    rastDesc.SlopeScaledDepthBias = 0;

    return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStates::NoCullScissorDesc()
{
    D3D11_RASTERIZER_DESC rastDesc;

    rastDesc.AntialiasedLineEnable = false;
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = true;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.MultisampleEnable = true;
    rastDesc.ScissorEnable = true;
    rastDesc.SlopeScaledDepthBias = 0;

    return rastDesc;
}

D3D11_RASTERIZER_DESC RasterizerStates::WireframeDesc()
{
    D3D11_RASTERIZER_DESC rastDesc;

    rastDesc.AntialiasedLineEnable = false;
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = true;
    rastDesc.FillMode = D3D11_FILL_WIREFRAME;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.MultisampleEnable = true;
    rastDesc.ScissorEnable = false;
    rastDesc.SlopeScaledDepthBias = 0;

    return rastDesc;
}

void DepthStencilStates::Initialize(ID3D11Device* device)
{
    D3D11_DEPTH_STENCIL_DESC depthDisabledDesc = DepthDisabledDesc();
    DXCall(device->CreateDepthStencilState(&depthDisabledDesc, &depthDisabled));
    D3D11_DEPTH_STENCIL_DESC depthEnabledDesc = DepthEnabledDesc();
    DXCall(device->CreateDepthStencilState(&depthEnabledDesc, &depthEnabled));
    D3D11_DEPTH_STENCIL_DESC reverseDepthEnabledDesc = ReverseDepthEnabledDesc();
    DXCall(device->CreateDepthStencilState(&reverseDepthEnabledDesc, &revDepthEnabled));
    D3D11_DEPTH_STENCIL_DESC depthWriteEnabledDesc = DepthWriteEnabledDesc();
    DXCall(device->CreateDepthStencilState(&depthWriteEnabledDesc, &depthWriteEnabled));
    D3D11_DEPTH_STENCIL_DESC reverseDepthWriteEnabledDesc = ReverseDepthWriteEnabledDesc();
    DXCall(device->CreateDepthStencilState(&reverseDepthWriteEnabledDesc, &revDepthWriteEnabled));
    D3D11_DEPTH_STENCIL_DESC depthStencilWriteEnabledDesc = DepthStencilWriteEnabledDesc();
    DXCall(device->CreateDepthStencilState(&depthStencilWriteEnabledDesc, &depthStencilWriteEnabled));
    D3D11_DEPTH_STENCIL_DESC stencilEnabledDesc = StencilEnabledDesc();
    DXCall(device->CreateDepthStencilState(&stencilEnabledDesc, &stencilEnabled));
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStates::DepthDisabledDesc()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = false;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = false;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.BackFace = dsDesc.FrontFace;

    return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStates::DepthEnabledDesc()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = false;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.BackFace = dsDesc.FrontFace;

    return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStates::ReverseDepthEnabledDesc()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    dsDesc.StencilEnable = false;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.BackFace = dsDesc.FrontFace;

    return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStates::DepthWriteEnabledDesc()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = false;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.BackFace = dsDesc.FrontFace;

    return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStates::ReverseDepthWriteEnabledDesc()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    dsDesc.StencilEnable = false;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.BackFace = dsDesc.FrontFace;

    return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStates::DepthStencilWriteEnabledDesc()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.BackFace = dsDesc.FrontFace;

    return dsDesc;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStates::StencilEnabledDesc()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = 0;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    dsDesc.BackFace = dsDesc.FrontFace;

    return dsDesc;
}

void SamplerStates::Initialize(ID3D11Device* device)
{
    D3D11_SAMPLER_DESC linearDesc = LinearDesc();
    DXCall(device->CreateSamplerState(&linearDesc, &linear));
    D3D11_SAMPLER_DESC linearClampDesc = LinearClampDesc();
    DXCall(device->CreateSamplerState(&linearClampDesc, &linearClamp));
    D3D11_SAMPLER_DESC linearBorderDesc = LinearBorderDesc();
    DXCall(device->CreateSamplerState(&linearBorderDesc, &linearBorder));
    D3D11_SAMPLER_DESC pointDesc = PointDesc();
    DXCall(device->CreateSamplerState(&pointDesc, &point));
    D3D11_SAMPLER_DESC anisotropicDesc = AnisotropicDesc();
    DXCall(device->CreateSamplerState(&anisotropicDesc, &anisotropic));
    D3D11_SAMPLER_DESC shadowMapDesc = ShadowMapDesc();
    DXCall(device->CreateSamplerState(&shadowMapDesc, &shadowMap));
    D3D11_SAMPLER_DESC shadowMapPCFDesc = ShadowMapPCFDesc();
    DXCall(device->CreateSamplerState(&shadowMapPCFDesc, &shadowMapPCF));
}

D3D11_SAMPLER_DESC SamplerStates::LinearDesc()
{
    D3D11_SAMPLER_DESC sampDesc;

    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStates::LinearClampDesc()
{
    D3D11_SAMPLER_DESC sampDesc;

    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStates::LinearBorderDesc()
{
    D3D11_SAMPLER_DESC sampDesc;

    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStates::PointDesc()
{
    D3D11_SAMPLER_DESC sampDesc;

    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStates::AnisotropicDesc()
{
    D3D11_SAMPLER_DESC sampDesc;

    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 16;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStates::ShadowMapDesc()
{
    D3D11_SAMPLER_DESC sampDesc;

    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    return sampDesc;
}

D3D11_SAMPLER_DESC SamplerStates::ShadowMapPCFDesc()
{
    D3D11_SAMPLER_DESC sampDesc;

    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    return sampDesc;
}

}