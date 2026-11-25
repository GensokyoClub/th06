#include "stdafx.h"

#include "BombData.hpp"

#include "EffectManager.hpp"
#include "ItemManager.hpp"
#include "ScreenEffect.hpp"

namespace th06
{

#pragma var_order(angleY, angleX, i, vm, vecLength, bombPivot)
void BombReimuACalc(Player *player)
{
    i32 i;
    f32 vecLength;
    D3DXVECTOR3 bombPivot;
    AnmVm *vm;
    f32 angleX;
    f32 angleY;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndPlayerSpellcard();
        player->bomb.isActive = false;
        return;
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer == 0)
    {
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_BOMB_PORTRAIT, TH_REIMU_A_BOMB_NAME);
        player->bomb.duration = 300;
        player->stateTimer = 360;

        for (i = 0; i < REIMU_A_BOMB_PROJECTILES; i++)
        {
            player->bomb.projectileState[i] = 0;
        }
        g_ItemManager.MagnetAllItems();
        g_EffectManager.SpawnParticles(PARTICLE_FLASH, &player->position, 1, COLOR_NEONBLUE);

        player->cancelRegions[8].pos.x = player->position.x;
        player->cancelRegions[8].pos.y = player->position.y;

        player->cancelRegions[8].size.x = 256.0f;
        player->cancelRegions[8].size.y = 256.0f;
    }
    if (player->bomb.timer >= 60 && player->bomb.timer < 180)
    {
        if (MULTIPLE_OF(player->bomb.timer, 16))
        {
            i = ((i32)player->bomb.timer - 60) / 16;
            if (i != 0)
            {
                player->bomb.projectileState[i] = 1;
                player->bomb.projectileCustom[i] = 4.0f;
                player->bomb.projectilePosition[i] = player->position;

                angleX = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;

                player->bomb.projectileVelocity[i].x = cosf(angleX) * player->bomb.projectileCustom[i];
                player->bomb.projectileVelocity[i].y = sinf(angleX) * player->bomb.projectileCustom[i];
                player->damageRegionDamageDealt[i] = 0;

                vm = &player->bomb.vms[i * VMS_PER_BOMB_PROJECTILE];
                for (i32 j = 0; j < VMS_PER_BOMB_PROJECTILE; j++, vm++)
                {
                    g_AnmManager->SetNewScript(vm, ANM_SCRIPT_FANTASY_SEAL_START + j);
                }
                g_SoundPlayer.PlaySoundId(SOUND_FANTASY_SEAL_START);
            }
        }
    }
    player->playerState = PLAYER_STATE_INVULNERABLE;
    for (i = 0; i < REIMU_A_BOMB_PROJECTILES; i++)
    {
        if (player->bomb.projectileState[i] == 0)
        {
            continue;
        }
        if (player->bomb.projectileState[i] == 1)
        {
            if (player->bomb.timer.HasTicked())
            {
                if (player->positionOfLastEnemyHit.x > -100.0f)
                {
                    bombPivot = player->positionOfLastEnemyHit;
                }
                else
                {
                    bombPivot = player->position;
                }
                angleX = bombPivot.x - player->bomb.projectilePosition[i].x;
                angleY = bombPivot.y - player->bomb.projectilePosition[i].y;

                vecLength = sqrtf(ZUN_SQUARE(angleX) + ZUN_SQUARE(angleY)) / (player->bomb.projectileCustom[i] / 8.0f);
                if (vecLength < 1.0f)
                {
                    vecLength = 1.0f;
                }
                angleX = angleX / vecLength + player->bomb.projectileVelocity[i].x;
                angleY = angleY / vecLength + player->bomb.projectileVelocity[i].y;
                vecLength = sqrtf(ZUN_SQUARE(angleX) + ZUN_SQUARE(angleY));

                player->bomb.projectileCustom[i] = ZUN_MIN(vecLength, 10.0f);

                if (player->bomb.projectileCustom[i] < 1.0f)
                {
                    player->bomb.projectileCustom[i] = 1.0f;
                }

                player->bomb.projectileVelocity[i].x = (angleX * player->bomb.projectileCustom[i]) / vecLength;
                player->bomb.projectileVelocity[i].y = (angleY * player->bomb.projectileCustom[i]) / vecLength;

                player->damageRegionSize[i].x = 48.0f;
                player->damageRegionSize[i].y = 48.0f;

                player->damageRegionPosition[i] = player->bomb.projectilePosition[i];
                player->damageRegionDamage[i] = 8;

                player->cancelRegions[i].pos.x = player->bomb.projectilePosition[i].x;
                player->cancelRegions[i].pos.y = player->bomb.projectilePosition[i].y;

                player->cancelRegions[i].size.x = 48.0f;
                player->cancelRegions[i].size.y = 48.0f;

                if (player->damageRegionDamageDealt[i] >= 100 || player->bomb.timer >= player->bomb.duration - 30)
                {
                    g_EffectManager.SpawnParticles(PARTICLE_SPARK_GREEN, &player->bomb.projectilePosition[i], 8);
                    g_EffectManager.SpawnParticles(PARTICLE_FLASH, &player->bomb.projectilePosition[i], 1,
                                                   COLOR_NEONBLUE);
                    player->bomb.projectileState[i] = 2;

                    player->bomb.vms[i * VMS_PER_BOMB_PROJECTILE + 0].pendingInterrupt = 1;
                    player->bomb.vms[i * VMS_PER_BOMB_PROJECTILE + 1].pendingInterrupt = 1;
                    player->bomb.vms[i * VMS_PER_BOMB_PROJECTILE + 2].pendingInterrupt = 1;
                    player->bomb.vms[i * VMS_PER_BOMB_PROJECTILE + 3].pendingInterrupt = 1;

                    player->damageRegionSize[i].x = 256.0f;
                    player->damageRegionSize[i].y = 256.0f;

                    player->damageRegionDamage[i] = 200;

                    player->cancelRegions[i].size.x = 256.0f;
                    player->cancelRegions[i].size.y = 256.0f;

                    player->bomb.projectileVelocity[i] / 8.0f; // ZUN moment

                    g_SoundPlayer.PlaySoundId(SOUND_FANTASY_SEAL_ATTACK);
                    ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0, 0);
                }
            }
        }
        else if (player->bomb.projectileState[i] != 0 && player->bomb.timer.HasTicked())
        {
            if (++player->bomb.projectileState[i] >= 30)
            {
                player->bomb.projectileState[i] = 0;
            }
        }
        player->bomb.projectilePosition[i].x += g_Supervisor.gameSpeed * player->bomb.projectileVelocity[i].x;
        player->bomb.projectilePosition[i].y += g_Supervisor.gameSpeed * player->bomb.projectileVelocity[i].y;

        g_AnmManager->RunAnm(&player->bomb.vms[i * VMS_PER_BOMB_PROJECTILE + 0]);
        g_AnmManager->RunAnm(&player->bomb.vms[i * VMS_PER_BOMB_PROJECTILE + 1]);
        g_AnmManager->RunAnm(&player->bomb.vms[i * VMS_PER_BOMB_PROJECTILE + 2]);
        g_AnmManager->RunAnm(&player->bomb.vms[i * VMS_PER_BOMB_PROJECTILE + 3]);
    }
    player->bomb.timer++;
}

#pragma var_order(darknessLevel, viewport, darkeningTimeLeft)
static void DarkenViewport(Player *player)
{
    FloatRect viewport;
    f32 darkeningTimeLeft;
    i32 darknessLevel; // Controls alpha level of black rectangle drawn over view

    viewport.left = GAME_REGION_POS_X;
    viewport.top = GAME_REGION_POS_Y;
    viewport.right = GAME_REGION_POS_RIGHT;
    viewport.bottom = GAME_REGION_POS_BOTTOM;

    if (player->bomb.timer < 60)
    {
        darkeningTimeLeft = RAMP_UP(176.0f, (f32)player->bomb.timer, 60.0f);
        darknessLevel = darkeningTimeLeft >= 176.0f ? 176 : (i32)darkeningTimeLeft;
    }
    else if (player->bomb.timer >= player->bomb.duration + -60)
    {
        darkeningTimeLeft = RAMP_UP(176.0f, player->bomb.duration - (f32)player->bomb.timer, 60.0f);
        darknessLevel = darkeningTimeLeft < 0.0f ? 0 : (i32)darkeningTimeLeft;
    }
    else
    {
        darknessLevel = 176;
    }

    ScreenEffect_DrawRect(&viewport, COLOR_SET_ALPHA(COLOR_BLACK, darknessLevel));
}

void BombReimuADraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *vm = &player->bomb.vms[0];
    for (i32 i = 0; i < REIMU_A_BOMB_PROJECTILES; i++)
    {
        if (player->bomb.projectileState[i] == 0)
        {
            vm += VMS_PER_BOMB_PROJECTILE;
            continue;
        }

        vm->pos = player->bomb.projectilePosition[i] + vm->offset;
        vm->pos[0] += g_GameManager.gameRegionPosition.x;
        vm->pos[1] += g_GameManager.gameRegionPosition.y;
        vm->pos[2] = 0.0f;
        g_AnmManager->Draw2D(vm);
        vm++;

        vm->pos = player->bomb.projectilePosition[i] + vm->offset;
        vm->pos[0] += g_GameManager.gameRegionPosition.x;
        vm->pos[1] += g_GameManager.gameRegionPosition.y;
        vm->pos[2] = 0.0f;
        g_AnmManager->Draw2D(vm);
        vm++;

        vm->pos = player->bomb.projectilePosition[i] + vm->offset;
        vm->pos[0] += g_GameManager.gameRegionPosition.x;
        vm->pos[1] += g_GameManager.gameRegionPosition.y;
        vm->pos[2] = 0.0f;
        g_AnmManager->Draw2D(vm);
        vm++;

        vm->pos = player->bomb.projectilePosition[i] + vm->offset;
        vm->pos[0] += g_GameManager.gameRegionPosition.x;
        vm->pos[1] += g_GameManager.gameRegionPosition.y;
        vm->pos[2] = 0.0f;
        g_AnmManager->Draw2D(vm);
        vm++;
    }
}

#pragma var_order(i, vm, unusedVector)
void BombReimuBCalc(Player *player)
{
    AnmVm *vm;
    i32 i;
    D3DXVECTOR3 unusedVector;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndPlayerSpellcard();
        player->bomb.isActive = false;
        return;
    }

    if (player->bomb.timer.HasTicked() && player->bomb.timer == 0)
    {
        g_ItemManager.MagnetAllItems();
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_ENEMY_SPELLCARD_PORTRAIT, TH_REIMU_B_BOMB_NAME);
        player->bomb.duration = 140;
        player->stateTimer = 200;

        vm = &player->bomb.vms[0];
        for (i = 0; i < REIMU_B_BOMB_PROJECTILES; i++, vm++)
        {
            g_AnmManager->SetNewScript(vm, ANM_SCRIPT_EVIL_SEALING_CIRCLE_START + i);
        }

        g_SoundPlayer.PlaySoundId(SOUND_CHARGE_2);
        player->bomb.projectilePosition[0].x = player->position.x;
        player->bomb.projectilePosition[0].y = GAME_REGION_CENTER_Y;
        player->bomb.projectilePosition[0].z = 0.42f;
        player->bomb.projectilePosition[1].x = GAME_REGION_CENTER_X;
        player->bomb.projectilePosition[1].y = player->position.y;
        player->bomb.projectilePosition[1].z = 0.415f;
        player->bomb.projectilePosition[2].x = player->position.x;
        player->bomb.projectilePosition[2].y = GAME_REGION_CENTER_Y;
        player->bomb.projectilePosition[2].z = 0.41f;
        player->bomb.projectilePosition[3].x = GAME_REGION_CENTER_X;
        player->bomb.projectilePosition[3].y = player->position.y;
        player->bomb.projectilePosition[3].z = 0.405f;
        ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 60, 2, 6, 0);
    }
    else
    {
        if (player->bomb.timer == 60)
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

        for (i = 0; i < REIMU_B_BOMB_PROJECTILES; i++)
        {
            g_AnmManager->RunAnm(&player->bomb.vms[i]);
            if (player->bomb.timer.HasTicked() && !MULTIPLE_OF(player->bomb.timer, 2))
            {
                player->cancelRegions[i].pos.x = player->bomb.projectilePosition[i].x + player->bomb.vms[i].offset.x;
                player->cancelRegions[i].pos.y = player->bomb.projectilePosition[i].y + player->bomb.vms[i].offset.y;
                player->damageRegionSize[i].x = player->cancelRegions[i].size.x;
                player->damageRegionSize[i].y = player->cancelRegions[i].size.y;
                player->damageRegionPosition[i] = player->bomb.projectilePosition[i] + player->bomb.vms[i].offset;
                player->damageRegionDamage[i] = 8;
            }
        }
    }

    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bomb.timer++;
}

void BombReimuBDraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *vm = &player->bomb.vms[0];
    for (i32 i = 0; i < REIMU_B_BOMB_PROJECTILES; i++, vm++)
    {
        vm->pos = player->bomb.projectilePosition[i] + vm->offset;
        vm->pos.x += g_GameManager.gameRegionPosition.x;
        vm->pos.y += g_GameManager.gameRegionPosition.y;
        vm->pos.z = 0.0f;
        g_AnmManager->Draw2DRotated(vm);
    }
}

#pragma var_order(i, vm, unusedVector)
void BombMarisaACalc(Player *player)
{
    AnmVm *vm;
    i32 i;
    D3DXVECTOR3 unusedVector;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndPlayerSpellcard();
        player->bomb.isActive = false;
        return;
    }

    if (player->bomb.timer.HasTicked() && player->bomb.timer == 0)
    {
        g_ItemManager.MagnetAllItems();
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_ENEMY_SPELLCARD_PORTRAIT, TH_MARISA_A_BOMB_NAME);
        player->bomb.duration = 250;
        player->stateTimer = 300;

        vm = &player->bomb.vms[0];
        for (i = 0; i < MARISA_A_BOMB_PROJECTILES; i++, vm++)
        {
            // Modulo 3 for the 3 colors of stars.
            // Since there are only 8 VMs the color distribution isn't evenly split.
            // Blue: 3
            // Green: 3
            // Red: 2
            g_AnmManager->SetNewScript(vm, ANM_SCRIPT_STARDUST_REVERIE_START + i % 3);
            player->bomb.projectilePosition[i] = player->position;

            float starAngle = i * ZUN_2PI / 8.0f;
            player->bomb.projectileVelocity[i].x = cosf(starAngle) * 2.0f;
            player->bomb.projectileVelocity[i].y = sinf(starAngle) * 2.0f;
            player->bomb.projectileVelocity[i].z = 0.0f;
        }
        g_SoundPlayer.PlaySoundId(SOUND_CHARGE_2);
        ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 120, 4, 1, 0);
    }
    else
    {
        for (i = 0; i < MARISA_A_BOMB_PROJECTILES; i++)
        {
            player->bomb.projectilePosition[i] += player->bomb.projectileVelocity[i] * g_Supervisor.gameSpeed;

            if (player->bomb.timer.HasTicked() && !MULTIPLE_OF(player->bomb.timer, 3))
            {
                player->cancelRegions[i].pos.x = player->bomb.projectilePosition[i].x;
                player->cancelRegions[i].pos.y = player->bomb.projectilePosition[i].y;
                player->cancelRegions[i].size.x = 128.0f;
                player->cancelRegions[i].size.y = 128.0f;
                player->damageRegionSize[i].x = 128.0f;
                player->damageRegionSize[i].y = 128.0f;
                player->damageRegionPosition[i] = player->bomb.projectilePosition[i];
                player->damageRegionDamage[i] = 8;
            }
            g_AnmManager->RunAnm(&player->bomb.vms[i]);
        }
    }
    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bomb.timer++;
}

void BombMarisaADraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *vm = &player->bomb.vms[0];
    for (i32 i = 0; i < MARISA_A_BOMB_PROJECTILES; i++)
    {
        vm->pos = player->bomb.projectilePosition[i];
        vm->pos.x += g_GameManager.gameRegionPosition.x;
        vm->pos.y += g_GameManager.gameRegionPosition.y;
        vm->pos.z = 0.0f;
        vm->scaleX = 3.2f;
        vm->scaleY = 3.2f;
        g_AnmManager->Draw2DRotated(vm);

        vm->pos -= player->bomb.projectileVelocity[i] * 6.0f;
        vm->pos.x += -32.0f;
        vm->pos.y += -32.0f;
        vm->pos.z = 0.0f;
        vm->scaleX = 2.2f;
        vm->scaleY = 2.2f;
        g_AnmManager->Draw2DRotated(vm);

        vm->pos -= player->bomb.projectileVelocity[i] * 2.0f;
        vm->pos.x += 64.0f;
        vm->pos.y += 64.0f;
        vm->pos.z = 0.0f;

        vm->pos -= player->bomb.projectileVelocity[i] * 2.0f;
        vm->pos.x += -32.0f;
        vm->pos.y += -32.0f;
        vm->pos.z = 0.0f;
        vm->scaleX = 1.0f;
        vm->scaleY = 1.0f;
        g_AnmManager->Draw2DRotated(vm);
        vm++;
    }
}

#pragma var_order(i, vm, unusedVector)
void BombMarisaBCalc(Player *player)
{
    AnmVm *vm;
    i32 i;
    D3DXVECTOR3 unusedVector;

    if (player->bomb.timer >= player->bomb.duration)
    {
        g_Gui.EndPlayerSpellcard();
        player->bomb.isActive = false;
        player->speedMultiplierDuringBomb.x = player->speedMultiplierDuringBomb.y = 1.0f;
        return;
    }

    if (player->bomb.timer.HasTicked() && player->bomb.timer == 0)
    {
        g_ItemManager.MagnetAllItems();
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_BOMB_PORTRAIT, TH_MARISA_B_BOMB_NAME);
        player->bomb.duration = 300;
        player->stateTimer = 360;

        vm = &player->bomb.vms[0];
        for (i = 0; i < MARISA_B_BOMB_PROJECTILES; i++, vm++)
        {
            g_AnmManager->SetNewScript(vm, ANM_SCRIPT_MASTER_SPARK_START + i);
            player->bomb.projectilePosition[i] = player->position;
        }

        g_SoundPlayer.PlaySoundId(SOUND_MASTER_SPARK);
        player->speedMultiplierDuringBomb.x = player->speedMultiplierDuringBomb.y = 0.3f;
    }
    else
    {
        if (player->bomb.timer == 60)
        {
            ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 60, 1, 7, 0);
        }
        else if (player->bomb.timer == 120)
        {
            ScreenEffect_RegisterChain(SCREEN_EFFECT_SHAKE, 200, 24, 0, 0);
        }

        if (player->bomb.timer.HasTicked() && !MULTIPLE_OF(player->bomb.timer, 4))
        {
            player->cancelRegions[0].pos.x = GAME_REGION_CENTER_X;
            player->cancelRegions[0].pos.y = player->position.y / 2.0f;
            player->cancelRegions[0].size.x = GAME_REGION_WIDTH;
            player->cancelRegions[0].size.y = player->position.y;
            player->damageRegionSize[0].x = GAME_REGION_WIDTH;
            player->damageRegionSize[0].y = player->position.y;
            player->damageRegionPosition[0].x = player->cancelRegions[0].pos.x;
            player->damageRegionPosition[0].y = player->cancelRegions[0].pos.y;
            player->damageRegionDamage[0] = 12;
        }

        g_AnmManager->RunAnm(&player->bomb.vms[0]);
        g_AnmManager->RunAnm(&player->bomb.vms[1]);
        g_AnmManager->RunAnm(&player->bomb.vms[2]);
        g_AnmManager->RunAnm(&player->bomb.vms[3]);
    }

    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bomb.timer++;
}

void BombMarisaBDraw(Player *player)
{
    DarkenViewport(player);
    AnmVm *vm = &player->bomb.vms[0];
    for (i32 i = 0; i < MARISA_B_BOMB_PROJECTILES; i++)
    {
        float angle = (((ZUN_PI / 5.0f) * i) / 3.0f - ZUN_PI) + (ZUN_2PI / 5.0f);
        vm->pos = player->position;
        vm->pos.x += (cosf(angle) * vm->sprite->height * vm->scaleY) / 2.0f;
        vm->pos.y += (sinf(angle) * vm->sprite->height * vm->scaleY) / 2.0f;
        angle = ZUN_HALF_PI - angle;
        vm->rotation.z = ReduceAngleAdd(angle, ZUN_PI);
        vm->pos.x += g_GameManager.gameRegionPosition.x;
        vm->pos.y += g_GameManager.gameRegionPosition.y;
        vm->pos.z = 0.0f;
        g_AnmManager->Draw2DRotated(vm);
        vm++;
    }
}
} // namespace th06
