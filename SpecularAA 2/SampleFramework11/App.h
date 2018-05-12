//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once

#include "PCH.h"

#include "Window.h"
#include "DeviceManager.h"
#include "DeviceStates.h"
#include "Timer.h"
#include "SpriteFont.h"
#include "SpriteRenderer.h"

namespace SampleFramework11
{

class App
{

public:

    static App* GlobalApp;

    App(LPCWSTR appName, LPCWSTR iconResource = NULL);
    virtual ~App();

    void Run();

    void RenderText(const std::wstring& text, Float2 pos);
    void RenderCenteredText(const std::wstring& text);

protected:

    virtual void Initialize();
    virtual void LoadContent();
    virtual void Update(const Timer& timer) = 0;
    virtual void Render(const Timer& timer) = 0;

    virtual void BeforeReset();
    LRESULT WindowResized(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual void AfterReset();

    void Exit();
    void ToggleFullScreen(bool fullScreen);
    void CalculateFPS();

    void SaveScreenshot(const wchar* filePath);

    Window window;
    DeviceManager deviceManager;
    Timer timer;

    BlendStates blendStates;
    RasterizerStates rasterizerStates;
    DepthStencilStates depthStencilStates;
    SamplerStates samplerStates;

    SpriteFont font;
    SpriteRenderer spriteRenderer;

    static const uint32 NumTimeDeltaSamples = 64;
    float timeDeltaBuffer[NumTimeDeltaSamples];
    uint32 currentTimeDeltaSample;
    uint32 fps;

    StagingTexture2D captureTexture;

public:

    // Accessors
    Window& Window() { return window; }
    DeviceManager& DeviceManager() { return deviceManager; }
    SpriteFont& Font() { return font; }
    SpriteRenderer& SpriteRenderer() { return spriteRenderer; }
};

}