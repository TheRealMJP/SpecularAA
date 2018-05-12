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
#include "SampleFramework11/Input.h"
#include "SampleFramework11/Slider.h"
#include "SampleFramework11/TextGUI.h"

using namespace SampleFramework11;

// Forward declares
namespace SampleFramework11
{
    class SpriteRenderer;
    class SpriteFont;
}

class SpecularAAModeGUI : public TextGUI
{
public:

    enum Values
    {
        Disabled = 0,
        VMF = 1,
        LEAN = 2,
        CLEAN = 3,
        Toksvig = 4,
        PrecomputedVMF = 5,
        PrecomputedToksvig = 6,

        NumValues
    };

    SpecularAAModeGUI() : TextGUI(L"Specular AA Mode", Disabled, NumValues, KeyboardState::K)
    {
        static const WCHAR* Names[NumValues] =
        {
            L"Disabled",
            L"VMF",
            L"LEAN",
            L"CLEAN",
            L"Toksvig",
            L"Precomputed VMF",
            L"Precomputed Toksvig",
        };

        SetNames(Names);
    }
};

class NormalMapGUI : public TextGUI
{
public:

    enum Values
    {
        Hex = 0,
        Pattern = 1,
        Triangles = 2,
        Bricks = 3,
        Bumpy = 4,
        Blank = 5,


        NumValues
    };

    static const WCHAR* Names[NumValues];

    NormalMapGUI() : TextGUI(L"Normal Map", Hex, NumValues, KeyboardState::N)
    {
        SetNames(Names);
    }
};

class SuperSamplingModeGUI : public TextGUI
{
public:

    enum Values
    {
        Disabled = 0,
        ShaderSSAA,
        TextureSpaceLighting,

        NumValues
    };

    SuperSamplingModeGUI() : TextGUI(L"Super-Sampling Mode", Disabled, NumValues, KeyboardState::P)
    {
        static const WCHAR* Names[NumValues] =
        {
            L"Disabled",
            L"Shader SSAA",
            L"Texture-Space Lighting",
        };

        SetNames(Names);
    }
};

class SpecularBRDFGUI : public TextGUI
{
public:

    enum Values
    {
        Beckmann,
        GGX,

        NumValues
    };

    SpecularBRDFGUI() : TextGUI(L"Specular BRDF", GGX, NumValues, KeyboardState::L)
    {
        static const WCHAR* Names[NumValues] =
        {
            L"Beckmann",
            L"GGX",
        };

        SetNames(Names);
    }
};

class AppSettings
{
public:

    // Sliders for adjusting values
    static Slider BloomThreshold;
    static Slider BloomMagnitude;
    static Slider BloomBlurSigma;
    static Slider KeyValue;
    static Slider AdaptationRate;
    static Slider Roughness;
    static Slider ShaderSSSamples;
    static Slider SampleRadius;
    static Slider LEANScaleFactor;

    static BoolGUI MSAAMode;
    static BoolGUI EnableDiffuse;
    static BoolGUI EnableSpecular;
    static BoolGUI VMFDiffuseAA;

    static SpecularAAModeGUI SpecularAAMode;
    static NormalMapGUI NormalMap;
    static SuperSamplingModeGUI SuperSamplingMode;
    static SpecularBRDFGUI SpecularBRDF;

    // Collections of GUI objects
    static std::vector<GUIObject*> GUIObjects;
    static std::vector<Slider*> Sliders;
    static std::vector<TextGUI*> TextGUIs;

    // Initialization
    static void Initialize(ID3D11Device* device);

    // Resize adjustment
    static void AdjustGUI(uint32 displayWidth, uint32 displayHeight);

    // Frame update/render
    static void Update(const KeyboardState& kbState, const MouseState& mouseState);
    static void Render(SpriteRenderer& spriteRenderer, SpriteFont& font);
};