#include "stdafx.h"

#include "BombData.hpp"

#include "EffectManager.hpp"
#include "ItemManager.hpp"
#include "ScreenEffect.hpp"

namespace th06
{

#pragma var_order(angleY, angleX, i, vm, vecLength, bombPivot, bombIdx)
void BombReimuACalc(Player *player)
{
    i32 i;
    f32 vecLength;
    i32 bombIdx;
    D3DXVECTOR3 bombPivot;
    AnmVm *vm;
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
        player->stateTimer = 360;

        for (i = 0; i < 8; i++)
        {
            player->bombInfo.projectileState[i] = 0;
        }
        g_ItemManager.MagnetAllItems();
        g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &player->positionCenter, 1, COLOR_NEONBLUE);

        player->cancelRegions[8].pos.x = player->positionCenter.x;
        player->cancelRegions[8].pos.y = player->positionCenter.y;

        player->cancelRegions[8].size.x = 256.0f;
        player->cancelRegions[8].size.y = 256.0f;
    }
    if (player->bombInfo.timer >= 60 && player->bombInfo.timer < 180)
    {
        if (MULTIPLE_OF(player->bombInfo.timer, 16))
        {
            i = ((i32)player->bombInfo.timer - 60) / 16;
            if (i != 0)
            {
                player->bombInfo.projectileState[i] = 1;
                player->bombInfo.projectileCustom[i] = 4.0f;
                player->bombInfo.projectilePosition[i] = player->positionCenter;

                angleX = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;

                player->bombInfo.projectileVelocity[i].x =
                    cosf(angleX) * player->bombInfo.projectileCustom[i];

                player->bombInfo.projectileVelocity[i].y =
                    sinf(angleX) * player->bombInfo.projectileCustom[i];
                player->damageRegionDamageDealt[i] = 0;

                vm = &player->bombInfo.vms[0][i * 4];
                for (bombIdx = 0; bombIdx < 4; bombIdx++, vm++)
                {
                    g_AnmManager->ExecuteAnmIdx(vm, ANM_SCRIPT_PLAYER_REIMU_A_BOMB_ARRAY + bombIdx);
                }
                g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_REIMU_A);
            }
        }
    }
    player->playerState = PLAYER_STATE_INVULNERABLE;
    for (i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.projectileState); i++)
    {
        if (player->bombInfo.projectileState[i] == 0)
        {
            continue;
        }
        if (player->bombInfo.projectileState[i] == 1)
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
                angleX = bombPivot.x - player->bombInfo.projectilePosition[i].x;
                angleY = bombPivot.y - player->bombInfo.projectilePosition[i].y;

                vecLength = sqrtf(ZUN_SQUARE(angleX) + ZUN_SQUARE(angleY)) /
                            (player->bombInfo.projectileCustom[i] / 8.0f);
                if (vecLength < 1.0f)
                {
                    vecLength = 1.0f;
                }
                angleX = angleX / vecLength + player->bombInfo.projectileVelocity[i].x;
                angleY = angleY / vecLength + player->bombInfo.projectileVelocity[i].y;
                vecLength = sqrtf(ZUN_SQUARE(angleX) + ZUN_SQUARE(angleY));

                player->bombInfo.projectileCustom[i] = ZUN_MIN(vecLength, 10.0f);

                if (player->bombInfo.projectileCustom[i] < 1.0f)
                {
                    player->bombInfo.projectileCustom[i] = 1.0f;
                }

                player->bombInfo.projectileVelocity[i].x =
                    (angleX * player->bombInfo.projectileCustom[i]) / vecLength;
                player->bombInfo.projectileVelocity[i].y =
                    (angleY * player->bombInfo.projectileCustom[i]) / vecLength;

                player->damageRegionSize[i].x = 48.0f;
                player->damageRegionSize[i].y = 48.0f;

                player->damageRegionPosition[i] = player->bombInfo.projectilePosition[i];
                player->damageRegionDamage[i] = 8;

                player->cancelRegions[i].pos.x = player->bombInfo.projectilePosition[i].x;
                player->cancelRegions[i].pos.y = player->bombInfo.projectilePosition[i].y;

                player->cancelRegions[i].size.x = 48.0f;
                player->cancelRegions[i].size.y = 48.0f;

                if (player->damageRegionDamageDealt[i] >= 100 ||
                    player->bombInfo.timer >= player->bombInfo.duration - 30)
                {
                    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_6, &player->bombInfo.projectilePosition[i], 8,
                                                   COLOR_WHITE);
                    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &player->bombInfo.projectilePosition[i], 1,
                                                   COLOR_NEONBLUE);
                    player->bombInfo.projectileState[i] = 2;

                    player->bombInfo.vms[0][i * 4].pendingInterrupt = 1;
                    player->bombInfo.vms[0][i * 4 + 1].pendingInterrupt = 1;
                    player->bombInfo.vms[0][i * 4 + 2].pendingInterrupt = 1;
                    player->bombInfo.vms[0][i * 4 + 3].pendingInterrupt = 1;

                    player->damageRegionSize[i].x = 256.0f;
                    player->damageRegionSize[i].y = 256.0f;

                    player->damageRegionDamage[i] = 200;

                    player->cancelRegions[i].size.x = 256.0f;
                    player->cancelRegions[i].size.y = 256.0f;

                    player->bombInfo.projectileVelocity[i] / 8.0f; // ZUN moment

                    g_SoundPlayer.PlaySoundByIdx(SOUND_15);
                    ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0);
                }
            }
        }
        else if (player->bombInfo.projectileState[i] != 0 && player->bombInfo.timer.HasTicked())
        {
            if (++player->bombInfo.projectileState[i] >= 30)
            {
                player->bombInfo.projectileState[i] = 0;
            }
        }
        player->bombInfo.projectilePosition[i].x +=
            g_Supervisor.effectiveFramerateMultiplier * player->bombInfo.projectileVelocity[i].x;
        player->bombInfo.projectilePosition[i].y +=
            g_Supervisor.effectiveFramerateMultiplier * player->bombInfo.projectileVelocity[i].y;

        g_AnmManager->ExecuteScript(&player->bombInfo.vms[0][i * 4]);
        g_AnmManager->ExecuteScript(&player->bombInfo.vms[0][i * 4 + 1]);
        g_AnmManager->ExecuteScript(&player->bombInfo.vms[0][i * 4 + 2]);
        g_AnmManager->ExecuteScript(&player->bombInfo.vms[0][i * 4 + 3]);
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

    ScreenEffect_DrawSquare(&viewport, COLOR_SET_ALPHA(COLOR_BLACK, darknessLevel));
}

void BombReimuADraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *vm = &player->bombInfo.vms[0][0];
    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.vms); i++)
    {
        if (player->bombInfo.projectileState[i] == 0)
        {
            vm = &vm[4];
            continue;
        }

        vm->pos = player->bombInfo.projectilePosition[i] + vm->posOffset;
        vm->pos[0] += g_GameManager.gameRegionScreenPos.x;
        vm->pos[1] += g_GameManager.gameRegionScreenPos.y;
        vm->pos[2] = 0.0f;
        g_AnmManager->DrawNoRotation(vm);
        vm++;

        vm->pos = player->bombInfo.projectilePosition[i] + vm->posOffset;
        vm->pos[0] += g_GameManager.gameRegionScreenPos.x;
        vm->pos[1] += g_GameManager.gameRegionScreenPos.y;
        vm->pos[2] = 0.0f;
        g_AnmManager->DrawNoRotation(vm);
        vm++;

        vm->pos = player->bombInfo.projectilePosition[i] + vm->posOffset;
        vm->pos[0] += g_GameManager.gameRegionScreenPos.x;
        vm->pos[1] += g_GameManager.gameRegionScreenPos.y;
        vm->pos[2] = 0.0f;
        g_AnmManager->DrawNoRotation(vm);
        vm++;

        vm->pos = player->bombInfo.projectilePosition[i] + vm->posOffset;
        vm->pos[0] += g_GameManager.gameRegionScreenPos.x;
        vm->pos[1] += g_GameManager.gameRegionScreenPos.y;
        vm->pos[2] = 0.0f;
        g_AnmManager->DrawNoRotation(vm);
        vm++;
    }
}

#pragma var_order(i, vm, unusedVector)
void BombReimuBCalc(Player *player)
{
    AnmVm *vm;
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
        player->stateTimer = 200;
        vm = player->bombInfo.vms[0];

        for (i = 0; i < 4; i++, vm++)
        {
            g_AnmManager->ExecuteAnmIdx(vm, ANM_SCRIPT_PLAYER_REIMU_B_BOMB_ARRAY + i);
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_REIMARI);
        player->bombInfo.projectilePosition[0].x = player->positionCenter.x;
        player->bombInfo.projectilePosition[0].y = GAME_REGION_HEIGHT / 2.0f;
        player->bombInfo.projectilePosition[0].z = 0.42f;
        player->bombInfo.projectilePosition[1].x = GAME_REGION_WIDTH / 2.0f;
        player->bombInfo.projectilePosition[1].y = player->positionCenter.y;
        player->bombInfo.projectilePosition[1].z = 0.415f;
        player->bombInfo.projectilePosition[2].x = player->positionCenter.x;
        player->bombInfo.projectilePosition[2].y = GAME_REGION_HEIGHT / 2.0f;
        player->bombInfo.projectilePosition[2].z = 0.41f;
        player->bombInfo.projectilePosition[3].x = GAME_REGION_WIDTH / 2.0f;
        player->bombInfo.projectilePosition[3].y = player->positionCenter.y;
        player->bombInfo.projectilePosition[3].z = 0.405f;
        ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 60, 2, 6, 0);
    }
    else
    {
        if (player->bombInfo.timer == 60)
        {
            ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 80, 20, 0, 0);
        }

        player->cancelRegions[0].size.x = 62.0f;
        player->cancelRegions[0].size.y = GAME_REGION_HEIGHT;
        player->cancelRegions[1].size.x = GAME_REGION_WIDTH;
        player->cancelRegions[1].size.y = 62.0f;
        player->cancelRegions[2].size.x = 62.0f;
        player->cancelRegions[2].size.y = GAME_REGION_HEIGHT;
        player->cancelRegions[3].size.x = GAME_REGION_WIDTH;
        player->cancelRegions[3].size.y = 62.0f;

        for (i = 0; i < 4; i++)
        {
            g_AnmManager->ExecuteScript(&player->bombInfo.vms[0][i]);
            if (player->bombInfo.timer.HasTicked() && !MULTIPLE_OF(player->bombInfo.timer, 2))
            {
                player->cancelRegions[i].pos.x =
                    player->bombInfo.projectilePosition[i].x + player->bombInfo.vms[0][i].posOffset.x;
                player->cancelRegions[i].pos.y =
                    player->bombInfo.projectilePosition[i].y + player->bombInfo.vms[0][i].posOffset.y;
                player->damageRegionSize[i].x = player->cancelRegions[i].size.x;
                player->damageRegionSize[i].y = player->cancelRegions[i].size.y;
                player->damageRegionPosition[i] =
                    player->bombInfo.projectilePosition[i] + player->bombInfo.vms[0][i].posOffset;
                player->damageRegionDamage[i] = 8;
            }
        }
    }

    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bombInfo.timer++;
}

void BombReimuBDraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *vm = player->bombInfo.vms[0];
    for (i32 i = 0; i < 4; i++, vm++)
    {
        vm->pos = player->bombInfo.projectilePosition[i] + vm->posOffset;
        vm->pos.x += g_GameManager.gameRegionScreenPos.x;
        vm->pos.y += g_GameManager.gameRegionScreenPos.y;
        vm->pos.z = 0.0f;
        g_AnmManager->Draw(vm);
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
        player->stateTimer = 300;

        starSprite = player->bombInfo.vms[0];
        for (i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.vms); i++, starSprite++)
        {
            g_AnmManager->ExecuteAnmIdx(starSprite, ANM_SCRIPT_PLAYER_MARISA_A_BLUE_STAR + i % 3);
            player->bombInfo.projectilePosition[i] = player->positionCenter;

            float starAngle = i * ZUN_2PI / 8.0f;

            player->bombInfo.projectileVelocity[i].x = cosf(starAngle) * 2.0f;
            player->bombInfo.projectileVelocity[i].y = sinf(starAngle) * 2.0f;
            player->bombInfo.projectileVelocity[i].z = 0.0f;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_REIMARI);
        ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 120, 4, 1, 0);
    }
    else
    {
        for (i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.vms); i++)
        {
            player->bombInfo.projectilePosition[i] +=
                player->bombInfo.projectileVelocity[i] * g_Supervisor.effectiveFramerateMultiplier;

            if (player->bombInfo.timer.HasTicked() && !MULTIPLE_OF(player->bombInfo.timer, 3))
            {
                player->cancelRegions[i].pos.x = player->bombInfo.projectilePosition[i].x;
                player->cancelRegions[i].pos.y = player->bombInfo.projectilePosition[i].y;
                player->cancelRegions[i].size.x = 128.0f;
                player->cancelRegions[i].size.y = 128.0f;
                player->damageRegionSize[i].x = 128.0f;
                player->damageRegionSize[i].y = 128.0f;

                player->damageRegionPosition[i] = player->bombInfo.projectilePosition[i];
                player->damageRegionDamage[i] = 8;
            }
            g_AnmManager->ExecuteScript(&player->bombInfo.vms[0][i]);
        }
    }
    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bombInfo.timer++;
}

void BombMarisaADraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *vm = &player->bombInfo.vms[0][0];
    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.vms); i++)
    {
        vm->pos = player->bombInfo.projectilePosition[i];
        vm->pos.x += g_GameManager.gameRegionScreenPos.x;
        vm->pos.y += g_GameManager.gameRegionScreenPos.y;
        vm->pos.z = 0.0f;
        vm->scaleX = 3.2f;
        vm->scaleY = 3.2f;
        g_AnmManager->Draw(vm);

        vm->pos -= player->bombInfo.projectileVelocity[i] * 6.0f;
        vm->pos.x += -32.0f;
        vm->pos.y += -32.0f;
        vm->pos.z = 0.0f;
        vm->scaleX = 2.2f;
        vm->scaleY = 2.2f;
        g_AnmManager->Draw(vm);

        vm->pos -= player->bombInfo.projectileVelocity[i] * 2.0f;
        vm->pos.x += 64.0f;
        vm->pos.y += 64.0f;
        vm->pos.z = 0.0f;

        vm->pos -= player->bombInfo.projectileVelocity[i] * 2.0f;
        vm->pos.x += -32.0f;
        vm->pos.y += -32.0f;
        vm->pos.z = 0.0f;
        vm->scaleX = 1.0f;
        vm->scaleY = 1.0f;
        g_AnmManager->Draw(vm);
        vm++;
    }
}

#pragma var_order(i, vm, unusedVector)
void BombMarisaBCalc(Player *player)
{
    AnmVm *vm;
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
        player->stateTimer = 360;
        vm = player->bombInfo.vms[0];
        for (i = 0; i < 4; i++, vm++)
        {
            g_AnmManager->ExecuteAnmIdx(vm, ANM_SCRIPT_PLAYER_MARISA_B_MASTER_SPARK + i);
            player->bombInfo.projectilePosition[i] = player->positionCenter;
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
            player->cancelRegions[0].pos.x = GAME_REGION_WIDTH / 2.0f;
            player->cancelRegions[0].pos.y = player->positionCenter.y / 2.0f;
            player->cancelRegions[0].size.x = GAME_REGION_WIDTH;
            player->cancelRegions[0].size.y = player->positionCenter.y;
            player->damageRegionSize[0].x = GAME_REGION_WIDTH;
            player->damageRegionSize[0].y = player->positionCenter.y;
            player->damageRegionPosition[0].x = player->cancelRegions[0].pos.x;
            player->damageRegionPosition[0].y = player->cancelRegions[0].pos.y;
            player->damageRegionDamage[0] = 12;
        }

        g_AnmManager->ExecuteScript(&player->bombInfo.vms[0][0]);
        g_AnmManager->ExecuteScript(&player->bombInfo.vms[0][1]);
        g_AnmManager->ExecuteScript(&player->bombInfo.vms[0][2]);
        g_AnmManager->ExecuteScript(&player->bombInfo.vms[0][3]);
    }

    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bombInfo.timer++;
}

void BombMarisaBDraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *vm = player->bombInfo.vms[0];
    for (i32 i = 0; i < 4; i++)
    {
        float spriteAngle = (((ZUN_PI / 5.0f) * i) / 3.0f - ZUN_PI) + (ZUN_2PI / 5.0f);
        vm->pos = player->positionCenter;
        vm->pos.x += (cosf(spriteAngle) * vm->sprite->heightPx * vm->scaleY) / 2.0f;
        vm->pos.y += (sinf(spriteAngle) * vm->sprite->heightPx * vm->scaleY) / 2.0f;
        spriteAngle = ZUN_HALF_PI - spriteAngle;
        vm->rotation.z = ReduceAngleAdd(spriteAngle, ZUN_PI);
        vm->pos.x += g_GameManager.gameRegionScreenPos.x;
        vm->pos.y += g_GameManager.gameRegionScreenPos.y;
        vm->pos.z = 0.0f;
        g_AnmManager->Draw(vm);
        vm++;
    }
}
} // namespace th06
