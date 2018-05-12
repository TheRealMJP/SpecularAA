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
#include "AppSettings.h"

Slider AppSettings::BloomThreshold;
Slider AppSettings::BloomMagnitude;
Slider AppSettings::BloomBlurSigma;
Slider AppSettings::KeyValue;
Slider AppSettings::AdaptationRate;
Slider AppSettings::Roughness;
Slider AppSettings::ShaderSSSamples;
Slider AppSettings::SampleRadius;
Slider AppSettings::LEANScaleFactor;

BoolGUI AppSettings::MSAAMode(L"MSAA", true, KeyboardState::M);
BoolGUI AppSettings::EnableDiffuse(L"Enable Diffuse", true, KeyboardState::J);
BoolGUI AppSettings::EnableSpecular(L"Enable Specular", true, KeyboardState::I);
BoolGUI AppSettings::VMFDiffuseAA(L"Enable VMF Diffuse AA", false, KeyboardState::U);

SpecularAAModeGUI AppSettings::SpecularAAMode;
NormalMapGUI AppSettings::NormalMap;
SuperSamplingModeGUI AppSettings::SuperSamplingMode;
SpecularBRDFGUI AppSettings::SpecularBRDF;

std::vector<GUIObject*> AppSettings::GUIObjects;
std::vector<Slider*> AppSettings::Sliders;
std::vector<TextGUI*> AppSettings::TextGUIs;

const WCHAR* NormalMapGUI::Names[NormalMapGUI::NumValues] =
{
    L"Hex",
    L"Pattern",
    L"Triangles",
    L"Bricks",
    L"Bumpy",
    L"Blank",
};

void AppSettings::Initialize(ID3D11Device* device)
{
    // Sliders
    BloomThreshold.Initialize(device, 0.0f, 20.0f, 5.0f, L"Bloom Threshold");
    Sliders.push_back(&BloomThreshold);

    BloomMagnitude.Initialize(device, 0.0f, 2.0f, 1.0f, L"Bloom Magnitude");
    Sliders.push_back(&BloomMagnitude);

    BloomBlurSigma.Initialize(device, 0.5f, 1.5f, 0.8f, L"Bloom Blur Sigma");
    Sliders.push_back(&BloomBlurSigma);

    KeyValue.Initialize(device, 0.0f, 0.5f, 0.2f, L"Auto-Exposure Key Value");
    Sliders.push_back(&KeyValue);

    AdaptationRate.Initialize(device, 0.0f, 4.0f, 0.5f, L"Adaptation Rate");
    Sliders.push_back(&AdaptationRate);

    Roughness.Initialize(device, 0.0f, 0.2f, 0.05f, L"Material Roughness");
    Sliders.push_back(&Roughness);

    ShaderSSSamples.Initialize(device, 1.0f, 16.0f, 1.0f, L"Shader AA Samples");
    ShaderSSSamples.NumSteps() = 15;
    Sliders.push_back(&ShaderSSSamples);

    SampleRadius.Initialize(device, 1.0f, 5.0f, 2.5f, L"Sample Radius");
    Sliders.push_back(&SampleRadius);

    LEANScaleFactor.Initialize(device, 0.0f, 5.0f, 0.5f, L"LEAN Scale Factor");
    Sliders.push_back(&LEANScaleFactor);

    TextGUIs.push_back(&MSAAMode);
    TextGUIs.push_back(&SuperSamplingMode);
    TextGUIs.push_back(&EnableDiffuse);
    TextGUIs.push_back(&EnableSpecular);
    TextGUIs.push_back(&VMFDiffuseAA);

    TextGUIs.push_back(&SpecularAAMode);
    TextGUIs.push_back(&NormalMap);
    TextGUIs.push_back(&SpecularBRDF);

    for(uintptr i = 0; i < Sliders.size(); ++i)
        GUIObjects.push_back(Sliders[i]);

    for(uintptr i = 0; i < TextGUIs.size(); ++i)
        GUIObjects.push_back(TextGUIs[i]);
}

void AppSettings::AdjustGUI(uint32 displayWidth, uint32 displayHeight)
{
    float width = static_cast<float>(displayWidth);
    float x = width - 250;
    float y = 10.0f;
    const float Spacing = 40.0f;

    for(uintptr i = 0; i < Sliders.size(); ++i)
    {
        Sliders[i]->Position() = Float2(x, y);
        y += Spacing;
    }

    float textY = displayHeight - TextGUIs.size() * 25.0f - 25.0f;
    for(uintptr i = 0; i < TextGUIs.size(); ++i)
    {
        TextGUIs[i]->Position() = Float2(25.0f, textY);
        textY += 25.0f;
    }
}

void AppSettings::Update(const KeyboardState& kbState, const MouseState& mouseState)
{
    for(uintptr i = 0; i < GUIObjects.size(); ++i)
        GUIObjects[i]->Update(kbState, mouseState);

    ShaderSSSamples.Enabled() = SuperSamplingMode == SuperSamplingModeGUI::ShaderSSAA;
    SampleRadius.Enabled() = SuperSamplingMode == SuperSamplingModeGUI::ShaderSSAA;
    LEANScaleFactor.Enabled() = SpecularAAMode == SpecularAAModeGUI::LEAN || SpecularAAMode == SpecularAAModeGUI::CLEAN;
}

void AppSettings::Render(SpriteRenderer& spriteRenderer, SpriteFont& font)
{
    for(uintptr i = 0; i < GUIObjects.size(); ++i)
        GUIObjects[i]->Render(spriteRenderer, font);
}