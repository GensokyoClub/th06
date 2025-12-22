#pragma once

// #include <Windows.h>
// #include <d3d8types.h>
// #include <string.h>

#include "Chain.hpp"
#include "ZunColor.hpp"
#include "ZunResult.hpp"
#include "ZunTimer.hpp"
#include "inttypes.hpp"

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
    // In fade effects, effectParam1 is an RGB color to fade to
    // In shake effects, effectParam1 controls the "base" view offset, and effectParam2 controls the shakiness
    // multiplier over time
    static ScreenEffect *RegisterChain(i32 effect, u32 ticks, u32 effectParam1, u32 effectParam2,
                                       u32 unusedEffectParam);

    static ZunResult AddedCallback(ScreenEffect *effect);
    static ZunResult DeletedCallback(ScreenEffect *effect);

    static ChainCallbackResult DrawFadeIn(ScreenEffect *effect);
    static ChainCallbackResult CalcFadeIn(ScreenEffect *effect);
    static ChainCallbackResult ShakeScreen(ScreenEffect *effect);
    static ChainCallbackResult DrawFadeOut(ScreenEffect *effect);
    static ChainCallbackResult CalcFadeOut(ScreenEffect *effect);

    static void DrawSquare(ZunRect *rect, ZunColor rectColor);
    static void Clear(ZunColor color);
    static void SetViewport(ZunColor color);

    enum ScreenEffects usedEffect;
    ChainElem *calcChainElement;
    ChainElem *drawChainElement;
    u32 unused;
    i32 fadeAlpha;
    i32 effectLength;
    i32 genericParam;   // effectParam1
    i32 shakinessParam; // effectParam2
    i32 unusedParam;
    ZunTimer timer;
};
