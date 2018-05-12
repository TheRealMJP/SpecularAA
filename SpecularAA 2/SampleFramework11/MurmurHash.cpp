//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "PCH.h"
#include "MurmurHash.h"

namespace SampleFramework11
{

#ifdef _M_X64

// 64-bit hash for 64-bit platforms
uint64 MurmurHash64(const void* key, int len, uint32 seed)
{
    const uint64 m = 0xc6a4a7935bd1e995;
    const int r = 47;

    uint64 h = seed ^ (len * m);

    const uint64* data = (const uint64*)key;
    const uint64* end = data + (len/8);

    while(data != end)
    {
        uint64 k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char * data2 = (const unsigned char*)data;

    switch(len & 7)
    {
        case 7: h ^= uint64(data2[6]) << 48;
        case 6: h ^= uint64(data2[5]) << 40;
        case 5: h ^= uint64(data2[4]) << 32;
        case 4: h ^= uint64(data2[3]) << 24;
        case 3: h ^= uint64(data2[2]) << 16;
        case 2: h ^= uint64(data2[1]) << 8;
        case 1: h ^= uint64(data2[0]);
                h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

#else

// 64-bit hash for 32-bit platforms
uint64 MurmurHash64(const void* key, int len, uint32 seed)
{
    const unsigned int m = 0x5bd1e995;
    const int r = 24;

    unsigned int h1 = seed ^ len;
    unsigned int h2 = 0;

    const unsigned int * data = (const unsigned int *)key;

    while(len >= 8)
    {
        unsigned int k1 = *data++;
        k1 *= m; k1 ^= k1 >> r; k1 *= m;
        h1 *= m; h1 ^= k1;
        len -= 4;

        unsigned int k2 = *data++;
        k2 *= m; k2 ^= k2 >> r; k2 *= m;
        h2 *= m; h2 ^= k2;
        len -= 4;
    }

    if(len >= 4)
    {
        unsigned int k1 = *data++;
        k1 *= m; k1 ^= k1 >> r; k1 *= m;
        h1 *= m; h1 ^= k1;
        len -= 4;
    }

    switch(len)
    {
        case 3: h2 ^= ((unsigned char*)data)[2] << 16;
        case 2: h2 ^= ((unsigned char*)data)[1] << 8;
        case 1: h2 ^= ((unsigned char*)data)[0];
            h2 *= m;
    };

    h1 ^= h2 >> 18; h1 *= m;
    h2 ^= h1 >> 22; h2 *= m;
    h1 ^= h2 >> 17; h1 *= m;
    h2 ^= h1 >> 19; h2 *= m;

    uint64 h = h1;

    h = (h << 32) | h2;

    return h;
}

#endif

}
