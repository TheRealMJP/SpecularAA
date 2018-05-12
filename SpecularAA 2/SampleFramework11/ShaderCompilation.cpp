//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "PCH.h"

#include "ShaderCompilation.h"

#include "Utility.h"
#include "Exceptions.h"
#include "InterfacePointers.h"
#include "FileIO.h"
#include "MurmurHash.h"

#define EnableShaderCaching_ 1

namespace SampleFramework11
{

static std::string GetExpandedShaderCode(LPCWSTR path)
{
    std::string fileContents = ReadFileAsString(path);
    std::wstring fileDirectory = GetDirectoryFromFilePath(path);

    // Look for includes
    size_t lineStart = 0;
    size_t lineEnd = std::string::npos;
    while(true)
    {
        size_t lineEnd = fileContents.find('\n', lineStart);
        size_t lineLength = 0;
        if(lineEnd == std::string::npos)
            lineLength = std::string::npos;
        else
            lineLength = lineEnd - lineStart;

        std::string line = fileContents.substr(lineStart, lineLength);
        if(line.find("#include") == 0)
        {
            size_t startQuote = line.find('\"');
            size_t endQuote = line.find('\"', startQuote + 1);
            std::string includePath = line.substr(startQuote + 1, endQuote - startQuote - 1);
            std::wstring fullIncludePath = fileDirectory + AnsiToWString(includePath.c_str());
            if(FileExists(fullIncludePath.c_str()) == false)
                throw Exception(L"Couldn't find #included file \"" + fullIncludePath + L"\"");

            std::string includeCode = GetExpandedShaderCode(fullIncludePath.c_str());
            fileContents.insert(lineEnd + 1, includeCode);
            lineEnd += includeCode.length();
        }

        if(lineEnd == std::string::npos)
            break;

        lineStart = lineEnd + 1;
    }

    return fileContents;
}

static const std::wstring baseCacheDir = L"ShaderCache\\";

#if _DEBUG
    static const std::wstring cacheSubDir = L"Debug\\";
#else
    static const std::wstring cacheSubDir = L"Release\\";
#endif

static const std::wstring cacheDir = baseCacheDir + cacheSubDir;

static std::wstring MakeShaderCacheName(LPCWSTR path, LPCSTR functionName, const D3D_SHADER_MACRO* defines)
{
    std::wstring cacheName = cacheDir;
    cacheName += GetFileNameWithoutExtension(path) + L"_" + AnsiToWString(functionName);
    while(defines && defines->Name != NULL && defines != NULL)
    {
        cacheName += L"_";
        cacheName += AnsiToWString(defines->Name);
        cacheName += L"=";
        cacheName += AnsiToWString(defines->Definition);
        ++defines;
    }

    cacheName += L".cache";

    return cacheName;
}

ID3DBlob* CompileShader(LPCWSTR path, LPCSTR functionName, LPCSTR profile,
                          const D3D_SHADER_MACRO* defines, bool forceOptimization)
{
    #if EnableShaderCaching_
        // Come up with the unique name of the cached data
        std::wstring cacheName = MakeShaderCacheName(path, functionName, defines);

        // Make a hash off the expanded shader codes
        std::string shaderCode = GetExpandedShaderCode(path);
        uint64 shaderHash = MurmurHash64(shaderCode.c_str(), int(shaderCode.length()));

        std::wstring hashFileName = cacheName + L".hash";
        if(FileExists(hashFileName.c_str()) && FileExists(cacheName.c_str()))
        {
            // Check the hash stored in the cache
            uint64 storedHash = 0;
            ReadFromFile(hashFileName.c_str(), storedHash);

            if(storedHash == shaderHash)
            {
                File cacheFile(cacheName.c_str(), File::OpenRead);
                uint64 fileSize = cacheFile.Size();

                ID3DBlob* blob;
                DXCall(D3DCreateBlob(SIZE_T(fileSize), &blob));
                cacheFile.Read(fileSize, blob->GetBufferPointer());

                return blob;
            }
        }
    #endif

    // Loop until we succeed, or an exception is thrown
    while(true)
    {
        UINT flags = D3DCOMPILE_WARNINGS_ARE_ERRORS;
        #ifdef _DEBUG
            flags |= D3DCOMPILE_DEBUG;
            if(forceOptimization == false)
                flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
        #endif

        ID3DBlob* compiledShader;
        ID3DBlobPtr errorMessages;
        HRESULT hr = D3DCompileFromFile(path, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, functionName,
                                        profile, flags, 0, &compiledShader, &errorMessages);

        if (FAILED(hr))
        {
            if (errorMessages)
            {
                wchar message[1024];
                message[0] = NULL;
                char* blobdata = reinterpret_cast<char*>(errorMessages->GetBufferPointer());

                MultiByteToWideChar(CP_ACP, 0, blobdata, static_cast<int>(errorMessages->GetBufferSize()), message, 1024);
                std::wstring fullMessage = L"Error compiling shader file \"";
                fullMessage += path;
                fullMessage += L"\" - ";
                fullMessage += message;

                // Pop up a message box allowing user to retry compilation
                int retVal = MessageBoxW(NULL, fullMessage.c_str(), L"Shader Compilation Error", MB_RETRYCANCEL);
                if(retVal != IDRETRY)
                    throw DXException(hr, fullMessage.c_str());

                #if EnableShaderCaching_
                    shaderCode = GetExpandedShaderCode(path);
                    shaderHash = MurmurHash64(shaderCode.c_str(), int(shaderCode.length()));
                #endif
            }
            else
            {
                _ASSERT(false);
                throw DXException(hr);
            }
        }
        else
        {
            #if EnableShaderCaching_
                // Create the cache directory if it doesn't exist
                if(DirectoryExists(baseCacheDir.c_str()) == false)
                    Win32Call(CreateDirectory(baseCacheDir.c_str(), NULL));

                if(DirectoryExists(cacheDir.c_str()) == false)
                    Win32Call(CreateDirectory(cacheDir.c_str(), NULL));

                // Write the compiled shader to disk
                uint64 shaderSize = compiledShader->GetBufferSize();
                File cacheFile(cacheName.c_str(), File::OpenWrite);
                cacheFile.Write(shaderSize, compiledShader->GetBufferPointer());

                // Write the hash to disk
                WriteToFile(hashFileName.c_str(), shaderHash);
            #endif

            return compiledShader;
        }
    }
}

ID3D11VertexShader* CompileVSFromFile(ID3D11Device* device,
                                      LPCWSTR path,
                                      LPCSTR functionName,
                                      LPCSTR profile,
                                      const D3D_SHADER_MACRO* defines,
                                      ID3DBlob** byteCode,
                                      bool forceOptimization)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, forceOptimization);
    ID3D11VertexShader* shader = NULL;
    DXCall(device->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
                                      NULL, &shader));

    if(byteCode != NULL)
        *byteCode = compiledShader.Detach();

    return shader;
}

ID3D11PixelShader* CompilePSFromFile(ID3D11Device* device,
                                     LPCWSTR path,
                                     LPCSTR functionName,
                                     LPCSTR profile,
                                     const D3D_SHADER_MACRO* defines,
                                     bool forceOptimization)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, forceOptimization);
    ID3D11PixelShader* shader = NULL;
    DXCall(device->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
                                     NULL, &shader));

    return shader;
}

ID3D11GeometryShader* CompileGSFromFile(ID3D11Device* device,
                                        LPCWSTR path,
                                        LPCSTR functionName,
                                        LPCSTR profile,
                                        const D3D_SHADER_MACRO* defines,
                                        bool forceOptimization)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, forceOptimization);
    ID3D11GeometryShader* shader = NULL;
    DXCall(device->CreateGeometryShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
                                        NULL, &shader));

    return shader;
}

ID3D11HullShader* CompileHSFromFile(ID3D11Device* device,
                                    LPCWSTR path,
                                    LPCSTR functionName,
                                    LPCSTR profile,
                                    const D3D_SHADER_MACRO* defines,
                                    bool forceOptimization)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, forceOptimization);
    ID3D11HullShader* shader = NULL;
    DXCall(device->CreateHullShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
                                    NULL, &shader));

    return shader;
}

ID3D11DomainShader* CompileDSFromFile(ID3D11Device* device,
                                      LPCWSTR path,
                                      LPCSTR functionName,
                                      LPCSTR profile,
                                      const D3D_SHADER_MACRO* defines,
                                      bool forceOptimization)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, forceOptimization);
    ID3D11DomainShader* shader = NULL;
    DXCall(device->CreateDomainShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
                                      NULL, &shader));

    return shader;
}

ID3D11ComputeShader* CompileCSFromFile(ID3D11Device* device,
                                       LPCWSTR path,
                                       LPCSTR functionName,
                                       LPCSTR profile,
                                       const D3D_SHADER_MACRO* defines,
                                       bool forceOptimization)
{
    ID3DBlobPtr compiledShader = CompileShader(path, functionName, profile, defines, forceOptimization);
    ID3D11ComputeShader* shader = NULL;
    DXCall(device->CreateComputeShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
                                       NULL, &shader));

    return shader;
}

// == CompileOptions ==============================================================================

CompileOptions::CompileOptions()
{
    Reset();
}

void CompileOptions::Add(const std::string& name, uint32 value)
{
    _ASSERT(numDefines < MaxDefines);

    defines[numDefines].Name = buffer + bufferIdx;
    for(uint32 i = 0; i < name.length(); ++i)
        buffer[bufferIdx++] = name[i];
    ++bufferIdx;

    std::string stringVal = ToAnsiString(value);
    defines[numDefines].Definition = buffer + bufferIdx;
    for(uint32 i = 0; i < stringVal.length(); ++i)
        buffer[bufferIdx++] = stringVal[i];
    ++bufferIdx;

    ++numDefines;
}

void CompileOptions::Reset()
{
    numDefines = 0;
    bufferIdx = 0;

    for(uint32 i = 0; i < MaxDefines; ++i)
    {
        defines[i].Name = NULL;
        defines[i].Definition = NULL;
    }

    ZeroMemory(buffer, BufferSize);
}

const D3D_SHADER_MACRO* CompileOptions::Defines() const
{
    return defines;
}

}