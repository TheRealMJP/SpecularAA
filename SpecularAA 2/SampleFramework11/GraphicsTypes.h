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

#include "InterfacePointers.h"
#include "Utility.h"

namespace SampleFramework11
{

struct RenderTarget2D
{
    ID3D11Texture2DPtr Texture;
    ID3D11RenderTargetViewPtr RTView;
    ID3D11ShaderResourceViewPtr SRView;
    ID3D11UnorderedAccessViewPtr UAView;
    uint32 Width;
    uint32 Height;
    uint32 NumMipLevels;
    uint32 MultiSamples;
    uint32 MSQuality;
    DXGI_FORMAT Format;
    bool32 AutoGenMipMaps;
    uint32 ArraySize;
    bool32 CubeMap;
    std::vector<ID3D11RenderTargetViewPtr> RTVArraySlices;
    std::vector<ID3D11ShaderResourceViewPtr> SRVArraySlices;

    RenderTarget2D();

  void Initialize(      ID3D11Device* device,
                        uint32 width,
                        uint32 height,
                        DXGI_FORMAT format,
                        uint32 numMipLevels = 1,
                        uint32 multiSamples = 1,
                        uint32 msQuality = 0,
                        bool32 autoGenMipMaps = false,
                        bool32 createUAV = false,
                        uint32 arraySize = 1,
                        bool32 cubeMap = false);
};

struct DepthStencilBuffer
{
    ID3D11Texture2DPtr Texture;
    ID3D11DepthStencilViewPtr DSView;
    ID3D11DepthStencilViewPtr ReadOnlyDSView;
    ID3D11ShaderResourceViewPtr SRView;
    uint32 Width;
    uint32 Height;
    uint32 MultiSamples;
    uint32 MSQuality;
    DXGI_FORMAT Format;
    uint32 ArraySize;
    std::vector<ID3D11DepthStencilViewPtr> ArraySlices;

    DepthStencilBuffer();

    void Initialize(    ID3D11Device* device,
                        uint32 width,
                        uint32 height,
                        DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                        bool32 useAsShaderResource = false,
                        uint32 multiSamples = 1,
                        uint32 msQuality = 0,
                        uint32 arraySize = 1);
};

struct RWBuffer
{
    ID3D11BufferPtr Buffer;
    ID3D11ShaderResourceViewPtr SRView;
    ID3D11UnorderedAccessViewPtr UAView;
    uint32 Size;
    uint32 Stride;
    uint32 NumElements;
    bool32 RawBuffer;
    DXGI_FORMAT Format;

    RWBuffer();

    void Initialize(ID3D11Device* device, DXGI_FORMAT format, uint32 stride, uint32 numElements, bool32 rawBuffer = false);
};

struct StructuredBuffer
{
    ID3D11BufferPtr Buffer;
    ID3D11ShaderResourceViewPtr SRView;
    ID3D11UnorderedAccessViewPtr UAView;
    uint32 Size;
    uint32 Stride;
    uint32 NumElements;

    StructuredBuffer();

    void Initialize(ID3D11Device* device, uint32 stride, uint32 numElements, bool32 useAsUAV = false,
                    bool32 appendConsume = false, bool32 useAsDrawIndirect = false, const void* initData = NULL);

  void WriteToFile(const wchar* path, ID3D11Device* device, ID3D11DeviceContext* context);
  void ReadFromFile(const wchar* path, ID3D11Device* device);
};

struct StagingBuffer
{
    ID3D11BufferPtr Buffer;
    uint32 Size;

    StagingBuffer();

    void Initialize(ID3D11Device* device, uint32 size);
    void* Map(ID3D11DeviceContext* context);
    void Unmap(ID3D11DeviceContext* context);
};

struct StagingTexture2D
{
    ID3D11Texture2DPtr Texture;
    uint32 Width;
    uint32 Height;
    uint32 NumMipLevels;
    uint32 MultiSamples;
    uint32 MSQuality;
    DXGI_FORMAT Format;
    uint32 ArraySize;

    StagingTexture2D();

    void Initialize(    ID3D11Device* device,
                        uint32 width,
                        uint32 height,
                        DXGI_FORMAT format,
                        uint32 numMipLevels = 1,
                        uint32 multiSamples = 1,
                        uint32 msQuality = 0,
                        uint32 arraySize = 1);

    void* Map(ID3D11DeviceContext* context, uint32 subResourceIndex, uint32& pitch);
    void Unmap(ID3D11DeviceContext* context, uint32 subResourceIndex);
};

template<typename T> class ConstantBuffer
{
public:

    T Data;

protected:

    ID3D11BufferPtr buffer;
    bool initialized;

public:

    ConstantBuffer() : initialized(false)
    {
        ZeroMemory(&Data, sizeof(T));
    }

    ID3D11Buffer* Buffer() const
    {
        return buffer;
    }

    void Initialize(ID3D11Device* device)
    {
        D3D11_BUFFER_DESC desc;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.ByteWidth = static_cast<uint32>(sizeof(T) + (16 - (sizeof(T) % 16)));

        DXCall(device->CreateBuffer(&desc, NULL, &buffer));

        initialized = true;
    }

    void ApplyChanges(ID3D11DeviceContext* deviceContext)
    {
        _ASSERT(initialized);

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DXCall(deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        CopyMemory(mappedResource.pData, &Data, sizeof(T));
        deviceContext->Unmap(buffer, 0);
    }

    void SetVS(ID3D11DeviceContext* deviceContext, uint32 slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->VSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetPS(ID3D11DeviceContext* deviceContext, uint32 slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->PSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetGS(ID3D11DeviceContext* deviceContext, uint32 slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->GSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetHS(ID3D11DeviceContext* deviceContext, uint32 slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->HSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetDS(ID3D11DeviceContext* deviceContext, uint32 slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->DSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetCS(ID3D11DeviceContext* deviceContext, uint32 slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->CSSetConstantBuffers(slot, 1, bufferArray);
    }
};

// For aligning to float4 boundaries
#define Float4Align __declspec(align(16))

class PIXEvent
{
public:

    PIXEvent(const wchar* markerName)
    {
        int retVal = D3DPERF_BeginEvent(0xFFFFFFFF, markerName);
        //_ASSERT(retVal >= 0);
    }

    ~PIXEvent()
    {
       int retVal = D3DPERF_EndEvent();
       //_ASSERT(retVal >= 0);
    }
};

}