#include "BulletManager.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "Enemy.hpp"
#include "GLFunc.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "Rng.hpp"
#include "ZunColor.hpp"
#include "ZunMath.hpp"
#include "utils.hpp"

namespace th06
{
DIFFABLE_STATIC(BulletManager, g_BulletManager);
DIFFABLE_STATIC(ChainElem, g_BulletManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_BulletManagerDrawChain);
DIFFABLE_STATIC_ARRAY_ASSIGN(u32, 28, g_EffectsColorWithTextureBlending) = {
    0xff000000, 0xff303030, 0xff606060, 0xff500000, 0xff900000, 0xffff2020, 0xff400040,
    0xff800080, 0xffff30ff, 0xff000050, 0xff000090, 0xff2020ff, 0xff203060, 0xff304090,
    0xff3080ff, 0xff005000, 0xff009000, 0xff20ff20, 0xff206000, 0xff409010, 0xff80ff20,
    0xff505000, 0xff909000, 0xffffff20, 0xff603000, 0xff904010, 0xfff08020, 0xffffffff};

DIFFABLE_STATIC_ARRAY_ASSIGN(u32, 28, g_EffectsColorWithoutTextureBlending) = {
    0xfff0f0f0, 0xfff0f0f0, 0xffffffff, 0xffffe0e0, 0xffffe0e0, 0xffffe0e0, 0xffffe0ff,
    0xffffe0ff, 0xffffe0ff, 0xffe0e0ff, 0xffe0e0ff, 0xffe0e0ff, 0xffe0ffff, 0xffe0ffff,
    0xffe0ffff, 0xffe0ffe0, 0xffe0ffe0, 0xffe0ffe0, 0xffe0ffe0, 0xffe0ffe0, 0xffe0ffe0,
    0xffffffe0, 0xffffffe0, 0xffffffe0, 0xffffe0e0, 0xffffe0e0, 0xffffe0e0, 0xffffffff};
DIFFABLE_STATIC_ARRAY_ASSIGN(u32, 16, g_BulletSpriteOffset16Px) = {0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 0};
DIFFABLE_STATIC_ARRAY_ASSIGN(u32, 8, g_BulletSpriteOffset32Px) = {0, 1, 1, 2, 2, 3, 4, 0};

DIFFABLE_STATIC_ASSIGN(u32 *, g_EffectsColor) = g_EffectsColorWithTextureBlending;

struct BulletTypeInfo
{
    u32 bulletAnmScriptIdx;
    u32 bulletSpawnEffectFastAnmScriptIdx;
    u32 bulletSpawnEffectNormalAnmScriptIdx;
    u32 bulletSpawnEffectSlowAnmScriptIdx;
    u32 bulletSpawnEffectDonutAnmScriptIdx;
};

#define ASB3(x) ANM_SCRIPT_BULLET3_##x
#define ASB4(x) ANM_SCRIPT_BULLET4_##x
DIFFABLE_STATIC_ARRAY_ASSIGN(BulletTypeInfo, 10, g_BulletTypeInfos) = {
    {ASB3(PELLET), ASB3(SPAWN_PELLET_FAST), ASB3(SPAWN_PELLET_NORMAL), ASB3(SPAWN_PELLET_SLOW),
     ASB3(SPAWN_DONUT_SMALL)},
    {ASB3(RING_BALL), ASB3(SPAWN_BIG_BALL_FAST), ASB3(SPAWN_BIG_BALL_NORMAL), ASB3(SPAWN_BIG_BALL_SLOW),
     ASB3(SPAWN_DONUT_MEDIUM)},
    {ASB3(RICE), ASB3(SPAWN_BIG_BALL_FAST), ASB3(SPAWN_BIG_BALL_NORMAL), ASB3(SPAWN_BIG_BALL_SLOW),
     ASB3(SPAWN_DONUT_MEDIUM)},
    {ASB3(BALL), ASB3(SPAWN_BIG_BALL_FAST), ASB3(SPAWN_BIG_BALL_NORMAL), ASB3(SPAWN_BIG_BALL_SLOW),
     ASB3(SPAWN_DONUT_MEDIUM)},
    {ASB3(KUNAI), ASB3(SPAWN_BIG_BALL_FAST), ASB3(SPAWN_BIG_BALL_NORMAL), ASB3(SPAWN_BIG_BALL_SLOW),
     ASB3(SPAWN_DONUT_MEDIUM)},
    {ASB3(SHARD), ASB3(SPAWN_BIG_BALL_FAST), ASB3(SPAWN_BIG_BALL_NORMAL), ASB3(SPAWN_BIG_BALL_SLOW),
     ASB3(SPAWN_DONUT_MEDIUM)},
    {ASB3(BIG_BALL), ASB3(SPAWN_BIG_BALL_HUGE), ASB3(SPAWN_BIG_BALL_HUGE), ASB3(SPAWN_BIG_BALL_HUGE),
     ASB3(SPAWN_DONUT_BIG)},
    {ASB3(FIREBALL), ASB3(SPAWN_BIG_BALL_HUGE), ASB3(SPAWN_BIG_BALL_HUGE), ASB3(SPAWN_BIG_BALL_HUGE),
     ASB3(SPAWN_DONUT_BIG)},
    {ASB3(DAGGER), ASB3(SPAWN_BIG_BALL_HUGE), ASB3(SPAWN_BIG_BALL_HUGE), ASB3(SPAWN_BIG_BALL_HUGE),
     ASB3(SPAWN_DONUT_BIG)},
    {ASB4(BUBBLE), ASB4(SPAWN_BUBBLE_SLOW), ASB4(SPAWN_BUBBLE_SLOW), ASB4(SPAWN_BUBBLE_SLOW),
     ASB4(SPAWN_BUBBLE_NORMAL)},
};

void BulletManager::InitializeToZero()
{
    memset(this, 0, sizeof(BulletManager));
}

BulletManager::BulletManager()
{
    this->InitializeToZero();
}

u32 BulletManager::SpawnSingleBullet(EnemyBulletShooter *bulletProps, i32 bulletIdx1, i32 bulletIdx2, f32 angle)
{
    f32 bulletAngle;
    Bullet *bullet;
    i32 local_c;
    f32 bulletSpeed;

    local_c = 0;
    bullet = &this->bullets[this->nextBulletIndex];
    for (local_c = 0; local_c < ARRAY_SIZE_SIGNED(this->bullets); local_c++)
    {
        this->nextBulletIndex++;

        if (ARRAY_SIZE_SIGNED(this->bullets) <= this->nextBulletIndex)
        {
            this->nextBulletIndex = 0;
        }

        if (bullet->state != 0)
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

    if (local_c >= ARRAY_SIZE_SIGNED(this->bullets))
    {
        return 1;
    }

    bulletAngle = 0.0f;
    bulletSpeed = bulletProps->speed1 - (bulletProps->speed1 - bulletProps->speed2) * bulletIdx2 / bulletProps->count2;
    switch (bulletProps->aimMode)
    {
    case FAN_AIMED:
    case FAN:
        if ((bulletProps->count1 & 1) != 0)
        {
            bulletAngle = ((bulletIdx1 + 1) / 2) * bulletProps->angle2 + bulletAngle;
        }
        else
        {
            bulletAngle = (bulletIdx1 / 2) * bulletProps->angle2 + bulletProps->angle2 * 0.5f + bulletAngle;
        }

        if ((bulletIdx1 & 1) != 0)
        {
            bulletAngle *= -1.0f;
        }

        if (bulletProps->aimMode == FAN_AIMED)
        {
            bulletAngle += angle;
        }

        bulletAngle += bulletProps->angle1;
        break;
    case CIRCLE_AIMED:
        bulletAngle += angle;
    case CIRCLE:
        bulletAngle += bulletIdx1 * ZUN_2PI / bulletProps->count1;
        bulletAngle += bulletIdx2 * bulletProps->angle2 + bulletProps->angle1;
        break;
    case OFFSET_CIRCLE_AIMED:
        bulletAngle += angle;
    case OFFSET_CIRCLE:
        bulletAngle += ZUN_PI / bulletProps->count1;
        bulletAngle += bulletIdx1 * ZUN_2PI / bulletProps->count1;
        bulletAngle += bulletProps->angle1;
        break;
    case RANDOM_ANGLE:
        bulletAngle = g_Rng.GetRandomF32InRange(bulletProps->angle1 - bulletProps->angle2) + bulletProps->angle2;
        break;
    case RANDOM_SPEED:
        bulletSpeed = g_Rng.GetRandomF32InRange(bulletProps->speed1 - bulletProps->speed2) + bulletProps->speed2;
        bulletAngle += bulletIdx1 * ZUN_2PI / bulletProps->count1;
        bulletAngle += bulletIdx2 * bulletProps->angle2 + bulletProps->angle1;
        break;
    case RANDOM:
        bulletAngle = g_Rng.GetRandomF32InRange(bulletProps->angle1 - bulletProps->angle2) + bulletProps->angle2;
        bulletSpeed = g_Rng.GetRandomF32InRange(bulletProps->speed1 - bulletProps->speed2) + bulletProps->speed2;
    }

    bullet->state = 1;
    bullet->unk_5c2 = 1;
    bullet->speed = bulletSpeed;
    bullet->angle = utils::AddNormalizeAngle(bulletAngle, 0.0f);
    bullet->pos = bulletProps->position;
    bullet->pos.z = 0.1f;
    sincosmul(&bullet->velocity, bullet->angle, bulletSpeed);
    bullet->exFlags = bulletProps->flags;
    bullet->spriteOffset = bulletProps->spriteOffset;
    bullet->sprites.spriteBullet = this->bulletTypeTemplates[bulletProps->sprite].spriteBullet;
    bullet->sprites.spriteSpawnEffectDonut = this->bulletTypeTemplates[bulletProps->sprite].spriteSpawnEffectDonut;
    bullet->sprites.grazeSize = this->bulletTypeTemplates[bulletProps->sprite].grazeSize;
    bullet->sprites.unk_55c = this->bulletTypeTemplates[bulletProps->sprite].unk_55c;
    bullet->sprites.bulletHeight = this->bulletTypeTemplates[bulletProps->sprite].bulletHeight;

    if (bullet->exFlags & 2)
    {
        // TODO: Make an inline function for this?
        // It's the same damn code, copy pasted four times.
        bullet->sprites.spriteSpawnEffectFast = this->bulletTypeTemplates[bulletProps->sprite].spriteSpawnEffectFast;

        if (bullet->sprites.spriteBullet.sprite->heightPx <= 16.0f)
        {
            g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectFast,
                                          bullet->sprites.spriteSpawnEffectFast.activeSpriteIndex +
                                              g_BulletSpriteOffset16Px[bulletProps->spriteOffset]);
        }
        else if (bullet->sprites.spriteBullet.sprite->heightPx <= 32.0f)
        {
            if (bullet->sprites.spriteBullet.anmFileIndex != 0x207)
            {
                g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectFast,
                                              bullet->sprites.spriteSpawnEffectFast.activeSpriteIndex +
                                                  g_BulletSpriteOffset32Px[bulletProps->spriteOffset]);
            }
            else
            {
                g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectFast,
                                              bullet->sprites.spriteSpawnEffectFast.activeSpriteIndex + 1);
            }
        }
        else
        {
            g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectFast,
                                          bullet->sprites.spriteSpawnEffectFast.activeSpriteIndex +
                                              bulletProps->spriteOffset);
        }

        bullet->state = 2;
    }
    else if (bullet->exFlags & 4)
    {
        bullet->sprites.spriteSpawnEffectNormal =
            this->bulletTypeTemplates[bulletProps->sprite].spriteSpawnEffectNormal;

        if (bullet->sprites.spriteBullet.sprite->heightPx <= 16.0f)
        {
            g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectNormal,
                                          bullet->sprites.spriteSpawnEffectNormal.activeSpriteIndex +
                                              g_BulletSpriteOffset16Px[bulletProps->spriteOffset]);
        }
        else if (bullet->sprites.spriteBullet.sprite->heightPx <= 32.0f)
        {
            if (bullet->sprites.spriteBullet.anmFileIndex != 0x207)
            {
                g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectNormal,
                                              bullet->sprites.spriteSpawnEffectNormal.activeSpriteIndex +
                                                  g_BulletSpriteOffset32Px[bulletProps->spriteOffset]);
            }
            else
            {
                g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectNormal,
                                              bullet->sprites.spriteSpawnEffectNormal.activeSpriteIndex + 1);
            }
        }
        else
        {
            g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectNormal,
                                          bullet->sprites.spriteSpawnEffectNormal.activeSpriteIndex +
                                              bulletProps->spriteOffset);
        }
        bullet->state = 3;
    }
    else if (bullet->exFlags & 8)
    {
        bullet->sprites.spriteSpawnEffectSlow = this->bulletTypeTemplates[bulletProps->sprite].spriteSpawnEffectSlow;
        if (bullet->sprites.spriteBullet.sprite->heightPx <= 16.0f)
        {
            g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectSlow,
                                          bullet->sprites.spriteSpawnEffectSlow.activeSpriteIndex +
                                              g_BulletSpriteOffset16Px[bulletProps->spriteOffset]);
        }
        else if (bullet->sprites.spriteBullet.sprite->heightPx <= 32.0f)
        {
            if (bullet->sprites.spriteBullet.anmFileIndex != 0x207)
            {
                g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectSlow,
                                              bullet->sprites.spriteSpawnEffectSlow.activeSpriteIndex +
                                                  g_BulletSpriteOffset32Px[bulletProps->spriteOffset]);
            }
            else
            {
                g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectSlow,
                                              bullet->sprites.spriteSpawnEffectSlow.activeSpriteIndex + 1);
            }
        }
        else
        {
            g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectSlow,
                                          bullet->sprites.spriteSpawnEffectSlow.activeSpriteIndex +
                                              bulletProps->spriteOffset);
        }

        bullet->state = 4;
    }
    g_AnmManager->SetActiveSprite(&bullet->sprites.spriteBullet,
                                  bullet->sprites.spriteBullet.activeSpriteIndex + bulletProps->spriteOffset);

    if (bullet->sprites.spriteBullet.sprite->heightPx <= 16.0f)
    {
        g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectDonut,
                                      bullet->sprites.spriteSpawnEffectDonut.activeSpriteIndex +
                                          g_BulletSpriteOffset16Px[bulletProps->spriteOffset]);
    }
    else if (bullet->sprites.spriteBullet.sprite->heightPx <= 32.0f)
    {
        if (bullet->sprites.spriteBullet.anmFileIndex != 0x207)
        {
            g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectDonut,
                                          bullet->sprites.spriteSpawnEffectDonut.activeSpriteIndex +
                                              g_BulletSpriteOffset32Px[bulletProps->spriteOffset]);
        }
        else
        {
            g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectDonut,
                                          bullet->sprites.spriteSpawnEffectDonut.activeSpriteIndex + 1);
        }
    }
    else
    {
        g_AnmManager->SetActiveSprite(&bullet->sprites.spriteSpawnEffectDonut,
                                      bullet->sprites.spriteSpawnEffectDonut.activeSpriteIndex +
                                          bulletProps->spriteOffset);
    }

    if (bullet->exFlags & 0x10)
    {
        if (bulletProps->exFloats[1] <= -999.0f)
        {
            sincosmul(&bullet->ex4Acceleration, bulletAngle, bulletProps->exFloats[0]);
        }
        else
        {
            sincosmul(&bullet->ex4Acceleration, bulletProps->exFloats[1], bulletProps->exFloats[0]);
        }

        if (bulletProps->exInts[0] > 0)
        {
            bullet->ex5Int0 = bulletProps->exInts[0];
        }
        else
        {
            bullet->ex5Int0 = 99999;
        }

        bullet->ex4Acceleration.z = 0.0f;
    }
    else if (bullet->exFlags & 0x20)
    {
        bullet->ex5Float0 = bulletProps->exFloats[0];
        bullet->ex5Float1 = bulletProps->exFloats[1];
        bullet->ex5Int0 = bulletProps->exInts[0];
    }

    if (bullet->exFlags & 0x1c0)
    {
        bullet->dirChangeRotation = bulletProps->exFloats[0];

        if (bulletProps->exFloats[1] >= 0.0f)
        {
            bullet->dirChangeSpeed = bulletProps->exFloats[1];
        }
        else
        {
            bullet->dirChangeSpeed = bulletSpeed;
        }

        bullet->dirChangeInterval = bulletProps->exInts[0];
        bullet->dirChangeMaxTimes = bulletProps->exInts[1];
        bullet->dirChangeNumTimes = 0;
    }

    if (bullet->exFlags & 0xc00)
    {
        if (bulletProps->exFloats[0] >= 0.0f)
        {
            bullet->dirChangeSpeed = bulletProps->exFloats[0];
        }
        else
        {
            bullet->dirChangeSpeed = bulletSpeed;
        }

        bullet->dirChangeMaxTimes = bulletProps->exInts[0];
        bullet->dirChangeNumTimes = 0;
    }
    return 0;
}

void BulletManager::RemoveAllBullets(ZunBool turnIntoItem)
{
    f32 cosine;
    f32 sine;
    f32 offset;
    Laser *laser;
    Bullet *bullet;
    i32 i;
    ZunVec3 itemPos;

    for (bullet = &g_BulletManager.bullets[0], i = 0; i < ARRAY_SIZE_SIGNED(g_BulletManager.bullets); i++, bullet++)
    {
        if (bullet->state == 0 || bullet->state == 5)
        {
            continue;
        }

        if (turnIntoItem)
        {
            g_ItemManager.SpawnItem(&bullet->pos, ITEM_POINT_BULLET, 1);
            memset(bullet, 0, sizeof(Bullet));
        }
        else
        {
            bullet->state = 5;
        }
    }

    for (laser = this->lasers, i = 0; i < ARRAY_SIZE_SIGNED(this->lasers); i++, laser++)
    {
        if (!laser->inUse)
        {
            continue;
        }

        if (laser->state < 2)
        {
            laser->state = 2;
            laser->timer.InitializeForPopup();

            if (turnIntoItem)
            {
                offset = laser->startOffset;
                fsincos_wrapper(&sine, &cosine, laser->angle);

                while (laser->endOffset > offset)
                {
                    itemPos.x = cosine * offset + laser->pos.x;
                    itemPos.y = sine * offset + laser->pos.y;
                    itemPos.z = 0.0f;
                    g_ItemManager.SpawnItem(&itemPos, ITEM_POINT_BULLET, 1);
                    offset += 32.0f;
                }
            }
        }

        laser->grazeInterval = 0;
    }
}

void BulletManager::TurnAllBulletsIntoPoints()
{
    this->RemoveAllBullets(true);
}

i32 BulletManager::DespawnBullets(i32 maxBonusScore, ZunBool awardPoints)
{
    i32 bulletScore;
    i32 totalBonusScore;
    i32 awardedBullets;
    i32 i;
    f32 sine;
    f32 cosine;
    f32 offset;
    Laser *laser;
    Bullet *bullets;
    ZunVec3 itemPos;

    totalBonusScore = 0;
    bulletScore = 2000;
    awardedBullets = 0;
    bullets = &g_BulletManager.bullets[0];
    for (i = 0; i < ARRAY_SIZE_SIGNED(g_BulletManager.bullets); i++, bullets++)
    {
        if (bullets->state == 0)
        {
            continue;
        }

        if (awardPoints)
        {
            g_ItemManager.SpawnItem(&bullets->pos, ITEM_POINT_BULLET, 1);
        }

        g_AsciiManager.CreatePopup1(&bullets->pos, bulletScore,
                                    bulletScore >= maxBonusScore ? COLOR_YELLOW : COLOR_WHITE);

        totalBonusScore += bulletScore;
        awardedBullets++;
        bulletScore += 10;

        if (bulletScore > maxBonusScore)
        {
            bulletScore = maxBonusScore;
        }

        bullets->state = 5;
    }

    laser = &this->lasers[0];
    for (i = 0; i < ARRAY_SIZE_SIGNED(this->lasers); i++, laser++)
    {
        if (!laser->inUse)
        {
            continue;
        }

        if (laser->state < 2)
        {
            laser->state = 2;
            laser->timer.InitializeForPopup();

            if (awardPoints != 0)
            {
                g_ItemManager.SpawnItem(&laser->pos, ITEM_POINT_BULLET, 1);
                offset = laser->startOffset;
                fsincos_wrapper(&sine, &cosine, laser->angle);

                while (laser->endOffset > offset)
                {
                    itemPos.x = cosine * offset + laser->pos.x;
                    itemPos.y = sine * offset + laser->pos.y;
                    itemPos.z = 0.0f;
                    g_ItemManager.SpawnItem(&itemPos, ITEM_POINT_BULLET, 1);
                    offset += 32.0f;
                }
            }
        }

        laser->grazeInterval = 0;
    }

    g_GameManager.score += totalBonusScore;

    if (totalBonusScore != 0)
    {
        g_Gui.ShowBonusScore(totalBonusScore);
    }

    return totalBonusScore;
}

ZunResult BulletManager::SpawnBulletPattern(EnemyBulletShooter *bulletProps)
{
    i32 idx1, idx2;
    f32 angle;

    angle = g_Player.AngleToPlayer(&bulletProps->position);
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
    if ((bulletProps->flags & 0x200) != 0)
    {
        g_SoundPlayer.PlaySoundByIdx(bulletProps->sfx);
    }
    return ZUN_SUCCESS;
}

Laser *BulletManager::SpawnLaserPattern(EnemyLaserShooter *bulletProps)
{
    Laser *laser;
    i32 idx;

    for (laser = this->lasers, idx = 0; idx < ARRAY_SIZE_SIGNED(this->lasers); idx++, laser++)
    {

        if (laser->inUse)
        {
            continue;
        }

        g_AnmManager->SetAndExecuteScriptIdx(&laser->vm0, bulletProps->sprite + ANM_SCRIPT_BULLET3_LASER);
        g_AnmManager->SetActiveSprite(&laser->vm0, laser->vm0.activeSpriteIndex + bulletProps->spriteOffset);

        g_AnmManager->InitializeAndSetSprite(&laser->vm1, g_BulletSpriteOffset16Px[bulletProps->spriteOffset] +
                                                              ANM_SPRITE_BULLET3_SPAWN_BIG_BALL);

        laser->vm1.flags.blendMode = AnmVmBlendMode_One;
        laser->pos = bulletProps->position;
        laser->color = bulletProps->spriteOffset;
        laser->inUse = true;
        laser->angle = bulletProps->angle;

        if (bulletProps->type == 0)
        {
            laser->angle += g_Player.AngleToPlayer(&bulletProps->position);
        }

        laser->flags = bulletProps->flags;
        laser->timer.InitializeForPopup();
        laser->startOffset = bulletProps->startOffset;
        laser->endOffset = bulletProps->endOffset;
        laser->startLength = bulletProps->startLength;
        laser->width = bulletProps->width;
        laser->speed = bulletProps->speed;
        laser->startTime = bulletProps->startTime;
        laser->duration = bulletProps->duration;
        laser->endTime = bulletProps->stopTime;
        laser->grazeDelay = bulletProps->grazeDelay;
        laser->grazeInterval = bulletProps->grazeDistance;

        if (laser->startTime == 0)
        {
            laser->state = 1;
        }
        else
        {
            laser->state = 0;
        }
        break;
    }
    return laser;
}

ZunResult BulletManager::RegisterChain(char *bulletAnmPath)
{
    BulletManager *mgr = &g_BulletManager;

    if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
    {
        g_EffectsColor = g_EffectsColorWithTextureBlending;
    }
    else
    {
        g_EffectsColor = g_EffectsColorWithoutTextureBlending;
    }

    mgr->InitializeToZero();
    mgr->bulletAnmPath = bulletAnmPath;
    g_BulletManagerCalcChain.callback = (ChainCallback)BulletManager::OnUpdate;
    g_BulletManagerCalcChain.addedCallback = NULL;
    g_BulletManagerCalcChain.deletedCallback = NULL;
    g_BulletManagerCalcChain.addedCallback = (ChainAddedCallback)BulletManager::AddedCallback;
    g_BulletManagerCalcChain.deletedCallback = (ChainDeletedCallback)BulletManager::DeletedCallback;
    g_BulletManagerCalcChain.arg = mgr;

    if (g_Chain.AddToCalcChain(&g_BulletManagerCalcChain, TH_CHAIN_PRIO_CALC_BULLETMANAGER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    g_BulletManagerDrawChain.callback = (ChainCallback)BulletManager::OnDraw;
    g_BulletManagerDrawChain.addedCallback = NULL;
    g_BulletManagerDrawChain.deletedCallback = NULL;
    g_BulletManagerDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_BulletManagerDrawChain, TH_CHAIN_PRIO_DRAW_BULLETMANAGER);
    return ZUN_SUCCESS;
}

ChainCallbackResult BulletManager::OnUpdate(BulletManager *mgr)
{
    i32 res;
    ZunVec3 laserSize;
    i32 laserColor;
    ZunVec3 laserCenter;
    f32 local_14;

    Bullet *curBullet;
    Laser *curLaser;
    f32 bulletSpeed;
    i32 idx;
    i32 grazeState;

    curBullet = &mgr->bullets[0];

    if (g_GameManager.isTimeStopped)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    g_ItemManager.OnUpdate();
    mgr->bulletCount = 0;
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->bullets); idx++, curBullet++)
    {
        if (curBullet->state == 0)
            continue;

        mgr->bulletCount++;
        switch (curBullet->state)
        {
        case 2:
            curBullet->pos += curBullet->velocity / 2.0f * g_Supervisor.effectiveFramerateMultiplier;

            if (g_AnmManager->ExecuteScript(&curBullet->sprites.spriteSpawnEffectFast) == 0)
            {
                break;
            }
            goto HELL;
        case 3:
            curBullet->pos += curBullet->velocity / 2.5f * g_Supervisor.effectiveFramerateMultiplier;

            if (g_AnmManager->ExecuteScript(&curBullet->sprites.spriteSpawnEffectNormal) == 0)
            {
                break;
            }
            goto HELL;
        case 4:
            curBullet->pos += curBullet->velocity / 3.0f * g_Supervisor.effectiveFramerateMultiplier;

            if (g_AnmManager->ExecuteScript(&curBullet->sprites.spriteSpawnEffectSlow) == 0)
            {
                break;
            }
        HELL:
            curBullet->state = 1;
            curBullet->timer.InitializeForPopup();
        case 1:
            if (curBullet->exFlags != 0)
            {
                if (curBullet->exFlags & 1)
                {
                    if ((ZunBool)(curBullet->timer.current <= 16))
                    {
                        bulletSpeed = 5.0f - curBullet->timer.AsFramesFloat() * 5.0f / 16.0f;
                        sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed + curBullet->speed);
                    }
                    else
                    {
                        curBullet->exFlags ^= 1;
                    }
                }
                else if (curBullet->exFlags & 0x10)
                {
                    if ((ZunBool)(curBullet->timer.current >= curBullet->ex5Int0))
                    {
                        curBullet->exFlags &= ~0x10;
                    }
                    else
                    {
                        curBullet->velocity += curBullet->ex4Acceleration * g_Supervisor.effectiveFramerateMultiplier;
                        curBullet->angle = ZUN_ATAN2F(curBullet->velocity.y, curBullet->velocity.x);
                    }
                }
                else if (curBullet->exFlags & 0x20)
                {
                    if ((ZunBool)(curBullet->timer.current >= curBullet->ex5Int0))
                    {
                        curBullet->exFlags &= ~0x20;
                    }
                    else
                    {
                        curBullet->angle = utils::AddNormalizeAngle(
                            curBullet->angle, g_Supervisor.effectiveFramerateMultiplier * curBullet->ex5Float1);
                        curBullet->speed += g_Supervisor.effectiveFramerateMultiplier * curBullet->ex5Float0;
                        // Has to be done in asm. Just, great.
                        sincosmul(&curBullet->velocity, curBullet->angle, curBullet->speed);
                    }
                }
                if (curBullet->exFlags & 0x40)
                {
                    if ((ZunBool)(curBullet->timer.current >=
                                  curBullet->dirChangeInterval * (curBullet->dirChangeNumTimes + 1)))
                    {
                        curBullet->dirChangeNumTimes++;

                        if (curBullet->dirChangeNumTimes >= curBullet->dirChangeMaxTimes)
                        {
                            curBullet->exFlags &= ~0x40;
                        }

                        curBullet->angle = curBullet->angle + curBullet->dirChangeRotation;
                        curBullet->speed = curBullet->dirChangeSpeed;
                        bulletSpeed = curBullet->speed;
                    }
                    else
                    {
                        bulletSpeed =
                            curBullet->speed - ((curBullet->timer.AsFramesFloat() -
                                                 (curBullet->dirChangeInterval * curBullet->dirChangeNumTimes)) *
                                                curBullet->speed) /
                                                   curBullet->dirChangeInterval;
                    }

                    sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed);
                }
                else if (curBullet->exFlags & 0x100)
                {
                    if ((ZunBool)(curBullet->timer.current >=
                                  curBullet->dirChangeInterval * (curBullet->dirChangeNumTimes + 1)))
                    {
                        curBullet->dirChangeNumTimes++;

                        if (curBullet->dirChangeNumTimes >= curBullet->dirChangeMaxTimes)
                        {
                            curBullet->exFlags &= ~0x100;
                        }

                        curBullet->angle = curBullet->dirChangeRotation;
                        curBullet->speed = curBullet->dirChangeSpeed;
                        bulletSpeed = curBullet->speed;
                    }
                    else
                    {
                        bulletSpeed =
                            curBullet->speed - ((curBullet->timer.AsFramesFloat() -
                                                 (curBullet->dirChangeInterval * curBullet->dirChangeNumTimes)) *
                                                curBullet->speed) /
                                                   curBullet->dirChangeInterval;
                    }

                    sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed);
                }
                else if (curBullet->exFlags & 0x80)
                {
                    if ((ZunBool)(curBullet->timer.current >=
                                  curBullet->dirChangeInterval * (curBullet->dirChangeNumTimes + 1)))
                    {
                        curBullet->dirChangeNumTimes++;

                        if (curBullet->dirChangeNumTimes >= curBullet->dirChangeMaxTimes)
                        {
                            curBullet->exFlags &= ~0x80;
                        }

                        curBullet->angle = g_Player.AngleToPlayer(&curBullet->pos) + curBullet->dirChangeRotation;
                        curBullet->speed = curBullet->dirChangeSpeed;
                        bulletSpeed = curBullet->speed;
                    }
                    else
                    {
                        bulletSpeed =
                            curBullet->speed - ((curBullet->timer.AsFramesFloat() -
                                                 (curBullet->dirChangeInterval * curBullet->dirChangeNumTimes)) *
                                                curBullet->speed) /
                                                   curBullet->dirChangeInterval;
                    }
                    sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed);
                }
                else if (curBullet->exFlags & 0x400)
                {
                    if (g_GameManager.IsInBounds(curBullet->pos.x, curBullet->pos.y,
                                                 curBullet->sprites.spriteBullet.sprite->widthPx,
                                                 curBullet->sprites.spriteBullet.sprite->heightPx) == 0)
                    {
                        if (curBullet->pos.x < 0.0f || curBullet->pos.x >= 384.0f)
                        {
                            curBullet->angle = -curBullet->angle - ZUN_PI;
                            curBullet->angle = utils::AddNormalizeAngle(curBullet->angle, 0.0);
                        }

                        if (curBullet->pos.y < 0.0f || curBullet->pos.y >= 448.0f)
                        {
                            curBullet->angle = -curBullet->angle;
                        }

                        curBullet->speed = curBullet->dirChangeSpeed;
                        bulletSpeed = curBullet->speed;
                        sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed);
                        curBullet->dirChangeNumTimes++;

                        if (curBullet->dirChangeNumTimes >= curBullet->dirChangeMaxTimes)
                        {
                            curBullet->exFlags &= ~0x400;
                        }
                    }
                }
                else if (curBullet->exFlags & 0x800)
                {
                    if (g_GameManager.IsInBounds(curBullet->pos.x, curBullet->pos.y,
                                                 curBullet->sprites.spriteBullet.sprite->widthPx,
                                                 curBullet->sprites.spriteBullet.sprite->heightPx) == 0)
                    {
                        if (curBullet->pos.x < 0.0f || curBullet->pos.x >= 384.0f)
                        {
                            curBullet->angle = -curBullet->angle - ZUN_PI;
                            curBullet->angle = utils::AddNormalizeAngle(curBullet->angle, 0.0f);
                        }

                        if (curBullet->pos.y < 0.0f)
                        {
                            curBullet->angle = -curBullet->angle;
                        }

                        curBullet->speed = curBullet->dirChangeSpeed;
                        bulletSpeed = curBullet->speed;
                        sincosmul(&curBullet->velocity, curBullet->angle, bulletSpeed);
                        curBullet->dirChangeNumTimes++;

                        if (curBullet->dirChangeNumTimes >= curBullet->dirChangeMaxTimes)
                        {
                            curBullet->exFlags &= ~0x800;
                        }
                    }
                }
            }

            curBullet->pos += curBullet->velocity * g_Supervisor.effectiveFramerateMultiplier;
            if (g_GameManager.IsInBounds(curBullet->pos.x, curBullet->pos.y,
                                         curBullet->sprites.spriteBullet.sprite->widthPx,
                                         curBullet->sprites.spriteBullet.sprite->heightPx) == 0)
            {
                if ((curBullet->exFlags & 0x40) == 0 && (curBullet->exFlags & 0x100) == 0 &&
                    (curBullet->exFlags & 0x80) == 0 && (curBullet->exFlags & 0x400) == 0 &&
                    (curBullet->exFlags & 0x800) == 0 && curBullet->unk_5c0 == 0)
                {
                    memset(curBullet, 0, sizeof(Bullet));
                    continue;
                }
                else
                {
                    curBullet->unk_5c0++;

                    if (curBullet->unk_5c0 >= 0x100)
                    {
                        memset(curBullet, 0, sizeof(Bullet));
                        continue;
                    }
                }
            }
            else
            {
                curBullet->unk_5c0 = 0;
            }

            if (curBullet->isGrazed == 0)
            {
                grazeState = g_Player.CheckGraze(&curBullet->pos, &curBullet->sprites.grazeSize);

                if (grazeState == 1)
                {
                    curBullet->isGrazed = 1;
                    goto bulletGrazed;
                }
                else if (grazeState == 2)
                {
                    curBullet->state = 5;
                    g_ItemManager.SpawnItem(&curBullet->pos, ITEM_POINT_BULLET, 1);
                }
            }
            else if (curBullet->isGrazed == 1)
            {
            bulletGrazed:
                grazeState = g_Player.CalcKillBoxCollision(&curBullet->pos, &curBullet->sprites.grazeSize);
                if (grazeState != 0)
                {
                    curBullet->state = 5;
                    if (grazeState == 2)
                    {
                        g_ItemManager.SpawnItem(&curBullet->pos, ITEM_POINT_BULLET, 1);
                    }
                }
            }
            g_AnmManager->ExecuteScript(&curBullet->sprites.spriteBullet);
            break;
        case 5:
            curBullet->pos += curBullet->velocity / 2.0f * g_Supervisor.effectiveFramerateMultiplier;
            if (g_AnmManager->ExecuteScript(&curBullet->sprites.spriteSpawnEffectDonut) != 0)
            {
                memset(curBullet, 0, sizeof(Bullet));
                continue;
            }
            break;
        }
        curBullet->timer.Tick();
    }

    curLaser = &mgr->lasers[0];
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->lasers); idx++, curLaser++)
    {
        if (!curLaser->inUse)
        {
            continue;
        }

        curLaser->endOffset += g_Supervisor.effectiveFramerateMultiplier * curLaser->speed;

        if (curLaser->startLength < curLaser->endOffset - curLaser->startOffset)
        {
            curLaser->startOffset = curLaser->endOffset - curLaser->startLength;
        }

        if (curLaser->startOffset < 0.0f)
        {
            curLaser->startOffset = 0.0f;
        }

        laserSize.y = curLaser->width / 2.0f;
        laserSize.x = curLaser->endOffset - curLaser->startOffset;
        laserCenter.x = (curLaser->endOffset - curLaser->startOffset) / 2.0f + curLaser->startOffset + curLaser->pos.x;
        laserCenter.y = curLaser->pos.y;
        curLaser->vm0.scaleX = curLaser->width / curLaser->vm0.sprite->widthPx;
        local_14 = curLaser->endOffset - curLaser->startOffset;
        curLaser->vm0.scaleY = local_14 / curLaser->vm0.sprite->heightPx;
        curLaser->vm0.rotation.z = ZUN_PI / 2.0f - curLaser->angle;

        switch (curLaser->state)
        {
        case 0:
            if (curLaser->flags & 1)
            {
                laserColor = curLaser->timer.AsFramesFloat() * 255.0f / curLaser->startTime;

                if (255 < laserColor)
                {
                    laserColor = 255;
                }

                curLaser->vm0.color = laserColor << 24;
            }
            else
            {
                res = ZUN_MIN(curLaser->startTime, 30);
                if (curLaser->startTime - res < curLaser->timer.AsFrames())
                {
                    local_14 = curLaser->timer.AsFramesFloat() * curLaser->width / curLaser->startTime;
                }
                else
                {
                    local_14 = 1.2f;
                }

                curLaser->vm0.scaleX = local_14 / 16.0f;
                laserSize.x = local_14 / 2.0f;
            }

            if ((ZunBool)(curLaser->timer.current >= curLaser->grazeDelay))
            {
                g_Player.CalcLaserHitbox(&laserCenter, &laserSize, &curLaser->pos, curLaser->angle,
                                         curLaser->timer.AsFrames() % 12 == 0);
            }

            if ((ZunBool)(curLaser->timer.current < curLaser->startTime))
            {
                break;
            }

            curLaser->timer.InitializeForPopup();
            curLaser->state++;
        case 1:
            g_Player.CalcLaserHitbox(&laserCenter, &laserSize, &curLaser->pos, curLaser->angle,
                                     curLaser->timer.AsFrames() % 12 == 0);

            if ((ZunBool)(curLaser->timer.current < curLaser->duration))
            {
                break;
            }

            curLaser->timer.InitializeForPopup();
            curLaser->state++;

            if (curLaser->endTime == 0)
            {
                curLaser->inUse = 0;
                continue;
            }
        case 2:
            if (curLaser->flags & 1)
            {
                laserColor = curLaser->timer.AsFramesFloat() * 255.0f / curLaser->startTime;

                if (255 < laserColor)
                {
                    laserColor = 255;
                }

                curLaser->vm0.color = laserColor << 24;
            }
            else
            {
                if (0 < curLaser->endTime)
                {
                    local_14 =
                        curLaser->width - (curLaser->timer.AsFramesFloat() * curLaser->width) / curLaser->endTime;
                    curLaser->vm0.scaleX = local_14 / 16.0f;
                    laserSize.x = local_14 / 2.0f;
                }
            }

            if ((ZunBool)(curLaser->timer.current < curLaser->grazeInterval))
            {
                g_Player.CalcLaserHitbox(&laserCenter, &laserSize, &curLaser->pos, curLaser->angle,
                                         curLaser->timer.AsFrames() % 12 == 0);
            }

            if ((ZunBool)(curLaser->timer.current < curLaser->endTime))
            {
                break;
            }

            curLaser->inUse = 0;
            continue;
        }

        if (curLaser->startOffset >= 640.0f)
        {
            curLaser->inUse = 0;
        }

        curLaser->timer.Tick();
        g_AnmManager->ExecuteScript(&curLaser->vm0);
    }

    mgr->time.Tick();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult BulletManager::OnDraw(BulletManager *mgr)
{
    i32 idx;
    f32 sine;
    Laser *curLaser;
    f32 laserOffset;
    f32 cosine;
    Bullet *curBullet1;
    Bullet *curBullet2;

    g_glFuncTable.glDepthFunc(GL_ALWAYS);

    for (curLaser = &mgr->lasers[0], idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->lasers); idx++, curLaser++)
    {
        if (!curLaser->inUse)
        {
            continue;
        }
        fsincos_wrapper(&sine, &cosine, curLaser->angle);
        laserOffset = (curLaser->endOffset - curLaser->startOffset) / 2.0f + curLaser->startOffset;
        curLaser->vm0.pos.x = cosine * laserOffset + curLaser->pos.x;
        curLaser->vm0.pos.y = sine * laserOffset + curLaser->pos.y;
        curLaser->vm0.pos.z = 0.0f;
        curLaser->color = COLOR_COMBINE_ALPHA(COLOR_WHITE, curLaser->color);
        g_AnmManager->Draw3(&curLaser->vm0);

        if (curLaser->startOffset < 16.0f || curLaser->speed == 0.0f)
        {
            curLaser->vm1.pos.x = cosine * curLaser->startOffset + curLaser->pos.x;
            curLaser->vm1.pos.y = sine * curLaser->startOffset + curLaser->pos.y;
            curLaser->vm1.pos.z = 0.0f;
            curLaser->vm1.color = curLaser->vm0.color;
            curLaser->vm1.flags.colorOp = AnmVmColorOp_Add;
            curLaser->vm1.color = COLOR_SET_ALPHA2(curLaser->vm1.color, 0xff);
            curLaser->vm1.scaleX = (curLaser->width / 10.0f) * ((16.0f - curLaser->startOffset) / 16.0f);
            curLaser->vm1.scaleY = curLaser->vm1.scaleX;

            if (curLaser->vm1.scaleY < 0.0f)
            {
                curLaser->vm1.scaleX = curLaser->width / 10.0f;
                curLaser->vm1.scaleY = curLaser->vm1.scaleX;
            }

            g_AnmManager->Draw3(&curLaser->vm1);
        }
    }

    g_ItemManager.OnDraw();

    if (g_Supervisor.hasD3dHardwareVertexProcessing)
    {
        for (curBullet1 = &mgr->bullets[0], idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->bullets); idx++, curBullet1++)
        {
            if (curBullet1->state == 0)
            {
                continue;
            }

            if (curBullet1->sprites.bulletHeight > 16)
            {
                BulletManager::DrawBullet(curBullet1);
            }
        }

        for (curBullet1 = &mgr->bullets[0], idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->bullets); idx++, curBullet1++)
        {
            if (curBullet1->state == 0)
            {
                continue;
            }

            if (curBullet1->sprites.bulletHeight == 16 &&
                (curBullet1->sprites.spriteBullet.anmFileIndex == ANM_SCRIPT_BULLET3_RING_BALL ||
                 curBullet1->sprites.spriteBullet.anmFileIndex == ANM_SCRIPT_BULLET3_BALL))
            {
                BulletManager::DrawBullet(curBullet1);
            }
        }

        for (curBullet1 = &mgr->bullets[0], idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->bullets); idx++, curBullet1++)
        {
            if (curBullet1->state == 0)
            {
                continue;
            }

            if (curBullet1->sprites.bulletHeight == 16 &&
                curBullet1->sprites.spriteBullet.anmFileIndex != ANM_SCRIPT_BULLET3_RING_BALL &&
                curBullet1->sprites.spriteBullet.anmFileIndex != ANM_SCRIPT_BULLET3_BALL)
            {
                BulletManager::DrawBullet(curBullet1);
            }
        }

        for (curBullet1 = &mgr->bullets[0], idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->bullets); idx++, curBullet1++)
        {
            if (curBullet1->state == 0)
            {
                continue;
            }

            if (curBullet1->sprites.bulletHeight == 8)
            {
                BulletManager::DrawBullet(curBullet1);
            }
        }
    }
    else
    {
        for (curBullet2 = &mgr->bullets[0], idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->bullets); idx++, curBullet2++)
        {
            if (curBullet2->state == 0)
            {
                continue;
            }

            if (curBullet2->sprites.bulletHeight > 16)
            {
                BulletManager::DrawBulletNoHwVertex(curBullet2);
            }
        }

        for (curBullet2 = &mgr->bullets[0], idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->bullets); idx++, curBullet2++)
        {
            if (curBullet2->state == 0)
            {
                continue;
            }

            if (curBullet2->sprites.bulletHeight == 16 &&
                (curBullet2->sprites.spriteBullet.anmFileIndex == ANM_SCRIPT_BULLET3_RING_BALL ||
                 curBullet2->sprites.spriteBullet.anmFileIndex == ANM_SCRIPT_BULLET3_BALL))
            {
                BulletManager::DrawBulletNoHwVertex(curBullet2);
            }
        }

        for (curBullet2 = &mgr->bullets[0], idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->bullets); idx++, curBullet2++)
        {
            if (curBullet2->state == 0)
            {
                continue;
            }

            if (curBullet2->sprites.bulletHeight == 16 &&
                curBullet2->sprites.spriteBullet.anmFileIndex != ANM_SCRIPT_BULLET3_RING_BALL &&
                curBullet2->sprites.spriteBullet.anmFileIndex != ANM_SCRIPT_BULLET3_BALL)
            {
                BulletManager::DrawBulletNoHwVertex(curBullet2);
            }
        }

        for (curBullet2 = &mgr->bullets[0], idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->bullets); idx++, curBullet2++)
        {
            if (curBullet2->state == 0)
            {
                continue;
            }

            if (curBullet2->sprites.bulletHeight == 8)
            {
                BulletManager::DrawBulletNoHwVertex(curBullet2);
            }
        }
    }

    g_glFuncTable.glDepthFunc(GL_LEQUAL);

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

void BulletManager::DrawBullet(Bullet *bullet)
{
    AnmVm *anmVm;

    switch (bullet->state)
    {
    case 2:
        anmVm = &bullet->sprites.spriteSpawnEffectFast;
        break;
    case 3:
        anmVm = &bullet->sprites.spriteSpawnEffectNormal;
        break;
    case 4:
        anmVm = &bullet->sprites.spriteSpawnEffectSlow;
        break;
    case 5:
        anmVm = &bullet->sprites.spriteSpawnEffectDonut;
        break;
    default:
        anmVm = &bullet->sprites.spriteBullet;
        break;
    }

    anmVm->pos.x = bullet->pos.x;
    anmVm->pos.y = bullet->pos.y;
    anmVm->pos.z = 0.0;
    anmVm->color = COLOR_COMBINE_ALPHA(COLOR_WHITE, anmVm->color);

    if (anmVm->autoRotate != 0)
    {
        anmVm->rotation.z = (ZUN_PI / 2.0f) - bullet->angle;
    }

    g_AnmManager->Draw2(anmVm);
}

void BulletManager::DrawBulletNoHwVertex(Bullet *bullet)
{
    AnmVm *anmVm;

    switch (bullet->state)
    {
    case 2:
        anmVm = &bullet->sprites.spriteSpawnEffectFast;
        break;
    case 3:
        anmVm = &bullet->sprites.spriteSpawnEffectNormal;
        break;
    case 4:
        anmVm = &bullet->sprites.spriteSpawnEffectSlow;
        break;
    case 5:
        anmVm = &bullet->sprites.spriteSpawnEffectDonut;
        break;
    default:
        anmVm = &bullet->sprites.spriteBullet;
        break;
    }

    anmVm->pos.x = g_GameManager.arcadeRegionTopLeftPos.x + bullet->pos.x;
    anmVm->pos.y = g_GameManager.arcadeRegionTopLeftPos.y + bullet->pos.y;
    anmVm->pos.z = 0.0;
    anmVm->color = COLOR_COMBINE_ALPHA(COLOR_WHITE, anmVm->color);

    if (anmVm->autoRotate != 0)
    {
        anmVm->rotation.z = (ZUN_PI / 2.0f) - bullet->angle;
    }

    g_AnmManager->Draw(anmVm);
}

ZunResult BulletManager::AddedCallback(BulletManager *mgr)
{
    u32 idx;

    if ((ZunBool)(g_Supervisor.curState != SUPERVISOR_STATE_GAMEMANAGER_REINIT))
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

    for (idx = 0; idx < 10; idx++)
    {
        g_AnmManager->SetAndExecuteScriptIdx(&mgr->bulletTypeTemplates[idx].spriteBullet,
                                             g_BulletTypeInfos[idx].bulletAnmScriptIdx);
        g_AnmManager->SetAndExecuteScriptIdx(&mgr->bulletTypeTemplates[idx].spriteSpawnEffectFast,
                                             g_BulletTypeInfos[idx].bulletSpawnEffectFastAnmScriptIdx);
        g_AnmManager->SetAndExecuteScriptIdx(&mgr->bulletTypeTemplates[idx].spriteSpawnEffectNormal,
                                             g_BulletTypeInfos[idx].bulletSpawnEffectNormalAnmScriptIdx);
        g_AnmManager->SetAndExecuteScriptIdx(&mgr->bulletTypeTemplates[idx].spriteSpawnEffectSlow,
                                             g_BulletTypeInfos[idx].bulletSpawnEffectSlowAnmScriptIdx);
        g_AnmManager->SetAndExecuteScriptIdx(&mgr->bulletTypeTemplates[idx].spriteSpawnEffectDonut,
                                             g_BulletTypeInfos[idx].bulletSpawnEffectDonutAnmScriptIdx);
        mgr->bulletTypeTemplates[idx].spriteBullet.baseSpriteIndex =
            mgr->bulletTypeTemplates[idx].spriteBullet.activeSpriteIndex;
        mgr->bulletTypeTemplates[idx].bulletHeight = mgr->bulletTypeTemplates[idx].spriteBullet.sprite->heightPx;

        if (mgr->bulletTypeTemplates[idx].spriteBullet.sprite->heightPx <= 8.0f)
        {
            mgr->bulletTypeTemplates[idx].grazeSize.x = 4.0f;
            mgr->bulletTypeTemplates[idx].grazeSize.y = 4.0f;
        }
        else if (mgr->bulletTypeTemplates[idx].spriteBullet.sprite->heightPx <= 16.0f)
        {
            switch (g_BulletTypeInfos[idx].bulletAnmScriptIdx)
            {
            case ANM_SCRIPT_BULLET3_RICE:
                mgr->bulletTypeTemplates[idx].grazeSize.x = 4.0f;
                mgr->bulletTypeTemplates[idx].grazeSize.y = 4.0f;
                break;
            case ANM_SCRIPT_BULLET3_KUNAI:
                mgr->bulletTypeTemplates[idx].grazeSize.x = 5.0f;
                mgr->bulletTypeTemplates[idx].grazeSize.y = 5.0f;
                break;
            case ANM_SCRIPT_BULLET3_SHARD:
                mgr->bulletTypeTemplates[idx].grazeSize.x = 4.0f;
                mgr->bulletTypeTemplates[idx].grazeSize.y = 4.0f;
                break;
            default:
                mgr->bulletTypeTemplates[idx].grazeSize.x = 6.0f;
                mgr->bulletTypeTemplates[idx].grazeSize.y = 6.0f;
                break;
            }
        }
        else if (mgr->bulletTypeTemplates[idx].spriteBullet.sprite->heightPx <= 32.0f)
        {
            switch (g_BulletTypeInfos[idx].bulletAnmScriptIdx)
            {
            case ANM_SCRIPT_BULLET3_FIREBALL:
                mgr->bulletTypeTemplates[idx].grazeSize.x = 11.0f;
                mgr->bulletTypeTemplates[idx].grazeSize.y = 11.0f;
                break;
            case ANM_SCRIPT_BULLET3_DAGGER:
                mgr->bulletTypeTemplates[idx].grazeSize.x = 9.0f;
                mgr->bulletTypeTemplates[idx].grazeSize.y = 9.0f;
                break;
            default:
                mgr->bulletTypeTemplates[idx].grazeSize.x = 16.0f;
                mgr->bulletTypeTemplates[idx].grazeSize.y = 16.0f;
            }
        }
        else
        {
            mgr->bulletTypeTemplates[idx].grazeSize.x = 32.0f;
            mgr->bulletTypeTemplates[idx].grazeSize.y = 32.0f;
        }
    }

    memset(&g_ItemManager, 0, sizeof(ItemManager));
    return ZUN_SUCCESS;
}

ZunResult BulletManager::DeletedCallback(BulletManager *arg)
{
    if ((i32)(g_Supervisor.curState != SUPERVISOR_STATE_GAMEMANAGER_REINIT))
    {
        g_AnmManager->ReleaseAnm(ANM_FILE_BULLET3);
        g_AnmManager->ReleaseAnm(ANM_FILE_BULLET4);
    }

    return ZUN_SUCCESS;
}

void BulletManager::CutChain()
{
    g_Chain.Cut(&g_BulletManagerCalcChain);
    g_Chain.Cut(&g_BulletManagerDrawChain);
}
}; // namespace th06
