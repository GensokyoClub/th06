#pragma once
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "Windows.h"
#include <d3dx8math.h>

struct ZunVec2
{
    f32 x;
    f32 y;

    f32 VectorLength()
    {
        return sqrt(this->x * this->x + this->y * this->y);
    }

    f64 VectorLengthF64()
    {
        return (f64)this->VectorLength();
    }

    D3DXVECTOR2 *AsD3dXVec()
    {
        return (D3DXVECTOR2 *)this;
    }
};
ZUN_ASSERT_SIZE(ZunVec2, 0x8);

struct ZunVec3
{
    f32 x;
    f32 y;
    f32 z;

    D3DXVECTOR3 *AsD3dXVec()
    {
        return (D3DXVECTOR3 *)this;
    }

    static void SetVecCorners(ZunVec3 *topLeftCorner, ZunVec3 *bottomRightCorner, const D3DXVECTOR3 *centerPosition,
                              const D3DXVECTOR3 *size)
    {
        topLeftCorner->x = centerPosition->x - size->x / 2.0f;
        topLeftCorner->y = centerPosition->y - size->y / 2.0f;
        bottomRightCorner->x = size->x / 2.0f + centerPosition->x;
        bottomRightCorner->y = size->y / 2.0f + centerPosition->y;
    }
};
ZUN_ASSERT_SIZE(ZunVec3, 0xC);

#define ZUN_MIN(x, y) ((x) > (y) ? (y) : (x))
#define ZUN_PI ((f32)(3.14159265358979323846))
#define ZUN_2PI ((f32)(ZUN_PI * 2.0f))

#define RADIANS(degrees) ((degrees * ZUN_PI / 180.0f))

#define sincos(in, out_sine, out_cosine)                                                                               \
    {                                                                                                                  \
        __asm { \
        __asm fld in \
        __asm fsincos \
        __asm fstp out_cosine \
        __asm fstp out_sine }                                            \
    }

void __inline fsincos_wrapper(f32 *out_sine, f32 *out_cosine, f32 angle)
{
    __asm {
        fld [angle]
        fsincos
        mov eax, [out_cosine]
        fstp [eax]
        mov eax, [out_sine]
        fstp [eax]
    }
}

void __inline sincosmul(D3DXVECTOR3 *out_vel, f32 input, f32 multiplier)
{
    __asm {
        mov eax, out_vel
        fld input
        fsincos
        fmul [multiplier]
        fstp [eax]
        fmul [multiplier]
        fstp [eax+4]
    }
}

f32 __inline invertf(f32 x)
{
    return 1.f / x;
}

f32 __inline rintf(f32 float_in)
{
    __asm {
        fld float_in
        frndint
        fstp float_in
    }
    return float_in;
}
