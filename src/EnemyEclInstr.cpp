#include "stdafx.h"

#include "EnemyEclInstr.hpp"
#include "EffectManager.hpp"
#include "EnemyManager.hpp"
#include "BulletManager.hpp"

#define GET_ARG(type, num) ((type *)&curInstr->args)[num]
#define GET_STRING_ARG(num) &GET_ARG(char, num)
#define GET_INT_ARG(num) GET_ARG(i32, num)
#define GET_SHORT_ARG(num) GET_ARG(i16, num)
#define GET_FLOAT_ARG(num) GET_ARG(float, num)
#define GET_VAR_ARG(num) GET_ARG(EclVarId, num)

namespace th06
{
#pragma var_order(i, currentBullet, effectIndex)
void ExInsCirnoRainbowBallJank(Enemy *enemy, EclRawInstr *curInstr)
{
    i32 i;

    Bullet *currentBullet = g_BulletManager.bullets;
    i32 effectIndex = GET_INT_ARG(1);

    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &enemy->position, 1, COLOR_WHITE);
    for (i = 0; i < MAX_ENEMY_BULLETS; i++, currentBullet++)
    {
        if (currentBullet->state == BULLET_STATE_INACTIVE || currentBullet->state == BULLET_STATE_DESPAWNING)
        {
            continue;
        }

        currentBullet->color = BULLET_WHITE;
        g_AnmManager->SetActiveSprite(&currentBullet->sprites.spriteBullet,
                                      currentBullet->sprites.spriteBullet.baseSpriteIndex + currentBullet->color);
        switch (effectIndex)
        {
        case 0:
            currentBullet->speed = 0.0f;
            currentBullet->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            break;
        case 1:
            currentBullet->exFlags |= EX_ACCELERATION;
            currentBullet->exDuration = 220;
            currentBullet->timer = 0;
            sincosmul(&currentBullet->ex4Acceleration, g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI, 0.01f);
            break;
        }
    }
}

void ExInsShootAtRandomArea(Enemy *enemy, EclRawInstr *curInstr)
{
    f32 bulletSpeed = GET_INT_ARG(1);
    enemy->bulletProps.position = enemy->position + enemy->shootOffset;
    enemy->bulletProps.position.x = (g_Rng.GetRandomF32InRange(bulletSpeed) + enemy->position.x) - bulletSpeed / 2.0f;
    bulletSpeed *= 0.75f;
    enemy->bulletProps.position.y = (g_Rng.GetRandomF32InRange(bulletSpeed) + enemy->position.y) - bulletSpeed / 2.0f;
    g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
}

#pragma var_order(i, propsSpeedBackup, starPatterTarget1, targetDistance, starPatternTarget0, patternPosition,         \
                  baseTargetPosition)
void ExInsShootStarPattern(Enemy *enemy, EclRawInstr *curInstr)
{
#define ENEMY_POS 0
#define PLAYER_POS 1

    if (enemy->currentContext.int2 >= enemy->currentContext.int3)
    {
        enemy->currentContext.funcSetFunc = NULL;
        return;
    }

    if (enemy->currentContext.int2 == 0)
    {
        g_EclManager.extra.coords[ENEMY_POS] = enemy->position;
        g_EclManager.extra.coords[PLAYER_POS] = g_Player.positionCenter;
        g_EclManager.extra.starAngleTable[0] = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
        g_EclManager.extra.starAngleTable[1] =
            utils::AddNormalizeAngle(g_EclManager.extra.starAngleTable[0], 4.0f * ZUN_PI / 5.0f);
    }
    if (enemy->currentContext.int2 % 30 == 0)
    {
        g_EclManager.extra.starAngleTable[0] = g_EclManager.extra.starAngleTable[1];
        g_EclManager.extra.starAngleTable[1] =
            utils::AddNormalizeAngle(g_EclManager.extra.starAngleTable[0], 4.0f * ZUN_PI / 5.0f);
        g_EclManager.extra.starAngleTable[2] =
            utils::AddNormalizeAngle(g_EclManager.extra.starAngleTable[1], 4.0f * ZUN_PI / 5.0f);
        g_EclManager.extra.starAngleTable[3] =
            utils::AddNormalizeAngle(g_EclManager.extra.starAngleTable[2], 4.0f * ZUN_PI / 5.0f);
        g_EclManager.extra.starAngleTable[4] =
            utils::AddNormalizeAngle(g_EclManager.extra.starAngleTable[3], 4.0f * ZUN_PI / 5.0f);
        g_EclManager.extra.starAngleTable[5] =
            utils::AddNormalizeAngle(g_EclManager.extra.starAngleTable[4], 4.0f * ZUN_PI / 5.0f);
    }
    if (enemy->currentContext.int2 % 6 == 0)
    {
        // Variable names are quick guesses at functionality and should not be trusted.
        D3DXVECTOR3 baseTargetPosition;
        i32 i;
        f32 propsSpeedBackup;
        f32 patternPosition;
        D3DXVECTOR3 starPatternTarget0;
        D3DXVECTOR3 starPatterTarget1;
        f32 targetDistance;

        patternPosition = (f32)enemy->currentContext.int2 / (f32)enemy->currentContext.int3;
        targetDistance = patternPosition * 0.1f;

        baseTargetPosition =
            (g_EclManager.extra.coords[PLAYER_POS] - g_EclManager.extra.coords[ENEMY_POS]) * targetDistance +
            g_EclManager.extra.coords[ENEMY_POS];
        baseTargetPosition.z = 0.0f;

        patternPosition += 0.5f;
        enemy->bulletProps.angle1 = (ZUN_PI / 3.0f) * patternPosition;

        for (i = 0; i < 5; i++)
        {
            targetDistance = (enemy->currentContext.int2 % 30) / 30.0f;
            sincosmul(&starPatternTarget0, g_EclManager.extra.starAngleTable[i], enemy->currentContext.float3);
            sincosmul(&starPatterTarget1, g_EclManager.extra.starAngleTable[i + 1], enemy->currentContext.float3);
            starPatternTarget0 = (starPatterTarget1 - starPatternTarget0) * targetDistance + starPatternTarget0;
            starPatternTarget0.z = 0.0f;
            enemy->bulletProps.position = baseTargetPosition + starPatternTarget0;
            propsSpeedBackup = enemy->bulletProps.speed1;
            enemy->bulletProps.speed1 =
                g_Rng.GetRandomF32InRange(enemy->bulletProps.speed2) + enemy->bulletProps.speed1;
            g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
            enemy->bulletProps.speed1 = propsSpeedBackup;
            enemy->bulletProps.angle1 -= (ZUN_PI / 6.0f) * patternPosition;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_16);
    }
    enemy->currentContext.int2++;
#undef ENEMY_POS
#undef PLAYER_POS
}

enum PatchouliSpells
{
    FIRE_EARTH_SIGN_LAVA_CROMLECH,
    WOOD_FIRE_SIGN_FOREST_BLAZE,
    WATER_WOOD_SIGN_WATER_ELF,
    METAL_WATER_SIGN_MERCURY_POISON,
    EARTH_METAL_SIGN_EMERALD_MEGALITH
};

#if BUILT_SINCE(100)
// clang-format off
// NOTE: The CI will try to reformat this into unreadable mess
i32 g_PatchouliShottypeVars[CHARACTER_COUNT][SHOTTYPES_PER_CHARACTER][3] = {
    { // Reimu
        { // A
            FIRE_EARTH_SIGN_LAVA_CROMLECH,
            METAL_WATER_SIGN_MERCURY_POISON,
            WOOD_FIRE_SIGN_FOREST_BLAZE
        },
        { // B
            WATER_WOOD_SIGN_WATER_ELF,
            METAL_WATER_SIGN_MERCURY_POISON,
            EARTH_METAL_SIGN_EMERALD_MEGALITH
        }
    },
    { // Marisa
        { // A
            WOOD_FIRE_SIGN_FOREST_BLAZE,
            EARTH_METAL_SIGN_EMERALD_MEGALITH,
            FIRE_EARTH_SIGN_LAVA_CROMLECH
        },
        { // B
            EARTH_METAL_SIGN_EMERALD_MEGALITH,
            WATER_WOOD_SIGN_WATER_ELF,
            METAL_WATER_SIGN_MERCURY_POISON
        }
    }
};
// clang-format on
#endif

void ExInsPatchouliShottypeSetVars(Enemy *enemy, EclRawInstr *curInstr)
{
#if BUILT_SINCE(100)
    enemy->currentContext.int1 = g_PatchouliShottypeVars[g_GameManager.character][g_GameManager.shotType][0];
    enemy->currentContext.int2 = g_PatchouliShottypeVars[g_GameManager.character][g_GameManager.shotType][1];
    enemy->currentContext.int3 = g_PatchouliShottypeVars[g_GameManager.character][g_GameManager.shotType][2];
#else
    // This kinda explains why this was originally an
    // ex instruction and not just some ECL if statements

    // clang-format off
    u8 spells[] = {
        FIRE_EARTH_SIGN_LAVA_CROMLECH,
        WOOD_FIRE_SIGN_FOREST_BLAZE,
        WATER_WOOD_SIGN_WATER_ELF,
        METAL_WATER_SIGN_MERCURY_POISON,
        EARTH_METAL_SIGN_EMERALD_MEGALITH,
    };
    // clang-format on

    i32 spell1 = g_Rng.GetRandomU16InRange(ARRAY_SIZE(spells));
    enemy->currentContext.int1 = spell1;
    if (spell1 != 4)
    {
        u8 temp = spells[spell1];
        spells[spell1] = spells[4];
        spells[4] = temp;
    }

    u32 spell2 = spells[g_Rng.GetRandomU16InRange(ARRAY_SIZE(spells) - 1)];
    enemy->currentContext.int2 = spell2;
    if (spell2 != 3)
    {
        u8 temp = spells[spell2];
        spells[spell2] = spells[3];
        spells[3] = temp;
    }

    u32 spell3 = g_Rng.GetRandomU16InRange(ARRAY_SIZE(spells) - 2);
    enemy->currentContext.int3 = spells[spell3];
#endif
}

#pragma var_order(playerBulletOffsetY, playerBulletOffsetX, bulletsLeft, i, currentBullet)
void ExInsStage56Func4(Enemy *enemy, EclRawInstr *curInstr)
{
    i32 bulletsLeft;
    Bullet *currentBullet;
    i32 i;
    f32 playerBulletOffsetX;
    f32 playerBulletOffsetY;

    if (GET_INT_ARG(1) < 2)
    {
        g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &enemy->position, 1, COLOR_WHITE);
        g_GameManager.isTimeStopped = GET_INT_ARG(1);
    }
    else
    {
        bulletsLeft = 14;
        currentBullet = g_BulletManager.bullets;
        if (g_GameManager.difficulty <= NORMAL)
        {
            for (i = 0; i < MAX_ENEMY_BULLETS; i++, currentBullet++)
            {
                if (currentBullet->state == BULLET_STATE_INACTIVE || currentBullet->state == BULLET_STATE_DESPAWNING)
                {
                    continue;
                }

                if (currentBullet->sprites.spriteBullet.sprite != NULL &&
                    currentBullet->sprites.spriteBullet.sprite->heightPx >= BULLET_SIZE_MEDIUM &&
                    currentBullet->color != BULLET_GREEN8 && (g_Rng.GetRandomU16() % 4 == 0))
                {
                    currentBullet->color = BULLET_GREEN8;
                    g_AnmManager->SetActiveSprite(&currentBullet->sprites.spriteBullet,
                                                  currentBullet->sprites.spriteBullet.baseSpriteIndex +
                                                      currentBullet->color);

                    playerBulletOffsetX = (currentBullet->pos.x) - g_Player.positionCenter.x;
                    playerBulletOffsetY = (currentBullet->pos.y) - g_Player.positionCenter.y;

                    if (sqrtf(ZUN_SQUARE(playerBulletOffsetX) + ZUN_SQUARE(playerBulletOffsetY)) > 128.0f)
                    {
                        currentBullet->angle = g_Rng.GetRandomF32InRange((ZUN_PI * 3.0f) / 4.0f) + (ZUN_PI / 4.0f);
                    }
                    else
                    {
                        currentBullet->angle = g_Player.AngleFromPlayer(&currentBullet->pos) + ZUN_HALF_PI +
                                               g_Rng.GetRandomF32InRange(ZUN_2PI);
                    }

                    sincosmul(&currentBullet->velocity, currentBullet->angle, currentBullet->speed);
                    bulletsLeft--;
                    if (bulletsLeft == 0)
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            bulletsLeft = 52;
            for (i = 0; i < MAX_ENEMY_BULLETS; i++, currentBullet++)
            {
                if (currentBullet->state == BULLET_STATE_INACTIVE || currentBullet->state == BULLET_STATE_DESPAWNING)
                {
                    continue;
                }

                if (currentBullet->sprites.spriteBullet.sprite != NULL &&
                    currentBullet->sprites.spriteBullet.sprite->heightPx >= BULLET_SIZE_MEDIUM &&
                    currentBullet->color != BULLET_GREEN8 && (g_Rng.GetRandomU16() % 4 == 0))
                {
                    currentBullet->color = BULLET_GREEN8;
                    g_AnmManager->SetActiveSprite(&currentBullet->sprites.spriteBullet,
                                                  currentBullet->sprites.spriteBullet.baseSpriteIndex +
                                                      currentBullet->color);

                    playerBulletOffsetX = (currentBullet->pos.x) - g_Player.positionCenter.x;
                    playerBulletOffsetY = (currentBullet->pos.y) - g_Player.positionCenter.y;

                    if (sqrtf(ZUN_SQUARE(playerBulletOffsetX) + ZUN_SQUARE(playerBulletOffsetY)) > 128.0f)
                    {
                        currentBullet->angle = g_Rng.GetRandomF32InRange(ZUN_2PI);
                    }
                    else
                    {
                        currentBullet->angle = g_Player.AngleFromPlayer(&currentBullet->pos) + ZUN_HALF_PI +
                                               g_Rng.GetRandomF32InRange(ZUN_2PI);
                    }

                    sincosmul(&currentBullet->velocity, currentBullet->angle, currentBullet->speed);
                    bulletsLeft--;
                    if (bulletsLeft == 0)
                    {
                        break;
                    }
                }
            }
        }
    }
    enemy->currentContext.int2 = 0;
}

#pragma var_order(patternPosition, i, bulletProps, sinOut, bpPositionOffset, matrixOutSeed, matrixIn, bulletAngle,     \
                  cosOut, matrixInSeed, matrixOut)
void ExInsStage5Func5(Enemy *enemy, EclRawInstr *curInstr)
{
    if (enemy->currentContext.int2 % 9 == 0)
    {
        D3DXVECTOR3 bpPositionOffset;
        f32 bulletAngle;
        f32 cosOut;
        i32 i;
        D3DXVECTOR3 matrixIn;
        f32 matrixInSeed;
        D3DXVECTOR3 matrixOut;
        f32 matrixOutSeed; // Later reused to store angles for trig function calls
        i32 patternPosition;
        f32 sinOut;
        EnemyShooter bulletProps;

        patternPosition = enemy->currentContext.int2 / 9;
        bulletProps.sprite = BULLET_DAGGER;
        bulletProps.aimMode = FAN_AIMED;
        if (g_GameManager.difficulty <= NORMAL)
        {
            bulletProps.count1 = 1;
        }
        else
        {
            bulletProps.count1 = 3;
        }
        bulletProps.count2 = 1;
        bulletProps.angle2 = ZUN_PI / 6.0f;
        bulletProps.unk_4a = 0;
        bulletProps.flags = 0;

        matrixOutSeed = 0.5f - patternPosition * 0.5f / 9.0f;
        matrixOut = g_Player.positionCenter - enemy->position;
        D3DXVec3Normalize(&matrixIn, &matrixOut);
        if (patternPosition & 1)
        {
            matrixInSeed = -256.0f;
        }
        else
        {
            matrixInSeed = 256.0f;
        }
        matrixIn *= matrixInSeed;
        matrixOut *= matrixOutSeed;
        bpPositionOffset = matrixOut + matrixIn;
        matrixIn = -matrixIn;

        matrixOutSeed = ZUN_PI / 4.0f;
        cosOut = cosf(matrixOutSeed);
        sinOut = sinf(matrixOutSeed);
        matrixOut = matrixIn;
        matrixIn.x = matrixOut.x * cosOut + matrixOut.y * sinOut;
        matrixIn.y = -matrixOut.x * sinOut + matrixOut.y * cosOut;
        matrixOutSeed = -ZUN_PI / 18.0f;
        cosOut = cosf(matrixOutSeed);
        sinOut = sinf(matrixOutSeed);
        bulletProps.angle1 = 0.0f;
        bulletAngle = -ZUN_PI / 4.0f;

        for (i = 0; i < 9; i++, bulletAngle += ZUN_PI / 18.0f)
        {
            matrixOut = matrixIn;
            matrixIn.x = matrixOut.x * cosOut + matrixOut.y * sinOut;
            matrixIn.y = -matrixOut.x * sinOut + matrixOut.y * cosOut;

            bulletProps.position = matrixIn + enemy->position + bpPositionOffset;
            bulletProps.speed1 = 2.0f;
            if ((patternPosition & 1) && g_GameManager.difficulty <= NORMAL)
            {
                bulletProps.angle1 = bulletAngle;
            }
            bulletProps.color = BULLET_BLUE8;
            g_BulletManager.SpawnBulletPattern(&bulletProps);
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_7);
    }
    enemy->currentContext.int2++;
}

#pragma var_order(effect, baseAngleModifier, distanceModifier, finalAngle, particlePos)
void ExInsBatWingEffect(Enemy *enemy, EclRawInstr *curInstr)
{
    i32 baseAngleModifier;
    f32 distanceModifier;
    Effect *effect;
    f32 finalAngle;
    D3DXVECTOR3 particlePos;

#if BUILT_SINCE(100)
    if (enemy->flags.isInvisible)
    {
        Enemy_ResetEffectArray(enemy);
        return;
    }
#endif

    enemy->exInsFunc6Angle += RADIANS(1.0f);
    if (enemy->exInsFunc6Angle >= RADIANS(45.0f))
    {
        enemy->exInsFunc6Angle -= RADIANS(90.0f);
    }

    // Run every 8 frames for first 30 frames, then every 4 for next 30, then every 2 for next 60, then every frame
    if (
#if BUILT_SINCE(100)
        enemy->exInsFunc6Timer.HasTicked() &&
#endif
        (enemy->exInsFunc6Timer > 120 || (enemy->exInsFunc6Timer > 60 && enemy->exInsFunc6Timer % 2 == 0) ||
         (enemy->exInsFunc6Timer > 30 && enemy->exInsFunc6Timer % 4 == 0) || enemy->exInsFunc6Timer % 8 == 0))
    {
        baseAngleModifier = enemy->exInsFunc6Timer % 16;
        baseAngleModifier = g_Rng.GetRandomU16InRange(baseAngleModifier / 2) + baseAngleModifier / 2;
        distanceModifier = (baseAngleModifier * 160.0f) / 16.0f + 32.0f;
        finalAngle = enemy->exInsFunc6Angle - (baseAngleModifier * RADIANS(180.0f)) / 40.0f;
        if (distanceModifier < RADIANS(-45.0f))
        {
            distanceModifier += RADIANS(90.0f);
        }

        particlePos = enemy->position;
        particlePos.x += cosf(finalAngle) * distanceModifier;
        particlePos.y += sinf(finalAngle) * distanceModifier;
        effect = g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_19, &particlePos, 1, COLOR_DEEPBLUE);
        effect->unk_11c.x = (g_Rng.GetRandomF32InRange(40.0f) - 20.0f) / 60.0f;
        effect->unk_11c.y = 8.0f * baseAngleModifier / 60.0f - (4.0f / 15.0f);
        effect->unk_11c.z = 0.0f;
        effect->unk_128 = -effect->unk_11c / 120.0f;

        particlePos = enemy->position;
        particlePos.x -= cosf(finalAngle) * distanceModifier;
        particlePos.y += sinf(finalAngle) * distanceModifier;
        effect = g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_19, &particlePos, 1, COLOR_DEEPBLUE);
        effect->unk_11c.x = (g_Rng.GetRandomF32InRange(40.0f) - 20.0f) / 60.0f;
        effect->unk_11c.y = 8.0f * baseAngleModifier / 60.0f - (4.0f / 15.0f);
        effect->unk_11c.z = 0.0f;
        effect->unk_128 = -effect->unk_11c / 120.0f;
    }

    enemy->exInsFunc6Timer++;
}

#pragma var_order(laserProps, i, lengthMultiplier, attackType, innerLoopCount, angleDiff, outerLoopCount, laserAngle,  \
                  randomAngleModifier, positionVectors)
void ExInsStage6Func7(Enemy *enemy, EclRawInstr *curInstr)
{
    f32 angleDiff;
    i32 i;
    i32 innerLoopCount;
    f32 laserAngle;
    EnemyShooter laserProps;
    f32 lengthMultiplier;
    i32 outerLoopCount;

    D3DXVECTOR3 positionVectors[8];

    i32 attackType = GET_INT_ARG(1);
    f32 randomAngleModifier = g_Rng.GetRandomF32InRange(ZUN_2PI);

    for (outerLoopCount = 0; outerLoopCount < 2; outerLoopCount++)
    {
        if (outerLoopCount == 0)
        {
            laserAngle = -ZUN_PI + randomAngleModifier;
            angleDiff = ZUN_PI / 4.0f;
        }
        else
        {
            laserAngle = (7.0f * -ZUN_PI / 8.0f) + randomAngleModifier;
            angleDiff = -ZUN_PI / 4.0f;
        }

        lengthMultiplier = 32.0f;
        for (i = 0; i < 8; i++)
        {
            positionVectors[i] = enemy->position;
            positionVectors[i].x += cosf(laserAngle) * lengthMultiplier;
            positionVectors[i].y += sinf(laserAngle) * lengthMultiplier;
            laserAngle += ZUN_PI / 4.0f;
        }

        if (outerLoopCount == 0)
        {
            laserAngle = -ZUN_PI + randomAngleModifier;
        }
        else
        {
            laserAngle = (7.0f * -ZUN_PI / 8.0f) + randomAngleModifier;
        }

        for (innerLoopCount = 0; innerLoopCount < 3; innerLoopCount++)
        {
            if (innerLoopCount < 2)
            {
                lengthMultiplier = 112.0f;
            }
            else
            {
                lengthMultiplier = 480.0f;
            }
            for (i = 0; i < 8; i++)
            {
                laserProps.position = positionVectors[i];
                laserProps.sprite = LASER_BEAM;
#if BUILT_SINCE(100)
                if (attackType == 0)
#endif
                {
                    if (g_GameManager.difficulty <= NORMAL)
                    {
                        laserProps.color = BULLET_RED;
                    }
                    else
                    {
                        laserProps.color = BULLET_CYAN;
                    }
                    laserProps.angle1 = laserAngle;
                    laserProps.speed1 = 0.0f;
                    laserProps.startOffset = 0.0f;
                    if (g_GameManager.difficulty <= NORMAL)
                    {
                        laserProps.endOffset = lengthMultiplier;
                        laserProps.startLength = lengthMultiplier;
                    }
                    else
                    {
                        laserProps.endOffset = 440.0f;
                        laserProps.startLength = 440.0f;
                    }
                    if (g_GameManager.difficulty <= NORMAL)
                    {
                        laserProps.width = 28.0f;
                    }
                    else
                    {
                        laserProps.width = 20.0f;
                    }
                    laserProps.startTime = innerLoopCount * 16 + 60;
#if BUILT_SINCE(100)
                    laserProps.duration = 90 - innerLoopCount * 16;
#else
                    laserProps.duration = 140 - innerLoopCount * 16;
#endif
                    laserProps.despawnDuration = 16;
                    laserProps.hitboxStartTime = 50;
                    laserProps.hitboxEndDelay = 16;
                    laserProps.flags = LASER_FLAG_UNUSED; // ???
                    laserProps.aimMode = LASER_UNAIMED;
                    g_BulletManager.SpawnLaserPattern(&laserProps);
                }
#if BUILT_SINCE(100)
                else
                {
                    enemy->bulletProps.position = laserProps.position;
                    g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
                }
#endif
                positionVectors[i].x += cosf(laserAngle) * lengthMultiplier;
                positionVectors[i].y += sinf(laserAngle) * lengthMultiplier;
                laserAngle += ZUN_PI / 4.0f;
            }
            laserAngle += angleDiff - ZUN_2PI;
        }
    }
}

#pragma var_order(bulletProps, changedBullets, i, currentBullet)
void ExInsStage6Func8(Enemy *enemy, EclRawInstr *curInstr)
{
    i32 changedBullets;
    Bullet *currentBullet;
    i32 i;

    changedBullets = 0;
    currentBullet = g_BulletManager.bullets;

    EnemyShooter bulletProps;

    for (i = 0; i < MAX_ENEMY_BULLETS; i++, currentBullet++)
    {
        if (currentBullet->state == BULLET_STATE_INACTIVE || currentBullet->state == BULLET_STATE_DESPAWNING)
        {
            continue;
        }

        if (currentBullet->sprites.spriteBullet.sprite != NULL &&
            currentBullet->sprites.spriteBullet.sprite->heightPx >= BULLET_SIZE_MEDIUM)
        {
            bulletProps.position = currentBullet->pos;
            bulletProps.sprite = BULLET_BALL;
            bulletProps.color = BULLET_DARK_RED;
            bulletProps.angle1 = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
            bulletProps.speed1 = 0.0f;
            bulletProps.count1 = 1;
            bulletProps.count2 = 1;
            bulletProps.flags = EX_SPAWN_EFFECT_LONG;
            bulletProps.aimMode = FAN;
            g_BulletManager.SpawnBulletPattern(&bulletProps);
            changedBullets++;
        }
    }

    enemy->currentContext.int3 = changedBullets;
}

#pragma var_order(unusedShooter, i, local64, currentBullet, randomAngleModifier)
void ExInsStage6Func9(Enemy *enemy, EclRawInstr *curInstr)
{
    Bullet *currentBullet;
#if BUILT_SINCE(013)
    f32 distance;
#endif
    i32 i;
    f32 randomAngleModifier;

    currentBullet = g_BulletManager.bullets;
    EnemyShooter unusedShooter;

    randomAngleModifier = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &enemy->position, 1, COLOR_WHITE);

    for (i = 0; i < MAX_ENEMY_BULLETS; i++, currentBullet++)
    {
        if (currentBullet->state == BULLET_STATE_INACTIVE || currentBullet->state == BULLET_STATE_DESPAWNING)
        {
            continue;
        }

        if (currentBullet->sprites.spriteBullet.sprite != NULL &&
            currentBullet->sprites.spriteBullet.sprite->heightPx < BULLET_SIZE_MEDIUM && currentBullet->speed == 0.0f)
        {
            // TODO: 0.08 codegen diffs
            currentBullet->exFlags |= EX_ACCELERATION;
            currentBullet->color = BULLET_RED;
#if BUILT_SINCE(100)
            g_AnmManager->SetActiveSprite(&currentBullet->sprites.spriteBullet,
                                          currentBullet->sprites.spriteBullet.baseSpriteIndex + currentBullet->color);
#endif
            currentBullet->speed = 0.01f;
            currentBullet->timer = 0;
            currentBullet->exDuration = 120;
#if BUILT_SINCE(013)
            distance = ZUN_SQUARE(enemy->position.x - currentBullet->pos.x) +
                       ZUN_SQUARE(enemy->position.y - currentBullet->pos.y);
            if (distance > 0.1f)
            {
                distance = sqrtf(distance);
            }
            else
            {
                distance = 0.0f;
            }

            sincosmul(&currentBullet->ex4Acceleration, (distance * ZUN_PI) / 256.0f + randomAngleModifier, 0.01f);
#else
            // Builds before 2002-08-25 take the square root of any distance, including zero.
            sincosmul(&currentBullet->ex4Acceleration,
                      (sqrtf(ZUN_SQUARE(enemy->position.x - currentBullet->pos.x) +
                             ZUN_SQUARE(enemy->position.y - currentBullet->pos.y)) *
                       ZUN_PI) /
                              256.0f +
                          randomAngleModifier,
                      0.01f);
#endif
        }
    }
}

#pragma var_order(unusedShooter, i, currentBullet, unusedRandomAngle)
void ExInsStage6Func11(Enemy *enemy, EclRawInstr *curInstr)
{
    Bullet *currentBullet = g_BulletManager.bullets;
    i32 i;
    EnemyShooter unusedShooter;

    f32 unusedRandomAngle = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &enemy->position, 1, COLOR_WHITE);

    for (i = 0; i < MAX_ENEMY_BULLETS; i++, currentBullet++)
    {
        if (currentBullet->state == BULLET_STATE_INACTIVE || currentBullet->state == BULLET_STATE_DESPAWNING)
        {
            continue;
        }

        if (currentBullet->sprites.spriteBullet.sprite != NULL &&
            currentBullet->sprites.spriteBullet.sprite->heightPx < BULLET_SIZE_MEDIUM && currentBullet->speed == 0.0f)
        {
            // TODO: 0.08 codegen diffs
            currentBullet->exFlags |= EX_ACCELERATION;
            currentBullet->color = BULLET_RED;
#if BUILT_SINCE(100)
            g_AnmManager->SetActiveSprite(&currentBullet->sprites.spriteBullet,
                                          currentBullet->sprites.spriteBullet.baseSpriteIndex + currentBullet->color);
#endif
            currentBullet->speed = 0.01f;
            currentBullet->timer = 0;
            currentBullet->exDuration = 120;

            sincosmul(&currentBullet->ex4Acceleration, g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI, 0.01f);
        }
    }
}

void ExInsHandleBatTransformation(Enemy *enemy, EclRawInstr *curInstr)
{
#if BUILT_SINCE(101)
    // 1.00 keeps transforming after the enemy dies.
    if (enemy->life <= 0)
    {
        return;
    }
#endif

    ExInsBatWingEffect(enemy, curInstr);
    if (g_Player.bombInfo.isInUse)
    {
        if (enemy->anmPoseLeft >= 0) // Check if poses are enabled
        {
            g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, ANM_SCRIPT_ENEMY_END);
            enemy->anmPoseLeft = -1; // Disable poses for duration of transformation
        }

        enemy->flags.isInteractable = false;
#if BUILT_SINCE(100)
        enemy->transformTimer = 60;
#endif
    }
    else
    {
#if BUILT_SINCE(100)
        if (enemy->transformTimer > 0 && (enemy->transformTimer--, enemy->transformTimer == 0))
#endif
        {
            if (enemy->anmPoseLeft < 0) // Check if poses are disabled
            {
                g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, ANM_OFFSET_ENEMY + 160);
                enemy->anmPoseLeft = 161;
            }

            enemy->flags.isInteractable = true;
        }
    }
}

void ExInsStage4Func12(Enemy *enemy, EclRawInstr *curInstr)
{
    for (i32 i = 0; i < 8; i++)
    {
        if (enemy->lasers[i] != NULL && enemy->lasers[i]->inUse)
        {
            enemy->bulletProps.position = D3DXVECTOR3(64.0f, 0.0f, 0.0f);
            utils::Rotate(&enemy->bulletProps.position, &enemy->bulletProps.position, enemy->lasers[i]->angle);
            enemy->bulletProps.position += enemy->position;
            g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
        }
    }
}

#if BUILT_SINCE(100)
#pragma var_order(i, bulletProps, basePatternAngle, numPatterns)
void ExInsStageXFunc13(Enemy *enemy, EclRawInstr *curInstr)
{
    EnemyShooter bulletProps = enemy->bulletProps;
    i32 i;

    i32 numPatterns = GET_INT_ARG(1);
    f32 basePatternAngle = enemy->currentContext.float2;
    if (enemy->currentContext.int3 % 6 == 0)
    {
        for (i = 0; i < numPatterns; i++, basePatternAngle += ZUN_2PI / numPatterns)
        {
            sincosmul(&bulletProps.position, basePatternAngle, enemy->currentContext.float3);
            bulletProps.position.x += GAME_REGION_WIDTH / 2.0f;
            bulletProps.position.y += GAME_REGION_HEIGHT / 2.0f;
            bulletProps.angle1 = basePatternAngle + enemy->currentContext.float1;
            g_BulletManager.SpawnBulletPattern(&bulletProps);
        }
    }
    enemy->currentContext.int3++;
}

#pragma var_order(bulletPosition, i, angleSin, currentLaser, angleCos, positionMultiplier)
void ExInsStageXFunc14(Enemy *enemy, EclRawInstr *curInstr)
{
    f32 angleCos;
    f32 angleSin;
    D3DXVECTOR3 bulletPosition;
    Laser *currentLaser;
    i32 i;
    f32 positionMultiplier;

    enemy->currentContext.int3 = 0;
    for (i = 0; i < 8; i++)
    {
        if (enemy->lasers[i] != NULL && enemy->lasers[i]->inUse)
        {
            currentLaser = enemy->lasers[i];
            positionMultiplier = currentLaser->startOffset;
            fsincos_wrapper(&angleSin, &angleCos, currentLaser->angle);

            while (currentLaser->endOffset > positionMultiplier)
            {
                bulletPosition.x = angleCos * positionMultiplier + currentLaser->pos.x;
                bulletPosition.y = angleSin * positionMultiplier + currentLaser->pos.y;
                bulletPosition.z = 0.0f;
                enemy->bulletProps.position = bulletPosition;
                g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
                positionMultiplier += 48.0f;
            }

            enemy->currentContext.int3++;
        }
    }
}

#pragma var_order(unusedShooter, totalIterations, i, innerBullet, enemyAngle, distance, currentBullet, bulletsAngle, j)
void ExInsStageXFunc15(Enemy *enemy, EclRawInstr *curInstr)
{
    f32 bulletsAngle;
    Bullet *currentBullet;
    f32 distance;
    f32 enemyAngle;
    i32 i;
    Bullet *innerBullet;
    i32 j;
    i32 totalIterations;

    totalIterations = 0;
    currentBullet = g_BulletManager.bullets;
    EnemyShooter unusedShooter;

    for (i = 0; i < MAX_ENEMY_BULLETS; i++, currentBullet++)
    {
        if (currentBullet->state == BULLET_STATE_INACTIVE || currentBullet->state == BULLET_STATE_DESPAWNING)
        {
            continue;
        }

        if (currentBullet->sprites.spriteBullet.sprite != NULL &&
            currentBullet->sprites.spriteBullet.sprite->heightPx >= BULLET_SIZE_MEDIUM)
        {
            totalIterations++;
            enemyAngle = atan2f(currentBullet->pos.y - enemy->position.y, currentBullet->pos.x - enemy->position.x);

            for (j = 0, innerBullet = g_BulletManager.bullets; j < MAX_ENEMY_BULLETS; j++, innerBullet++)
            {
                if (innerBullet->state == BULLET_STATE_INACTIVE || innerBullet->state == BULLET_STATE_DESPAWNING)
                {
                    continue;
                }

                if (innerBullet->sprites.spriteBullet.sprite != NULL &&
                    innerBullet->sprites.spriteBullet.sprite->heightPx < BULLET_SIZE_MEDIUM &&
                    innerBullet->speed == 0.0f &&
                    (distance = sqrtf(ZUN_SQUARE(innerBullet->pos.x - currentBullet->pos.x) +
                                      ZUN_SQUARE(innerBullet->pos.y - currentBullet->pos.y))) < 64.0f)
                {
                    innerBullet->exFlags |= EX_ACCELERATION;
                    innerBullet->speed = 0.01f;
                    innerBullet->timer = 0;
                    innerBullet->exDuration = 120;
                    bulletsAngle =
                        atan2f(innerBullet->pos.y - enemy->position.y, innerBullet->pos.x - enemy->position.x);
                    innerBullet->angle = (bulletsAngle - enemyAngle) * 2.2f + enemyAngle;
                    sincosmul(&innerBullet->ex4Acceleration, innerBullet->angle, 0.01f);
                    innerBullet->color += 1;
                    g_AnmManager->SetActiveSprite(&innerBullet->sprites.spriteBullet,
                                                  innerBullet->sprites.spriteBullet.baseSpriteIndex +
                                                      innerBullet->color);
                }
            }
        }
    }

    ExInsHandleBatTransformation(enemy, curInstr);
    enemy->currentContext.int3 = totalIterations;
}

#define RAGE_TIME_THRESHOLD 7200

void ExInsFlandreFinalContextUpdate(Enemy *enemy, EclRawInstr *curInstr)
{
    i32 remainingLife = enemy->life;
    if (enemy->phaseTimer >= RAGE_TIME_THRESHOLD)
    {
        remainingLife = 0;
    }

    if (GET_INT_ARG(1) == 0)
    {
        enemy->currentContext.float3 = 2.0f - (f32)((remainingLife * 1.0f) / 6000.0f);
        enemy->currentContext.counter1 = (remainingLife * 240) / 6000 + 40;
    }
    else
    {
        float rangeModifier = 320.0f - (remainingLife * 160.0f) / 6000.0f;
        enemy->currentContext.float2 = g_Rng.GetRandomF32InRange(rangeModifier) + (192.0f - rangeModifier / 2.0f);
        rangeModifier = 128.0f - (remainingLife * 64.0f) / 6000.0f;
        enemy->currentContext.float3 = g_Rng.GetRandomF32InRange(rangeModifier) + (96.0f - rangeModifier / 2.0f);
    }
}
#endif
} // namespace th06
