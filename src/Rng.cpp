#include "Rng.hpp"
#include "ZunMath.hpp"

Rng g_Rng;

u16 Rng::GetRandomU16(void)
{
    u16 a = (this->seed ^ 0x9630) - 0x6553;

    this->seed = RotateLeft16(a, 2) & 0xFFFF;
    this->generationCount++;
    return this->seed;
}

u32 Rng::GetRandomU32(void)
{
    return GetRandomU16() << 16 | GetRandomU16();
}

f32 Rng::GetRandomF32ZeroToOne(void)
{
    return (f32)GetRandomU32() / (f32)0xFFFFFFFFu;
}
