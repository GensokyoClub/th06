#pragma once
#include "decomp.hpp"
#include <Windows.h>
#include <d3dx8math.h>

struct Float2
{
    f32 x;
    f32 y;
};
ZUN_ASSERT_SIZE(Float2, 0x8);

struct Float3
{
    f32 x;
    f32 y;
    f32 z;
};
ZUN_ASSERT_SIZE(Float3, 0xC);

struct FloatRect
{
    f32 left;
    f32 top;
    f32 right;
    f32 bottom;
};
ZUN_ASSERT_SIZE(FloatRect, 0x10);

// The argument is evaluated twice.
#define ZUN_SQUARE(x) ((x) * (x))

inline void SetVecCorners(D3DXVECTOR3 *topLeftCorner, D3DXVECTOR3 *bottomRightCorner, const D3DXVECTOR3 *centerPosition,
                          const D3DXVECTOR3 *size)
{
    topLeftCorner->x = centerPosition->x - size->x / 2.0f;
    topLeftCorner->y = centerPosition->y - size->y / 2.0f;
    bottomRightCorner->x = size->x / 2.0f + centerPosition->x;
    bottomRightCorner->y = size->y / 2.0f + centerPosition->y;
}

#define ZUN_MIN(x, y) ((x) > (y) ? (y) : (x))
#define ZUN_PI ((f32)(3.14159265358979323846))
#define ZUN_2PI ((f32)(ZUN_PI * 2.0f))
#define ZUN_HALF_PI ((f32)(ZUN_PI / 2.0f))

#define RADIANS(degrees) ((degrees * ZUN_PI / 180.0f))

#define sincos(in, out_sine, out_cosine)                                                                               \
    {                                                                                                                  \
        __asm { \
        __asm fld in \
        __asm fsincos \
        __asm fstp out_cosine \
        __asm fstp out_sine }                                            \
    }

inline void fsincos_wrapper(f32 *out_sine, f32 *out_cosine, f32 angle)
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

inline void sincosmul(D3DXVECTOR3 *out_vel, f32 input, f32 multiplier)
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

inline f32 rintf(f32 float_in)
{
    __asm {
        fld float_in
        frndint
        fstp float_in
    }
    return float_in;
}
