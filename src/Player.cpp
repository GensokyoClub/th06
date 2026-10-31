#include "stdafx.h"

#include "Player.hpp"

#include "BombData.hpp"
#include "BulletData.hpp"
#include "ChainPriorities.hpp"
#include "EffectManager.hpp"
#include "EnemyManager.hpp"
#include "Gui.hpp"
#include "ScreenEffect.hpp"
#include "ItemManager.hpp"
#include "BulletManager.hpp"
#include "SoundPlayer.hpp"

namespace th06
{
static BombData g_BombData[] = {
    /* ReimuA  */ {BombReimuACalc, BombReimuADraw},
    /* ReimuB  */ {BombReimuBCalc, BombReimuBDraw},
    /* MarisaA */ {BombMarisaACalc, BombMarisaADraw},
    /* MarisaB */ {BombMarisaBCalc, BombMarisaBDraw},
};

Player g_Player;

#pragma var_order(bulletData, bulletFrame, pad)
static FireBulletResult FireSingleBullet(Player *player, PlayerBullet *bullet, i32 bulletIdx, i32 framesSinceLastBullet,
                                         CharacterPowerData *powerData)
{
    CharacterPowerBulletData *bulletData;
    i32 bulletFrame;
    i32 pad[3];

    while (g_GameManager.currentPower >= powerData->power)
    {
        powerData++;
    }

    bulletData = powerData->bullets + bulletIdx;

    if (bulletData->bulletType == PLAYER_LASER)
    {
        bulletFrame = bulletData->bulletFrame;
        if (!(i32)player->laserTimer[bulletFrame])
        {
            player->laserTimer[bulletFrame] = bulletData->waitBetweenBullets;

            bullet->bulletFrame = bulletFrame;
            bullet->spawnPositionIdx = bulletData->spawnPositionIdx;
            bullet->motion.x = bulletData->motion.x;
            bullet->motion.y = bulletData->motion.y;
            goto SHOOT_BULLET;
        }
    }
    else if (framesSinceLastBullet % bulletData->waitBetweenBullets == bulletData->bulletFrame)
    {
    SHOOT_BULLET:

        g_AnmManager->SetAndExecuteScriptIdx(&bullet->sprite, bulletData->anmFileIdx);
        if (!bulletData->spawnPositionIdx)
        {
            bullet->position = player->positionCenter;
        }
        else
        {
            bullet->position = player->orbsPosition[bulletData->spawnPositionIdx - 1];
        }
        bullet->position[0] += bulletData->motion.x;
        bullet->position[1] += bulletData->motion.y;

        bullet->position[2] = 0.495f;

        bullet->size.x = bulletData->size.x;
        bullet->size.y = bulletData->size.y;
        bullet->size.z = 1.0f;
        bullet->angle = bulletData->direction;
        bullet->speed = bulletData->velocity;

        bullet->velocity.x = cosf(bulletData->direction) * bulletData->velocity;
        bullet->velocity.y = sinf(bulletData->direction) * bulletData->velocity;

        bullet->lifetime = 0;

        bullet->bulletType = bulletData->bulletType;
        bullet->damage = bulletData->damage;
        if (bulletData->bulletSoundIdx >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx((SoundIdx)bulletData->bulletSoundIdx);
        }

        return bulletIdx >= powerData->numBullets - 1;
    }

    if (bulletIdx >= powerData->numBullets - 1)
    {
        return FBR_STOP_SPAWNING;
    }
    else
    {
        return FBR_SPAWN_MORE;
    }
}

static FireBulletResult FireBulletReimuA(Player *player, PlayerBullet *bullet, u32 bulletIdx, u32 framesSinceLastBullet)
{
    return FireSingleBullet(player, bullet, bulletIdx, framesSinceLastBullet, g_CharacterPowerDataReimuA);
}

static FireBulletResult FireBulletReimuB(Player *player, PlayerBullet *bullet, u32 bulletIdx, u32 framesSinceLastBullet)
{
    return FireSingleBullet(player, bullet, bulletIdx, framesSinceLastBullet, g_CharacterPowerDataReimuB);
}

static FireBulletResult FireBulletMarisaA(Player *player, PlayerBullet *bullet, u32 bulletIdx,
                                          u32 framesSinceLastBullet)
{
    return FireSingleBullet(player, bullet, bulletIdx, framesSinceLastBullet, g_CharacterPowerDataMarisaA);
}

static FireBulletResult FireBulletMarisaB(Player *player, PlayerBullet *bullet, u32 bulletIdx,
                                          u32 framesSinceLastBullet)
{
    return FireSingleBullet(player, bullet, bulletIdx, framesSinceLastBullet, g_CharacterPowerDataMarisaB);
}

CharacterData g_CharData[] = {
    /* ReimuA  */ {4.0f, 2.0f, 4.0f, 2.0f, FireBulletReimuA, FireBulletReimuA},
    /* ReimuB  */ {4.0f, 2.0f, 4.0f, 2.0f, FireBulletReimuB, FireBulletReimuB},
    /* MarisaA */ {5.0f, 2.5f, 5.0f, 2.5f, FireBulletMarisaA, FireBulletMarisaA},
    /* MarisaB */ {5.0f, 2.5f, 5.0f, 2.5f, FireBulletMarisaB, FireBulletMarisaB},
    /* Rin???  */ {4.0f, 2.0f, 4.0f, 2.0f, NULL, NULL},
};

#pragma var_order(idx, curBulletIdx, curBullet)
static void SpawnBullets(Player *p, u32 timer)
{
    u32 idx = 0;
    PlayerBullet *curBullet = p->bullets;
    i32 curBulletIdx = 0;
    for (; curBulletIdx < MAX_PLAYER_BULLETS; curBulletIdx++, curBullet++)
    {
        if (curBullet->bulletState != PLAYER_BULLET_STATE_INACTIVE)
        {
            continue;
        }
    WHILE_LOOP:
        FireBulletResult bulletResult;
        if (!p->isFocus)
        {
            bulletResult = p->fireBulletCallback(p, curBullet, idx, timer);
        }
        else
        {
            bulletResult = p->fireBulletFocusCallback(p, curBullet, idx, timer);
        }
        if (bulletResult >= 0)
        {
            curBullet->sprite.pos.x = curBullet->position.x;
            curBullet->sprite.pos.y = curBullet->position.y;
            curBullet->sprite.pos[2] = 0.495f;
            curBullet->bulletState = PLAYER_BULLET_STATE_FIRED;
        }
        if (bulletResult == FBR_STOP_SPAWNING)
        {
            return;
        }
        if (bulletResult > 0)
        {
            return;
        }
        idx++;
        if (bulletResult == FBR_SPAWN_MORE)
        {
            goto WHILE_LOOP;
        }
    }
}

#pragma var_order(vectorY, vectorX, idx, vecLength, bullet)
static void UpdatePlayerBullets(Player *player)
{
    f32 vectorX;
    f32 vectorY;
    PlayerBullet *bullet;
    f32 vecLength;
    i32 idx;

    for (idx = 0; idx < ARRAY_SIZE_SIGNED(player->laserTimer); idx++)
    {
        if ((i32)player->laserTimer[idx] != 0)
        {
            player->laserTimer[idx]--;
        }
    }
    bullet = &player->bullets[0];
    for (idx = 0; idx < MAX_PLAYER_BULLETS; idx++, bullet++)
    {
        if (bullet->bulletState == PLAYER_BULLET_STATE_INACTIVE)
        {
            continue;
        }

        switch (bullet->bulletType)
        {
        case PLAYER_BULLET_TYPE_1:
            if (bullet->bulletState == PLAYER_BULLET_STATE_FIRED)
            {
                if (player->positionOfLastEnemyHit.x > -100.0f && (i32)bullet->lifetime < 40 &&
                    bullet->lifetime.HasTicked())
                {
                    vectorX = player->positionOfLastEnemyHit.x - bullet->position[0];
                    vectorY = player->positionOfLastEnemyHit.y - bullet->position[1];

                    vecLength = sqrtf(ZUN_SQUARE(vectorX) + ZUN_SQUARE(vectorY)) / (bullet->speed / 4.0f);
                    if (vecLength < 1.0f)
                    {
                        vecLength = 1.0f;
                    }

                    vectorX = vectorX / vecLength + bullet->velocity.x;
                    vectorY = vectorY / vecLength + bullet->velocity.y;

                    vecLength = sqrtf(ZUN_SQUARE(vectorX) + ZUN_SQUARE(vectorY));

                    bullet->speed = ZUN_MIN(vecLength, 10.0f);

                    if (bullet->speed < 1.0f)
                    {
                        bullet->speed = 1.0f;
                    }

                    bullet->velocity.x = vectorX * bullet->speed / vecLength;
                    bullet->velocity.y = vectorY * bullet->speed / vecLength;
                }
                else
                {
                    if (bullet->speed < 10.0f)
                    {
                        bullet->speed += 0.33333333f;
                        vectorX = bullet->velocity.x;
                        vectorY = bullet->velocity.y;
                        vecLength = sqrtf(ZUN_SQUARE(vectorX) + ZUN_SQUARE(vectorY));
                        bullet->velocity.x = vectorX * bullet->speed / vecLength;
                        bullet->velocity.y = vectorY * bullet->speed / vecLength;
                    }
                }
            }

            break;

        case PLAYER_BULLET_TYPE_2:
            if (bullet->bulletState == PLAYER_BULLET_STATE_FIRED)
            {
                bullet->velocity.y -= 0.3f;
            }
            break;
        case PLAYER_LASER:

            if (player->laserTimer[bullet->bulletFrame] == 70)
            {
                bullet->sprite.pendingInterrupt = 1;
            }
            else if (player->laserTimer[bullet->bulletFrame] == 1)
            {
                bullet->sprite.pendingInterrupt = 1;
            }

            bullet->position = player->orbsPosition[bullet->spawnPositionIdx - 1];

            bullet->position.x += bullet->motion.x;
            bullet->position.y /= 2.0f;
            bullet->position.z = 0.44f;

            bullet->sprite.scaleY = bullet->position.y * 2.0f / 14.0f;

            bullet->size.y = bullet->position.y * 2.0f;
            break;
        }

        bullet->sprite.pos.x = bullet->position[0] += bullet->velocity.x * g_Supervisor.effectiveFramerateMultiplier;
        bullet->sprite.pos.y = bullet->position[1] += bullet->velocity.y * g_Supervisor.effectiveFramerateMultiplier;
        bullet->sprite.pos.z = bullet->position.z;

        if (bullet->bulletType != PLAYER_LASER &&
            !g_GameManager.IsInBounds(bullet->position[0], bullet->position[1], bullet->sprite.sprite->widthPx,
                                      bullet->sprite.sprite->heightPx))
        {
            bullet->bulletState = PLAYER_BULLET_STATE_INACTIVE;
        }

        if (g_AnmManager->ExecuteScript(&bullet->sprite))
        {
            bullet->bulletState = PLAYER_BULLET_STATE_INACTIVE;
        }
        bullet->lifetime++;
    }
}

#pragma var_order(i, bullet)
static void DrawBullets(Player *p)
{
    PlayerBullet *bullet = p->bullets;
    i32 i = 0;
    for (; i < MAX_PLAYER_BULLETS; i++, bullet++)
    {
        if (bullet->bulletState != PLAYER_BULLET_STATE_FIRED)
        {
            continue;
        }
        if (bullet->sprite.autoRotate)
        {
            bullet->sprite.rotation.z = RADIANS(90.0f) - utils::AddNormalizeAngle(bullet->angle, RADIANS(180.0f));
        }
        g_AnmManager->Draw2(&bullet->sprite);
    }
}

#pragma var_order(i, bullet)
static void DrawBulletExplosions(Player *p)
{
    PlayerBullet *bullet = p->bullets;
    i32 i = 0;
    for (; i < MAX_PLAYER_BULLETS; i++, bullet++)
    {
        if (bullet->bulletState != PLAYER_BULLET_STATE_COLLIDED)
        {
            continue;
        }
        if (bullet->sprite.autoRotate)
        {
            bullet->sprite.rotation.z = RADIANS(90.0f) - utils::AddNormalizeAngle(bullet->angle, RADIANS(180.0f));
        }
        bullet->sprite.pos[2] = 0.4f;
        g_AnmManager->Draw2(&bullet->sprite);
    }
}

static ZunResult UpdateFireBulletsTimer(Player *p)
{
    if ((i32)p->fireBulletTimer < 0)
    {
        return ZUN_SUCCESS;
    }

    if (p->fireBulletTimer.HasTicked() &&
        (!g_Player.bombInfo.isInUse || g_GameManager.character != MARISA || g_GameManager.shotType != SHOT_TYPE_B))
    {
        SpawnBullets(p, (i32)p->fireBulletTimer);
    }

    p->fireBulletTimer++;

    if ((i32)p->fireBulletTimer >= 30 || p->playerState == PLAYER_STATE_DEAD || p->playerState == PLAYER_STATE_SPAWNING)
    {
        p->fireBulletTimer = -1;
    }
    return ZUN_SUCCESS;
}

#pragma var_order(bullet, idx, enemyBottomRight, bulletBottomRight, enemyTopLeft, damage, bulletTopLeft)
i32 Player::CalcDamageToEnemy(D3DXVECTOR3 *enemyPos, D3DXVECTOR3 *enemyHitboxSize, ZunBool *hitByBomb)
{
    D3DXVECTOR3 bulletTopLeft;
    i32 damage;
    D3DXVECTOR3 enemyTopLeft;
    i32 idx;
    PlayerBullet *bullet;

    D3DXVECTOR3 bulletBottomRight;
    D3DXVECTOR3 enemyBottomRight;

    damage = 0;

    SetVecCorners(&enemyTopLeft, &enemyBottomRight, enemyPos, enemyHitboxSize);
    bullet = &this->bullets[0];
    if (hitByBomb)
    {
        *hitByBomb = false;
    }
    for (idx = 0; idx < MAX_PLAYER_BULLETS; idx++, bullet++)
    {
        if (bullet->bulletState == PLAYER_BULLET_STATE_INACTIVE ||
            bullet->bulletState != PLAYER_BULLET_STATE_FIRED && bullet->bulletType != PLAYER_BULLET_TYPE_2)
        {
            continue;
        }

        SetVecCorners(&bulletTopLeft, &bulletBottomRight, &bullet->position, &bullet->size);

        if (bulletTopLeft.y > enemyBottomRight.y || bulletTopLeft.x > enemyBottomRight.x ||
            bulletBottomRight.y < enemyTopLeft.y || bulletBottomRight.x < enemyTopLeft.x)
        {
            continue;
        }
        /* Bullet is hitting the enemy */
        if (!this->bombInfo.isInUse)
        {
            damage += bullet->damage;
        }
        else
        {
            damage += bullet->damage / 3 != 0 ? bullet->damage / 3 : 1;
        }

        if (bullet->bulletType == PLAYER_BULLET_TYPE_2)
        {
            bullet->damage = bullet->damage / 4;
            if (bullet->damage == 0)
            {
                bullet->damage = 1;
            }
            switch (bullet->sprite.anmFileIndex)
            {
            case ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1:
                bullet->size.x = 32.0f;
                bullet->size.y = 32.0f;
                break;
            case ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2:
                bullet->size.x = 42.0f;
                bullet->size.y = 42.0f;
                break;
            case ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3:
                bullet->size.x = 48.0f;
                bullet->size.y = 48.0f;
                break;
            case ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4:
                bullet->size.x = 48.0f;
                bullet->size.y = 48.0f;
            }
            if (MULTIPLE_OF(bullet->lifetime, 6))
            {
                g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_5, &bullet->position, 1, COLOR_WHITE);
            }
        }

        if (bullet->bulletType != PLAYER_LASER)
        {
            if (bullet->bulletState == PLAYER_BULLET_STATE_FIRED)
            {
                g_AnmManager->SetAndExecuteScriptIdx(&bullet->sprite, bullet->sprite.anmFileIndex + 32);
                g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_5, &bullet->position, 1, COLOR_WHITE);
                bullet->position[2] = 0.1f;
            }
            bullet->bulletState = PLAYER_BULLET_STATE_COLLIDED;
            bullet->velocity.x /= 8.0f;
            bullet->velocity.y /= 8.0f;
        }
        else
        {
            this->particleTimer++;
            if (this->particleTimer % 8 == 0)
            {
                bulletTopLeft = *enemyPos;
                bulletTopLeft.x = bullet->position.x;

                g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_5, &bulletTopLeft, 1, COLOR_WHITE);
            }
        }
    }
    for (idx = 0; idx < PLAYER_BOMB_REGION_COUNT; idx++)
    {
        if (this->bombRegionSizes[idx].x <= 0.0f)
        {
            continue;
        }

        bulletTopLeft = this->bombRegionPositions[idx] - this->bombRegionSizes[idx] / 2.0f;
        bulletBottomRight = this->bombRegionPositions[idx] + this->bombRegionSizes[idx] / 2.0f;
        if (bulletTopLeft.x > enemyBottomRight.x || bulletBottomRight.x < enemyTopLeft.x ||
            bulletTopLeft.y > enemyBottomRight.y || bulletBottomRight.y < enemyTopLeft.y)
        {
            continue;
        }
        damage += this->bombRegionDamages[idx];
        this->bombRegionTotalDamages[idx] += this->bombRegionDamages[idx];
        this->particleTimer++;
        if (this->particleTimer % 4 == 0)
        {
            g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_3, enemyPos, 1, COLOR_WHITE);
        }
        if (this->bombInfo.isInUse && hitByBomb)
        {
            *hitByBomb = true;
        }
    }
    return damage;
}

#pragma var_order(bombProjectileTopLeft, curBombIdx, bulletBottomRight, bulletTopLeft, curBombProjectile,              \
                  bombProjectileBottomRight)
i32 Player::CheckCollision(D3DXVECTOR3 *bulletCenter, D3DXVECTOR3 *bulletSize)
{
    PlayerRect *curBombProjectile;
    D3DXVECTOR3 bulletTopLeft, bulletBottomRight;
    D3DXVECTOR3 bombProjectileTopLeft, bombProjectileBottomRight;
    i32 curBombIdx;

    curBombProjectile = this->bombProjectiles;
    bulletTopLeft.x = bulletCenter->x - bulletSize->x / 2.0f;
    bulletTopLeft.y = bulletCenter->y - bulletSize->y / 2.0f;
    bulletBottomRight.x = bulletCenter->x + bulletSize->x / 2.0f;
    bulletBottomRight.y = bulletCenter->y + bulletSize->y / 2.0f;
    for (curBombIdx = 0; curBombIdx < ARRAY_SIZE_SIGNED(this->bombProjectiles); curBombIdx++, curBombProjectile++)
    {
        if (curBombProjectile->size.x == 0.0f)
        {
            continue;
        }
        bombProjectileTopLeft.x = curBombProjectile->pos.x - curBombProjectile->size.x / 2.0f;
        bombProjectileTopLeft.y = curBombProjectile->pos.y - curBombProjectile->size.y / 2.0f;
        bombProjectileBottomRight.x = curBombProjectile->pos.x + curBombProjectile->size.x / 2.0f;
        bombProjectileBottomRight.y = curBombProjectile->pos.y + curBombProjectile->size.y / 2.0f;
        if (!(bombProjectileTopLeft.x > bulletBottomRight.x || bombProjectileBottomRight.x < bulletTopLeft.x ||
              bombProjectileTopLeft.y > bulletBottomRight.y || bombProjectileBottomRight.y < bulletTopLeft.y))
        {
            return COLLISION_GRAZE;
        }
    }
    if (this->hitboxTopLeft.x > bulletBottomRight.x || this->hitboxTopLeft.y > bulletBottomRight.y ||
        this->hitboxBottomRight.x < bulletTopLeft.x || this->hitboxBottomRight.y < bulletTopLeft.y)
    {
        return COLLISION_NONE;
    }
    else if (this->playerState != PLAYER_STATE_ALIVE)
    {
        return COLLISION_HIT;
    }
    else
    {
        this->Die();
        return COLLISION_HIT;
    }
}

#pragma var_order(bombTopLeft, i, bulletBottomRight, bulletTopLeft, bombProjectile, bombBottomRight)
i32 Player::CheckGraze(D3DXVECTOR3 *center, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 bombBottomRight;
    PlayerRect *bombProjectile;
    D3DXVECTOR3 bombTopLeft;
    D3DXVECTOR3 bulletBottomRight;
    D3DXVECTOR3 bulletTopLeft;
    i32 i;

    bulletTopLeft.x = center->x - size->x / 2.0f - 20.0f;
    bulletTopLeft.y = center->y - size->y / 2.0f - 20.0f;
    bulletBottomRight.x = center->x + size->x / 2.0f + 20.0f;
    bulletBottomRight.y = center->y + size->y / 2.0f + 20.0f;
    bombProjectile = this->bombProjectiles;

    for (i = 0; i < ARRAY_SIZE_SIGNED(this->bombProjectiles); i++, bombProjectile++)
    {
        if (bombProjectile->size.x == 0.0f)
        {
            continue;
        }

        bombTopLeft.x = bombProjectile->pos.x - bombProjectile->size.x / 2.0f;
        bombTopLeft.y = bombProjectile->pos.y - bombProjectile->size.y / 2.0f;
        bombBottomRight.x = bombProjectile->size.x / 2.0f + bombProjectile->pos.x;
        bombBottomRight.y = bombProjectile->size.y / 2.0f + bombProjectile->pos.y;

        // Bomb clips bullet's hitbox, destroys bullet upon return
        if (!(bombTopLeft.x > bulletBottomRight.x || bombBottomRight.x < bulletTopLeft.x ||
              bombTopLeft.y > bulletBottomRight.y || bombBottomRight.y < bulletTopLeft.y))
        {
            return CANCELED_BY_BOMB;
        }
    }

    if (this->playerState == PLAYER_STATE_DEAD || this->playerState == PLAYER_STATE_SPAWNING)
    {
        return GRAZE_FALSE;
    }
    if (this->hitboxTopLeft.x > bulletBottomRight.x || this->hitboxBottomRight.x < bulletTopLeft.x ||
        this->hitboxTopLeft.y > bulletBottomRight.y || this->hitboxBottomRight.y < bulletTopLeft.y)
    {
        return GRAZE_FALSE;
    }

    // Bullet clips player's graze hitbox, add score and check for death upon return
    this->ScoreGraze(center);
    return GRAZE_TRUE;
}

#pragma var_order(itemBottomRight, itemTopLeft)
ZunBool Player::CalcItemBoxCollision(D3DXVECTOR3 *itemCenter, D3DXVECTOR3 *itemSize)
{
    if (this->playerState != PLAYER_STATE_ALIVE && this->playerState != PLAYER_STATE_INVULNERABLE)
    {
        return false;
    }

    D3DXVECTOR3 itemTopLeft;
    D3DXVECTOR3 itemBottomRight;

    itemTopLeft = *itemCenter - *itemSize / 2.0f;
    itemBottomRight = *itemCenter + *itemSize / 2.0f;

    if (this->grabItemTopLeft.x > itemBottomRight.x || this->grabItemBottomRight.x < itemTopLeft.x ||
        this->grabItemTopLeft.y > itemBottomRight.y || this->grabItemBottomRight.y < itemTopLeft.y)
    {
        return false;
    }
    else
    {
        return true;
    }
}

#pragma var_order(playerRelativeTopLeft, laserBottomRight, laserTopLeft, playerRelativeBottomRight)
i32 Player::CalcLaserHitbox(D3DXVECTOR3 *laserCenter, D3DXVECTOR3 *laserSize, D3DXVECTOR3 *rotation, f32 angle,
                            ZunBool canGraze)
{
    D3DXVECTOR3 laserTopLeft;
    D3DXVECTOR3 laserBottomRight;
    D3DXVECTOR3 playerRelativeTopLeft;
    D3DXVECTOR3 playerRelativeBottomRight;

    laserTopLeft = this->positionCenter - *rotation;
    utils::Rotate(&laserBottomRight, &laserTopLeft, angle);
    laserBottomRight.z = 0.0f;
    laserTopLeft = laserBottomRight + *rotation;
    playerRelativeTopLeft = laserTopLeft - this->hitboxSize;
    playerRelativeBottomRight = laserTopLeft + this->hitboxSize;

    laserTopLeft = *laserCenter - *laserSize / 2.0f;
    laserBottomRight = *laserCenter + *laserSize / 2.0f;

    if (!(playerRelativeTopLeft.x > laserBottomRight.x || playerRelativeBottomRight.x < laserTopLeft.x ||
          playerRelativeTopLeft.y > laserBottomRight.y || playerRelativeBottomRight.y < laserTopLeft.y))
    {
        goto LASER_COLLISION;
    }
    if (!canGraze)
    {
        return COLLISION_NONE;
    }

    laserTopLeft.x -= 48.0f;
    laserTopLeft.y -= 48.0f;
    laserBottomRight.x += 48.0f;
    laserBottomRight.y += 48.0f;

    if (playerRelativeTopLeft.x > laserBottomRight.x || playerRelativeBottomRight.x < laserTopLeft.x ||
        playerRelativeTopLeft.y > laserBottomRight.y || playerRelativeBottomRight.y < laserTopLeft.y)
    {
        return COLLISION_NONE;
    }
    if (this->playerState == PLAYER_STATE_DEAD || this->playerState == PLAYER_STATE_SPAWNING)
    {
        return COLLISION_NONE;
    }

    this->ScoreGraze(&this->positionCenter);
    return COLLISION_GRAZE;

LASER_COLLISION:
    if (this->playerState != PLAYER_STATE_ALIVE)
    {
        return COLLISION_NONE;
    }

    this->Die();
    return COLLISION_HIT;
}

void Player::ScoreGraze(D3DXVECTOR3 *center)
{
    D3DXVECTOR3 particlePosition;

#if BUILT_SINCE(102b)
    if (!g_Player.bombInfo.isInUse)
#endif
    {
        // Until 1.02b graze also counts while a bomb is active.
        if (g_GameManager.grazeInStage < 9999)
        {
            g_GameManager.grazeInStage++;
        }
        if (g_GameManager.grazeInTotal < 999999)
        {
            g_GameManager.grazeInTotal++;
        }
    }

    particlePosition = (this->positionCenter + *center) / 2.0f;
    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_8, &particlePosition, 1, COLOR_WHITE);
    g_GameManager.AddScore(500);
    g_GameManager.IncreaseSubrank(6);
    g_Gui.flags.flag3 = 2;
    g_SoundPlayer.PlaySoundByIdx(SOUND_GRAZE);
}

void Player::Die()
{
    g_EnemyManager.spellcardInfo.isCapturing = false;
    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &this->positionCenter, 1, COLOR_NEONBLUE);
    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_6, &this->positionCenter, 16, COLOR_WHITE);
    this->playerState = PLAYER_STATE_DEAD;
    this->invulnerabilityTimer = 0;
    g_SoundPlayer.PlaySoundByIdx(SOUND_PICHUN);
    g_GameManager.deaths++;
    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->laserTimer); i++)
    {
        this->laserTimer[i] = 2;
    }
}

static void StartFireBulletTimer(Player *p)
{
    if ((i32)p->fireBulletTimer < 0)
    {
        p->fireBulletTimer = 0;
    }
}

#pragma var_order(playerDirection, speedY, speedX, orbOffsetY, orbOffsetX)
ZunResult Player::HandlePlayerInputs()
{
    f32 orbOffsetX;
    f32 orbOffsetY;

    f32 speedX = 0.0f;
    f32 speedY = 0.0f;

    PlayerDirection playerDirection = this->playerDirection;

    this->playerDirection = MOVEMENT_NONE;
    if (IS_PRESSED(TH_BUTTON_UP))
    {
        this->playerDirection = MOVEMENT_UP;
        if (IS_PRESSED(TH_BUTTON_LEFT))
        {
            this->playerDirection = MOVEMENT_UP_LEFT;
        }
        if (IS_PRESSED(TH_BUTTON_RIGHT))
        {
            this->playerDirection = MOVEMENT_UP_RIGHT;
        }
    }
    else
    {
        if (IS_PRESSED(TH_BUTTON_DOWN))
        {
            this->playerDirection = MOVEMENT_DOWN;
            if (IS_PRESSED(TH_BUTTON_LEFT))
            {
                this->playerDirection = MOVEMENT_DOWN_LEFT;
            }
            if (IS_PRESSED(TH_BUTTON_RIGHT))
            {
                this->playerDirection = MOVEMENT_DOWN_RIGHT;
            }
        }
        else
        {
            if (IS_PRESSED(TH_BUTTON_LEFT))
            {
                this->playerDirection = MOVEMENT_LEFT;
            }
            if (IS_PRESSED(TH_BUTTON_RIGHT))
            {
                this->playerDirection = MOVEMENT_RIGHT;
            }
        }
    }
    if (IS_PRESSED(TH_BUTTON_FOCUS))
    {
        this->isFocus = true;
    }
    else
    {
        this->isFocus = false;
    }

    switch (this->playerDirection)
    {
    case MOVEMENT_RIGHT:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedX = this->characterData.orthogonalMovementSpeedFocus;
        }
        else
        {
            speedX = this->characterData.orthogonalMovementSpeed;
        }
        break;
    case MOVEMENT_LEFT:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedX = -this->characterData.orthogonalMovementSpeedFocus;
        }
        else
        {
            speedX = -this->characterData.orthogonalMovementSpeed;
        }
        break;
    case MOVEMENT_UP:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedY = -this->characterData.orthogonalMovementSpeedFocus;
        }
        else
        {
            speedY = -this->characterData.orthogonalMovementSpeed;
        }
        break;
    case MOVEMENT_DOWN:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedY = this->characterData.orthogonalMovementSpeedFocus;
        }
        else
        {
            speedY = this->characterData.orthogonalMovementSpeed;
        }
        break;
    case MOVEMENT_UP_LEFT:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedX = -this->characterData.diagonalMovementSpeedFocus;
        }
        else
        {
            speedX = -this->characterData.diagonalMovementSpeed;
        }
        speedY = speedX;
        break;
    case MOVEMENT_DOWN_LEFT:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedX = -this->characterData.diagonalMovementSpeedFocus;
        }
        else
        {
            speedX = -this->characterData.diagonalMovementSpeed;
        }
        speedY = -speedX;
        break;
    case MOVEMENT_UP_RIGHT:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedX = this->characterData.diagonalMovementSpeedFocus;
        }
        else
        {
            speedX = this->characterData.diagonalMovementSpeed;
        }
        speedY = -speedX;
        break;
    case MOVEMENT_DOWN_RIGHT:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedX = this->characterData.diagonalMovementSpeedFocus;
        }
        else
        {
            speedX = this->characterData.diagonalMovementSpeed;
        }
        speedY = speedX;
    }

    if (speedX < 0.0f && this->previousSpeed.x >= 0.0f)
    {
        g_AnmManager->SetAndExecuteScriptIdx(&this->playerSprite, ANM_SCRIPT_PLAYER_MOVING_LEFT);
    }
    else if (speedX == 0.0f && this->previousSpeed.x < 0.0f)
    {
        g_AnmManager->SetAndExecuteScriptIdx(&this->playerSprite, ANM_SCRIPT_PLAYER_STOPPING_LEFT);
    }

    if (speedX > 0.0f && this->previousSpeed.x <= 0.0f)
    {
        g_AnmManager->SetAndExecuteScriptIdx(&this->playerSprite, ANM_SCRIPT_PLAYER_MOVING_RIGHT);
    }
    else if (speedX == 0.0f && this->previousSpeed.x > 0.0f)
    {
        g_AnmManager->SetAndExecuteScriptIdx(&this->playerSprite, ANM_SCRIPT_PLAYER_STOPPING_RIGHT);
    }

    this->previousSpeed.x = speedX;
    this->previousSpeed.y = speedY;

    this->positionCenter[0] += speedX * this->speedMultiplierDuringBomb.x * g_Supervisor.effectiveFramerateMultiplier;
    this->positionCenter[1] += speedY * this->speedMultiplierDuringBomb.y * g_Supervisor.effectiveFramerateMultiplier;

    if (this->positionCenter[0] < g_GameManager.playerMovementAreaTopLeftPos.x)
    {
        this->positionCenter[0] = g_GameManager.playerMovementAreaTopLeftPos.x;
    }
    else if (this->positionCenter[0] >
             g_GameManager.playerMovementAreaTopLeftPos.x + g_GameManager.playerMovementAreaSize.x)
    {
        this->positionCenter[0] = g_GameManager.playerMovementAreaTopLeftPos.x + g_GameManager.playerMovementAreaSize.x;
    }

    if (this->positionCenter[1] < g_GameManager.playerMovementAreaTopLeftPos.y)
    {
        this->positionCenter[1] = g_GameManager.playerMovementAreaTopLeftPos.y;
    }
    else if (this->positionCenter[1] >
             g_GameManager.playerMovementAreaTopLeftPos.y + g_GameManager.playerMovementAreaSize.y)
    {
        this->positionCenter[1] = g_GameManager.playerMovementAreaTopLeftPos.y + g_GameManager.playerMovementAreaSize.y;
    }

    this->hitboxTopLeft = this->positionCenter - this->hitboxSize;

    this->hitboxBottomRight = this->positionCenter + this->hitboxSize;

    this->grabItemTopLeft = this->positionCenter - this->grabItemSize;

    this->grabItemBottomRight = this->positionCenter + this->grabItemSize;

    this->orbsPosition[0] = this->positionCenter;
    this->orbsPosition[1] = this->positionCenter;

    orbOffsetX = orbOffsetY = 0.0f;

    if (g_GameManager.currentPower < 8)
    {
        this->orbState = ORB_HIDDEN;
    }
    else if (this->orbState == ORB_HIDDEN)
    {
        this->orbState = ORB_UNFOCUSED;
    }

    switch (this->orbState)
    {
        float intermediateFloat;

    case ORB_HIDDEN:
        this->focusMovementTimer = 0;
        break;

    case ORB_UNFOCUSED:
        orbOffsetX = 24.0f;
        this->focusMovementTimer = 0;
        if (this->isFocus)
        {
            this->orbState = ORB_FOCUSING;
        }
        else
        {
            break;
        }

    CASE_ORB_FOCUSING:
    case ORB_FOCUSING:
        this->focusMovementTimer++;

        intermediateFloat = (f32)this->focusMovementTimer / 8.0f;
        orbOffsetY = (1.0f - intermediateFloat) * 32.0f + -32.0f;
        intermediateFloat *= intermediateFloat;
        orbOffsetX = -16.0f * intermediateFloat + 24.0f;

        if (this->focusMovementTimer >= 8)
        {
            this->orbState = ORB_FOCUSED;
        }
        if (!this->isFocus)
        {
            this->orbState = ORB_UNFOCUSING;
            this->focusMovementTimer = 8 - (i32)this->focusMovementTimer;

            goto CASE_ORB_UNFOCUSING;
        }
        else
        {
            break;
        }

    case ORB_FOCUSED:
        orbOffsetX = 8.0f;
        orbOffsetY = -32.0f;
        this->focusMovementTimer = 0;
        if (!this->isFocus)
        {
            this->orbState = ORB_UNFOCUSING;
        }
        else
        {
            break;
        }

    CASE_ORB_UNFOCUSING:
    case ORB_UNFOCUSING:
        this->focusMovementTimer++;

        intermediateFloat = (f32)this->focusMovementTimer / 8.0f;
        orbOffsetY = (32.0f * intermediateFloat) + -32.0f;
        intermediateFloat *= intermediateFloat;
        intermediateFloat = 1.0f - intermediateFloat;
        orbOffsetX = -16.0f * intermediateFloat + 24.0f;
        if (this->focusMovementTimer >= 8)
        {
            this->orbState = ORB_UNFOCUSED;
        }
        if (this->isFocus)
        {
            this->orbState = ORB_FOCUSING;
            this->focusMovementTimer = 8 - (i32)this->focusMovementTimer;
            goto CASE_ORB_FOCUSING;
        }
    }

    this->orbsPosition[0].x -= orbOffsetX;
    this->orbsPosition[1].x += orbOffsetX;
    this->orbsPosition[0].y += orbOffsetY;
    this->orbsPosition[1].y += orbOffsetY;
    if (IS_PRESSED(TH_BUTTON_SHOOT) && !g_Gui.IsMsgActive())
    {
        StartFireBulletTimer(this);
    }
    this->previousFrameInput = g_CurFrameInput;
    return ZUN_SUCCESS;
}

static ChainCallbackResult Player_OnUpdate(Player *p)
{
    i32 idx;

    if (g_GameManager.isTimeStopped)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    for (idx = 0; idx < PLAYER_BOMB_REGION_COUNT; idx++)
    {
        p->bombRegionSizes[idx].x = 0.0f;
    }
    for (idx = 0; idx < PLAYER_BOMB_PROJECTILE_COUNT; idx++)
    {
        p->bombProjectiles[idx].size.x = 0.0f;
    }
    if (p->bombInfo.isInUse)
    {
        p->bombInfo.calc(p);
    }
    else if (!g_Gui.IsMsgActive() && p->respawnTimer != 0 && g_GameManager.bombsRemaining > 0 &&
             WAS_PRESSED(TH_BUTTON_BOMB) && p->bombInfo.calc != NULL)
    {
        g_GameManager.bombsUsed++;
        g_GameManager.bombsRemaining--;
        g_Gui.flags.flag1 = 2;
        p->bombInfo.isInUse = true;
        p->bombInfo.timer = 0;
        p->bombInfo.duration = 999;
        p->bombInfo.calc(p);
        g_EnemyManager.spellcardInfo.isCapturing = false;
        g_GameManager.DecreaseSubrank(200);
        g_EnemyManager.spellcardInfo.usedBomb = g_EnemyManager.spellcardInfo.isActive;
    }
    if (p->playerState == PLAYER_STATE_DEAD)
    {
        if (p->respawnTimer != 0)
        {
            p->respawnTimer--;
            if (p->respawnTimer == 0)
            {
                g_GameManager.powerItemCountForScore = 0;
                if (g_GameManager.livesRemaining > 0)
                {
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_POWER_BIG, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_POWER_SMALL, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_POWER_SMALL, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_POWER_SMALL, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_POWER_SMALL, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_POWER_SMALL, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    if (g_GameManager.currentPower <= 16)
                    {
                        g_GameManager.currentPower = 0;
                    }
                    else
                    {
                        g_GameManager.currentPower -= 16;
                    }
                    g_Gui.flags.flag2 = 2;
                }
                else
                {
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_FULL_POWER, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_FULL_POWER, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_FULL_POWER, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_FULL_POWER, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&p->positionCenter, ITEM_FULL_POWER, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_GameManager.currentPower = 0;
                    g_Gui.flags.flag2 = 2;
                    // Disable earning lives from score if a continue is used
                    g_GameManager.livesEarned = -1;
                }
                g_GameManager.DecreaseSubrank(1600);
            }
        }
        else
        {
            float scaleFactor = (f32)p->invulnerabilityTimer / 30.0f;
            p->playerSprite.scaleY = 3.0f * scaleFactor + 1.0f;
            p->playerSprite.scaleX = 1.0f - 1.0f * scaleFactor;
            p->playerSprite.color =
                COLOR_SET_ALPHA(COLOR_WHITE, (u32)(255.0f - (f32)p->invulnerabilityTimer * 255.0f / 30.0f));
            p->playerSprite.flags.blendMode = AnmBlendMode_Additive;
            p->previousSpeed.x = 0.0f;
            p->previousSpeed.y = 0.0f;
            if ((i32)p->invulnerabilityTimer >= 30)
            {
                p->playerState = PLAYER_STATE_SPAWNING;
                p->positionCenter[0] = g_GameManager.gameRegionSize.x / 2.0f;
                p->positionCenter[1] = g_GameManager.gameRegionSize.y - 64.0f;
                p->positionCenter[2] = 0.2f;
                p->invulnerabilityTimer = 0;
                p->playerSprite.scaleY = p->playerSprite.scaleX = 3.0f;
                g_AnmManager->SetAndExecuteScriptIdx(&p->playerSprite, ANM_SCRIPT_PLAYER_IDLE);
                if (g_GameManager.livesRemaining <= 0)
                {
                    g_GameManager.isInRetryMenu = true;
                }
                else
                {
                    g_GameManager.livesRemaining--;
                    g_Gui.flags.flag0 = 2;
                    if (g_GameManager.difficulty < EXTRA && !g_GameManager.isInPracticeMode)
                    {
#if BUILT_SINCE(102g)
                        g_GameManager.bombsRemaining = g_Supervisor.defaultConfig.bombCount;
#else
                        g_GameManager.bombsRemaining = g_Supervisor.cfg.bombCount;
#endif
                    }
                    else
                    {
                        g_GameManager.bombsRemaining = 3;
                    }
                    g_Gui.flags.flag1 = 2;
                    goto spawning;
                }
            }
        }
    }
    else if (p->playerState == PLAYER_STATE_SPAWNING)
    {
    spawning:
        p->bulletGracePeriod = 90;
        float scaleFactor = 1.0f - (f32)p->invulnerabilityTimer / 30.0f;
        p->playerSprite.scaleY = 2.0f * scaleFactor + 1.0f;
        p->playerSprite.scaleX = 1.0f - 1.0f * scaleFactor;
        p->playerSprite.flags.blendMode = AnmBlendMode_Additive;
        p->speedMultiplierDuringBomb.x = p->speedMultiplierDuringBomb.y = 1.0f;
        p->playerSprite.color = COLOR_SET_ALPHA(COLOR_WHITE, (i32)p->invulnerabilityTimer * 255 / 30);
        p->respawnTimer = 0;
        if ((i32)p->invulnerabilityTimer >= 30)
        {
            p->playerState = PLAYER_STATE_INVULNERABLE;
            p->playerSprite.scaleY = p->playerSprite.scaleX = 1.0f;
            p->playerSprite.color = COLOR_WHITE;
            p->playerSprite.flags.blendMode = AnmBlendMode_Normal;
            p->invulnerabilityTimer = 240;
            p->respawnTimer = 6;
        }
    }
    if (p->bulletGracePeriod != 0)
    {
        p->bulletGracePeriod--;
        g_BulletManager.RemoveAllBullets(0);
    }
    if (p->playerState == PLAYER_STATE_INVULNERABLE)
    {
        p->invulnerabilityTimer--;
        if ((i32)p->invulnerabilityTimer <= 0)
        {
            p->playerState = PLAYER_STATE_ALIVE;
            p->invulnerabilityTimer = 0;
            p->playerSprite.flags.colorOp = AnmColorOp_Modulate;
            p->playerSprite.color = COLOR_WHITE;
        }
        else if ((i32)p->invulnerabilityTimer % 8 < 2)
        {
            p->playerSprite.flags.colorOp = AnmColorOp_Add;
            p->playerSprite.color = 0xff404040;
        }
        else
        {
            p->playerSprite.flags.colorOp = AnmColorOp_Modulate;
            p->playerSprite.color = COLOR_WHITE;
        }
    }
    else
    {
        p->invulnerabilityTimer++;
    }
    if (p->playerState != PLAYER_STATE_DEAD && p->playerState != PLAYER_STATE_SPAWNING)
    {
        p->HandlePlayerInputs();
    }
    g_AnmManager->ExecuteScript(&p->playerSprite);
    UpdatePlayerBullets(p);
    if (p->orbState != ORB_HIDDEN)
    {
        g_AnmManager->ExecuteScript(&p->orbsSprite[0]);
        g_AnmManager->ExecuteScript(&p->orbsSprite[1]);
    }
    p->positionOfLastEnemyHit = D3DXVECTOR3(-999.0f, -999.0f, 0.0f);
    UpdateFireBulletsTimer(p);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult Player_OnDrawHighPrio(Player *p)
{
    DrawBullets(p);
    if (p->bombInfo.isInUse && p->bombInfo.draw != NULL)
    {
        p->bombInfo.draw(p);
    }
    p->playerSprite.pos.x = g_GameManager.gameRegionScreenPos.x + p->positionCenter.x;
    p->playerSprite.pos.y = g_GameManager.gameRegionScreenPos.y + p->positionCenter.y;
    p->playerSprite.pos.z = 0.49f;
    if (!g_GameManager.isInRetryMenu)
    {
        g_AnmManager->DrawNoRotation(&p->playerSprite);
        if (p->orbState != ORB_HIDDEN &&
            (p->playerState == PLAYER_STATE_ALIVE || p->playerState == PLAYER_STATE_INVULNERABLE))
        {
            p->orbsSprite[0].pos = p->orbsPosition[0];
            p->orbsSprite[1].pos = p->orbsPosition[1];
            p->orbsSprite[0].pos[0] += g_GameManager.gameRegionScreenPos.x;
            p->orbsSprite[0].pos[1] += g_GameManager.gameRegionScreenPos.y;
            p->orbsSprite[1].pos[0] += g_GameManager.gameRegionScreenPos.x;
            p->orbsSprite[1].pos[1] += g_GameManager.gameRegionScreenPos.y;
            p->orbsSprite[0].pos.z = 0.491f;
            p->orbsSprite[1].pos.z = 0.491f;
            g_AnmManager->Draw(&p->orbsSprite[0]);
            g_AnmManager->Draw(&p->orbsSprite[1]);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult Player_OnDrawLowPrio(Player *p)
{
    DrawBulletExplosions(p);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(relY, relX)
f32 Player::AngleFromPlayer(D3DXVECTOR3 *pos)
{
    // NOTE: This is *not* a ZunVec2
    float relX = pos->x - this->positionCenter.x;
    float relY = pos->y - this->positionCenter.y;
    if (relY == 0.0f && relX == 0.0f)
    {
        return RADIANS(90.0f);
    }
    return atan2f(relY, relX);
}

#pragma var_order(relY, relX)
f32 Player::AngleToPlayer(D3DXVECTOR3 *pos)
{
    // NOTE: This is *not* a ZunVec2
    float relX = this->positionCenter[0] - pos->x;
    float relY = this->positionCenter[1] - pos->y;
    if (relY == 0.0f && relX == 0.0f)
    {
        // Shoot down. An angle of 0 means to the right, and the angle goes
        // clockwise.
        return RADIANS(90.0f);
    }
    return atan2f(relY, relX);
}

static ZunResult Player_AddedCallback(Player *p)
{
    PlayerBullet *curBullet;
    i32 idx;

    switch (g_GameManager.character)
    {
    case REIMU:
        if (g_Supervisor.IsNotLoadingNextStage() &&
            g_AnmManager->LoadAnm(ANM_FILE_PLAYER, "data/player00.anm", ANM_OFFSET_PLAYER) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        g_AnmManager->SetAndExecuteScriptIdx(&p->playerSprite, ANM_SCRIPT_PLAYER_IDLE);
        break;
    case MARISA:
        if (g_Supervisor.IsNotLoadingNextStage() &&
            g_AnmManager->LoadAnm(ANM_FILE_PLAYER, "data/player01.anm", ANM_OFFSET_PLAYER) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        g_AnmManager->SetAndExecuteScriptIdx(&p->playerSprite, ANM_SCRIPT_PLAYER_IDLE);
        break;
    }
    p->positionCenter[0] = g_GameManager.gameRegionSize.x / 2.0f;
    p->positionCenter[1] = g_GameManager.gameRegionSize.y - 64.0f;
    p->positionCenter[2] = 0.49f;
    p->orbsPosition[0].z = 0.49f;
    p->orbsPosition[1].z = 0.49f;
    for (idx = 0; idx < PLAYER_BOMB_REGION_COUNT; idx++)
    {
        p->bombRegionSizes[idx].x = 0.0f;
    }
    p->hitboxSize.x = 1.25f;
    p->hitboxSize.y = 1.25f;
    p->hitboxSize.z = 5.0f;
    p->grabItemSize.x = 12.0f;
    p->grabItemSize.y = 12.0f;
    p->grabItemSize.z = 5.0f;
    p->playerDirection = MOVEMENT_NONE;
    p->characterData = g_CharData[CURRENT_SHOTTYPE()];
    p->characterData.diagonalMovementSpeed = p->characterData.orthogonalMovementSpeed / sqrtf(2.0f);
    p->characterData.diagonalMovementSpeedFocus = p->characterData.orthogonalMovementSpeedFocus / sqrtf(2.0f);
    p->fireBulletCallback = p->characterData.fireBulletCallback;
    p->fireBulletFocusCallback = p->characterData.fireBulletFocusCallback;
    p->playerState = PLAYER_STATE_SPAWNING;
    p->invulnerabilityTimer = 120;
    p->orbState = ORB_HIDDEN;
    g_AnmManager->SetAndExecuteScriptIdx(&p->orbsSprite[0], ANM_SCRIPT_PLAYER_ORB_LEFT);
    g_AnmManager->SetAndExecuteScriptIdx(&p->orbsSprite[1], ANM_SCRIPT_PLAYER_ORB_RIGHT);
    for (curBullet = &p->bullets[0], idx = 0; idx < MAX_PLAYER_BULLETS; idx++, curBullet++)
    {
        curBullet->bulletState = 0;
    }
    p->fireBulletTimer = -1;
    p->bombInfo.calc = g_BombData[CURRENT_SHOTTYPE()].calc;
    p->bombInfo.draw = g_BombData[CURRENT_SHOTTYPE()].draw;
    p->bombInfo.isInUse = false;
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(p->laserTimer); idx++)
    {
        p->laserTimer[idx] = 0;
    }
    p->speedMultiplierDuringBomb.x = p->speedMultiplierDuringBomb.y = 1.0f;
    p->respawnTimer = 8;
    return ZUN_SUCCESS;
}

static ZunResult Player_DeletedCallback(Player *p)
{
    if (g_Supervisor.IsNotLoadingNextStage())
    {
        g_AnmManager->ReleaseAnm(ANM_FILE_PLAYER);
    }
    return ZUN_SUCCESS;
}

ZunResult Player_RegisterChain(u8 unk)
{
    Player *p = &g_Player;
    memset(p, 0, sizeof(Player));

    p->invulnerabilityTimer = 0;
    p->unk_9e1 = unk;
    p->chainCalc = g_FuncChain.CreateElem((ChainCallback)Player_OnUpdate);
    p->chainDraw1 = g_FuncChain.CreateElem((ChainCallback)Player_OnDrawHighPrio);
    p->chainDraw2 = g_FuncChain.CreateElem((ChainCallback)Player_OnDrawLowPrio);
    p->chainCalc->arg = p;
    p->chainDraw1->arg = p;
    p->chainDraw2->arg = p;
    p->chainCalc->addedCallback = (ChainAddedCallback)Player_AddedCallback;
    p->chainCalc->deletedCallback = (ChainDeletedCallback)Player_DeletedCallback;
    if (g_FuncChain.AddToCalcChain(p->chainCalc, TH_CHAIN_PRIO_CALC_PLAYER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    g_FuncChain.AddToDrawChain(p->chainDraw1, TH_CHAIN_PRIO_DRAW_LOW_PRIO_PLAYER);
    g_FuncChain.AddToDrawChain(p->chainDraw2, TH_CHAIN_PRIO_DRAW_HIGH_PRIO_PLAYER);
    return ZUN_SUCCESS;
}

void Player_CutChain()
{
    g_FuncChain.Cut(g_Player.chainCalc);
    g_Player.chainCalc = NULL;
    g_FuncChain.Cut(g_Player.chainDraw1);
    g_Player.chainDraw1 = NULL;
    g_FuncChain.Cut(g_Player.chainDraw2);
    g_Player.chainDraw2 = NULL;
}
} // namespace th06
