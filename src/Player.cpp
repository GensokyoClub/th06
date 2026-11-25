#include "stdafx.h"

#include "Player.hpp"

#include "BombData.hpp"
#include "BulletData.hpp"
#include "EffectManager.hpp"
#include "EnemyManager.hpp"
#include "ItemManager.hpp"

namespace th06
{
static BombData g_BombData[] = {
    /* ReimuA  */ {BombReimuACalc, BombReimuADraw},
    /* ReimuB  */ {BombReimuBCalc, BombReimuBDraw},
    /* MarisaA */ {BombMarisaACalc, BombMarisaADraw},
    /* MarisaB */ {BombMarisaBCalc, BombMarisaBDraw},
};

Player g_Player;

static FireBulletResult FireSingleBullet(Player *player, PlayerBullet *bullet, i32 entryIdx, i32 fireTimer,
                                         PowerData *powerData)
{
    while (g_GameManager.currentPower >= powerData->nextPowerThreshold)
    {
        powerData++;
    }

    ShtEntry *shtEntry = &powerData->entries[entryIdx];

    if (shtEntry->bulletType == PLAYER_LASER)
    {
        i32 delayTimerIdx = shtEntry->delay;
        if ((i32)player->shotTimer[delayTimerIdx] == 0)
        {
            player->shotTimer[delayTimerIdx] = shtEntry->fireRate;

            bullet->delayTimerIdx = delayTimerIdx;
            bullet->spawnPositionIdx = shtEntry->spawnPositionIdx;
            bullet->offset.x = shtEntry->offset.x;
            bullet->offset.y = shtEntry->offset.y;
            goto SHOOT_BULLET;
        }
    }
    else if (fireTimer % shtEntry->fireRate == shtEntry->delay)
    {
    SHOOT_BULLET:
        i32 pad[3]; // TODO: Unused D3DXVECTOR3?

        g_AnmManager->SetScript(&bullet->vm, shtEntry->anmScript);
        if (shtEntry->spawnPositionIdx == SPAWN_FROM_PLAYER)
        {
            bullet->position = player->position;
        }
        else
        {
            bullet->position = player->optionPosition[OPTION_SPAWN_INDEX(shtEntry)];
        }
        bullet->position[0] += shtEntry->offset.x;
        bullet->position[1] += shtEntry->offset.y;

        bullet->position[2] = 0.495f;

        bullet->size.x = shtEntry->size.x;
        bullet->size.y = shtEntry->size.y;
        bullet->size.z = 1.0f;
        bullet->angle = shtEntry->angle;
        bullet->speed = shtEntry->speed;

        bullet->velocity.x = cosf(shtEntry->angle) * shtEntry->speed;
        bullet->velocity.y = sinf(shtEntry->angle) * shtEntry->speed;

        bullet->lifetime = 0;

        bullet->bulletType = shtEntry->bulletType;
        bullet->damage = shtEntry->damage;
        if (IS_VALID_SFX(shtEntry->shootSfx))
        {
            g_SoundPlayer.PlaySoundId(shtEntry->shootSfx);
        }

        return entryIdx >= powerData->numEntries - 1;
    }

    if (entryIdx >= powerData->numEntries - 1)
    {
        return FBR_STOP_SPAWNING;
    }
    else
    {
        return FBR_SPAWN_MORE;
    }
}

static FireBulletResult FireBulletReimuA(Player *player, PlayerBullet *bullet, i32 entryIdx, i32 fireTimer)
{
    return FireSingleBullet(player, bullet, entryIdx, fireTimer, g_PowerDataReimuA);
}

static FireBulletResult FireBulletReimuB(Player *player, PlayerBullet *bullet, i32 entryIdx, i32 fireTimer)
{
    return FireSingleBullet(player, bullet, entryIdx, fireTimer, g_PowerDataReimuB);
}

static FireBulletResult FireBulletMarisaA(Player *player, PlayerBullet *bullet, i32 entryIdx, i32 fireTimer)
{
    return FireSingleBullet(player, bullet, entryIdx, fireTimer, g_PowerDataMarisaA);
}

static FireBulletResult FireBulletMarisaB(Player *player, PlayerBullet *bullet, i32 entryIdx, i32 fireTimer)
{
    return FireSingleBullet(player, bullet, entryIdx, fireTimer, g_PowerDataMarisaB);
}

CharacterData g_CharData[] = {
    /* ReimuA  */ {4.0f, 2.0f, 4.0f, 2.0f, FireBulletReimuA, FireBulletReimuA},
    /* ReimuB  */ {4.0f, 2.0f, 4.0f, 2.0f, FireBulletReimuB, FireBulletReimuB},
    /* MarisaA */ {5.0f, 2.5f, 5.0f, 2.5f, FireBulletMarisaA, FireBulletMarisaA},
    /* MarisaB */ {5.0f, 2.5f, 5.0f, 2.5f, FireBulletMarisaB, FireBulletMarisaB},
    /* Rin???  */ {4.0f, 2.0f, 4.0f, 2.0f, NULL, NULL},
};

#pragma var_order(entryIdx, i, curBullet)
static void SpawnBullets(Player *player, i32 fireTimer)
{
    i32 entryIdx = 0;
    PlayerBullet *curBullet = player->bullets;
    i32 i = 0;
    for (; i < MAX_PLAYER_BULLETS; i++, curBullet++)
    {
        if (curBullet->bulletState != PLAYER_BULLET_STATE_INACTIVE)
        {
            continue;
        }
    WHILE_LOOP:
        FireBulletResult bulletResult;
        if (!player->isFocused)
        {
            bulletResult = player->fireBulletCallback(player, curBullet, entryIdx, fireTimer);
        }
        else
        {
            bulletResult = player->fireBulletFocusCallback(player, curBullet, entryIdx, fireTimer);
        }
        // Checks if a bullet was shot
        if (bulletResult >= 0)
        {
            curBullet->vm.pos.x = curBullet->position.x;
            curBullet->vm.pos.y = curBullet->position.y;
            curBullet->vm.pos[2] = 0.495f;
            curBullet->bulletState = PLAYER_BULLET_STATE_FIRED;
        }
        if (bulletResult == FBR_STOP_SPAWNING)
        {
            return;
        }
        // Checks if the final bullet was shot
        if (bulletResult > 0)
        {
            return;
        }
        entryIdx++;
        if (bulletResult == FBR_SPAWN_MORE)
        {
            goto WHILE_LOOP;
        }
    }
}

#pragma var_order(vectorY, vectorX, i, vecLength, bullet)
static void UpdatePlayerBullets(Player *player)
{
    f32 vectorX;
    f32 vectorY;
    PlayerBullet *bullet;
    f32 vecLength;
    i32 i;

    for (i = 0; i < PLAYER_SHOT_TIMER_COUNT; i++)
    {
        if ((i32)player->shotTimer[i] != 0)
        {
            player->shotTimer[i]--;
        }
    }
    bullet = &player->bullets[0];
    for (i = 0; i < MAX_PLAYER_BULLETS; i++, bullet++)
    {
        if (bullet->bulletState == PLAYER_BULLET_STATE_INACTIVE)
        {
            continue;
        }

        switch (bullet->bulletType)
        {
        case PLAYER_BULLET_HOMING:
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

        case PLAYER_BULLET_ACCELERATE:
            if (bullet->bulletState == PLAYER_BULLET_STATE_FIRED)
            {
                bullet->velocity.y -= 0.3f;
            }
            break;

        case PLAYER_LASER:
            if (player->shotTimer[bullet->delayTimerIdx] == 70)
            {
                bullet->vm.pendingInterrupt = 1;
            }
            else if (player->shotTimer[bullet->delayTimerIdx] == 1)
            {
                bullet->vm.pendingInterrupt = 1;
            }

            bullet->position = player->optionPosition[OPTION_SPAWN_INDEX(bullet)];

            bullet->position.x += bullet->offset.x;
            bullet->position.y /= 2.0f;
            bullet->position.z = 0.44f;

            bullet->vm.scaleY = bullet->position.y * 2.0f / 14.0f;

            bullet->size.y = bullet->position.y * 2.0f;
            break;
        }

        bullet->vm.pos.x = bullet->position[0] += bullet->velocity.x * g_Supervisor.gameSpeed;
        bullet->vm.pos.y = bullet->position[1] += bullet->velocity.y * g_Supervisor.gameSpeed;
        bullet->vm.pos.z = bullet->position.z;

        if (bullet->bulletType != PLAYER_LASER &&
            !g_GameManager.IsInBounds(bullet->position[0], bullet->position[1], bullet->vm.sprite->width,
                                      bullet->vm.sprite->height))
        {
            bullet->bulletState = PLAYER_BULLET_STATE_INACTIVE;
        }

        if (g_AnmManager->RunAnm(&bullet->vm) != SCRIPT_RUNNING)
        {
            bullet->bulletState = PLAYER_BULLET_STATE_INACTIVE;
        }
        bullet->lifetime++;
    }
}

#pragma var_order(i, bullet)
static void DrawBullets(Player *player)
{
    PlayerBullet *bullet = player->bullets;
    i32 i = 0;
    for (; i < MAX_PLAYER_BULLETS; i++, bullet++)
    {
        if (bullet->bulletState != PLAYER_BULLET_STATE_FIRED)
        {
            continue;
        }
        if (bullet->vm.autoRotate != AnmAutoRotate_Off)
        {
            bullet->vm.rotation.z = RADIANS(90.0f) - ReduceAngleAdd(bullet->angle, RADIANS(180.0f));
        }
        g_AnmManager->Draw3D(&bullet->vm);
    }
}

#pragma var_order(i, bullet)
static void DrawBulletExplosions(Player *player)
{
    PlayerBullet *bullet = player->bullets;
    i32 i = 0;
    for (; i < MAX_PLAYER_BULLETS; i++, bullet++)
    {
        if (bullet->bulletState != PLAYER_BULLET_STATE_COLLIDED)
        {
            continue;
        }
        if (bullet->vm.autoRotate != AnmAutoRotate_Off)
        {
            bullet->vm.rotation.z = RADIANS(90.0f) - ReduceAngleAdd(bullet->angle, RADIANS(180.0f));
        }
        bullet->vm.pos[2] = 0.4f;
        g_AnmManager->Draw3D(&bullet->vm);
    }
}

static ZunResult UpdateFireBulletsTimer(Player *player)
{
    if ((i32)player->fireBulletTimer < 0)
    {
        return ZUN_SUCCESS;
    }

    if (player->fireBulletTimer.HasTicked() &&
        (!g_Player.bomb.isActive || g_GameManager.character != MARISA || g_GameManager.shotType != SHOT_TYPE_B))
    {
        SpawnBullets(player, (i32)player->fireBulletTimer);
    }

    player->fireBulletTimer++;

    if ((i32)player->fireBulletTimer >= SHOT_TIMER_MAX || player->playerState == PLAYER_STATE_DEAD ||
        player->playerState == PLAYER_STATE_SPAWNING)
    {
        player->fireBulletTimer = -1;
    }
    return ZUN_SUCCESS;
}

#pragma var_order(bullet, i, enemyBottomRight, bulletBottomRight, enemyTopLeft, damage, bulletTopLeft)
i32 Player::CalcDamageToEnemy(D3DXVECTOR3 *enemyPosition, D3DXVECTOR3 *enemySize, BOOL *hitByBomb)
{
    D3DXVECTOR3 bulletTopLeft;
    i32 damage;
    D3DXVECTOR3 enemyTopLeft;
    i32 i;
    PlayerBullet *bullet;

    D3DXVECTOR3 bulletBottomRight;
    D3DXVECTOR3 enemyBottomRight;

    damage = 0;

    SetVecCorners(&enemyTopLeft, &enemyBottomRight, enemyPosition, enemySize);
    bullet = &this->bullets[0];
    if (hitByBomb)
    {
        *hitByBomb = false;
    }
    for (i = 0; i < MAX_PLAYER_BULLETS; i++, bullet++)
    {
        if (bullet->bulletState == PLAYER_BULLET_STATE_INACTIVE ||
            bullet->bulletState != PLAYER_BULLET_STATE_FIRED && bullet->bulletType != PLAYER_BULLET_ACCELERATE)
        {
            continue;
        }

        SetVecCorners(&bulletTopLeft, &bulletBottomRight, &bullet->position, &bullet->size);

        // Bullet is hitting the enemy
        if (bulletTopLeft.y > enemyBottomRight.y || bulletTopLeft.x > enemyBottomRight.x ||
            bulletBottomRight.y < enemyTopLeft.y || bulletBottomRight.x < enemyTopLeft.x)
        {
            continue;
        }

        if (!this->bomb.isActive)
        {
            damage += bullet->damage;
        }
        else
        {
            // Bullets do less damage during bombs
            damage += bullet->damage / 3 != 0 ? bullet->damage / 3 : 1;
        }

        if (bullet->bulletType == PLAYER_BULLET_ACCELERATE)
        {
            bullet->damage = bullet->damage / 4;
            if (bullet->damage == 0)
            {
                bullet->damage = 1;
            }
            switch (bullet->vm.scriptId)
            {
            case ANM_SCRIPT_MARISA_MISSILE_TINY:
                bullet->size.x = 32.0f;
                bullet->size.y = 32.0f;
                break;
            case ANM_SCRIPT_MARISA_MISSILE_SMALL:
                bullet->size.x = 42.0f;
                bullet->size.y = 42.0f;
                break;
            case ANM_SCRIPT_MARISA_MISSILE_MEDIUM:
                bullet->size.x = 48.0f;
                bullet->size.y = 48.0f;
                break;
            case ANM_SCRIPT_MARISA_MISSILE_LARGE:
                bullet->size.x = 48.0f;
                bullet->size.y = 48.0f;
            }
            if (MULTIPLE_OF(bullet->lifetime, 6))
            {
                g_EffectManager.SpawnParticles(PARTICLE_SPARK_RED, &bullet->position);
            }
        }

        if (bullet->bulletType != PLAYER_LASER)
        {
            if (bullet->bulletState == PLAYER_BULLET_STATE_FIRED)
            {
                // Hit effects for each bullet are just a
                // constant offset from the script of the bullet.
                g_AnmManager->SetScript(&bullet->vm, bullet->vm.scriptId + ANM_SCRIPT_PLAYER_BULLET_HIT_OFFSET);
                g_EffectManager.SpawnParticles(PARTICLE_SPARK_RED, &bullet->position);
                bullet->position[2] = 0.1f;
            }
            bullet->bulletState = PLAYER_BULLET_STATE_COLLIDED;
            bullet->velocity.x /= 8.0f;
            bullet->velocity.y /= 8.0f;
        }
        else if (++this->particleTimer % 8 == 0)
        {
            bulletTopLeft = *enemyPosition;
            bulletTopLeft.x = bullet->position.x;

            g_EffectManager.SpawnParticles(PARTICLE_SPARK_RED, &bulletTopLeft);
        }
    }
    for (i = 0; i < MAX_DAMAGE_REGION_COUNT; i++)
    {
        if (this->damageRegionSize[i].x <= 0.0f)
        {
            continue;
        }

        bulletTopLeft = this->damageRegionPosition[i] - this->damageRegionSize[i] / 2.0f;
        bulletBottomRight = this->damageRegionPosition[i] + this->damageRegionSize[i] / 2.0f;
        if (bulletTopLeft.x > enemyBottomRight.x || bulletBottomRight.x < enemyTopLeft.x ||
            bulletTopLeft.y > enemyBottomRight.y || bulletBottomRight.y < enemyTopLeft.y)
        {
            continue;
        }
        damage += this->damageRegionDamage[i];
        this->damageRegionDamageDealt[i] += this->damageRegionDamage[i];
        if (++this->particleTimer % 4 == 0)
        {
            g_EffectManager.SpawnParticles(PARTICLE_BOMB_FLASH, enemyPosition);
        }
        if (this->bomb.isActive && hitByBomb)
        {
            *hitByBomb = true;
        }
    }
    return damage;
}

#pragma var_order(cancelRegionTopLeft, i, bulletBottomRight, bulletTopLeft, curCancelRegion, cancelRegionBottomRight)
i32 Player::CheckCollision(D3DXVECTOR3 *position, D3DXVECTOR3 *size)
{
    PlayerRect *curCancelRegion;
    D3DXVECTOR3 bulletTopLeft, bulletBottomRight;
    D3DXVECTOR3 cancelRegionTopLeft, cancelRegionBottomRight;
    i32 i;

    curCancelRegion = this->cancelRegions;
    bulletTopLeft.x = position->x - size->x / 2.0f;
    bulletTopLeft.y = position->y - size->y / 2.0f;
    bulletBottomRight.x = position->x + size->x / 2.0f;
    bulletBottomRight.y = position->y + size->y / 2.0f;

    for (i = 0; i < MAX_CANCEL_REGION_COUNT; i++, curCancelRegion++)
    {
        if (curCancelRegion->size.x == 0.0f)
        {
            continue;
        }
        cancelRegionTopLeft.x = curCancelRegion->pos.x - curCancelRegion->size.x / 2.0f;
        cancelRegionTopLeft.y = curCancelRegion->pos.y - curCancelRegion->size.y / 2.0f;
        cancelRegionBottomRight.x = curCancelRegion->pos.x + curCancelRegion->size.x / 2.0f;
        cancelRegionBottomRight.y = curCancelRegion->pos.y + curCancelRegion->size.y / 2.0f;
        if (!(cancelRegionTopLeft.x > bulletBottomRight.x || cancelRegionBottomRight.x < bulletTopLeft.x ||
              cancelRegionTopLeft.y > bulletBottomRight.y || cancelRegionBottomRight.y < bulletTopLeft.y))
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

#pragma var_order(cancelRegionTopLeft, i, bulletBottomRight, bulletTopLeft, curCancelRegion, cancelRegionBottomRight)
i32 Player::CheckGraze(D3DXVECTOR3 *position, D3DXVECTOR3 *size)
{
    PlayerRect *curCancelRegion;
    D3DXVECTOR3 bulletTopLeft, bulletBottomRight;
    D3DXVECTOR3 cancelRegionTopLeft, cancelRegionBottomRight;
    i32 i;

    bulletTopLeft.x = position->x - size->x / 2.0f - 20.0f;
    bulletTopLeft.y = position->y - size->y / 2.0f - 20.0f;
    bulletBottomRight.x = position->x + size->x / 2.0f + 20.0f;
    bulletBottomRight.y = position->y + size->y / 2.0f + 20.0f;
    curCancelRegion = this->cancelRegions;

    for (i = 0; i < MAX_CANCEL_REGION_COUNT; i++, curCancelRegion++)
    {
        if (curCancelRegion->size.x == 0.0f)
        {
            continue;
        }

        cancelRegionTopLeft.x = curCancelRegion->pos.x - curCancelRegion->size.x / 2.0f;
        cancelRegionTopLeft.y = curCancelRegion->pos.y - curCancelRegion->size.y / 2.0f;
        cancelRegionBottomRight.x = curCancelRegion->size.x / 2.0f + curCancelRegion->pos.x;
        cancelRegionBottomRight.y = curCancelRegion->size.y / 2.0f + curCancelRegion->pos.y;

        // Bomb clips bullet's hitbox, destroys bullet upon return
        if (!(cancelRegionTopLeft.x > bulletBottomRight.x || cancelRegionBottomRight.x < bulletTopLeft.x ||
              cancelRegionTopLeft.y > bulletBottomRight.y || cancelRegionBottomRight.y < bulletTopLeft.y))
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
    this->ScoreGraze(position);
    return GRAZE_TRUE;
}

#pragma var_order(itemBottomRight, itemTopLeft)
BOOL Player::CalcItemBoxCollision(D3DXVECTOR3 *itemPosition, D3DXVECTOR3 *itemSize)
{
    if (this->playerState != PLAYER_STATE_ALIVE && this->playerState != PLAYER_STATE_INVULNERABLE)
    {
        return false;
    }

    D3DXVECTOR3 itemTopLeft;
    D3DXVECTOR3 itemBottomRight;

    itemTopLeft = *itemPosition - *itemSize / 2.0f;
    itemBottomRight = *itemPosition + *itemSize / 2.0f;

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
i32 Player::CalcLaserHitbox(D3DXVECTOR3 *laserCenter, D3DXVECTOR3 *laserSize, D3DXVECTOR3 *laserPosition, f32 angle,
                            BOOL canGraze)
{
    D3DXVECTOR3 laserTopLeft;
    D3DXVECTOR3 laserBottomRight;
    D3DXVECTOR3 playerRelativeTopLeft;
    D3DXVECTOR3 playerRelativeBottomRight;

    laserTopLeft = this->position - *laserPosition;
    RotatePoint(&laserBottomRight, &laserTopLeft, angle);
    laserBottomRight.z = 0.0f;
    laserTopLeft = laserBottomRight + *laserPosition;
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

    this->ScoreGraze(&this->position);
    return COLLISION_GRAZE;

LASER_COLLISION:
    if (this->playerState != PLAYER_STATE_ALIVE)
    {
        return COLLISION_NONE;
    }

    this->Die();
    return COLLISION_HIT;
}

void Player::ScoreGraze(D3DXVECTOR3 *grazedPosition)
{
    D3DXVECTOR3 particlePosition;

    // Until 1.02b graze also counts while a bomb is active.
#if BUILT_SINCE(102b)
    if (!g_Player.bomb.isActive)
#endif
    {
        if (g_GameManager.grazeInStage < 9999)
        {
            g_GameManager.grazeInStage++;
        }
        if (g_GameManager.grazeInTotal < 999999)
        {
            g_GameManager.grazeInTotal++;
        }
    }

    particlePosition = (this->position + *grazedPosition) / 2.0f;
    g_EffectManager.SpawnParticles(PARTICLE_SPARK_SMALL, &particlePosition);
    g_GameManager.AddScore(500);
    g_GameManager.IncreaseSubrank(6);
    g_Gui.redrawFrames.grazeCounter = 2;
    g_SoundPlayer.PlaySoundId(SOUND_GRAZE);
}

void Player::Die()
{
    g_EnemyManager.spellcardInfo.isCapturing = false;
    g_EffectManager.SpawnParticles(PARTICLE_FLASH, &this->position, 1, COLOR_NEONBLUE);
    g_EffectManager.SpawnParticles(PARTICLE_SPARK_GREEN, &this->position, 16);
    this->playerState = PLAYER_STATE_DEAD;
    this->stateTimer = 0;
    g_SoundPlayer.PlaySoundId(SOUND_PICHUN);
    g_GameManager.deaths++;
    for (i32 i = 0; i < PLAYER_SHOT_TIMER_COUNT; i++)
    {
        this->shotTimer[i] = 2;
    }
}

static void StartFireBulletTimer(Player *player)
{
    if ((i32)player->fireBulletTimer < 0)
    {
        player->fireBulletTimer = 0;
    }
}

#pragma var_order(playerDirection, speedY, speedX, optionOffsetY, optionOffsetX)
ZunResult Player::HandlePlayerInputs()
{
    f32 optionOffsetX;
    f32 optionOffsetY;

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
        this->isFocused = true;
    }
    else
    {
        this->isFocused = false;
    }

    switch (this->playerDirection)
    {
    case MOVEMENT_RIGHT:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedX = this->characterData.linearMovementSpeedFocus;
        }
        else
        {
            speedX = this->characterData.linearMovementSpeed;
        }
        break;
    case MOVEMENT_LEFT:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedX = -this->characterData.linearMovementSpeedFocus;
        }
        else
        {
            speedX = -this->characterData.linearMovementSpeed;
        }
        break;
    case MOVEMENT_UP:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedY = -this->characterData.linearMovementSpeedFocus;
        }
        else
        {
            speedY = -this->characterData.linearMovementSpeed;
        }
        break;
    case MOVEMENT_DOWN:
        if (IS_PRESSED(TH_BUTTON_FOCUS))
        {
            speedY = this->characterData.linearMovementSpeedFocus;
        }
        else
        {
            speedY = this->characterData.linearMovementSpeed;
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
        g_AnmManager->SetScript(&this->vm, ANM_SCRIPT_PLAYER_MOVING_LEFT);
    }
    else if (speedX == 0.0f && this->previousSpeed.x < 0.0f)
    {
        g_AnmManager->SetScript(&this->vm, ANM_SCRIPT_PLAYER_STOPPING_LEFT);
    }

    if (speedX > 0.0f && this->previousSpeed.x <= 0.0f)
    {
        g_AnmManager->SetScript(&this->vm, ANM_SCRIPT_PLAYER_MOVING_RIGHT);
    }
    else if (speedX == 0.0f && this->previousSpeed.x > 0.0f)
    {
        g_AnmManager->SetScript(&this->vm, ANM_SCRIPT_PLAYER_STOPPING_RIGHT);
    }

    this->previousSpeed.x = speedX;
    this->previousSpeed.y = speedY;

    this->position[0] += speedX * this->speedMultiplierDuringBomb.x * g_Supervisor.gameSpeed;
    this->position[1] += speedY * this->speedMultiplierDuringBomb.y * g_Supervisor.gameSpeed;

    if (this->position[0] < g_GameManager.playerMovementAreaTopLeftPos.x)
    {
        this->position[0] = g_GameManager.playerMovementAreaTopLeftPos.x;
    }
    else if (this->position[0] > g_GameManager.playerMovementAreaTopLeftPos.x + g_GameManager.playerMovementAreaSize.x)
    {
        this->position[0] = g_GameManager.playerMovementAreaTopLeftPos.x + g_GameManager.playerMovementAreaSize.x;
    }

    if (this->position[1] < g_GameManager.playerMovementAreaTopLeftPos.y)
    {
        this->position[1] = g_GameManager.playerMovementAreaTopLeftPos.y;
    }
    else if (this->position[1] > g_GameManager.playerMovementAreaTopLeftPos.y + g_GameManager.playerMovementAreaSize.y)
    {
        this->position[1] = g_GameManager.playerMovementAreaTopLeftPos.y + g_GameManager.playerMovementAreaSize.y;
    }

    this->hitboxTopLeft = this->position - this->hitboxSize;

    this->hitboxBottomRight = this->position + this->hitboxSize;

    this->grabItemTopLeft = this->position - this->grabItemSize;

    this->grabItemBottomRight = this->position + this->grabItemSize;

    this->optionPosition[0] = this->position;
    this->optionPosition[1] = this->position;

    optionOffsetX = optionOffsetY = 0.0f;

    if (g_GameManager.currentPower < LEVEL_1)
    {
        this->orbState = OPTION_HIDDEN;
    }
    else if (this->orbState == OPTION_HIDDEN)
    {
        this->orbState = OPTION_UNFOCUSED;
    }

    switch (this->orbState)
    {
        float intermediateFloat;

    case OPTION_HIDDEN:
        this->focusMovementTimer = 0;
        break;

    case OPTION_UNFOCUSED:
        optionOffsetX = 24.0f;
        this->focusMovementTimer = 0;
        if (this->isFocused)
        {
            this->orbState = OPTION_FOCUSING;
        }
        else
        {
            break;
        }

    CASE_ORB_FOCUSING:
    case OPTION_FOCUSING:
        this->focusMovementTimer++;

        intermediateFloat = (f32)this->focusMovementTimer / 8.0f;
        optionOffsetY = (1.0f - intermediateFloat) * 32.0f + -32.0f;
        intermediateFloat *= intermediateFloat;
        optionOffsetX = -16.0f * intermediateFloat + 24.0f;

        if (this->focusMovementTimer >= 8)
        {
            this->orbState = OPTION_FOCUSED;
        }
        if (!this->isFocused)
        {
            this->orbState = OPTION_UNFOCUSING;
            this->focusMovementTimer = 8 - (i32)this->focusMovementTimer;

            goto CASE_ORB_UNFOCUSING;
        }
        else
        {
            break;
        }

    case OPTION_FOCUSED:
        optionOffsetX = 8.0f;
        optionOffsetY = -32.0f;
        this->focusMovementTimer = 0;
        if (!this->isFocused)
        {
            this->orbState = OPTION_UNFOCUSING;
        }
        else
        {
            break;
        }

    CASE_ORB_UNFOCUSING:
    case OPTION_UNFOCUSING:
        this->focusMovementTimer++;

        intermediateFloat = (f32)this->focusMovementTimer / 8.0f;
        optionOffsetY = (32.0f * intermediateFloat) + -32.0f;
        intermediateFloat *= intermediateFloat;
        intermediateFloat = 1.0f - intermediateFloat;
        optionOffsetX = -16.0f * intermediateFloat + 24.0f;
        if (this->focusMovementTimer >= 8)
        {
            this->orbState = OPTION_UNFOCUSED;
        }
        if (this->isFocused)
        {
            this->orbState = OPTION_FOCUSING;
            this->focusMovementTimer = 8 - (i32)this->focusMovementTimer;
            goto CASE_ORB_FOCUSING;
        }
    }

    this->optionPosition[0].x -= optionOffsetX;
    this->optionPosition[1].x += optionOffsetX;
    this->optionPosition[0].y += optionOffsetY;
    this->optionPosition[1].y += optionOffsetY;

    if (IS_PRESSED(TH_BUTTON_SHOOT) && !g_Gui.IsMsgActive())
    {
        StartFireBulletTimer(this);
    }
    this->previousFrameInput = g_CurFrameInput;
    return ZUN_SUCCESS;
}

static ChainCallbackResult Player_OnUpdate(Player *player)
{
    i32 i;

    if (g_GameManager.isTimeStopped)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    for (i = 0; i < MAX_DAMAGE_REGION_COUNT; i++)
    {
        player->damageRegionSize[i].x = 0.0f;
    }
    for (i = 0; i < MAX_CANCEL_REGION_COUNT; i++)
    {
        player->cancelRegions[i].size.x = 0.0f;
    }
    if (player->bomb.isActive)
    {
        player->bomb.calc(player);
    }
    else if (!g_Gui.IsMsgActive() && player->respawnTimer != 0 && g_GameManager.bombsRemaining > 0 &&
             WAS_PRESSED(TH_BUTTON_BOMB) && player->bomb.calc != NULL)
    {
        g_GameManager.bombsUsed++;
        g_GameManager.bombsRemaining--;
        g_Gui.redrawFrames.bombIcons = 2;
        player->bomb.isActive = true;
        player->bomb.timer = 0;
        player->bomb.duration = 999;
        player->bomb.calc(player);
        g_EnemyManager.spellcardInfo.isCapturing = false;
        g_GameManager.DecreaseSubrank(200);
        g_EnemyManager.spellcardInfo.usedBombTick = g_EnemyManager.spellcardInfo.ticksActive;
    }
    if (player->playerState == PLAYER_STATE_DEAD)
    {
        if (player->respawnTimer != 0)
        {
            player->respawnTimer--;
            if (player->respawnTimer == 0)
            {
                g_GameManager.fullPowerScoreIdx = 0;
                if (g_GameManager.livesRemaining > 0)
                {
                    g_ItemManager.SpawnItem(&player->position, ITEM_POWER_BIG, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&player->position, ITEM_POWER_SMALL, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&player->position, ITEM_POWER_SMALL, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&player->position, ITEM_POWER_SMALL, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&player->position, ITEM_POWER_SMALL, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&player->position, ITEM_POWER_SMALL, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    if (g_GameManager.currentPower <= POWER_PER_LEVEL)
                    {
                        g_GameManager.currentPower = 0;
                    }
                    else
                    {
                        g_GameManager.currentPower -= POWER_PER_LEVEL;
                    }
                    g_Gui.redrawFrames.powerGauge = 2;
                }
                else
                {
                    g_ItemManager.SpawnItem(&player->position, ITEM_FULL_POWER, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&player->position, ITEM_FULL_POWER, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&player->position, ITEM_FULL_POWER, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&player->position, ITEM_FULL_POWER, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_ItemManager.SpawnItem(&player->position, ITEM_FULL_POWER, ITEM_STATE_SPAWNED_BY_PLAYER_DEATH);
                    g_GameManager.currentPower = 0;
                    g_Gui.redrawFrames.powerGauge = 2;
                    // Disable earning lives from score if a continue is used
                    g_GameManager.livesEarned = -1;
                }
                g_GameManager.DecreaseSubrank(1600);
            }
        }
        else
        {
            float scaleFactor = (f32)player->stateTimer / 30.0f;
            player->vm.scaleY = 3.0f * scaleFactor + 1.0f;
            player->vm.scaleX = 1.0f - 1.0f * scaleFactor;
            player->vm.color = COLOR_SET_ALPHA(COLOR_WHITE, (u32)(RAMP_DOWN(255.0f, (f32)player->stateTimer, 30.0f)));
            player->vm.flags.blendMode = AnmBlendMode_Additive;
            player->previousSpeed.x = 0.0f;
            player->previousSpeed.y = 0.0f;
            if ((i32)player->stateTimer >= 30)
            {
                player->playerState = PLAYER_STATE_SPAWNING;
                player->position[0] = g_GameManager.gameRegionSize.x / 2.0f;
                player->position[1] = g_GameManager.gameRegionSize.y - 64.0f;
                player->position[2] = 0.2f;
                player->stateTimer = 0;
                player->vm.scaleY = player->vm.scaleX = 3.0f;
                g_AnmManager->SetScript(&player->vm, ANM_SCRIPT_PLAYER_IDLE);
                if (g_GameManager.livesRemaining <= 0)
                {
                    g_GameManager.isInRetryMenu = true;
                }
                else
                {
                    g_GameManager.livesRemaining--;
                    g_Gui.redrawFrames.lifeIcons = 2;
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
                        g_GameManager.bombsRemaining = DEFAULT_BOMB_COUNT;
                    }
                    g_Gui.redrawFrames.bombIcons = 2;
                    goto spawning;
                }
            }
        }
    }
    else if (player->playerState == PLAYER_STATE_SPAWNING)
    {
    spawning:
        player->bulletGracePeriod = 90;
        float scaleFactor = 1.0f - (f32)player->stateTimer / 30.0f;
        player->vm.scaleY = 2.0f * scaleFactor + 1.0f;
        player->vm.scaleX = 1.0f - 1.0f * scaleFactor;
        player->vm.flags.blendMode = AnmBlendMode_Additive;
        player->speedMultiplierDuringBomb.x = player->speedMultiplierDuringBomb.y = 1.0f;
        player->vm.color = COLOR_SET_ALPHA(COLOR_WHITE, RAMP_UP(255, (i32)player->stateTimer, 30));
        player->respawnTimer = 0;
        if ((i32)player->stateTimer >= 30)
        {
            player->playerState = PLAYER_STATE_INVULNERABLE;
            player->vm.scaleY = player->vm.scaleX = 1.0f;
            player->vm.color = COLOR_WHITE;
            player->vm.flags.blendMode = AnmBlendMode_Normal;
            player->stateTimer = 240;
            player->respawnTimer = 6;
        }
    }
    if (player->bulletGracePeriod != 0)
    {
        player->bulletGracePeriod--;
        g_BulletManager.DespawnBullets(BULLET_CLEAR);
    }
    if (player->playerState == PLAYER_STATE_INVULNERABLE)
    {
        player->stateTimer--;
        if ((i32)player->stateTimer <= 0)
        {
            player->playerState = PLAYER_STATE_ALIVE;
            player->stateTimer = 0;
            player->vm.flags.colorOp = AnmColorOp_Modulate;
            player->vm.color = COLOR_WHITE;
        }
        else if ((i32)player->stateTimer % 8 < 2)
        {
            player->vm.flags.colorOp = AnmColorOp_Add;
            player->vm.color = 0xff404040;
        }
        else
        {
            player->vm.flags.colorOp = AnmColorOp_Modulate;
            player->vm.color = COLOR_WHITE;
        }
    }
    else
    {
        player->stateTimer++;
    }
    if (player->playerState != PLAYER_STATE_DEAD && player->playerState != PLAYER_STATE_SPAWNING)
    {
        player->HandlePlayerInputs();
    }
    g_AnmManager->RunAnm(&player->vm);
    UpdatePlayerBullets(player);
    if (player->orbState != OPTION_HIDDEN)
    {
        g_AnmManager->RunAnm(&player->optionVms[0]);
        g_AnmManager->RunAnm(&player->optionVms[1]);
    }
    player->positionOfLastEnemyHit = D3DXVECTOR3(-999.0f, -999.0f, 0.0f);
    UpdateFireBulletsTimer(player);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult Player_OnDraw(Player *player)
{
    DrawBullets(player);
    if (player->bomb.isActive && player->bomb.draw != NULL)
    {
        player->bomb.draw(player);
    }
    player->vm.pos.x = g_GameManager.gameRegionScreenPos.x + player->position.x;
    player->vm.pos.y = g_GameManager.gameRegionScreenPos.y + player->position.y;
    player->vm.pos.z = 0.49f;
    if (!g_GameManager.isInRetryMenu)
    {
        g_AnmManager->Draw2D(&player->vm);
        if (player->orbState != OPTION_HIDDEN &&
            (player->playerState == PLAYER_STATE_ALIVE || player->playerState == PLAYER_STATE_INVULNERABLE))
        {
            player->optionVms[0].pos = player->optionPosition[0];
            player->optionVms[1].pos = player->optionPosition[1];
            player->optionVms[0].pos[0] += g_GameManager.gameRegionScreenPos.x;
            player->optionVms[0].pos[1] += g_GameManager.gameRegionScreenPos.y;
            player->optionVms[1].pos[0] += g_GameManager.gameRegionScreenPos.x;
            player->optionVms[1].pos[1] += g_GameManager.gameRegionScreenPos.y;
            player->optionVms[0].pos.z = 0.491f;
            player->optionVms[1].pos.z = 0.491f;
            g_AnmManager->Draw2DRotated(&player->optionVms[0]);
            g_AnmManager->Draw2DRotated(&player->optionVms[1]);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult Player_OnDrawEffects(Player *player)
{
    DrawBulletExplosions(player);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(relY, relX)
f32 Player::AngleFromPlayer(D3DXVECTOR3 *pos)
{
    // NOTE: This is *not* a Float2
    float relX = pos->x - this->position.x;
    float relY = pos->y - this->position.y;
    if (relY == 0.0f && relX == 0.0f)
    {
        return RADIANS(90.0f);
    }
    return atan2f(relY, relX);
}

#pragma var_order(relY, relX)
f32 Player::AngleToPlayer(D3DXVECTOR3 *pos)
{
    // NOTE: This is *not* a Float2
    float relX = this->position[0] - pos->x;
    float relY = this->position[1] - pos->y;
    if (relY == 0.0f && relX == 0.0f)
    {
        // Shoot down. An angle of 0 means to the right, and the angle goes
        // clockwise.
        return RADIANS(90.0f);
    }
    return atan2f(relY, relX);
}

static ZunResult Player_AddedCallback(Player *player)
{
    i32 i;

    switch (g_GameManager.character)
    {
    case REIMU:
        if (g_Supervisor.IsNotLoadingNextStage() &&
            g_AnmManager->LoadAnm(ANM_FILE_PLAYER, "data/player00.anm", ANM_OFFSET_PLAYER) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        g_AnmManager->SetScript(&player->vm, ANM_SCRIPT_PLAYER_IDLE);
        break;
    case MARISA:
        if (g_Supervisor.IsNotLoadingNextStage() &&
            g_AnmManager->LoadAnm(ANM_FILE_PLAYER, "data/player01.anm", ANM_OFFSET_PLAYER) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        g_AnmManager->SetScript(&player->vm, ANM_SCRIPT_PLAYER_IDLE);
        break;
    }
    player->position[0] = g_GameManager.gameRegionSize.x / 2.0f;
    player->position[1] = g_GameManager.gameRegionSize.y - 64.0f;
    player->position[2] = 0.49f;
    player->optionPosition[0].z = 0.49f;
    player->optionPosition[1].z = 0.49f;
    for (i = 0; i < MAX_DAMAGE_REGION_COUNT; i++)
    {
        player->damageRegionSize[i].x = 0.0f;
    }
    player->hitboxSize.x = 1.25f;
    player->hitboxSize.y = 1.25f;
    player->hitboxSize.z = 5.0f;
    player->grabItemSize.x = 12.0f;
    player->grabItemSize.y = 12.0f;
    player->grabItemSize.z = 5.0f;
    player->playerDirection = MOVEMENT_NONE;
    player->characterData = g_CharData[CURRENT_SHOTTYPE()];
    player->characterData.diagonalMovementSpeed = player->characterData.linearMovementSpeed / sqrtf(2.0f);
    player->characterData.diagonalMovementSpeedFocus = player->characterData.linearMovementSpeedFocus / sqrtf(2.0f);
    player->fireBulletCallback = player->characterData.fireBulletCallback;
    player->fireBulletFocusCallback = player->characterData.fireBulletFocusCallback;
    player->playerState = PLAYER_STATE_SPAWNING;
    player->stateTimer = 120;
    player->orbState = OPTION_HIDDEN;
    g_AnmManager->SetScript(&player->optionVms[0], ANM_SCRIPT_PLAYER_OPTION_LEFT);
    g_AnmManager->SetScript(&player->optionVms[1], ANM_SCRIPT_PLAYER_OPTION_RIGHT);
    {
        PlayerBullet *curBullet = &player->bullets[0];
        for (i = 0; i < MAX_PLAYER_BULLETS; i++, curBullet++)
        {
            curBullet->bulletState = PLAYER_BULLET_STATE_INACTIVE;
        }
    }
    player->fireBulletTimer = -1;
    player->bomb.calc = g_BombData[CURRENT_SHOTTYPE()].calc;
    player->bomb.draw = g_BombData[CURRENT_SHOTTYPE()].draw;
    player->bomb.isActive = false;
    for (i = 0; i < PLAYER_SHOT_TIMER_COUNT; i++)
    {
        player->shotTimer[i] = 0;
    }
    player->speedMultiplierDuringBomb.x = player->speedMultiplierDuringBomb.y = 1.0f;
    player->respawnTimer = 8;
    return ZUN_SUCCESS;
}

static ZunResult Player_DeletedCallback(Player *player)
{
    if (g_Supervisor.IsNotLoadingNextStage())
    {
        g_AnmManager->ReleaseAnm(ANM_FILE_PLAYER);
    }
    return ZUN_SUCCESS;
}

ZunResult Player_RegisterChain(u8 unk)
{
    Player *player = &g_Player;
    memset(player, 0, sizeof(Player));

    player->stateTimer = 0;
    player->unk_9e1 = unk;
    player->chainCalc = g_FuncChain.CreateElem((ChainCallback)Player_OnUpdate);
    player->chainDraw = g_FuncChain.CreateElem((ChainCallback)Player_OnDraw);
    player->chainDrawEffects = g_FuncChain.CreateElem((ChainCallback)Player_OnDrawEffects);
    player->chainCalc->arg = player;
    player->chainDraw->arg = player;
    player->chainDrawEffects->arg = player;
    player->chainCalc->addedCallback = (ChainAddedCallback)Player_AddedCallback;
    player->chainCalc->deletedCallback = (ChainDeletedCallback)Player_DeletedCallback;
    if (g_FuncChain.AddToCalcChain(player->chainCalc, TH_CHAIN_PRIO_CALC_PLAYER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    g_FuncChain.AddToDrawChain(player->chainDraw, TH_CHAIN_PRIO_DRAW_LOW_PRIO_PLAYER);
    g_FuncChain.AddToDrawChain(player->chainDrawEffects, TH_CHAIN_PRIO_DRAW_HIGH_PRIO_PLAYER);
    return ZUN_SUCCESS;
}

void Player_CutChain()
{
    g_FuncChain.Cut(g_Player.chainCalc);
    g_Player.chainCalc = NULL;
    g_FuncChain.Cut(g_Player.chainDraw);
    g_Player.chainDraw = NULL;
    g_FuncChain.Cut(g_Player.chainDrawEffects);
    g_Player.chainDrawEffects = NULL;
}
} // namespace th06
