#include "stdafx.h"

#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "ItemManager.hpp"

namespace th06
{
D3DCOLOR g_EffectsColorWithTextureBlending[] = {
    0xff000000, 0xff303030, 0xff606060, 0xff500000, 0xff900000, 0xffff2020, 0xff400040,
    0xff800080, 0xffff30ff, 0xff000050, 0xff000090, 0xff2020ff, 0xff203060, 0xff304090,
    0xff3080ff, 0xff005000, 0xff009000, 0xff20ff20, 0xff206000, 0xff409010, 0xff80ff20,
    0xff505000, 0xff909000, 0xffffff20, 0xff603000, 0xff904010, 0xfff08020, 0xffffffff,
};

D3DCOLOR g_EffectsColorWithoutTextureBlending[] = {
    0xfff0f0f0, 0xfff0f0f0, 0xffffffff, 0xffffe0e0, 0xffffe0e0, 0xffffe0e0, 0xffffe0ff,
    0xffffe0ff, 0xffffe0ff, 0xffe0e0ff, 0xffe0e0ff, 0xffe0e0ff, 0xffe0ffff, 0xffe0ffff,
    0xffe0ffff, 0xffe0ffe0, 0xffe0ffe0, 0xffe0ffe0, 0xffe0ffe0, 0xffe0ffe0, 0xffe0ffe0,
    0xffffffe0, 0xffffffe0, 0xffffffe0, 0xffffe0e0, 0xffffe0e0, 0xffffe0e0, 0xffffffff,
};
D3DCOLOR *g_EffectsColor = g_EffectsColorWithTextureBlending;

enum SpawnEffectColors
{
    SPAWN_EFFECT_WHITE,
    SPAWN_EFFECT_RED,
    SPAWN_EFFECT_BLUE,
    SPAWN_EFFECT_GREEN,
    SPAWN_EFFECT_YELLOW
};

SpawnEffectColors g_BulletSpawnEffects16Colors[] = {
    SPAWN_EFFECT_WHITE,  // BULLET_GRAY
    SPAWN_EFFECT_RED,    // BULLET_DARK_RED
    SPAWN_EFFECT_RED,    // BULLET_RED
    SPAWN_EFFECT_RED,    // BULLET_DARK_PURPLE
    SPAWN_EFFECT_RED,    // BULLET_PURPLE
    SPAWN_EFFECT_BLUE,   // BULLET_DARK_BLUE
    SPAWN_EFFECT_BLUE,   // BULLET_BLUE
    SPAWN_EFFECT_BLUE,   // BULLET_DARK_CYAN
    SPAWN_EFFECT_BLUE,   // BULLET_CYAN
    SPAWN_EFFECT_GREEN,  // BULLET_DARK_GREEN
    SPAWN_EFFECT_GREEN,  // BULLET_GREEN
    SPAWN_EFFECT_GREEN,  // BULLET_LIME
    SPAWN_EFFECT_YELLOW, // BULLET_DARK_YELLOW
    SPAWN_EFFECT_YELLOW, // BULLET_YELLOW
    SPAWN_EFFECT_YELLOW, // BULLET_ORANGE
    SPAWN_EFFECT_WHITE,  // BULLET_WHITE
};
SpawnEffectColors g_BulletSpawnEffects8Colors[] = {
    SPAWN_EFFECT_WHITE,  // BULLET_GRAY8
    SPAWN_EFFECT_RED,    // BULLET_RED8
    SPAWN_EFFECT_RED,    // BULLET_PURPLE8
    SPAWN_EFFECT_BLUE,   // BULLET_BLUE8
    SPAWN_EFFECT_BLUE,   // BULLET_CYAN8
    SPAWN_EFFECT_GREEN,  // BULLET_GREEN8
    SPAWN_EFFECT_YELLOW, // BULLET_YELLOW8
    SPAWN_EFFECT_WHITE,  // BULLET_WHITE8
};

ChainElem g_BulletManagerCalcChainElem;
ChainElem g_BulletManagerDrawChainElem;
BulletManager g_BulletManager;

struct BulletSpriteInfo
{
    u32 bullet;
    u32 spawnEffectFast;
    u32 spawnEffectNormal;
    u32 spawnEffectSlow;
    u32 despawnEffect;
};

// clang-format off
#define BULLET_EFFECT_SCRIPTS(fast, normal, slow, despawn)                                                             \
    MACRO_CAT(ANM_SCRIPT_BULLET_SPAWN_, fast),                                                                         \
    MACRO_CAT(ANM_SCRIPT_BULLET_SPAWN_, normal),                                                                       \
    MACRO_CAT(ANM_SCRIPT_BULLET_SPAWN_, slow),                                                                         \
    MACRO_CAT(ANM_SCRIPT_BULLET_DESPAWN_, despawn)

#define BULLET_SCRIPTS_TINY(bullet)                                                                                    \
    MACRO_CAT(ANM_SCRIPT_, bullet), BULLET_EFFECT_SCRIPTS(TINY_FAST, TINY, TINY_SLOW, TINY)

#define BULLET_SCRIPTS_SMALL(bullet)                                                                                   \
    MACRO_CAT(ANM_SCRIPT_, bullet), BULLET_EFFECT_SCRIPTS(SMALL_FAST, SMALL, SMALL_SLOW, SMALL)

#define BULLET_SCRIPTS_MEDIUM(bullet)                                                                                  \
    MACRO_CAT(ANM_SCRIPT_, bullet), BULLET_EFFECT_SCRIPTS(LARGE, LARGE, LARGE, LARGE)

#define BULLET_SCRIPTS_LARGE(bullet)                                                                                   \
    MACRO_CAT(ANM_SCRIPT_, bullet), BULLET_EFFECT_SCRIPTS(LARGE, LARGE, LARGE, LARGE)

#define BULLET_SCRIPTS_HUGE(bullet)                                                                                    \
    MACRO_CAT(ANM_SCRIPT_, bullet), BULLET_EFFECT_SCRIPTS(HUGE, HUGE, HUGE, HUGE)

const BulletSpriteInfo g_BulletAnmScripts[] = {
    {BULLET_SCRIPTS_TINY(BULLET_PELLET)},
    {BULLET_SCRIPTS_SMALL(BULLET_RING_BALL)},
    {BULLET_SCRIPTS_SMALL(BULLET_RICE)},
    {BULLET_SCRIPTS_SMALL(BULLET_BALL)},
    {BULLET_SCRIPTS_SMALL(BULLET_KUNAI)},
    {BULLET_SCRIPTS_SMALL(BULLET_SHARD)},
    {BULLET_SCRIPTS_LARGE(BULLET_BIG_BALL)},
    {BULLET_SCRIPTS_MEDIUM(BULLET_FIREBALL)},
    {BULLET_SCRIPTS_LARGE(BULLET_DAGGER)},
    {BULLET_SCRIPTS_HUGE(BULLET_BUBBLE)},
};
// clang-format on

void BulletManager::InitializeToZero()
{
    memset(this, 0, sizeof(BulletManager));
}

BulletManager::BulletManager()
{
    this->InitializeToZero();
}

#pragma var_order(bulletSpeed, i, bullet, bulletAngle)
u32 BulletManager::SpawnSingleBullet(EnemyShooter *bulletProps, i32 bulletIdx1, i32 bulletIdx2, f32 angle)
{
    i32 i = 0;
    Bullet *bullet = &this->bullets[this->nextBulletIndex];
    for (i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        this->nextBulletIndex++;

        if (this->nextBulletIndex >= MAX_ENEMY_BULLETS)
        {
            this->nextBulletIndex = 0;
        }

        if (bullet->state != BULLET_STATE_INACTIVE)
        {
            bullet++;
            if (this->nextBulletIndex == 0)
            {
                bullet = &this->bullets[0];
            }
            continue;
        }

        break;
    }

    if (i >= MAX_ENEMY_BULLETS)
    {
        return 1;
    }

    f32 bulletAngle = 0.0f;
    f32 bulletSpeed =
        bulletProps->speed1 - (bulletProps->speed1 - bulletProps->speed2) * bulletIdx2 / bulletProps->count2;
    switch (bulletProps->aimMode)
    {
    case BULLET_FAN_AIMED:
    case BULLET_FAN:
        if (bulletProps->count1 & 1)
        {
            bulletAngle += ((bulletIdx1 + 1) / 2) * bulletProps->angle2;
        }
        else
        {
            bulletAngle += (bulletIdx1 / 2) * bulletProps->angle2 + bulletProps->angle2 * 0.5f;
        }

        if (bulletIdx1 & 1)
        {
            bulletAngle *= -1.0f;
        }

        if (bulletProps->aimMode == BULLET_FAN_AIMED)
        {
            bulletAngle += angle;
        }

        bulletAngle += bulletProps->angle1;
        break;
    case BULLET_CIRCLE_AIMED:
        bulletAngle += angle;
        // fallthrough
    case BULLET_CIRCLE:
        bulletAngle += bulletIdx1 * ZUN_2PI / bulletProps->count1;
        bulletAngle += bulletIdx2 * bulletProps->angle2 + bulletProps->angle1;
        break;
    case BULLET_OFFSET_CIRCLE_AIMED:
        bulletAngle += angle;
        // fallthrough
    case BULLET_OFFSET_CIRCLE:
        bulletAngle += ZUN_PI / bulletProps->count1;
        bulletAngle += bulletIdx1 * ZUN_2PI / bulletProps->count1;
        bulletAngle += bulletProps->angle1;
        break;
    case BULLET_RANDOM_ANGLE:
        bulletAngle = g_Rng.GetRandomF32InRange(bulletProps->angle1 - bulletProps->angle2) + bulletProps->angle2;
        break;
    case BULLET_RANDOM_SPEED:
        bulletSpeed = g_Rng.GetRandomF32InRange(bulletProps->speed1 - bulletProps->speed2) + bulletProps->speed2;
        bulletAngle += bulletIdx1 * ZUN_2PI / bulletProps->count1;
        bulletAngle += bulletIdx2 * bulletProps->angle2 + bulletProps->angle1;
        break;
    case BULLET_RANDOM:
        bulletAngle = g_Rng.GetRandomF32InRange(bulletProps->angle1 - bulletProps->angle2) + bulletProps->angle2;
        bulletSpeed = g_Rng.GetRandomF32InRange(bulletProps->speed1 - bulletProps->speed2) + bulletProps->speed2;
    }

    bullet->state = BULLET_STATE_FIRED;
    bullet->unk_5c2 = 1;
    bullet->speed = bulletSpeed;
    bullet->angle = ReduceAngle(bulletAngle);
    bullet->pos = bulletProps->position;
    bullet->pos[2] = 0.1f;
#if BUILT_SINCE(013)
    sincosmul(&bullet->velocity, bullet->angle, bulletSpeed);
#else
    // Builds before 2002-08-25 (1.00 to 1.02c) aim the velocity with the angle before it is normalized.
    sincosmul(&bullet->velocity, bulletAngle, bulletSpeed);
#endif
    bullet->exFlags = bulletProps->flags;
    bullet->color = bulletProps->color;
    bullet->vm = this->bulletTemplates[bulletProps->sprite].vm;
    bullet->despawnEffect = this->bulletTemplates[bulletProps->sprite].despawnEffect;
    bullet->grazeSize = this->bulletTemplates[bulletProps->sprite].grazeSize;
    bullet->unk_55c = this->bulletTemplates[bulletProps->sprite].unk_55c;
    bullet->height = this->bulletTemplates[bulletProps->sprite].height;

    if (bullet->exFlags & EX_SPAWN_EFFECT_SHORT)
    {
        bullet->spawnEffectFast = this->bulletTemplates[bulletProps->sprite].spawnEffectFast;

        if (bullet->vm.sprite->height <= BULLET_SIZE_SMALL)
        {
            g_AnmManager->SetSprite(&bullet->spawnEffectFast, bullet->spawnEffectFast.spriteId +
                                                                  g_BulletSpawnEffects16Colors[bulletProps->color]);
        }
        else if (bullet->vm.sprite->height <= BULLET_SIZE_LARGE)
        {
            if (bullet->vm.scriptId != ANM_SCRIPT_BULLET_FIREBALL)
            {
                g_AnmManager->SetSprite(&bullet->spawnEffectFast, bullet->spawnEffectFast.spriteId +
                                                                      g_BulletSpawnEffects8Colors[bulletProps->color]);
            }
            else
            {
                g_AnmManager->SetSprite(&bullet->spawnEffectFast, bullet->spawnEffectFast.spriteId + SPAWN_EFFECT_RED);
            }
        }
        else // BULLET_SIZE_HUGE
        {
            g_AnmManager->SetSprite(&bullet->spawnEffectFast, bullet->spawnEffectFast.spriteId + bulletProps->color);
        }

        bullet->state = BULLET_STATE_SPAWNING_FAST;
    }
    else if (bullet->exFlags & EX_SPAWN_EFFECT)
    {
        bullet->spawnEffectNormal = this->bulletTemplates[bulletProps->sprite].spawnEffectNormal;

        if (bullet->vm.sprite->height <= BULLET_SIZE_SMALL)
        {
            g_AnmManager->SetSprite(&bullet->spawnEffectNormal, bullet->spawnEffectNormal.spriteId +
                                                                    g_BulletSpawnEffects16Colors[bulletProps->color]);
        }
        else if (bullet->vm.sprite->height <= BULLET_SIZE_LARGE)
        {
            if (bullet->vm.scriptId != ANM_SCRIPT_BULLET_FIREBALL)
            {
                g_AnmManager->SetSprite(&bullet->spawnEffectNormal,
                                        bullet->spawnEffectNormal.spriteId +
                                            g_BulletSpawnEffects8Colors[bulletProps->color]);
            }
            else
            {
                g_AnmManager->SetSprite(&bullet->spawnEffectNormal,
                                        bullet->spawnEffectNormal.spriteId + SPAWN_EFFECT_RED);
            }
        }
        else // BULLET_SIZE_HUGE
        {
            g_AnmManager->SetSprite(&bullet->spawnEffectNormal,
                                    bullet->spawnEffectNormal.spriteId + bulletProps->color);
        }
        bullet->state = BULLET_STATE_SPAWNING_NORMAL;
    }
    else if (bullet->exFlags & EX_SPAWN_EFFECT_LONG)
    {
        bullet->spawnEffectSlow = this->bulletTemplates[bulletProps->sprite].spawnEffectSlow;
        if (bullet->vm.sprite->height <= BULLET_SIZE_SMALL)
        {
            g_AnmManager->SetSprite(&bullet->spawnEffectSlow, bullet->spawnEffectSlow.spriteId +
                                                                  g_BulletSpawnEffects16Colors[bulletProps->color]);
        }
        else if (bullet->vm.sprite->height <= BULLET_SIZE_LARGE)
        {
            if (bullet->vm.scriptId != ANM_SCRIPT_BULLET_FIREBALL)
            {
                g_AnmManager->SetSprite(&bullet->spawnEffectSlow, bullet->spawnEffectSlow.spriteId +
                                                                      g_BulletSpawnEffects8Colors[bulletProps->color]);
            }
            else
            {
                g_AnmManager->SetSprite(&bullet->spawnEffectSlow, bullet->spawnEffectSlow.spriteId + SPAWN_EFFECT_RED);
            }
        }
        else // BULLET_SIZE_HUGE
        {
            g_AnmManager->SetSprite(&bullet->spawnEffectSlow, bullet->spawnEffectSlow.spriteId + bulletProps->color);
        }

        bullet->state = BULLET_STATE_SPAWNING_SLOW;
    }
    g_AnmManager->SetSprite(&bullet->vm, bullet->vm.spriteId + bulletProps->color);

    if (bullet->vm.sprite->height <= BULLET_SIZE_SMALL)
    {
        g_AnmManager->SetSprite(&bullet->despawnEffect,
                                bullet->despawnEffect.spriteId + g_BulletSpawnEffects16Colors[bulletProps->color]);
    }
    else if (bullet->vm.sprite->height <= BULLET_SIZE_LARGE)
    {
        if (bullet->vm.scriptId != ANM_SCRIPT_BULLET_FIREBALL)
        {
            g_AnmManager->SetSprite(&bullet->despawnEffect,
                                    bullet->despawnEffect.spriteId + g_BulletSpawnEffects8Colors[bulletProps->color]);
        }
        else
        {
            g_AnmManager->SetSprite(&bullet->despawnEffect, bullet->despawnEffect.spriteId + SPAWN_EFFECT_RED);
        }
    }
    else // BULLET_SIZE_HUGE
    {
        g_AnmManager->SetSprite(&bullet->despawnEffect, bullet->despawnEffect.spriteId + bulletProps->color);
    }

    if (bullet->exFlags & EX_ACCELERATION)
    {
        if (bulletProps->exFloats[1] <= -999.0f)
        {
            sincosmul(&bullet->exAcceleration, bulletAngle, bulletProps->exFloats[0]);
        }
        else
        {
            sincosmul(&bullet->exAcceleration, bulletProps->exFloats[1], bulletProps->exFloats[0]);
        }

        if (bulletProps->exInts[0] > 0)
        {
            bullet->exDuration = bulletProps->exInts[0];
        }
        else
        {
            bullet->exDuration = 99999;
        }

        bullet->exAcceleration.z = 0.0f;
    }
    else if (bullet->exFlags & EX_VELOCITY)
    {
        bullet->exVelocity = bulletProps->exFloats[0];
        bullet->exAngularVelocity = bulletProps->exFloats[1];
        bullet->exDuration = bulletProps->exInts[0];
    }

    if (bullet->exFlags & EX_ANGLE)
    {
        bullet->exAngle = bulletProps->exFloats[0];

        if (bulletProps->exFloats[1] >= 0.0f)
        {
            bullet->exSpeed = bulletProps->exFloats[1];
        }
        else
        {
            bullet->exSpeed = bulletSpeed;
        }

        bullet->exInterval = bulletProps->exInts[0];
        bullet->exCountMax = bulletProps->exInts[1];
        bullet->exCount = 0;
    }

    if (bullet->exFlags & EX_BOUNCE)
    {
        if (bulletProps->exFloats[0] >= 0.0f)
        {
            bullet->exSpeed = bulletProps->exFloats[0];
        }
        else
        {
            bullet->exSpeed = bulletSpeed;
        }

        bullet->exCountMax = bulletProps->exInts[0];
        bullet->exCount = 0;
    }
    return 0;
}

#pragma var_order(itemPos, i, sine, bullet, laser, cosine)
void BulletManager::DespawnBullets(ZunBool spawnCancelItems)
{
    i32 i;
    f32 sine;
    f32 cosine;
    D3DXVECTOR3 itemPos;

    Bullet *bullet = &g_BulletManager.bullets[0];
    for (i = 0; i < MAX_ENEMY_BULLETS; i++, bullet++)
    {
        if (bullet->state == BULLET_STATE_INACTIVE || bullet->state == BULLET_STATE_DESPAWNING)
        {
            continue;
        }

        if (spawnCancelItems)
        {
            g_ItemManager.SpawnItem(&bullet->pos, ITEM_STAR, ITEM_STATE_MAGNETED);
            memset(bullet, 0, sizeof(Bullet));
        }
        else
        {
            bullet->state = BULLET_STATE_DESPAWNING;
        }
    }

    Laser *laser = this->lasers;
    for (i = 0; i < MAX_ENEMY_LASERS; i++, laser++)
    {
        if (!laser->inUse)
        {
            continue;
        }

        if (laser->state < LASER_STATE_DESPAWNING)
        {
            laser->state = LASER_STATE_DESPAWNING;
            laser->timer = 0;

            if (spawnCancelItems)
            {
                float offset = laser->startOffset;
                fsincos_wrapper(&sine, &cosine, laser->angle);

                while (laser->endOffset > offset)
                {
                    itemPos.x = cosine * offset + laser->pos.x;
                    itemPos.y = sine * offset + laser->pos.y;
                    itemPos.z = 0.0f;
                    g_ItemManager.SpawnItem(&itemPos, ITEM_STAR, ITEM_STATE_MAGNETED);
                    offset += 32.0f;
                }
            }
        }

        laser->hitboxEndDelay = 0;
    }
}

void BulletManager::CancelAllBullets()
{
    this->DespawnBullets(BULLET_CANCEL);
}

#pragma var_order(bulletScore, totalBonusScore, awardedBullets, i, sine, bullet, itemPos, laser, cosine)
i32 BulletManager::DespawnBulletsForScore(i32 maxBonusScore, ZunBool spawnCancelItems)
{
    i32 i;
    f32 sine;
    f32 cosine;
    D3DXVECTOR3 itemPos;

    i32 totalBonusScore = 0;
    i32 bulletScore = 2000;
    i32 awardedBullets = 0;

    Bullet *bullet = &g_BulletManager.bullets[0];
    for (i = 0; i < MAX_ENEMY_BULLETS; i++, bullet++)
    {
        if (bullet->state == BULLET_STATE_INACTIVE)
        {
            continue;
        }

        if (spawnCancelItems)
        {
            g_ItemManager.SpawnItem(&bullet->pos, ITEM_STAR, ITEM_STATE_MAGNETED);
        }

        g_AsciiManager.CreateScorePopup(&bullet->pos, bulletScore,
                                        bulletScore >= maxBonusScore ? COLOR_YELLOW : COLOR_WHITE);

        totalBonusScore += bulletScore;
        awardedBullets++;
        bulletScore += 10;

        if (bulletScore > maxBonusScore)
        {
            bulletScore = maxBonusScore;
        }

        bullet->state = BULLET_STATE_DESPAWNING;
    }

    Laser *laser = &this->lasers[0];
    for (i = 0; i < MAX_ENEMY_LASERS; i++, laser++)
    {
        if (!laser->inUse)
        {
            continue;
        }

        if (laser->state < LASER_STATE_DESPAWNING)
        {
            laser->state = LASER_STATE_DESPAWNING;
            laser->timer = 0;

            if (spawnCancelItems)
            {
#if BUILT_SINCE(101)
                g_ItemManager.SpawnItem(&laser->pos, ITEM_STAR, ITEM_STATE_MAGNETED);
#else
                // 1.00 bug: the laser's item spawns at the last bullet visited by the loop above.
                g_ItemManager.SpawnItem(&bullet->pos, ITEM_STAR, ITEM_STATE_MAGNETED);
#endif
                float offset = laser->startOffset;
                fsincos_wrapper(&sine, &cosine, laser->angle);

                while (laser->endOffset > offset)
                {
                    itemPos.x = cosine * offset + laser->pos.x;
                    itemPos.y = sine * offset + laser->pos.y;
                    itemPos.z = 0.0f;
                    g_ItemManager.SpawnItem(&itemPos, ITEM_STAR, ITEM_STATE_MAGNETED);
                    offset += 32.0f;
                }
            }
        }

        laser->hitboxEndDelay = 0;
    }

    g_GameManager.AddScore(totalBonusScore);

    if (totalBonusScore != 0)
    {
        g_Gui.ShowBonusScore(totalBonusScore);
    }

    return totalBonusScore;
}

ZunResult BulletManager::SpawnBulletPattern(EnemyShooter *bulletProps)
{
    i32 idx1, idx2;

    f32 angle = g_Player.AngleToPlayer(&bulletProps->position);
    for (idx1 = 0; idx1 < bulletProps->count2; idx1++)
    {
        for (idx2 = 0; idx2 < bulletProps->count1; idx2++)
        {
            if (this->SpawnSingleBullet(bulletProps, idx2, idx1, angle) != 0)
            {
                goto out;
            }
        }
    }

out:
    if (bulletProps->flags & EX_SPAWN_SOUND)
    {
        g_SoundPlayer.PlaySoundId(bulletProps->shootSfx);
    }
    return ZUN_SUCCESS;
}

#pragma var_order(i, laser)
Laser *BulletManager::SpawnLaserPattern(EnemyShooter *bulletProps)
{
    i32 i;

    Laser *laser = this->lasers;
    for (i = 0; i < MAX_ENEMY_LASERS; i++, laser++)
    {
        if (laser->inUse)
        {
            continue;
        }

        g_AnmManager->SetScript(&laser->vm, bulletProps->sprite + ANM_SCRIPT_BULLET_LINE_LASER);
        g_AnmManager->SetSprite(&laser->vm, laser->vm.spriteId + bulletProps->color);

        g_AnmManager->SetNewSprite(&laser->baseGlowVm, g_BulletSpawnEffects16Colors[bulletProps->color] +
                                                           ANM_SPRITE_BULLET3_SPAWN_BIG_BALL);

        laser->baseGlowVm.flags.blendMode = AnmBlendMode_Additive;
        laser->pos = bulletProps->position;
        laser->color = bulletProps->color;
        laser->inUse = true;
        laser->angle = bulletProps->angle1;

        if (bulletProps->aimMode == LASER_AIMED)
        {
            laser->angle += g_Player.AngleToPlayer(&bulletProps->position);
        }

        laser->flags = bulletProps->flags;
        laser->timer = 0;
        laser->startOffset = bulletProps->startOffset;
        laser->endOffset = bulletProps->endOffset;
        laser->length = bulletProps->length;
        laser->width = bulletProps->width;
        laser->speed = bulletProps->speed1;
        laser->startTime = bulletProps->startTime;
        laser->duration = bulletProps->duration;
        laser->despawnDuration = bulletProps->despawnDuration;
        laser->hitboxStartTime = bulletProps->hitboxStartTime;
        laser->hitboxEndDelay = bulletProps->hitboxEndDelay;

        if (laser->startTime == 0)
        {
            laser->state = LASER_STATE_ACTIVE;
        }
        else
        {
            laser->state = LASER_STATE_START_DELAY;
        }
        break;
    }
    return laser;
}

#pragma var_order(checkResult, i, bulletSpeed, size, laserSize, curBullet, laserAlpha, curLaser, laserCenter)
static ChainCallbackResult BulletManager_OnUpdate(BulletManager *mgr)
{
    f32 bulletSpeed;
    i32 i;
    i32 checkResult;

    Bullet *curBullet = &mgr->bullets[0];

    if (g_GameManager.isTimeStopped)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    g_ItemManager.OnUpdate();

#if BUILT_SINCE(100)
    mgr->bulletCount = 0;
#endif

    for (i = 0; i < MAX_ENEMY_BULLETS; i++, curBullet++)
    {
        if (curBullet->state == BULLET_STATE_INACTIVE)
            continue;

#if BUILT_SINCE(100)
        mgr->bulletCount++;
#endif
        switch (curBullet->state)
        {
        case BULLET_STATE_SPAWNING_FAST:
            curBullet->pos += curBullet->velocity / 2.0f * g_Supervisor.gameSpeed;

            if (g_AnmManager->RunAnm(&curBullet->spawnEffectFast) == SCRIPT_RUNNING)
            {
                break;
            }
            goto spawn_animation_done;
        case BULLET_STATE_SPAWNING_NORMAL:
            curBullet->pos += curBullet->velocity / 2.5f * g_Supervisor.gameSpeed;

            if (g_AnmManager->RunAnm(&curBullet->spawnEffectNormal) == SCRIPT_RUNNING)
            {
                break;
            }
            goto spawn_animation_done;
        case BULLET_STATE_SPAWNING_SLOW:
            curBullet->pos += curBullet->velocity / 3.0f * g_Supervisor.gameSpeed;

            if (g_AnmManager->RunAnm(&curBullet->spawnEffectSlow) == SCRIPT_RUNNING)
            {
                break;
            }
        spawn_animation_done:
            curBullet->state = BULLET_STATE_FIRED;
            curBullet->timer = 0;
            // fallthrough
        case BULLET_STATE_FIRED:
            if (curBullet->exFlags != 0)
            {
                if (curBullet->exFlags & EX_SPEEDUP)
                {
                    if (curBullet->timer <= 16)
                    {
                        bulletSpeed = RAMP_DOWN(5.0f, (f32)curBullet->timer, 16.0f);
                        sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed + curBullet->speed);
                    }
                    else
                    {
                        curBullet->exFlags ^= EX_SPEEDUP;
                    }
                }
                else if (curBullet->exFlags & EX_ACCELERATION)
                {
                    if (curBullet->timer >= curBullet->exDuration)
                    {
                        curBullet->exFlags &= ~EX_ACCELERATION;
                    }
                    else
                    {
                        curBullet->velocity += curBullet->exAcceleration * g_Supervisor.gameSpeed;
                        curBullet->angle = atan2f(curBullet->velocity.y, curBullet->velocity.x);
                    }
                }
                else if (curBullet->exFlags & EX_VELOCITY)
                {
                    if (curBullet->timer >= curBullet->exDuration)
                    {
                        curBullet->exFlags &= ~EX_VELOCITY;
                    }
                    else
                    {
                        curBullet->angle =
                            ReduceAngleAdd(curBullet->angle, curBullet->exAngularVelocity * g_Supervisor.gameSpeed);
                        curBullet->speed += curBullet->exVelocity * g_Supervisor.gameSpeed;
                        sincosmul(&curBullet->velocity, curBullet->angle, curBullet->speed);
                    }
                }
                if (curBullet->exFlags & EX_ANGLE_ADD)
                {
                    if (curBullet->timer >= curBullet->exInterval * (curBullet->exCount + 1))
                    {
                        if (++curBullet->exCount >= curBullet->exCountMax)
                        {
                            curBullet->exFlags &= ~EX_ANGLE_ADD;
                        }

                        curBullet->angle = curBullet->angle + curBullet->exAngle;
                        curBullet->speed = curBullet->exSpeed;
                        bulletSpeed = curBullet->speed;
                    }
                    else
                    {
                        bulletSpeed = RAMP_DOWN(curBullet->speed,
                                                (f32)curBullet->timer - (curBullet->exInterval * curBullet->exCount),
                                                curBullet->exInterval);
                    }

                    sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed);
                }
                else if (curBullet->exFlags & EX_ANGLE_SET)
                {
                    if (curBullet->timer >= curBullet->exInterval * (curBullet->exCount + 1))
                    {
                        if (++curBullet->exCount >= curBullet->exCountMax)
                        {
                            curBullet->exFlags &= ~EX_ANGLE_SET;
                        }

                        curBullet->angle = curBullet->exAngle;
                        curBullet->speed = curBullet->exSpeed;
                        bulletSpeed = curBullet->speed;
                    }
                    else
                    {
                        bulletSpeed = RAMP_DOWN(curBullet->speed,
                                                (f32)curBullet->timer - (curBullet->exInterval * curBullet->exCount),
                                                curBullet->exInterval);
                    }

                    sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed);
                }
                else if (curBullet->exFlags & EX_ANGLE_PLAYER)
                {
                    if (curBullet->timer >= curBullet->exInterval * (curBullet->exCount + 1))
                    {
                        if (++curBullet->exCount >= curBullet->exCountMax)
                        {
                            curBullet->exFlags &= ~EX_ANGLE_PLAYER;
                        }

                        curBullet->angle = g_Player.AngleToPlayer(&curBullet->pos) + curBullet->exAngle;
                        curBullet->speed = curBullet->exSpeed;
                        bulletSpeed = curBullet->speed;
                    }
                    else
                    {
                        bulletSpeed = RAMP_DOWN(curBullet->speed,
                                                (f32)curBullet->timer - (curBullet->exInterval * curBullet->exCount),
                                                curBullet->exInterval);
                    }
                    sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed);
                }
                else if (curBullet->exFlags & EX_BOUNCE_TBLR)
                {
                    if (!g_GameManager.IsInBounds(curBullet->pos[0], curBullet->pos[1], curBullet->vm.sprite->width,
                                                  curBullet->vm.sprite->height))
                    {
                        if (curBullet->pos.x < GAME_REGION_LEFT || curBullet->pos.x >= GAME_REGION_RIGHT)
                        {
                            curBullet->angle = -curBullet->angle - ZUN_PI;
                            curBullet->angle = ReduceAngle(curBullet->angle);
                        }

                        if (curBullet->pos.y < GAME_REGION_TOP || curBullet->pos.y >= GAME_REGION_BOTTOM)
                        {
                            curBullet->angle = -curBullet->angle;
                        }

                        curBullet->speed = curBullet->exSpeed;
                        bulletSpeed = curBullet->speed;
                        sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed);

                        if (++curBullet->exCount >= curBullet->exCountMax)
                        {
                            curBullet->exFlags &= ~EX_BOUNCE_TBLR;
                        }
                    }
                }
                else if (curBullet->exFlags & EX_BOUNCE_TLR)
                {
                    if (!g_GameManager.IsInBounds(curBullet->pos[0], curBullet->pos[1], curBullet->vm.sprite->width,
                                                  curBullet->vm.sprite->height))
                    {
                        if (curBullet->pos.x < GAME_REGION_LEFT || curBullet->pos.x >= GAME_REGION_RIGHT)
                        {
                            curBullet->angle = -curBullet->angle - ZUN_PI;
                            curBullet->angle = ReduceAngle(curBullet->angle);
                        }

                        if (curBullet->pos.y < GAME_REGION_TOP)
                        {
                            curBullet->angle = -curBullet->angle;
                        }

                        curBullet->speed = curBullet->exSpeed;
                        bulletSpeed = curBullet->speed;
                        sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed);

                        if (++curBullet->exCount >= curBullet->exCountMax)
                        {
                            curBullet->exFlags &= ~EX_BOUNCE_TLR;
                        }
                    }
                }
            }

            curBullet->pos += curBullet->velocity * g_Supervisor.gameSpeed;
            if (!g_GameManager.IsInBounds(curBullet->pos[0], curBullet->pos[1], curBullet->vm.sprite->width,
                                          curBullet->vm.sprite->height))
            {
                // If any of these ex are set the bullet could *maybe* come back inbounds,
                // so give it a bit of leniency for being offscreen.
                // BUG: EX_ACCELERATION and EX_VELOCITY should have been included
                if (!(curBullet->exFlags & EX_ANGLE_ADD) && !(curBullet->exFlags & EX_ANGLE_SET) &&
                    !(curBullet->exFlags & EX_ANGLE_PLAYER) && !(curBullet->exFlags & EX_BOUNCE_TBLR) &&
                    !(curBullet->exFlags & EX_BOUNCE_TLR) && curBullet->outOfBoundsTime == 0)
                {
                    memset(curBullet, 0, sizeof(Bullet));
                    continue;
                }
                else if (++curBullet->outOfBoundsTime >= 256)
                {
                    memset(curBullet, 0, sizeof(Bullet));
                    continue;
                }
            }
            else
            {
                curBullet->outOfBoundsTime = 0;
            }

            if (curBullet->isGrazed == FALSE)
            {
                checkResult = g_Player.CheckGraze(&curBullet->pos, &curBullet->grazeSize);

                if (checkResult == GRAZE_TRUE)
                {
                    curBullet->isGrazed = TRUE;
                    goto bulletGrazed;
                }
                else if (checkResult == CANCELED_BY_BOMB)
                {
                    curBullet->state = BULLET_STATE_DESPAWNING;
                    g_ItemManager.SpawnItem(&curBullet->pos, ITEM_STAR, ITEM_STATE_MAGNETED);
                }
            }
            else if (curBullet->isGrazed == TRUE)
            {
            bulletGrazed:
                checkResult = g_Player.CheckCollision(&curBullet->pos, &curBullet->grazeSize);
                if (checkResult != COLLISION_NONE)
                {
                    curBullet->state = BULLET_STATE_DESPAWNING;
                    if (checkResult == COLLISION_GRAZE)
                    {
                        g_ItemManager.SpawnItem(&curBullet->pos, ITEM_STAR, ITEM_STATE_MAGNETED);
                    }
                }
            }
            g_AnmManager->RunAnm(&curBullet->vm);
            break;
        case BULLET_STATE_DESPAWNING:
            curBullet->pos += curBullet->velocity / 2.0f * g_Supervisor.gameSpeed;
            if (g_AnmManager->RunAnm(&curBullet->despawnEffect) != SCRIPT_RUNNING)
            {
                memset(curBullet, 0, sizeof(Bullet));
                continue;
            }
            break;
        }
        curBullet->timer++;
    }

    Laser *curLaser = &mgr->lasers[0];

    D3DXVECTOR3 laserSize;
    i32 laserAlpha;
    D3DXVECTOR3 laserCenter;
    f32 size;

#define LASER_GRAZE_RATE 12

    for (i = 0; i < MAX_ENEMY_LASERS; i++, curLaser++)
    {
        if (!curLaser->inUse)
        {
            continue;
        }

        curLaser->endOffset += curLaser->speed * g_Supervisor.gameSpeed;

        if (curLaser->length < curLaser->endOffset - curLaser->startOffset)
        {
            curLaser->startOffset = curLaser->endOffset - curLaser->length;
        }

        if (curLaser->startOffset < 0.0f)
        {
            curLaser->startOffset = 0.0f;
        }

        laserSize.y = curLaser->width / 2.0f;
        laserSize.x = curLaser->endOffset - curLaser->startOffset;
        laserCenter[0] =
            curLaser->pos.x + (curLaser->startOffset + (curLaser->endOffset - curLaser->startOffset) / 2.0f);
        laserCenter[1] = curLaser->pos.y;
        curLaser->vm.scaleX = curLaser->width / curLaser->vm.sprite->width;
        size = curLaser->endOffset - curLaser->startOffset;
        curLaser->vm.scaleY = size / curLaser->vm.sprite->height;
        curLaser->vm.rotation.z = ZUN_HALF_PI - curLaser->angle;

        switch (curLaser->state)
        {
        case LASER_STATE_START_DELAY:
            if (curLaser->flags & LASER_FLAG_FADE_IN_OUT)
            {
                laserAlpha = RAMP_UP(255.0f, (f32)curLaser->timer, curLaser->startTime);

                if (laserAlpha > 255)
                {
                    laserAlpha = 255;
                }

                // BUG: This is meant to only set the alpha value,
                // but overwrites the entire color with black.
                // The fading flag is unused, so this doesn't matter.
                curLaser->vm.color = laserAlpha << COLOR_ALPHA_SHIFT;
            }
            else
            {
                i32 res = ZUN_MIN(curLaser->startTime, 30);
                if (curLaser->startTime - res < (i32)curLaser->timer)
                {
                    size = RAMP_UP(curLaser->width, (f32)curLaser->timer, curLaser->startTime);
                }
                else
                {
                    size = 1.2f;
                }

                curLaser->vm.scaleX = size / 16.0f;
                // Bug: ZUN intended to set laserSize.y instead of laserSize.x
                // This way, between hitboxStartTime and startTime, the laser would have a thinner hitbox.
                // Setting laserSize.x results in a tiny hitbox at the laser midpoint.
                laserSize.x = size / 2.0f;
            }

            if (curLaser->timer >= curLaser->hitboxStartTime)
            {
                g_Player.CalcLaserHitbox(&laserCenter, &laserSize, &curLaser->pos, curLaser->angle,
                                         MULTIPLE_OF(curLaser->timer, LASER_GRAZE_RATE));
            }

            if (curLaser->timer < curLaser->startTime)
            {
                break;
            }

            curLaser->timer = 0;
            curLaser->state++;
            // fallthrough
        case LASER_STATE_ACTIVE:
            g_Player.CalcLaserHitbox(&laserCenter, &laserSize, &curLaser->pos, curLaser->angle,
                                     MULTIPLE_OF(curLaser->timer, LASER_GRAZE_RATE));

            if (curLaser->timer < curLaser->duration)
            {
                break;
            }

            curLaser->timer = 0;
            curLaser->state++;

            if (curLaser->despawnDuration == 0)
            {
                curLaser->inUse = false;
                continue;
            }
            // fallthrough
        case LASER_STATE_DESPAWNING:
            if (curLaser->flags & LASER_FLAG_FADE_IN_OUT)
            {
                // BUG: This replays fading in rather than actually fading out
                laserAlpha = RAMP_UP(255.0f, (f32)curLaser->timer, curLaser->startTime);

                if (laserAlpha > 255)
                {
                    laserAlpha = 255;
                }

                // BUG: This is meant to only set the alpha value,
                // but overwrites the entire color with black.
                // The fading flag is unused, so this doesn't matter.
                curLaser->vm.color = laserAlpha << COLOR_ALPHA_SHIFT;
            }
            else if (curLaser->despawnDuration > 0)
            {
                size = RAMP_DOWN(curLaser->width, (f32)curLaser->timer, curLaser->despawnDuration);
                curLaser->vm.scaleX = size / 16.0f;
                // Bug: ZUN intended to set laserSize.y instead of laserSize.x
                // This way, for hitboxEndDelay ticks after the laser starts despawning,
                // the laser would have a thinner hitbox.
                // Setting laserSize.x results in a tiny hitbox at the laser midpoint.
                laserSize.x = size / 2.0f;
            }

            if (curLaser->timer < curLaser->hitboxEndDelay)
            {
                g_Player.CalcLaserHitbox(&laserCenter, &laserSize, &curLaser->pos, curLaser->angle,
                                         MULTIPLE_OF(curLaser->timer, LASER_GRAZE_RATE));
            }

            if (curLaser->timer < curLaser->despawnDuration)
            {
                break;
            }

            curLaser->inUse = false;
            continue;
        }

        // BUG(?): This is trying to despawn lasers that're completely out of bounds,
        // but the game window width is way bigger than the size of the game region
        // so it likely never triggers. Maybe ZUN made a typo with his macros?
        if (curLaser->startOffset >= GAME_WINDOW_WIDTH)
        {
            curLaser->inUse = false;
        }

        curLaser->timer++;
        g_AnmManager->RunAnm(&curLaser->vm);
    }

    mgr->time++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static void DrawBullet(Bullet *bullet)
{
    AnmVm *anmVm;

    switch (bullet->state)
    {
    case BULLET_STATE_SPAWNING_FAST:
        anmVm = &bullet->spawnEffectFast;
        break;
    case BULLET_STATE_SPAWNING_NORMAL:
        anmVm = &bullet->spawnEffectNormal;
        break;
    case BULLET_STATE_SPAWNING_SLOW:
        anmVm = &bullet->spawnEffectSlow;
        break;
    case BULLET_STATE_DESPAWNING:
        anmVm = &bullet->despawnEffect;
        break;
    default:
        anmVm = &bullet->vm;
        break;
    }

    anmVm->pos.x = bullet->pos.x;
    anmVm->pos.y = bullet->pos.y;
    anmVm->pos.z = 0.0f;
    anmVm->color = COLOR_COMBINE_ALPHA(COLOR_WHITE, anmVm->color);

    if (anmVm->autoRotate != AnmAutoRotate_Off)
    {
        anmVm->rotation.z = ZUN_HALF_PI - bullet->angle;
    }

    g_AnmManager->Draw3D(anmVm);
}

static void DrawBulletNoHwVertex(Bullet *bullet)
{
    AnmVm *anmVm;

    switch (bullet->state)
    {
    case BULLET_STATE_SPAWNING_FAST:
        anmVm = &bullet->spawnEffectFast;
        break;
    case BULLET_STATE_SPAWNING_NORMAL:
        anmVm = &bullet->spawnEffectNormal;
        break;
    case BULLET_STATE_SPAWNING_SLOW:
        anmVm = &bullet->spawnEffectSlow;
        break;
    case BULLET_STATE_DESPAWNING:
        anmVm = &bullet->despawnEffect;
        break;
    default:
        anmVm = &bullet->vm;
        break;
    }

    anmVm->pos[0] = g_GameManager.gameRegionScreenPos.x + bullet->pos[0];
    anmVm->pos[1] = g_GameManager.gameRegionScreenPos.y + bullet->pos[1];
    anmVm->pos[2] = 0.0f;
    anmVm->color = COLOR_COMBINE_ALPHA(COLOR_WHITE, anmVm->color);

    if (anmVm->autoRotate != AnmAutoRotate_Off)
    {
        anmVm->rotation.z = ZUN_HALF_PI - bullet->angle;
    }

    g_AnmManager->Draw2DRotated(anmVm);
}

#pragma var_order(i, sine, curLaser, laserOffset, cosine)
static ChainCallbackResult BulletManager_OnDraw(BulletManager *mgr)
{
    i32 i;
    f32 sine;
    Laser *curLaser;
    f32 laserOffset;
    f32 cosine;

    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

    curLaser = &mgr->lasers[0];
    for (i = 0; i < MAX_ENEMY_LASERS; i++, curLaser++)
    {
        if (!curLaser->inUse)
        {
            continue;
        }
        fsincos_wrapper(&sine, &cosine, curLaser->angle);
        laserOffset = (curLaser->endOffset - curLaser->startOffset) / 2.0f + curLaser->startOffset;
        curLaser->vm.pos[0] = cosine * laserOffset + curLaser->pos[0];
        curLaser->vm.pos[1] = sine * laserOffset + curLaser->pos[1];
        curLaser->vm.pos[2] = 0.0f;
        curLaser->color = COLOR_COMBINE_ALPHA(COLOR_WHITE, curLaser->color);
        g_AnmManager->Draw3DRotated(&curLaser->vm);

        if (curLaser->startOffset < 16.0f || curLaser->speed == 0.0f)
        {
            curLaser->baseGlowVm.pos[0] = cosine * curLaser->startOffset + curLaser->pos[0];
            curLaser->baseGlowVm.pos[1] = sine * curLaser->startOffset + curLaser->pos[1];
            curLaser->baseGlowVm.pos[2] = 0.0f;
            curLaser->baseGlowVm.color = curLaser->vm.color;
            curLaser->baseGlowVm.flags.colorOp = AnmColorOp_Add;
            curLaser->baseGlowVm.color = COLOR_SET_ALPHA(curLaser->baseGlowVm.color, 0xff);
            curLaser->baseGlowVm.scaleY = curLaser->baseGlowVm.scaleX =
                (curLaser->width / 10.0f) * ((16.0f - curLaser->startOffset) / 16.0f);

            if (curLaser->baseGlowVm.scaleY < 0.0f)
            {
                curLaser->baseGlowVm.scaleY = curLaser->baseGlowVm.scaleX = curLaser->width / 10.0f;
            }

            g_AnmManager->Draw3DRotated(&curLaser->baseGlowVm);
        }
    }

    g_ItemManager.OnDraw();

    if (g_Supervisor.hasD3dHardwareVertexProcessing)
    {
        Bullet *curBullet = &mgr->bullets[0];
        for (i = 0; i < MAX_ENEMY_BULLETS; i++, curBullet++)
        {
            if (curBullet->state == BULLET_STATE_INACTIVE)
            {
                continue;
            }

            if (curBullet->height > BULLET_SIZE_SMALL)
            {
                DrawBullet(curBullet);
            }
        }

        curBullet = &mgr->bullets[0];
        for (i = 0; i < MAX_ENEMY_BULLETS; i++, curBullet++)
        {
            if (curBullet->state == BULLET_STATE_INACTIVE)
            {
                continue;
            }

            if (curBullet->height == BULLET_SIZE_SMALL && (curBullet->vm.scriptId == ANM_SCRIPT_BULLET_RING_BALL ||
                                                           curBullet->vm.scriptId == ANM_SCRIPT_BULLET_BALL))
            {
                DrawBullet(curBullet);
            }
        }

        curBullet = &mgr->bullets[0];
        for (i = 0; i < MAX_ENEMY_BULLETS; i++, curBullet++)
        {
            if (curBullet->state == BULLET_STATE_INACTIVE)
            {
                continue;
            }

            if (curBullet->height == BULLET_SIZE_SMALL && curBullet->vm.scriptId != ANM_SCRIPT_BULLET_RING_BALL &&
                curBullet->vm.scriptId != ANM_SCRIPT_BULLET_BALL)
            {
                DrawBullet(curBullet);
            }
        }

        curBullet = &mgr->bullets[0];
        for (i = 0; i < MAX_ENEMY_BULLETS; i++, curBullet++)
        {
            if (curBullet->state == BULLET_STATE_INACTIVE)
            {
                continue;
            }

            if (curBullet->height == BULLET_SIZE_TINY)
            {
                DrawBullet(curBullet);
            }
        }
    }
    else
    {
        Bullet *curBullet = &mgr->bullets[0];
        for (i = 0; i < MAX_ENEMY_BULLETS; i++, curBullet++)
        {
            if (curBullet->state == BULLET_STATE_INACTIVE)
            {
                continue;
            }

            if (curBullet->height > BULLET_SIZE_SMALL)
            {
                DrawBulletNoHwVertex(curBullet);
            }
        }

        curBullet = &mgr->bullets[0];
        for (i = 0; i < MAX_ENEMY_BULLETS; i++, curBullet++)
        {
            if (curBullet->state == BULLET_STATE_INACTIVE)
            {
                continue;
            }

            if (curBullet->height == BULLET_SIZE_SMALL && (curBullet->vm.scriptId == ANM_SCRIPT_BULLET_RING_BALL ||
                                                           curBullet->vm.scriptId == ANM_SCRIPT_BULLET_BALL))
            {
                DrawBulletNoHwVertex(curBullet);
            }
        }

        curBullet = &mgr->bullets[0];
        for (i = 0; i < MAX_ENEMY_BULLETS; i++, curBullet++)
        {
            if (curBullet->state == BULLET_STATE_INACTIVE)
            {
                continue;
            }

            if (curBullet->height == BULLET_SIZE_SMALL && curBullet->vm.scriptId != ANM_SCRIPT_BULLET_RING_BALL &&
                curBullet->vm.scriptId != ANM_SCRIPT_BULLET_BALL)
            {
                DrawBulletNoHwVertex(curBullet);
            }
        }

        curBullet = &mgr->bullets[0];
        for (i = 0; i < MAX_ENEMY_BULLETS; i++, curBullet++)
        {
            if (curBullet->state == BULLET_STATE_INACTIVE)
            {
                continue;
            }

            if (curBullet->height == BULLET_SIZE_TINY)
            {
                DrawBulletNoHwVertex(curBullet);
            }
        }
    }

    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ZunResult BulletManager_AddedCallback(BulletManager *mgr)
{
    if (g_Supervisor.IsNotLoadingNextStage())
    {
        if (g_AnmManager->LoadAnm(ANM_FILE_BULLET3, "data/etama3.anm", ANM_OFFSET_BULLET3) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }

        if (g_AnmManager->LoadAnm(ANM_FILE_BULLET4, "data/etama4.anm", ANM_OFFSET_BULLET4) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
    }

    for (u32 i = 0; i < ENUM_VALUE_COUNT(BulletType); i++)
    {
        g_AnmManager->SetScript(&mgr->bulletTemplates[i].vm, g_BulletAnmScripts[i].bullet);
        g_AnmManager->SetScript(&mgr->bulletTemplates[i].spawnEffectFast, g_BulletAnmScripts[i].spawnEffectFast);
        g_AnmManager->SetScript(&mgr->bulletTemplates[i].spawnEffectNormal, g_BulletAnmScripts[i].spawnEffectNormal);
        g_AnmManager->SetScript(&mgr->bulletTemplates[i].spawnEffectSlow, g_BulletAnmScripts[i].spawnEffectSlow);
        g_AnmManager->SetScript(&mgr->bulletTemplates[i].despawnEffect, g_BulletAnmScripts[i].despawnEffect);
        mgr->bulletTemplates[i].vm.baseSpriteId = mgr->bulletTemplates[i].vm.spriteId;
        mgr->bulletTemplates[i].height = mgr->bulletTemplates[i].vm.sprite->height;

        if (mgr->bulletTemplates[i].vm.sprite->height <= BULLET_SIZE_TINY)
        {
            mgr->bulletTemplates[i].grazeSize.x = 4.0f;
            mgr->bulletTemplates[i].grazeSize.y = 4.0f;
        }
        else if (mgr->bulletTemplates[i].vm.sprite->height <= BULLET_SIZE_SMALL)
        {
            switch (g_BulletAnmScripts[i].bullet)
            {
            case ANM_SCRIPT_BULLET_RICE:
                mgr->bulletTemplates[i].grazeSize.x = 4.0f;
                mgr->bulletTemplates[i].grazeSize.y = 4.0f;
                break;
            case ANM_SCRIPT_BULLET_KUNAI:
                mgr->bulletTemplates[i].grazeSize.x = 5.0f;
                mgr->bulletTemplates[i].grazeSize.y = 5.0f;
                break;
            case ANM_SCRIPT_BULLET_SHARD:
                mgr->bulletTemplates[i].grazeSize.x = 4.0f;
                mgr->bulletTemplates[i].grazeSize.y = 4.0f;
                break;
            default:
                mgr->bulletTemplates[i].grazeSize.x = 6.0f;
                mgr->bulletTemplates[i].grazeSize.y = 6.0f;
                break;
            }
        }
        else if (mgr->bulletTemplates[i].vm.sprite->height <= BULLET_SIZE_LARGE)
        {
            switch (g_BulletAnmScripts[i].bullet)
            {
            case ANM_SCRIPT_BULLET_FIREBALL:
                mgr->bulletTemplates[i].grazeSize.x = 11.0f;
                mgr->bulletTemplates[i].grazeSize.y = 11.0f;
                break;
            case ANM_SCRIPT_BULLET_DAGGER:
                mgr->bulletTemplates[i].grazeSize.x = 9.0f;
                mgr->bulletTemplates[i].grazeSize.y = 9.0f;
                break;
            default: // ANM_SCRIPT_BULLET_BIG_BALL
                mgr->bulletTemplates[i].grazeSize.x = 16.0f;
                mgr->bulletTemplates[i].grazeSize.y = 16.0f;
            }
        }
        else // BULLET_SIZE_HUGE
        {
            mgr->bulletTemplates[i].grazeSize.x = 32.0f;
            mgr->bulletTemplates[i].grazeSize.y = 32.0f;
        }
    }

    memset(&g_ItemManager, 0, sizeof(ItemManager));
    return ZUN_SUCCESS;
}

static ZunResult BulletManager_DeletedCallback(BulletManager *arg)
{
    if (g_Supervisor.IsNotLoadingNextStage())
    {
        g_AnmManager->ReleaseAnm(ANM_FILE_BULLET3);
        g_AnmManager->ReleaseAnm(ANM_FILE_BULLET4);
    }

    return ZUN_SUCCESS;
}

ZunResult BulletManager_RegisterChain(const char *bulletAnmPath)
{
    BulletManager *mgr = &g_BulletManager;

    if (!g_Supervisor.IsHardwareBlendingDisabled())
    {
        g_EffectsColor = g_EffectsColorWithTextureBlending;
    }
    else
    {
        g_EffectsColor = g_EffectsColorWithoutTextureBlending;
    }

    mgr->InitializeToZero();
    mgr->bulletAnmPath = bulletAnmPath;
    g_BulletManagerCalcChainElem.SetCallback((ChainCallback)BulletManager_OnUpdate);
    g_BulletManagerCalcChainElem.addedCallback = (ChainAddedCallback)BulletManager_AddedCallback;
    g_BulletManagerCalcChainElem.deletedCallback = (ChainDeletedCallback)BulletManager_DeletedCallback;
    g_BulletManagerCalcChainElem.arg = mgr;

    if (g_FuncChain.AddToCalcChain(&g_BulletManagerCalcChainElem, TH_CHAIN_PRIO_CALC_BULLETMANAGER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    g_BulletManagerDrawChainElem.SetCallback((ChainCallback)BulletManager_OnDraw);
    g_BulletManagerDrawChainElem.arg = mgr;
    g_FuncChain.AddToDrawChain(&g_BulletManagerDrawChainElem, TH_CHAIN_PRIO_DRAW_BULLETMANAGER);
    return ZUN_SUCCESS;
}

void BulletManager_CutChain()
{
    g_FuncChain.Cut(&g_BulletManagerCalcChainElem);
    g_FuncChain.Cut(&g_BulletManagerDrawChainElem);
}
} // namespace th06
