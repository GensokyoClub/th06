#pragma once
#include "Effect.hpp"
#include "ZunColor.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"

namespace th06
{

enum ParticleEffects
{
    PARTICLE_DEATH_POP,
    PARTICLE_DEATH_SWIRL,
    PARTICLE_DEATH_POP_LARGE,
    PARTICLE_BOMB_FLASH,
    PARTICLE_SPARK,
    PARTICLE_SPARK_RED,
    PARTICLE_SPARK_GREEN,
    PARTICLE_SPARK_BLUE,
    PARTICLE_SPARK_SMALL,
    PARTICLE_SPARK_RED_SMALL,
    PARTICLE_SPARK_GREEN_SMALL,
    PARTICLE_SPARK_BLUE_SMALL,
    PARTICLE_FLASH,
    PARTICLE_SPELL_ORBIT,
    PARTICLE_SPELL_ORBIT_DUPE,
    PARTICLE_SPELL_ORBIT_DUPE2,
    PARTICLE_SPELL_BACKGROUND, // why???
    PARTICLE_GLOW,
    PARTICLE_DIM_GLOW,
    PARTICLE_DIM_SPARK, // WARNING: Does not spray like other sparks
};

// This exists only because Enemy::deathSparkParticle is offset by 4
enum SparkEffects
{
    SPARK_TYPE_WHITE,
    SPARK_TYPE_RED,
    SPARK_TYPE_GREEN,
    SPARK_TYPE_BLUE,
    SPARK_TYPE_WHITE_SMALL,
    SPARK_TYPE_RED_SMALL,
    SPARK_TYPE_GREEN_SMALL,
    SPARK_TYPE_BLUE_SMALL
};

#define MAX_EFFECT_COUNT 512

struct EffectManager
{
    EffectManager()
    {
        this->Reset();
    }

    void Reset();
    Effect *SpawnParticles(i32 effectId, D3DXVECTOR3 *pos, i32 count = 1, ZunColor tint = COLOR_WHITE);

    i32 nextIndex;
    i32 activeEffects;
    Effect effects[MAX_EFFECT_COUNT + 1]; // +1 dummy slot to avoid null checks for failed spawns
};
ZUN_ASSERT_TYPE(EffectManager, 0x2f984, 4);

ZunResult EffectManager_RegisterChain();
void EffectManager_CutChain();

extern EffectManager g_EffectManager;
} // namespace th06
