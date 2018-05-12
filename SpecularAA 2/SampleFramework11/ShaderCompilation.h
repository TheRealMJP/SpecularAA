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

namespace SampleFramework11
{

// Compiles a shader from file and returns the compiled bytecode
ID3DBlob* CompileShader(LPCWSTR path,
                        LPCSTR functionName,
                        LPCSTR profile,
                        const D3D_SHADER_MACRO* defines = NULL,
                        bool forceOptimization = false);

// Compiles a shader from file and creates the appropriate shader instance
ID3D11VertexShader* CompileVSFromFile(ID3D11Device* device,
                                      LPCWSTR path,
                                      LPCSTR functionName = "VS",
                                      LPCSTR profile = "vs_5_0",
                                      const D3D_SHADER_MACRO* defines = NULL,
                                      ID3DBlob** byteCode = NULL,
                                      bool forceOptimization = false);

ID3D11PixelShader* CompilePSFromFile(ID3D11Device* device,
                                     LPCWSTR path,
                                     LPCSTR functionName = "PS",
                                     LPCSTR profile = "ps_5_0",
                                     const D3D_SHADER_MACRO* defines = NULL,
                                     bool forceOptimization = false);

ID3D11GeometryShader* CompileGSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName = "GS",
                                        LPCSTR profile = "gs_5_0",
                                        const D3D_SHADER_MACRO* defines = NULL,
                                        bool forceOptimization = false);

ID3D11HullShader* CompileHSFromFile(ID3D11Device* device,
                                    LPCWSTR path,
                                    LPCSTR functionName = "HS",
                                    LPCSTR profile = "hs_5_0",
                                    const D3D_SHADER_MACRO* defines = NULL,
                                    bool forceOptimization = false);

ID3D11DomainShader* CompileDSFromFile(ID3D11Device* device,
                                      LPCWSTR path,
                                      LPCSTR functionName = "DS",
                                      LPCSTR profile = "ds_5_0",
                                      const D3D_SHADER_MACRO* defines = NULL,
                                      bool forceOptimization = false);

ID3D11ComputeShader* CompileCSFromFile(ID3D11Device* device,
                                       LPCWSTR path,
                                       LPCSTR functionName = "CS",
                                       LPCSTR profile = "cs_5_0",
                                       const D3D_SHADER_MACRO* defines = NULL,
                                       bool forceOptimization = false);


class CompileOptions
{
public:

    // constants
    static const uint32 MaxDefines = 16;
    static const uint32 BufferSize = 1024;

    CompileOptions();

    void Add(const std::string& name, uint32 value);
    void Reset();

    const D3D10_SHADER_MACRO* Defines() const;

protected:

    D3D_SHADER_MACRO defines[MaxDefines + 1];
    CHAR buffer[BufferSize];
    uint32 numDefines;
    uint32 bufferIdx;
};

}

