#pragma once
#include <Windows.h>
#include <d3d8types.h>
#include <string.h>

#include "Chain.hpp"
#include "ZunResult.hpp"
#include "ZunTimer.hpp"
#include "decomp.hpp"

namespace th06
{
struct ZunRect
{
    f32 left;
    f32 top;
    f32 right;
    f32 bottom;
};

enum ScreenEffects
{
    SCREEN_EFFECT_FADE_IN,
    SCREEN_EFFECT_SHAKE,
    SCREEN_EFFECT_FADE_OUT,
};

struct ScreenEffect
{
    ScreenEffects usedEffect;
    ChainElem *calcChainElement;
    ChainElem *drawChainElement;
    unreferenced_fields(0x4);
    i32 fadeAlpha;
    i32 effectLength;
    i32 genericParam;   // effectParam1
    i32 shakinessParam; // effectParam2
    i32 unusedParam;
    ZunTimer timer;
};
ZUN_ASSERT_TYPE(ScreenEffect, 0x30, 4);

// In fade effects, effectParam1 is an RGB color to fade to
// In shake effects, effectParam1 controls the "base" view offset, and effectParam2 controls the shakiness
// multiplier over time
ScreenEffect *ScreenEffect_RegisterChain(i32 effect, u32 ticks, u32 effectParam1, u32 effectParam2,
                                         u32 unusedEffectParam);

void ScreenEffect_DrawSquare(ZunRect *rect, D3DCOLOR rectColor);
void ScreenEffect_Clear(D3DCOLOR color);
void ScreenEffect_SetViewport(D3DCOLOR color);

} // namespace th06
