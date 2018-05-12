//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "PCH.h"
#include "Profiler.h"
#include "Utility.h"
#include "SpriteRenderer.h"
#include "SpriteFont.h"

using std::wstring;
using std::map;


namespace SampleFramework11
{

// == Profiler ====================================================================================

Profiler Profiler::GlobalProfiler;

void Profiler::Initialize(ID3D11Device* device, ID3D11DeviceContext* immContext)
{
    this->device = device;
    this->context = immContext;
}

void Profiler::StartProfile(const wstring& name)
{
    ProfileData& profileData = profiles[name];
    _ASSERT(profileData.QueryStarted == false);
    _ASSERT(profileData.QueryFinished == false);

    if(profileData.DisjointQuery[currFrame] == NULL)
    {
        // Create the queries
        D3D11_QUERY_DESC desc;
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        desc.MiscFlags = 0;
        DXCall(device->CreateQuery(&desc, &profileData.DisjointQuery[currFrame]));

        desc.Query = D3D11_QUERY_TIMESTAMP;
        DXCall(device->CreateQuery(&desc, &profileData.TimestampStartQuery[currFrame]));
        DXCall(device->CreateQuery(&desc, &profileData.TimestampEndQuery[currFrame]));
    }

    // Start a disjoint query first
    context->Begin(profileData.DisjointQuery[currFrame]);

    // Insert the start timestamp
    context->End(profileData.TimestampStartQuery[currFrame]);

    profileData.QueryStarted = true;
}

void Profiler::EndProfile(const wstring& name)
{
    ProfileData& profileData = profiles[name];
    _ASSERT(profileData.QueryStarted == true);
    _ASSERT(profileData.QueryFinished == false);

    // Insert the end timestamp
    context->End(profileData.TimestampEndQuery[currFrame]);

    // End the disjoint query
    context->End(profileData.DisjointQuery[currFrame]);

    profileData.QueryStarted = false;
    profileData.QueryFinished = true;
}

void Profiler::EndFrame(SpriteRenderer& spriteRenderer, SpriteFont& spriteFont)
{
    currFrame = (currFrame + 1) % QueryLatency;

    Float4x4 transform;
    transform.SetTranslation(Float3(25.0f, 100.0f, 0.0f));

    float queryTime = 0.0f;

    // Iterate over all of the profiles
    ProfileMap::iterator iter;
    for(iter = profiles.begin(); iter != profiles.end(); iter++)
    {
        ProfileData& profile = (*iter).second;
        if(profile.QueryFinished == false)
            continue;

        profile.QueryFinished = false;

        if(profile.DisjointQuery[currFrame] == NULL)
            continue;

        timer.Update();

        // Get the query data
        uint64 startTime = 0;
        while(context->GetData(profile.TimestampStartQuery[currFrame], &startTime, sizeof(startTime), 0) != S_OK);

        uint64 endTime = 0;
        while(context->GetData(profile.TimestampEndQuery[currFrame], &endTime, sizeof(endTime), 0) != S_OK);

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
        while(context->GetData(profile.DisjointQuery[currFrame], &disjointData, sizeof(disjointData), 0) != S_OK);

        timer.Update();
        queryTime += timer.DeltaMillisecondsF();

        float time = 0.0f;
        if(disjointData.Disjoint == false)
        {
            uint64 delta = endTime - startTime;
            float frequency = static_cast<float>(disjointData.Frequency);
            time = (delta / frequency) * 1000.0f;
        }

        profile.TimeSamples[profile.CurrSample] = time;
        profile.CurrSample = (profile.CurrSample + 1) % ProfileData::FilterSize;

        float sum = 0.0f;
        for(UINT i = 0; i < ProfileData::FilterSize; ++i)
            sum += profile.TimeSamples[i];
        time = sum / ProfileData::FilterSize;

        wstring output = (*iter).first + L": " + ToString(time) + L"ms";
        // spriteRenderer.RenderText(spriteFont, output.c_str(), transform, XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));

        transform._42 += 25.0f;
    }
}

// == ProfileBlock ================================================================================

ProfileBlock::ProfileBlock(const std::wstring& name) : name(name)
{
    Profiler::GlobalProfiler.StartProfile(name);
}

ProfileBlock::~ProfileBlock()
{
    Profiler::GlobalProfiler.EndProfile(name);
}

}