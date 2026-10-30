#pragma once
#include "Effect.hpp"
#include "ZunColor.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"

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

#define MAX_EFFECT_COUNT 512

struct EffectManager
{
    EffectManager()
    {
        this->Reset();
    }

    void Reset();
    Effect *SpawnParticles(i32 effectIdx, D3DXVECTOR3 *pos, i32 count, ZunColor color);

    i32 nextIndex;
    i32 activeEffects;
    Effect effects[MAX_EFFECT_COUNT + 1]; // +1 dummy slot to avoid null checks for failed spawns
};
ZUN_ASSERT_TYPE(EffectManager, 0x2f984, 4);

ZunResult EffectManager_RegisterChain();
void EffectManager_CutChain();

extern EffectManager g_EffectManager;
} // namespace th06
