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

#include "SpecularAA.h"
#include "SharedConstants.h"

#include "resource.h"
#include "SampleFramework11/InterfacePointers.h"
#include "SampleFramework11/Window.h"
#include "SampleFramework11/DeviceManager.h"
#include "SampleFramework11/Input.h"
#include "SampleFramework11/SpriteRenderer.h"
#include "SampleFramework11/Model.h"
#include "SampleFramework11/Utility.h"
#include "SampleFramework11/Camera.h"
#include "SampleFramework11/ShaderCompilation.h"
#include "SampleFramework11/Profiler.h"

using namespace SampleFramework11;
using std::wstring;

const uint32 WindowWidth = 1280;
const uint32 WindowHeight = 720;
const float WindowWidthF = static_cast<float>(WindowWidth);
const float WindowHeightF = static_cast<float>(WindowHeight);

const Float3 SunColor = Float3(10.0f, 8.0f, 5.0f) * 0;
const Float3 SunDirection = Float3::Normalize(Float3(0.2f, 0.977f, -0.4f));

static const float NearClip = 0.01f;
static const float FarClip = 100.0f;

static const float ModelScale = 1.0f;
static const Float4x4 ModelWorldMatrix = XMMatrixScaling(ModelScale, ModelScale, ModelScale) * XMMatrixRotationY(XM_PI);

SpecularAA::SpecularAA() :  App(L"Specular AA", MAKEINTRESOURCEW(IDI_DEFAULT)),
    camera(WindowWidthF / WindowHeightF, Pi_4 * 0.75f, NearClip, FarClip)
{
    deviceManager.SetBackBufferWidth(WindowWidth);
    deviceManager.SetBackBufferHeight(WindowHeight);
    deviceManager.SetMinFeatureLevel(D3D_FEATURE_LEVEL_11_0);

    window.SetClientArea(WindowWidth, WindowHeight);
}

void SpecularAA::BeforeReset()
{

}

void SpecularAA::AfterReset()
{
    float aspect = static_cast<float>(deviceManager.BackBufferWidth()) / deviceManager.BackBufferHeight();
    camera.SetAspectRatio(aspect);

    CreateRenderTargets();

    postProcessor.AfterReset(deviceManager.BackBufferWidth(), deviceManager.BackBufferHeight());

    AppSettings::AdjustGUI(deviceManager.BackBufferWidth(), deviceManager.BackBufferHeight());
}

void SpecularAA::CreateModelInputLayouts(Model& model, std::vector<ID3D11InputLayoutPtr>& inputLayouts, ID3D10BlobPtr compiledVS)
{
    ID3D11Device* device = deviceManager.Device();

    for(uint32 i = 0; i < model.Meshes().size(); ++i)
    {
        Mesh& mesh = model.Meshes()[i];
        ID3D11InputLayoutPtr inputLayout;
        DXCall(device->CreateInputLayout(mesh.InputElements(), mesh.NumInputElements(),
            compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), &inputLayout));
        inputLayouts.push_back(inputLayout);
    }
}

void SpecularAA::LoadContent()
{
    ID3D11DevicePtr device = deviceManager.Device();
    ID3D11DeviceContextPtr deviceContext = deviceManager.ImmediateContext();

    AppSettings::Initialize(device);
    AppSettings::AdjustGUI(deviceManager.BackBufferWidth(), deviceManager.BackBufferHeight());

    // Create a font + SpriteRenderer
    font.Initialize(L"Arial", 18, SpriteFont::Regular, true, device);
    spriteRenderer.Initialize(device);

    // Camera setup
    camera.SetPosition(Float3(0, 2.5f, -10.0f));

    // Load the tank scene
    model.GeneratePlaneScene(device, Float2(5.0f, 5.0f), Float3(), Quaternion());
    meshRenderer.Initialize(device, deviceManager.ImmediateContext(), &model, SunDirection, SunColor, ModelWorldMatrix);
    skybox.Initialize(device);

    // Init the post processor
    postProcessor.Initialize(device);
}

// Creates all required render targets
void SpecularAA::CreateRenderTargets()
{
    ID3D11Device* device = deviceManager.Device();
    uint32 width = deviceManager.BackBufferWidth();
    uint32 height = deviceManager.BackBufferHeight();

    colorTargetMSAA.Initialize(device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, 4, 0);
    colorTarget.Initialize(device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, 1, 0);
    depthBufferMSAA.Initialize(device, width, height, DXGI_FORMAT_D24_UNORM_S8_UINT, true, 4, 0);
    depthBuffer.Initialize(device, width, height, DXGI_FORMAT_D24_UNORM_S8_UINT, true, 1, 0);
}

void SpecularAA::Update(const Timer& timer)
{
    MouseState mouseState = MouseState::GetMouseState(window);
    KeyboardState kbState = KeyboardState::GetKeyboardState(window);

    if (kbState.IsKeyDown(KeyboardState::Escape))
        window.Destroy();

    float CamMoveSpeed = 5.0f * timer.DeltaSecondsF();
    const float CamRotSpeed = 0.180f * timer.DeltaSecondsF();
    const float MeshRotSpeed = 0.180f * timer.DeltaSecondsF();

    // Move the camera with keyboard input
    if(kbState.IsKeyDown(KeyboardState::LeftShift))
        CamMoveSpeed *= 0.25f;

    Float3 camPos = camera.Position();
    if(kbState.IsKeyDown(KeyboardState::W))
        camPos += camera.Forward() * CamMoveSpeed;
    else if (kbState.IsKeyDown(KeyboardState::S))
        camPos += camera.Back() * CamMoveSpeed;
    if(kbState.IsKeyDown(KeyboardState::A))
        camPos += camera.Left() * CamMoveSpeed;
    else if (kbState.IsKeyDown(KeyboardState::D))
        camPos += camera.Right() * CamMoveSpeed;
    if(kbState.IsKeyDown(KeyboardState::Q))
        camPos += camera.Up() * CamMoveSpeed;
    else if (kbState.IsKeyDown(KeyboardState::E))
        camPos += camera.Down() * CamMoveSpeed;
    camera.SetPosition(camPos);

    // Rotate the camera with the mouse
    if(mouseState.RButton.Pressed && mouseState.IsOverWindow)
    {
        float xRot = camera.XRotation();
        float yRot = camera.YRotation();
        xRot += mouseState.DY * CamRotSpeed;
        yRot += mouseState.DX * CamRotSpeed;
        camera.SetXRotation(xRot);
        camera.SetYRotation(yRot);
    }

    // Toggle VSYNC
    if(kbState.RisingEdge(KeyboardState::V))
        deviceManager.SetVSYNCEnabled(!deviceManager.VSYNCEnabled());

    AppSettings::Update(kbState, mouseState);
}

void SpecularAA::Render(const Timer& timer)
{
    ID3D11DeviceContextPtr context = deviceManager.ImmediateContext();

    if(AppSettings::NormalMap.Changed())
    {
        meshRenderer.CreateMaps();
        meshRenderer.GenerateMaps(context);
        meshRenderer.GenerateLEANMap(context);
    }
    else if(AppSettings::LEANScaleFactor.Changed())
        meshRenderer.GenerateLEANMap(context);
    else if((AppSettings::SpecularAAMode == SpecularAAModeGUI::PrecomputedToksvig ||
        AppSettings::SpecularAAMode == SpecularAAModeGUI::PrecomputedVMF)
        && AppSettings::Roughness.Changed())
        meshRenderer.GenerateMaps(context);

    RenderMainPass();

    if(AppSettings::MSAAMode)
        context->ResolveSubresource(colorTarget.Texture, 0, colorTargetMSAA.Texture, 0, colorTargetMSAA.Format);

    // Kick off post-processing
    D3DPERF_BeginEvent(0xFFFFFFFF, L"Post Processing");
    PostProcessor::Constants constants;
    constants.BloomThreshold = AppSettings::BloomThreshold;
    constants.BloomMagnitude = AppSettings::BloomMagnitude;
    constants.BloomBlurSigma = AppSettings::BloomBlurSigma;
    constants.Tau = AppSettings::AdaptationRate;
    constants.KeyValue = AppSettings::KeyValue;
    constants.TimeDelta = timer.DeltaSecondsF();

    postProcessor.SetConstants(constants);
    postProcessor.Render(context, colorTarget.SRView, deviceManager.BackBuffer());
    D3DPERF_EndEvent();

    ID3D11RenderTargetView* renderTargets[1] = { deviceManager.BackBuffer() };
    context->OMSetRenderTargets(1, renderTargets, NULL);

    D3D11_VIEWPORT vp;
    vp.Width = static_cast<float>(deviceManager.BackBufferWidth());
    vp.Height = static_cast<float>(deviceManager.BackBufferHeight());
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    context->RSSetViewports(1, &vp);

    RenderHUD();
}

void SpecularAA::RenderMainPass()
{
    PIXEvent event(L"Main Pass");

    ID3D11DeviceContextPtr context = deviceManager.ImmediateContext();

    ID3D11RenderTargetView* renderTargets[1];
    ID3D11DepthStencilView* ds;

    if(AppSettings::MSAAMode)
    {
        renderTargets[0] = colorTargetMSAA.RTView;
        ds = depthBufferMSAA.DSView;
    }
    else
    {
        renderTargets[0] = colorTarget.RTView;
        ds = depthBuffer.DSView;
    }

    context->OMSetRenderTargets(1, renderTargets, ds);

    context->ClearRenderTargetView(renderTargets[0], reinterpret_cast<float*>(&XMFLOAT4(0, 0, 0, 0)));
    context->ClearDepthStencilView(ds, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

    RenderModel();
}

void SpecularAA::RenderModel()
{
    ID3D11DeviceContext* context = deviceManager.ImmediateContext();

    // meshRenderer.RenderDepth(context, camera, ModelWorldMatrix);
    meshRenderer.Render(context, camera, ModelWorldMatrix, Uint2(colorTarget.Width, colorTarget.Height));

    skybox.RenderSky(context, SunDirection, true, camera.ViewMatrix(), camera.ProjectionMatrix());
}

void SpecularAA::RenderHUD()
{
    PIXEvent event(L"HUD Pass");

    spriteRenderer.Begin(deviceManager.ImmediateContext(), SpriteRenderer::Point);

    AppSettings::Render(spriteRenderer, font);

    Float4x4 transform = Float4x4::TranslationMatrix(Float3(25.0f, 25.0f, 0.0f));
    wstring fpsText(L"FPS: ");
    fpsText += ToString(fps) + L" (" + ToString(1000.0f / fps) + L"ms)";
    spriteRenderer.RenderText(font, fpsText.c_str(), transform, XMFLOAT4(1, 1, 0, 1));

    transform._42 += 25.0f;
    wstring vsyncText(L"VSYNC (V): ");
    vsyncText += deviceManager.VSYNCEnabled() ? L"Enabled" : L"Disabled";
    spriteRenderer.RenderText(font, vsyncText.c_str(), transform, XMFLOAT4(1, 1, 0, 1));

    Profiler::GlobalProfiler.EndFrame(spriteRenderer, font);

    spriteRenderer.End();
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    SpecularAA app;
    app.Run();
}