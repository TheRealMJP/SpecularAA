//=================================================================================================
//
//  Specular AA Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "PCH.h"

#include "MeshRenderer.h"
#include "AppSettings.h"
#include "SharedConstants.h"

#include "SampleFramework11/Exceptions.h"
#include "SampleFramework11/Utility.h"
#include "SampleFramework11/ShaderCompilation.h"
#include "SampleFramework11/DDSTextureLoader.h"

// Constants
static const uint32 ShadowMapSize = 2048;
static const float ShadowDist = 1.0f;
static const float Backup = 20.0f;
static const float NearClip = 1.0f;
static const float CascadeSplits[4] = { 0.125f, 0.25f, 0.5f, 1.0f };
static const float Bias = 0.005f;

const uint32 TGSize = 16;

MeshRenderer::MeshRenderer()
{
}

// Loads resources
void MeshRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, Model* model,
                              const Float3& lightDir, const Float3& lightColor, const Float4x4& world)
{
    this->device = device;
    this->model = model;

    blendStates.Initialize(device);
    rasterizerStates.Initialize(device);
    depthStencilStates.Initialize(device);
    samplerStates.Initialize(device);

    meshVSConstants.Initialize(device);
    meshPSConstants.Initialize(device);

    meshPSConstants.Data.LightDirWS = lightDir;
    meshPSConstants.Data.LightColor = lightColor;
    this->lightDir = lightDir;

    // Load the mesh shaders
    meshDepthVS.Attach(CompileVSFromFile(device, L"DepthOnly.hlsl", "VS", "vs_5_0", NULL, &compiledMeshDepthVS));

    meshVS[0].Attach(CompileVSFromFile(device, L"Mesh.hlsl", "VS", "vs_5_0", NULL, &compiledMeshVS));

    CompileOptions opts;
    opts.Add("ShaderSupersampling_", 1);
    meshVS[1].Attach(CompileVSFromFile(device, L"Mesh.hlsl", "VS", "vs_5_0", opts.Defines()));
    meshGS[1].Attach(CompileGSFromFile(device, L"Mesh.hlsl", "GS", "gs_5_0", opts.Defines()));

    opts.Reset();
    opts.Add("TextureSpaceLighting_", 1);
    meshVS[2].Attach(CompileVSFromFile(device, L"Mesh.hlsl", "VS", "vs_5_0", opts.Defines()));

    for(uint32 shaderSS = 0; shaderSS < SuperSamplingModeGUI::NumValues; ++shaderSS)
    {
        for(uint32 shaderAA = 0; shaderAA < SpecularAAModeGUI::NumValues; ++shaderAA)
        {
            for(uint32 specularBRDF = 0; specularBRDF < SpecularBRDFGUI::NumValues; ++specularBRDF)
            {
                if(shaderAA > 0 && shaderSS > 0)
                    continue;

                opts.Reset();
                opts.Add("ShaderSS_", shaderSS);
                opts.Add("ShaderAAMode_", shaderAA);
                opts.Add("UseGGX_", specularBRDF == SpecularBRDFGUI::GGX);
                opts.Add("UseBeckmann_", specularBRDF == SpecularBRDFGUI::Beckmann);

                meshPS[shaderSS][shaderAA][specularBRDF].Attach(CompilePSFromFile(device, L"Mesh.hlsl", "PS", "ps_5_0", opts.Defines()));
            }
        }
    }

    meshTLVS.Attach(CompileVSFromFile(device, L"Mesh.hlsl", "VSTextureLighting", "vs_5_0", NULL, &compiledMeshTLVS));
    meshTLPS.Attach(CompilePSFromFile(device, L"Mesh.hlsl", "PSTextureLighting"));

    for(uint64 i = 0; i < model->Meshes().size(); ++i)
    {
        Mesh& mesh = model->Meshes()[i];
        ID3D11InputLayoutPtr inputLayout;
        DXCall(device->CreateInputLayout(mesh.InputElements(), mesh.NumInputElements(),
            compiledMeshVS->GetBufferPointer(), compiledMeshVS->GetBufferSize(), &inputLayout));
        meshInputLayouts.push_back(inputLayout);

        DXCall(device->CreateInputLayout(mesh.InputElements(), mesh.NumInputElements(),
            compiledMeshDepthVS->GetBufferPointer(), compiledMeshDepthVS->GetBufferSize(), &inputLayout));
        meshDepthInputLayouts.push_back(inputLayout);

        DXCall(device->CreateInputLayout(mesh.InputElements(), mesh.NumInputElements(),
            compiledMeshTLVS->GetBufferPointer(), compiledMeshTLVS->GetBufferSize(), &inputLayout));
        meshTLInputLayouts.push_back(inputLayout);
    }

    opts.Reset();
    opts.Add("TGSize_", 16);

    generateLEANMap.Attach(CompileCSFromFile(device, L"GenerateMaps.hlsl", "GenerateLEANMap", "cs_5_0", opts.Defines()));
    generateVMFMap.Attach(CompileCSFromFile(device, L"GenerateMaps.hlsl", "SolveVMF", "cs_5_0", opts.Defines()));

    for(uint64 i = 0; i < NormalMapGUI::NumValues; ++i)
    {
        std::wstring path = L"..\\Content\\Textures\\";
        path += NormalMapGUI::Names[i];
        path += L".png";
        normalMaps[i] = LoadTexture(device, path.c_str());
    }

    std::vector<Float2> sampleOffsets(NumSampleOffsets);

    srand(0);
    for(uint64 i = 0; i < NumSampleOffsets; ++i)
    {
        float theta = RandFloat() * Pi2;
        float r = RandFloat() * 0.5f;
        sampleOffsets[i].x = std::cos(theta) * r;
        sampleOffsets[i].y = std::sin(theta) * r;
    }

    sampleOffsetsBuffer.Initialize(device, 8, NumSampleOffsets, FALSE, FALSE, FALSE, sampleOffsets.data());

    csConstants.Initialize(device);

    CreateMaps();
    GenerateMaps(context);
    GenerateLEANMap(context);
}

void MeshRenderer::CreateMaps()
{
    ID3D11ShaderResourceView* normalMap = normalMaps[AppSettings::NormalMap];
    ID3D11Texture2DPtr normalMapTexture;
    normalMap->GetResource(reinterpret_cast<ID3D11Resource**>(&normalMapTexture));

    D3D11_TEXTURE2D_DESC texDesc;
    normalMapTexture->GetDesc(&texDesc);

    // Init the LEAN map
    leanMap.Initialize(device, texDesc.Width, texDesc.Height, DXGI_FORMAT_R16G16B16A16_SNORM, texDesc.MipLevels, 1, 0, true, true, 2);

    // Generate the vMF map
    vmfMap.Initialize(device, texDesc.Width, texDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT, texDesc.MipLevels, 1, 0, false, true, NumVMFs);

    uint32 lightTexW = std::max<uint32>(texDesc.Width * 2, 1024);
    uint32 lightTexH = std::max<uint32>(texDesc.Height * 2, 1024);
    lightingTexture.Initialize(device, lightTexW, lightTexH, DXGI_FORMAT_R16G16B16A16_FLOAT,
        0, 1, 0, true, false, 1, false);

    roughnessMap.Initialize(device, texDesc.Width, texDesc.Height, DXGI_FORMAT_R8G8_UNORM, texDesc.MipLevels, 1, 0, false, true);
}


void MeshRenderer::GenerateMaps(ID3D11DeviceContext* context)
{
    ID3D11ShaderResourceView* normalMap = normalMaps[AppSettings::NormalMap];
    ID3D11Texture2DPtr normalMapTexture;
    normalMap->GetResource(reinterpret_cast<ID3D11Resource**>(&normalMapTexture));

    D3D11_TEXTURE2D_DESC texDesc;
    normalMapTexture->GetDesc(&texDesc);

    csConstants.Data.TextureSizeX = static_cast<float>(texDesc.Width);
    csConstants.Data.TextureSizeY = static_cast<float>(texDesc.Height);
    csConstants.Data.OutputSizeX = static_cast<float>(texDesc.Width);
    csConstants.Data.OutputSizeY = static_cast<float>(texDesc.Height);
    csConstants.Data.MipLevel = 0;
    csConstants.Data.Roughness = AppSettings::Roughness;
    csConstants.Data.ScaleFactor = std::pow(10.0f, AppSettings::LEANScaleFactor);
    csConstants.ApplyChanges(context);
    csConstants.SetCS(context, 0);

    SetCSInputs(context, normalMap);

    uint32 width = texDesc.Width;
    uint32 height = texDesc.Height;
    for(uint32 mipLevel = 0; mipLevel < texDesc.MipLevels; ++mipLevel)
    {
        ID3D11UnorderedAccessViewPtr vmfUAV = vmfMap.UAView;
        ID3D11UnorderedAccessViewPtr roughnessUAV = roughnessMap.UAView;

        if(mipLevel > 0)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = mipLevel;
            uavDesc.Format = vmfMap.Format;
            DXCall(device->CreateUnorderedAccessView(vmfMap.Texture, &uavDesc, &vmfUAV));

            uavDesc.Format = roughnessMap.Format;
            DXCall(device->CreateUnorderedAccessView(roughnessMap.Texture, &uavDesc, &roughnessUAV));
        }

        csConstants.Data.MipLevel = mipLevel;
        csConstants.Data.OutputSizeX = static_cast<float>(width);
        csConstants.Data.OutputSizeY = static_cast<float>(height);
        csConstants.ApplyChanges(context);

        SetCSShader(context, generateVMFMap);
        SetCSOutputs(context, vmfUAV, roughnessUAV);
        context->Dispatch(DispatchSize(TGSize, width), DispatchSize(TGSize, height), 1);

        width = std::max<uint32>(width / 2, 1);
        height = std::max<uint32>(height / 2, 1);
    }

    ClearCSOutputs(context);
    ClearCSInputs(context);
}

void MeshRenderer::GenerateLEANMap(ID3D11DeviceContext* context)
{
    PIXEvent event(L"Generate LEAN Map");

    ID3D11ShaderResourceView* normalMap = normalMaps[AppSettings::NormalMap];
    ID3D11Texture2DPtr normalMapTexture;
    normalMap->GetResource(reinterpret_cast<ID3D11Resource**>(&normalMapTexture));

    D3D11_TEXTURE2D_DESC texDesc;
    normalMapTexture->GetDesc(&texDesc);

    csConstants.Data.TextureSizeX = static_cast<float>(texDesc.Width);
    csConstants.Data.TextureSizeY = static_cast<float>(texDesc.Height);
    csConstants.Data.OutputSizeX = static_cast<float>(texDesc.Width);
    csConstants.Data.OutputSizeY = static_cast<float>(texDesc.Height);
    csConstants.Data.MipLevel = 0;
    csConstants.Data.Roughness = AppSettings::Roughness;
    csConstants.Data.ScaleFactor = std::pow(10.0f, AppSettings::LEANScaleFactor);
    csConstants.ApplyChanges(context);
    csConstants.SetCS(context, 0);

    SetCSInputs(context, normalMap);

    SetCSShader(context, generateLEANMap);
    SetCSOutputs(context, leanMap.UAView);
    context->Dispatch(DispatchSize(TGSize, texDesc.Width), DispatchSize(TGSize, texDesc.Height), 1);

    ClearCSOutputs(context);
    ClearCSInputs(context);

    context->GenerateMips(leanMap.SRView);
}

// Renders all meshes in the model, with shadows
void MeshRenderer::Render(ID3D11DeviceContext* context, const Camera& camera, const Float4x4& world,
                          const Uint2& renderTargetSize)
{
    ID3D11RenderTargetView* prevRTV[1] = { NULL };
    ID3D11DepthStencilView* prevDSV = NULL;
    context->OMGetRenderTargets(1, prevRTV, &prevDSV);

    uint32 numViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    D3D11_VIEWPORT prevViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    context->RSGetViewports(&numViewports, prevViewports);

    // Set states
    float blendFactor[4] = {1, 1, 1, 1};
    context->OMSetBlendState(blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);
    context->OMSetDepthStencilState(depthStencilStates.DepthWriteEnabled(), 0);
    context->RSSetState(rasterizerStates.BackFaceCull());

    if(AppSettings::SuperSamplingMode == SuperSamplingModeGUI::TextureSpaceLighting) {
        ID3D11RenderTargetView* rtv[1] = { lightingTexture.RTView };
        context->OMSetRenderTargets(1, rtv, NULL);

        context->OMSetDepthStencilState(depthStencilStates.DepthDisabled(), 0);
        context->RSSetState(rasterizerStates.NoCull());

        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = float(lightingTexture.Width);
        viewport.Height = float(lightingTexture.Height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        context->RSSetViewports(1, &viewport);
    }

    PIXEvent event(L"Mesh Rendering");

    ID3D11SamplerState* sampStates[3] = {
        samplerStates.Anisotropic(),
        samplerStates.ShadowMap(),
        samplerStates.Linear()
    };
    context->PSSetSamplers(0, 3, sampStates);

    // Set constant buffers
    meshVSConstants.Data.World = Float4x4::Transpose(world);
    meshVSConstants.Data.View = Float4x4::Transpose(camera.ViewMatrix());
    meshVSConstants.Data.WorldViewProjection = Float4x4::Transpose(world * camera.ViewProjectionMatrix());
    meshVSConstants.ApplyChanges(context);
    meshVSConstants.SetVS(context, 0);

    meshPSConstants.Data.CameraPosWS = camera.Position();
    meshPSConstants.Data.Roughness = AppSettings::Roughness;
    meshPSConstants.Data.ShaderSSSamples = static_cast<uint32>(AppSettings::ShaderSSSamples.Value());
    meshPSConstants.Data.RenderTargetSize = renderTargetSize;
    meshPSConstants.Data.SampleRadius = AppSettings::SampleRadius;
    meshPSConstants.Data.ScaleFactor = std::pow(10.0f, AppSettings::LEANScaleFactor);
    meshPSConstants.Data.EnableDiffuse = AppSettings::EnableDiffuse;
    meshPSConstants.Data.EnableSpecular = AppSettings::EnableSpecular;
    meshPSConstants.Data.VMFDiffuseAA = AppSettings::VMFDiffuseAA;
    meshPSConstants.ApplyChanges(context);
    meshPSConstants.SetPS(context, 0);

    uint32 specAAMode = AppSettings::SpecularAAMode;
    if(AppSettings::SuperSamplingMode > 0)
        specAAMode = 0;

    uint32 shaderSSAA = AppSettings::SuperSamplingMode == SuperSamplingModeGUI::ShaderSSAA;

    // Set shaders
    context->DSSetShader(NULL, NULL, 0);
    context->HSSetShader(NULL, NULL, 0);
    context->GSSetShader(meshGS[shaderSSAA], NULL, 0);
    context->VSSetShader(meshVS[AppSettings::SuperSamplingMode], NULL, 0);
    context->PSSetShader(meshPS[AppSettings::SuperSamplingMode][specAAMode][AppSettings::SpecularBRDF], NULL, 0);

    // Draw all meshes
    for(uint32 meshIdx = 0; meshIdx < model->Meshes().size(); ++meshIdx)
    {
        Mesh& mesh = model->Meshes()[meshIdx];

        // Set the vertices and indices
        ID3D11Buffer* vertexBuffers[1] = { mesh.VertexBuffer() };
        uint32 vertexStrides[1] = { mesh.VertexStride() };
        uint32 offsets[1] = { 0 };
        context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, offsets);
        context->IASetIndexBuffer(mesh.IndexBuffer(), mesh.IndexBufferFormat(), 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set the input layout
        context->IASetInputLayout(meshInputLayouts[meshIdx]);

        // Draw all parts
        for(uintptr partIdx = 0; partIdx < mesh.MeshParts().size(); ++partIdx)
        {
            const MeshPart& part = mesh.MeshParts()[partIdx];
            const MeshMaterial& material = model->Materials()[part.MaterialIdx];

            // Set the textures
            ID3D11ShaderResourceView* psTextures[5] =
            {                
                normalMaps[AppSettings::NormalMap],
                leanMap.SRView,
                vmfMap.SRView,
                roughnessMap.SRView,
                sampleOffsetsBuffer.SRView,
            };
            context->PSSetShaderResources(0, 5, psTextures);

            context->DrawIndexed(part.IndexCount, part.IndexStart, 0);
        }
    }

    ID3D11ShaderResourceView* nullSRVs[5] = { NULL };
    context->PSSetShaderResources(0, 5, nullSRVs);

    if(AppSettings::SuperSamplingMode == SuperSamplingModeGUI::TextureSpaceLighting)
    {
        context->GenerateMips(lightingTexture.SRView);
        context->OMSetRenderTargets(1, prevRTV, prevDSV);

        context->OMSetDepthStencilState(depthStencilStates.DepthWriteEnabled(), 0);
        context->RSSetState(rasterizerStates.BackFaceCull());
        context->RSSetViewports(1, prevViewports);

        context->VSSetShader(meshTLVS, NULL, 0);
        context->GSSetShader(NULL, NULL, 0);
        context->PSSetShader(meshTLPS, NULL, 0);

        ID3D11ShaderResourceView* srvs[1] = { lightingTexture.SRView };
        context->PSSetShaderResources(0, 1, srvs);

        // Draw all meshes
        for(uint32 meshIdx = 0; meshIdx < model->Meshes().size(); ++meshIdx)
        {
            Mesh& mesh = model->Meshes()[meshIdx];

            // Set the vertices and indices
            ID3D11Buffer* vertexBuffers[1] = { mesh.VertexBuffer() };
            uint32 vertexStrides[1] = { mesh.VertexStride() };
            uint32 offsets[1] = { 0 };
            context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, offsets);
            context->IASetIndexBuffer(mesh.IndexBuffer(), mesh.IndexBufferFormat(), 0);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Set the input layout
            context->IASetInputLayout(meshTLInputLayouts[meshIdx]);

            // Draw all parts
            for(uintptr partIdx = 0; partIdx < mesh.MeshParts().size(); ++partIdx)
            {
                const MeshPart& part = mesh.MeshParts()[partIdx];
                context->DrawIndexed(part.IndexCount, part.IndexStart, 0);
            }
        }

        context->PSSetShaderResources(0, 1, nullSRVs);
    }

    prevRTV[0]->Release();
    prevDSV->Release();
}

// Renders all meshes using depth-only rendering
void MeshRenderer::RenderDepth(ID3D11DeviceContext* context, const Camera& camera, const Float4x4& world)
{
    PIXEvent event(L"Mesh Depth Rendering");

    // Set states
    float blendFactor[4] = {1, 1, 1, 1};
    context->OMSetBlendState(blendStates.ColorWriteDisabled(), blendFactor, 0xFFFFFFFF);
    context->OMSetDepthStencilState(depthStencilStates.DepthWriteEnabled(), 0);
    context->RSSetState(rasterizerStates.BackFaceCull());

    // Set constant buffers
    meshVSConstants.Data.World = Float4x4::Transpose(world);
    meshVSConstants.Data.View = Float4x4::Transpose(camera.ViewMatrix());
    meshVSConstants.Data.WorldViewProjection = Float4x4::Transpose(world * camera.ViewProjectionMatrix());
    meshVSConstants.ApplyChanges(context);
    meshVSConstants.SetVS(context, 0);

    // Set shaders
    context->VSSetShader(meshDepthVS , NULL, 0);
    context->PSSetShader(NULL, NULL, 0);
    context->GSSetShader(NULL, NULL, 0);
    context->DSSetShader(NULL, NULL, 0);
    context->HSSetShader(NULL, NULL, 0);

    uint32 partCount = 0;
    for (uintptr meshIdx = 0; meshIdx < model->Meshes().size(); ++meshIdx)
    {
        Mesh& mesh = model->Meshes()[meshIdx];

        // Set the vertices and indices
        ID3D11Buffer* vertexBuffers[1] = { mesh.VertexBuffer() };
        uint32 vertexStrides[1] = { mesh.VertexStride() };
        uint32 offsets[1] = { 0 };
        context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, offsets);
        context->IASetIndexBuffer(mesh.IndexBuffer(), mesh.IndexBufferFormat(), 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Set the input layout
        context->IASetInputLayout(meshDepthInputLayouts[meshIdx]);

        // Draw all parts
        for (uintptr partIdx = 0; partIdx < mesh.MeshParts().size(); ++partIdx)
        {
            const MeshPart& part = mesh.MeshParts()[partIdx];
            context->DrawIndexed(part.IndexCount, part.IndexStart, 0);
        }
    }
}