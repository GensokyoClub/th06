#include "stdafx.h"

#include "BombData.hpp"

#include "EffectManager.hpp"
#include "Gui.hpp"
#include "ScreenEffect.hpp"
#include "ItemManager.hpp"
#include "SoundPlayer.hpp"

namespace th06
{

#pragma var_order(angleY, angleX, i, bombSprite, vecLength, bombPivot, bombIdx)
void BombReimuACalc(Player *player)
{
    i32 i;
    f32 vecLength;
    i32 bombIdx;
    D3DXVECTOR3 bombPivot;
    AnmVm *bombSprite;
    f32 angleX;
    f32 angleY;

    if (player->bombInfo.timer >= player->bombInfo.duration)
    {
        g_Gui.EndPlayerSpellcard();
        player->bombInfo.isInUse = false;
        return;
    }
    if (player->bombInfo.timer.HasTicked() && player->bombInfo.timer == 0)
    {
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_BOMB_PORTRAIT, TH_REIMU_A_BOMB_NAME);
        player->bombInfo.duration = 300;
        player->invulnerabilityTimer = 360;

        for (i = 0; i < 8; i++)
        {
            player->bombInfo.reimuABombProjectilesState[i] = 0;
        }
        g_ItemManager.MagnetAllItems();
        g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &player->positionCenter, 1, COLOR_NEONBLUE);

        player->bombProjectiles[8].pos.x = player->positionCenter.x;
        player->bombProjectiles[8].pos.y = player->positionCenter.y;

        player->bombProjectiles[8].size.x = 256.0f;
        player->bombProjectiles[8].size.y = 256.0f;
    }
    if (player->bombInfo.timer >= 60 && player->bombInfo.timer < 180)
    {
        if (MULTIPLE_OF(player->bombInfo.timer, 16))
        {
            i = ((i32)player->bombInfo.timer - 60) / 16;
            if (i != 0)
            {
                player->bombInfo.reimuABombProjectilesState[i] = 1;
                player->bombInfo.reimuABombProjectilesRelated[i] = 4.0f;
                player->bombInfo.bombRegionPositions[i] = player->positionCenter;

                angleX = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;

                player->bombInfo.bombRegionVelocities[i].x =
                    cosf(angleX) * player->bombInfo.reimuABombProjectilesRelated[i];

                player->bombInfo.bombRegionVelocities[i].y =
                    sinf(angleX) * player->bombInfo.reimuABombProjectilesRelated[i];
                player->bombRegionTotalDamages[i] = 0;

                for (bombSprite = &player->bombInfo.sprites[0][i * 4], bombIdx = 0; bombIdx < 4;
                     bombIdx++, bombSprite++)
                {
                    g_AnmManager->ExecuteAnmIdx(bombSprite, ANM_SCRIPT_PLAYER_REIMU_A_BOMB_ARRAY + bombIdx);
                }
                g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_REIMU_A);
            }
        }
    }
    player->playerState = PLAYER_STATE_INVULNERABLE;
    for (i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.reimuABombProjectilesState); i++)
    {
        if (player->bombInfo.reimuABombProjectilesState[i] == 0)
        {
            continue;
        }
        if (player->bombInfo.reimuABombProjectilesState[i] == 1)
        {
            if (player->bombInfo.timer.HasTicked())
            {
                if (player->positionOfLastEnemyHit.x > -100.0f)
                {
                    bombPivot = player->positionOfLastEnemyHit;
                }
                else
                {
                    bombPivot = player->positionCenter;
                }
                angleX = bombPivot.x - player->bombInfo.bombRegionPositions[i].x;
                angleY = bombPivot.y - player->bombInfo.bombRegionPositions[i].y;

                vecLength = sqrtf(ZUN_SQUARE(angleX) + ZUN_SQUARE(angleY)) /
                            (player->bombInfo.reimuABombProjectilesRelated[i] / 8.0f);
                if (vecLength < 1.0f)
                {
                    vecLength = 1.0f;
                }
                angleX = angleX / vecLength + player->bombInfo.bombRegionVelocities[i].x;
                angleY = angleY / vecLength + player->bombInfo.bombRegionVelocities[i].y;
                vecLength = sqrtf(ZUN_SQUARE(angleX) + ZUN_SQUARE(angleY));

                player->bombInfo.reimuABombProjectilesRelated[i] = ZUN_MIN(vecLength, 10.0f);

                if (player->bombInfo.reimuABombProjectilesRelated[i] < 1.0f)
                {
                    player->bombInfo.reimuABombProjectilesRelated[i] = 1.0f;
                }

                player->bombInfo.bombRegionVelocities[i].x =
                    (angleX * player->bombInfo.reimuABombProjectilesRelated[i]) / vecLength;
                player->bombInfo.bombRegionVelocities[i].y =
                    (angleY * player->bombInfo.reimuABombProjectilesRelated[i]) / vecLength;

                player->bombRegionSizes[i].x = 48.0f;
                player->bombRegionSizes[i].y = 48.0f;

                player->bombRegionPositions[i] = player->bombInfo.bombRegionPositions[i];
                player->bombRegionDamages[i] = 8;

                player->bombProjectiles[i].pos.x = player->bombInfo.bombRegionPositions[i].x;
                player->bombProjectiles[i].pos.y = player->bombInfo.bombRegionPositions[i].y;

                player->bombProjectiles[i].size.x = 48.0f;
                player->bombProjectiles[i].size.y = 48.0f;

                if (player->bombRegionTotalDamages[i] >= 100 ||
                    player->bombInfo.timer >= player->bombInfo.duration - 30)
                {
                    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_6, &player->bombInfo.bombRegionPositions[i], 8,
                                                   COLOR_WHITE);
                    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &player->bombInfo.bombRegionPositions[i], 1,
                                                   COLOR_NEONBLUE);
                    player->bombInfo.reimuABombProjectilesState[i] = 2;

                    player->bombInfo.sprites[0][i * 4].pendingInterrupt = 1;
                    player->bombInfo.sprites[0][i * 4 + 1].pendingInterrupt = 1;
                    player->bombInfo.sprites[0][i * 4 + 2].pendingInterrupt = 1;
                    player->bombInfo.sprites[0][i * 4 + 3].pendingInterrupt = 1;

                    player->bombRegionSizes[i].x = 256.0f;
                    player->bombRegionSizes[i].y = 256.0f;

                    player->bombRegionDamages[i] = 200;

                    player->bombProjectiles[i].size.x = 256.0f;
                    player->bombProjectiles[i].size.y = 256.0f;

                    player->bombInfo.bombRegionVelocities[i] / 8.0f; // ZUN moment

                    g_SoundPlayer.PlaySoundByIdx(SOUND_F);
                    ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0);
                }
            }
        }
        else if (player->bombInfo.reimuABombProjectilesState[i] != 0 && player->bombInfo.timer.HasTicked())
        {
            player->bombInfo.reimuABombProjectilesState[i]++;
            if (player->bombInfo.reimuABombProjectilesState[i] >= 30)
            {
                player->bombInfo.reimuABombProjectilesState[i] = 0;
            }
        }
        player->bombInfo.bombRegionPositions[i].x +=
            g_Supervisor.effectiveFramerateMultiplier * player->bombInfo.bombRegionVelocities[i].x;
        player->bombInfo.bombRegionPositions[i].y +=
            g_Supervisor.effectiveFramerateMultiplier * player->bombInfo.bombRegionVelocities[i].y;

        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i * 4]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i * 4 + 1]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i * 4 + 2]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i * 4 + 3]);
    }
    player->bombInfo.timer++;
}

#pragma var_order(darknessLevel, viewport, darkeningTimeLeft)
static void DarkenViewport(Player *player)
{
    ZunRect viewport;
    f32 darkeningTimeLeft;
    i32 darknessLevel; // Controls alpha level of black rectangle drawn over view

    viewport.left = GAME_REGION_POS_X;
    viewport.top = GAME_REGION_POS_Y;
    viewport.right = GAME_REGION_POS_RIGHT;
    viewport.bottom = GAME_REGION_POS_BOTTOM;

    if (player->bombInfo.timer < 60)
    {
        darkeningTimeLeft = ((f32)player->bombInfo.timer * 176.0f) / 60.0f;
        darknessLevel = darkeningTimeLeft >= 176.0f ? 176 : (i32)darkeningTimeLeft;
    }
    else if (player->bombInfo.timer >= player->bombInfo.duration + -60)
    {
        darkeningTimeLeft = ((player->bombInfo.duration - (f32)player->bombInfo.timer) * 176.0f) / 60.0f;
        darknessLevel = darkeningTimeLeft < 0.0f ? 0 : (i32)darkeningTimeLeft;
    }
    else
    {
        darknessLevel = 176;
    }

    ScreenEffect_DrawSquare(&viewport, darknessLevel << 24);
}

void BombReimuADraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *bombSprite = &player->bombInfo.sprites[0][0];
    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.sprites); i++)
    {
        if (player->bombInfo.reimuABombProjectilesState[i] == 0)
        {
            bombSprite = &bombSprite[4];
            continue;
        }

        bombSprite->pos = player->bombInfo.bombRegionPositions[i] + bombSprite->posOffset;
        bombSprite->pos[0] += g_GameManager.gameRegionScreenPos.x;
        bombSprite->pos[1] += g_GameManager.gameRegionScreenPos.y;
        bombSprite->pos[2] = 0.0f;
        g_AnmManager->DrawNoRotation(bombSprite);
        bombSprite++;

        bombSprite->pos = player->bombInfo.bombRegionPositions[i] + bombSprite->posOffset;
        bombSprite->pos[0] += g_GameManager.gameRegionScreenPos.x;
        bombSprite->pos[1] += g_GameManager.gameRegionScreenPos.y;
        bombSprite->pos[2] = 0.0f;
        g_AnmManager->DrawNoRotation(bombSprite);
        bombSprite++;

        bombSprite->pos = player->bombInfo.bombRegionPositions[i] + bombSprite->posOffset;
        bombSprite->pos[0] += g_GameManager.gameRegionScreenPos.x;
        bombSprite->pos[1] += g_GameManager.gameRegionScreenPos.y;
        bombSprite->pos[2] = 0.0f;
        g_AnmManager->DrawNoRotation(bombSprite);
        bombSprite++;

        bombSprite->pos = player->bombInfo.bombRegionPositions[i] + bombSprite->posOffset;
        bombSprite->pos[0] += g_GameManager.gameRegionScreenPos.x;
        bombSprite->pos[1] += g_GameManager.gameRegionScreenPos.y;
        bombSprite->pos[2] = 0.0f;
        g_AnmManager->DrawNoRotation(bombSprite);
        bombSprite++;
    }
}

#pragma var_order(i, bombSprite, unusedVector)
void BombReimuBCalc(Player *player)
{
    AnmVm *bombSprite;
    i32 i;
    D3DXVECTOR3 unusedVector;

    if (player->bombInfo.timer >= player->bombInfo.duration)
    {
        g_Gui.EndPlayerSpellcard();
        player->bombInfo.isInUse = false;
        return;
    }

    if (player->bombInfo.timer.HasTicked() && player->bombInfo.timer == 0)
    {
        g_ItemManager.MagnetAllItems();
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_ENEMY_SPELLCARD_PORTRAIT, TH_REIMU_B_BOMB_NAME);
        player->bombInfo.duration = 140;
        player->invulnerabilityTimer = 200;
        bombSprite = player->bombInfo.sprites[0];

        for (i = 0; i < 4; i++, bombSprite++)
        {
            g_AnmManager->ExecuteAnmIdx(bombSprite, ANM_SCRIPT_PLAYER_REIMU_B_BOMB_ARRAY + i);
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_REIMARI);
        player->bombInfo.bombRegionPositions[0].x = player->positionCenter.x;
        player->bombInfo.bombRegionPositions[0].y = GAME_REGION_HEIGHT / 2.0f;
        player->bombInfo.bombRegionPositions[0].z = 0.42f;
        player->bombInfo.bombRegionPositions[1].x = GAME_REGION_WIDTH / 2.0f;
        player->bombInfo.bombRegionPositions[1].y = player->positionCenter.y;
        player->bombInfo.bombRegionPositions[1].z = 0.415f;
        player->bombInfo.bombRegionPositions[2].x = player->positionCenter.x;
        player->bombInfo.bombRegionPositions[2].y = GAME_REGION_HEIGHT / 2.0f;
        player->bombInfo.bombRegionPositions[2].z = 0.41f;
        player->bombInfo.bombRegionPositions[3].x = GAME_REGION_WIDTH / 2.0f;
        player->bombInfo.bombRegionPositions[3].y = player->positionCenter.y;
        player->bombInfo.bombRegionPositions[3].z = 0.405f;
        ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 60, 2, 6, 0);
    }
    else
    {
        if (player->bombInfo.timer == 60)
        {
            ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 80, 20, 0, 0);
        }

        player->bombProjectiles[0].size.x = 62.0f;
        player->bombProjectiles[0].size.y = GAME_REGION_HEIGHT;
        player->bombProjectiles[1].size.x = GAME_REGION_WIDTH;
        player->bombProjectiles[1].size.y = 62.0f;
        player->bombProjectiles[2].size.x = 62.0f;
        player->bombProjectiles[2].size.y = GAME_REGION_HEIGHT;
        player->bombProjectiles[3].size.x = GAME_REGION_WIDTH;
        player->bombProjectiles[3].size.y = 62.0f;

        for (i = 0; i < 4; i++)
        {
            g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i]);
            if (player->bombInfo.timer.HasTicked() && !MULTIPLE_OF(player->bombInfo.timer, 2))
            {
                player->bombProjectiles[i].pos.x =
                    player->bombInfo.bombRegionPositions[i].x + player->bombInfo.sprites[0][i].posOffset.x;
                player->bombProjectiles[i].pos.y =
                    player->bombInfo.bombRegionPositions[i].y + player->bombInfo.sprites[0][i].posOffset.y;
                player->bombRegionSizes[i].x = player->bombProjectiles[i].size.x;
                player->bombRegionSizes[i].y = player->bombProjectiles[i].size.y;
                player->bombRegionPositions[i] =
                    player->bombInfo.bombRegionPositions[i] + player->bombInfo.sprites[0][i].posOffset;
                player->bombRegionDamages[i] = 8;
            }
        }
    }

    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bombInfo.timer++;
}

void BombReimuBDraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *bombSprite = player->bombInfo.sprites[0];
    for (i32 i = 0; i < 4; i++, bombSprite++)
    {
        bombSprite->pos = player->bombInfo.bombRegionPositions[i] + bombSprite->posOffset;
        bombSprite->pos.x += g_GameManager.gameRegionScreenPos.x;
        bombSprite->pos.y += g_GameManager.gameRegionScreenPos.y;
        bombSprite->pos.z = 0.0f;
        g_AnmManager->Draw(bombSprite);
    }
}

#pragma var_order(i, starSprite, unusedVector)
void BombMarisaACalc(Player *player)
{
    AnmVm *starSprite;
    i32 i;
    D3DXVECTOR3 unusedVector;

    if (player->bombInfo.timer >= player->bombInfo.duration)
    {
        g_Gui.EndPlayerSpellcard();
        player->bombInfo.isInUse = false;
        return;
    }

    if (player->bombInfo.timer.HasTicked() && player->bombInfo.timer == 0)
    {
        g_ItemManager.MagnetAllItems();
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_ENEMY_SPELLCARD_PORTRAIT, TH_MARISA_A_BOMB_NAME);
        player->bombInfo.duration = 250;
        player->invulnerabilityTimer = 300;

        starSprite = player->bombInfo.sprites[0];
        for (i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.sprites); i++, starSprite++)
        {
            g_AnmManager->ExecuteAnmIdx(starSprite, ANM_SCRIPT_PLAYER_MARISA_A_BLUE_STAR + i % 3);
            player->bombInfo.bombRegionPositions[i] = player->positionCenter;

            float starAngle = i * ZUN_2PI / 8.0f;

            player->bombInfo.bombRegionVelocities[i].x = cosf(starAngle) * 2.0f;
            player->bombInfo.bombRegionVelocities[i].y = sinf(starAngle) * 2.0f;
            player->bombInfo.bombRegionVelocities[i].z = 0.0f;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_REIMARI);
        ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 120, 4, 1, 0);
    }
    else
    {
        for (i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.sprites); i++)
        {
            player->bombInfo.bombRegionPositions[i] +=
                player->bombInfo.bombRegionVelocities[i] * g_Supervisor.effectiveFramerateMultiplier;

            if (player->bombInfo.timer.HasTicked() && !MULTIPLE_OF(player->bombInfo.timer, 3))
            {
                player->bombProjectiles[i].pos.x = player->bombInfo.bombRegionPositions[i].x;
                player->bombProjectiles[i].pos.y = player->bombInfo.bombRegionPositions[i].y;
                player->bombProjectiles[i].size.x = 128.0f;
                player->bombProjectiles[i].size.y = 128.0f;
                player->bombRegionSizes[i].x = 128.0f;
                player->bombRegionSizes[i].y = 128.0f;

                player->bombRegionPositions[i] = player->bombInfo.bombRegionPositions[i];
                player->bombRegionDamages[i] = 8;
            }
            g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i]);
        }
    }
    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bombInfo.timer++;
}

void BombMarisaADraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *bombSprite = &player->bombInfo.sprites[0][0];
    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.sprites); i++)
    {
        bombSprite->pos = player->bombInfo.bombRegionPositions[i];
        bombSprite->pos.x += g_GameManager.gameRegionScreenPos.x;
        bombSprite->pos.y += g_GameManager.gameRegionScreenPos.y;
        bombSprite->pos.z = 0.0f;
        bombSprite->scaleX = 3.2f;
        bombSprite->scaleY = 3.2f;
        g_AnmManager->Draw(bombSprite);

        bombSprite->pos -= player->bombInfo.bombRegionVelocities[i] * 6.0f;
        bombSprite->pos.x += -32.0f;
        bombSprite->pos.y += -32.0f;
        bombSprite->pos.z = 0.0f;
        bombSprite->scaleX = 2.2f;
        bombSprite->scaleY = 2.2f;
        g_AnmManager->Draw(bombSprite);

        bombSprite->pos -= player->bombInfo.bombRegionVelocities[i] * 2.0f;
        bombSprite->pos.x += 64.0f;
        bombSprite->pos.y += 64.0f;
        bombSprite->pos.z = 0.0f;

        bombSprite->pos -= player->bombInfo.bombRegionVelocities[i] * 2.0f;
        bombSprite->pos.x += -32.0f;
        bombSprite->pos.y += -32.0f;
        bombSprite->pos.z = 0.0f;
        bombSprite->scaleX = 1.0f;
        bombSprite->scaleY = 1.0f;
        g_AnmManager->Draw(bombSprite);
        bombSprite++;
    }
}

#pragma var_order(i, bombSprite, unusedVector)
void BombMarisaBCalc(Player *player)
{
    AnmVm *bombSprite;
    i32 i;
    D3DXVECTOR3 unusedVector;

    if (player->bombInfo.timer >= player->bombInfo.duration)
    {
        g_Gui.EndPlayerSpellcard();
        player->bombInfo.isInUse = false;
        player->speedMultiplierDuringBomb.x = player->speedMultiplierDuringBomb.y = 1.0f;
        return;
    }

    if (player->bombInfo.timer.HasTicked() && player->bombInfo.timer == 0)
    {
        g_ItemManager.MagnetAllItems();
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_BOMB_PORTRAIT, TH_MARISA_B_BOMB_NAME);
        player->bombInfo.duration = 300;
        player->invulnerabilityTimer = 360;
        bombSprite = player->bombInfo.sprites[0];
        for (i = 0; i < 4; i++, bombSprite++)
        {
            g_AnmManager->ExecuteAnmIdx(bombSprite, ANM_SCRIPT_PLAYER_MARISA_B_MASTER_SPARK + i);
            player->bombInfo.bombRegionPositions[i] = player->positionCenter;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_MARISA_B);
        player->speedMultiplierDuringBomb.x = player->speedMultiplierDuringBomb.y = 0.3f;
    }
    else
    {
        if (player->bombInfo.timer == 60)
        {
            ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 60, 1, 7, 0);
        }
        else if (player->bombInfo.timer == 120)
        {
            ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 200, 24, 0, 0);
        }

        if (player->bombInfo.timer.HasTicked() && !MULTIPLE_OF(player->bombInfo.timer, 4))
        {
            player->bombProjectiles[0].pos.x = GAME_REGION_WIDTH / 2.0f;
            player->bombProjectiles[0].pos.y = player->positionCenter.y / 2.0f;
            player->bombProjectiles[0].size.x = GAME_REGION_WIDTH;
            player->bombProjectiles[0].size.y = player->positionCenter.y;
            player->bombRegionSizes[0].x = GAME_REGION_WIDTH;
            player->bombRegionSizes[0].y = player->positionCenter.y;
            player->bombRegionPositions[0].x = player->bombProjectiles[0].pos.x;
            player->bombRegionPositions[0].y = player->bombProjectiles[0].pos.y;
            player->bombRegionDamages[0] = 12;
        }

        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][0]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][1]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][2]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][3]);
    }

    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bombInfo.timer++;
}

void BombMarisaBDraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *bombSprite = player->bombInfo.sprites[0];
    for (i32 i = 0; i < 4; i++)
    {
        float spriteAngle = (((ZUN_PI / 5.0f) * i) / 3.0f - ZUN_PI) + (ZUN_2PI / 5.0f);
        bombSprite->pos = player->positionCenter;
        bombSprite->pos.x += (cosf(spriteAngle) * bombSprite->sprite->heightPx * bombSprite->scaleY) / 2.0f;
        bombSprite->pos.y += (sinf(spriteAngle) * bombSprite->sprite->heightPx * bombSprite->scaleY) / 2.0f;
        spriteAngle = ZUN_HALF_PI - spriteAngle;
        bombSprite->rotation.z = utils::AddNormalizeAngle(spriteAngle, ZUN_PI);
        bombSprite->pos.x += g_GameManager.gameRegionScreenPos.x;
        bombSprite->pos.y += g_GameManager.gameRegionScreenPos.y;
        bombSprite->pos.z = 0.0f;
        g_AnmManager->Draw(bombSprite);
        bombSprite++;
    }
}
} // namespace th06
