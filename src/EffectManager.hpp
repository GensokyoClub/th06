#pragma once

#include "Chain.hpp"
#include "Effect.hpp"
#include "ZunColor.hpp"
#include "ZunMath.hpp"
#include "inttypes.hpp"

namespace th06
{

enum ParticleEffects
{
    PARTICLE_EFFECT_UNK_0,
    PARTICLE_EFFECT_UNK_1,
    PARTICLE_EFFECT_UNK_2,
    PARTICLE_EFFECT_UNK_3,
    PARTICLE_EFFECT_UNK_4,
    PARTICLE_EFFECT_UNK_5,
    PARTICLE_EFFECT_UNK_6,
    PARTICLE_EFFECT_UNK_7,
    PARTICLE_EFFECT_UNK_8,
    PARTICLE_EFFECT_UNK_9,
    PARTICLE_EFFECT_UNK_10,
    PARTICLE_EFFECT_UNK_11,
    PARTICLE_EFFECT_UNK_12,
    PARTICLE_EFFECT_UNK_13,
    PARTICLE_EFFECT_UNK_14,
    PARTICLE_EFFECT_UNK_15,
    PARTICLE_EFFECT_UNK_16,
    PARTICLE_EFFECT_UNK_17,
    PARTICLE_EFFECT_UNK_18,
    PARTICLE_EFFECT_UNK_19,
};
struct EffectManager
{
    i32 nextIndex;
    i32 activeEffects;
    Effect effects[513];

    EffectManager();

    static bool RegisterChain();
    static void CutChain();
    static ChainCallbackResult OnUpdate(EffectManager *mgr);
    static bool AddedCallback(EffectManager *mgr);
    static bool DeletedCallback(EffectManager *mgr);

    static i32 EffectCallbackRandomSplash(Effect *);
    static i32 EffectCallbackRandomSplashBig(Effect *);
    static i32 EffectCallbackStill(Effect *);
    static i32 EffectUpdateCallback4(Effect *);
    static i32 EffectCallbackAttract(Effect *);
    static i32 EffectCallbackAttractSlow(Effect *);

    static ChainCallbackResult OnDraw(EffectManager *mgr);
    void Reset();
    Effect *SpawnParticles(i32 effectIdx, ZunVec3 *pos, i32 count, ZunColor color);
};
ZUN_ASSERT_SIZE(EffectManager, 0x2f984);

extern EffectManager g_EffectManager;
}; // namespace th06
