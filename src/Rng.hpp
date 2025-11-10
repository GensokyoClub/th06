#pragma once

#include "diffbuild.hpp"
#include "inttypes.hpp"

namespace th06
{
struct Rng
{
    u16 seed;
    u32 generationCount;

    u16 GetRandomU16();
    u32 GetRandomU32();
    f32 GetRandomF32ZeroToOne();

    void Initialize(u16 seed)
    {
        this->seed = seed;
        this->generationCount = 0;
    }

    u16 GetRandomU16InRange(u16 range)
    {
        return range != 0 ? this->GetRandomU16() % range : 0;
    }

    u32 GetRandomU32InRange(u32 range)
    {
        return range != 0 ? this->GetRandomU32() % range : 0;
    }

    f32 GetRandomF32InRange(f32 range)
    {
        return this->GetRandomF32ZeroToOne() * range;
    }

    void GetRandomF32InBounds(f32 *res, f32 min, f32 max)
    {
        *res += this->GetRandomF32InRange(max - min) + min;
    }
};

extern Rng g_Rng;
}; // namespace th06
