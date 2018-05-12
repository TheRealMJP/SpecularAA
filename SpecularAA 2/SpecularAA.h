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

#include "SampleFramework11/App.h"
#include "SampleFramework11/InterfacePointers.h"
#include "SampleFramework11/Camera.h"
#include "SampleFramework11/Model.h"
#include "SampleFramework11/SpriteFont.h"
#include "SampleFramework11/SpriteRenderer.h"
#include "SampleFramework11/Skybox.h"
#include "SampleFramework11/GraphicsTypes.h"
#include "SampleFramework11/Slider.h"

#include "PostProcessor.h"
#include "MeshRenderer.h"

using namespace SampleFramework11;

class SpecularAA : public App
{

protected:

    FirstPersonCamera camera;

    SpriteFont font;
    SampleFramework11::SpriteRenderer spriteRenderer;
    Skybox skybox;

    PostProcessor postProcessor;
    DepthStencilBuffer depthBuffer;
    DepthStencilBuffer depthBufferMSAA;
    RenderTarget2D colorTargetMSAA;
    RenderTarget2D colorTarget;

    // Model
    Model model;
    MeshRenderer meshRenderer;

    virtual void LoadContent();
    virtual void Render(const Timer& timer);
    virtual void Update(const Timer& timer);
    virtual void BeforeReset();
    virtual void AfterReset();

    void CreateModelInputLayouts(Model& model, std::vector<ID3D11InputLayoutPtr>& inputLayouts, ID3D10BlobPtr compiledVS);

    void CreateRenderTargets();

    void RenderModel();
    void RenderMainPass();
    void RenderHUD();

public:

    SpecularAA();
};
