#include "BombData.hpp"

#include <cmath>

#include "EffectManager.hpp"
#include "Gui.hpp"
#include "Rng.hpp"
#include "ScreenEffect.hpp"
#include "i18n.hpp"
#include "utils.hpp"

namespace th06 {
BombData g_BombData[4] = {
    /* ReimuA  */ {BombData::BombReimuACalc, BombData::BombReimuADraw},
    /* ReimuB  */ {BombData::BombReimuBCalc, BombData::BombReimuBDraw},
    /* MarisaA */ {BombData::BombMarisaACalc, BombData::BombMarisaADraw},
    /* MarisaB */ {BombData::BombMarisaBCalc, BombData::BombMarisaBDraw},
};

void BombData::BombReimuACalc(Player *player) {
    i32 i;
    f32 vecLength;
    i32 bombIdx;
    ZunVec3 bombPivot;
    AnmVm *bombSprite;
    ZunVec2 angle;

    if (player->bombInfo.timer >= player->bombInfo.duration) {
        g_Gui.EndPlayerSpellcard();
        player->bombInfo.isInUse = 0;
        return;
    }
    if (player->bombInfo.timer.HasTicked() && player->bombInfo.timer == 0) {
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_BOMB_PORTRAIT,
                                   TH_REIMU_A_BOMB_NAME);
        player->bombInfo.duration = 300;
        player->invulnerabilityTimer.SetCurrent(360);

        for (i = 0; i < 8; i = i + 1) {
            player->bombInfo.reimuABombProjectilesState[i] = 0;
        }
        g_ItemManager.RemoveAllItems();
        g_EffectManager.SpawnParticles(
            PARTICLE_EFFECT_UNK_12, &player->positionCenter, 1, COLOR_NEONBLUE);

        player->bombProjectiles[8].posX = (player->positionCenter).x;
        player->bombProjectiles[8].posY = (player->positionCenter).y;

        player->bombProjectiles[8].sizeX = 256.0f;
        player->bombProjectiles[8].sizeY = 256.0f;
    }
    if (player->bombInfo.timer >= 60 && player->bombInfo.timer < 180) {

        if (player->bombInfo.timer.AsFrames() % 16 == 0 &&
            (i = (player->bombInfo.timer.AsFrames() - 60) / 16)) {
            player->bombInfo.reimuABombProjectilesState[i] = 1;
            player->bombInfo.reimuABombProjectilesRelated[i] = 4.0f;
            player->bombInfo.bombRegionPositions[i] = player->positionCenter;

            angle.x = g_Rng.GetRandomF32ZeroToOne() * ZUN_2PI - ZUN_PI;

            player->bombInfo.bombRegionVelocities[i].x =
                ZUN_COSF(angle.x) *
                player->bombInfo.reimuABombProjectilesRelated[i];

            player->bombInfo.bombRegionVelocities[i].y =
                ZUN_SINF(angle.x) *
                player->bombInfo.reimuABombProjectilesRelated[i];
            player->unk_838[i] = 0;

            for (bombSprite = &player->bombInfo.sprites[0][i * 4], bombIdx = 0;
                 bombIdx < 4; bombIdx++, bombSprite++) {
                g_AnmManager->ExecuteAnmIdx(
                    bombSprite, ANM_SCRIPT_PLAYER_REIMU_A_BOMB_ARRAY + bombIdx);
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_REIMU_A);
        }
    }
    player->playerState = PLAYER_STATE_INVULNERABLE;
    for (i = 0;
         i < ARRAY_SIZE_SIGNED(player->bombInfo.reimuABombProjectilesState);
         i++) {
        if (player->bombInfo.reimuABombProjectilesState[i] == 0) {
            continue;
        }
        if (player->bombInfo.reimuABombProjectilesState[i] == 1) {
            if (player->bombInfo.timer.HasTicked()) {
                if (player->positionOfLastEnemyHit.x > -100.0f) {
                    bombPivot = player->positionOfLastEnemyHit;
                } else {
                    bombPivot = player->positionCenter;
                }
                angle.x =
                    bombPivot.x - player->bombInfo.bombRegionPositions[i].x;
                angle.y =
                    bombPivot.y - player->bombInfo.bombRegionPositions[i].y;

                vecLength =
                    ZUN_SQRTF(angle.x * angle.x + angle.y * angle.y) /
                    (player->bombInfo.reimuABombProjectilesRelated[i] / 8.0f);
                if (vecLength < 1.0f) {
                    vecLength = 1.0f;
                }
                angle.x = angle.x / vecLength +
                          player->bombInfo.bombRegionVelocities[i].x;
                angle.y = angle.y / vecLength +
                          player->bombInfo.bombRegionVelocities[i].y;
                vecLength = ZUN_SQRTF(angle.x * angle.x + angle.y * angle.y);

                player->bombInfo.reimuABombProjectilesRelated[i] =
                    ZUN_MIN(vecLength, 10.0f);

                if (player->bombInfo.reimuABombProjectilesRelated[i] < 1.0f) {
                    player->bombInfo.reimuABombProjectilesRelated[i] = 1.0f;
                }

                player->bombInfo.bombRegionVelocities[i].x =
                    (angle.x *
                     player->bombInfo.reimuABombProjectilesRelated[i]) /
                    vecLength;
                player->bombInfo.bombRegionVelocities[i].y =
                    (angle.y *
                     player->bombInfo.reimuABombProjectilesRelated[i]) /
                    vecLength;

                player->bombRegionSizes[i].x = 48.0f;
                player->bombRegionSizes[i].y = 48.0f;

                player->bombRegionPositions[i] =
                    player->bombInfo.bombRegionPositions[i];
                player->bombRegionDamages[i] = 8;

                player->bombProjectiles[i].posX =
                    player->bombInfo.bombRegionPositions[i].x;
                player->bombProjectiles[i].posY =
                    player->bombInfo.bombRegionPositions[i].y;

                player->bombProjectiles[i].sizeX = 48.0f;
                player->bombProjectiles[i].sizeY = 48.0f;

                if (player->unk_838[i] >= 100 ||
                    player->bombInfo.timer >= player->bombInfo.duration - 30) {
                    g_EffectManager.SpawnParticles(
                        PARTICLE_EFFECT_UNK_6,
                        &player->bombInfo.bombRegionPositions[i], 8,
                        COLOR_WHITE);
                    g_EffectManager.SpawnParticles(
                        PARTICLE_EFFECT_UNK_12,
                        &player->bombInfo.bombRegionPositions[i], 1,
                        COLOR_NEONBLUE);
                    player->bombInfo.reimuABombProjectilesState[i] = 2;

                    player->bombInfo.sprites[0][i * 4].pendingInterrupt = 1;
                    player->bombInfo.sprites[0][i * 4 + 1].pendingInterrupt = 1;
                    player->bombInfo.sprites[0][i * 4 + 2].pendingInterrupt = 1;
                    player->bombInfo.sprites[0][i * 4 + 3].pendingInterrupt = 1;

                    player->bombRegionSizes[i].x = 256.0f;
                    player->bombRegionSizes[i].y = 256.0f;

                    player->bombRegionDamages[i] = 200;

                    player->bombProjectiles[i].sizeX = 256.0f;
                    player->bombProjectiles[i].sizeY = 256.0f;

                    player->bombInfo.bombRegionVelocities[i] /
                        100.0f; // ZUN moment

                    g_SoundPlayer.PlaySoundByIdx(SOUND_F);
                    ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 16, 8, 0,
                                                0);
                }
            }
        } else if (player->bombInfo.reimuABombProjectilesState[i] != 0 &&
                   player->bombInfo.timer.HasTicked()) {
            player->bombInfo.reimuABombProjectilesState[i]++;
            if (player->bombInfo.reimuABombProjectilesState[i] >= 30) {
                player->bombInfo.reimuABombProjectilesState[i] = 0;
            }
        }
        player->bombInfo.bombRegionPositions[i].x +=
            g_Supervisor.effectiveFramerateMultiplier *
            player->bombInfo.bombRegionVelocities[i].x;
        player->bombInfo.bombRegionPositions[i].y +=
            g_Supervisor.effectiveFramerateMultiplier *
            player->bombInfo.bombRegionVelocities[i].y;

        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i * 4]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i * 4 + 1]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i * 4 + 2]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i * 4 + 3]);
    }
    player->bombInfo.timer.Tick();
}

void BombData::BombReimuADraw(Player *player) {
    i32 idx;
    AnmVm *bombSprite;

    BombData::DarkenViewport(player);
    bombSprite = &player->bombInfo.sprites[0][0];
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(player->bombInfo.sprites); idx++) {
        if (player->bombInfo.reimuABombProjectilesState[idx] == 0) {
            bombSprite = &bombSprite[4];
            continue;
        }

        bombSprite->pos =
            player->bombInfo.bombRegionPositions[idx] + bombSprite->posOffset;
        player->SetToTopLeftPos(bombSprite);
        g_AnmManager->DrawNoRotation(bombSprite);
        bombSprite++;

        bombSprite->pos =
            player->bombInfo.bombRegionPositions[idx] + bombSprite->posOffset;
        player->SetToTopLeftPos(bombSprite);
        g_AnmManager->DrawNoRotation(bombSprite);
        bombSprite++;

        bombSprite->pos =
            player->bombInfo.bombRegionPositions[idx] + bombSprite->posOffset;
        player->SetToTopLeftPos(bombSprite);
        g_AnmManager->DrawNoRotation(bombSprite);
        bombSprite++;

        bombSprite->pos =
            player->bombInfo.bombRegionPositions[idx] + bombSprite->posOffset;
        player->SetToTopLeftPos(bombSprite);
        g_AnmManager->DrawNoRotation(bombSprite);
        bombSprite++;
    }
    return;
}

void BombData::DarkenViewport(Player *player) {
    ZunRect viewport;
    f32 darkeningTimeLeft;
    i32 darknessLevel; // Controls alpha level of black rectangle drawn over
                       // view

    viewport.left = 32.0f;
    viewport.top = 16.0f;
    viewport.right = 416.0f;
    viewport.bottom = 464.0f;

    if (player->bombInfo.timer < 60) {
        darkeningTimeLeft =
            (player->bombInfo.timer.AsFramesFloat() * 176.0f) / 60.0f;
        darknessLevel =
            darkeningTimeLeft >= 176.0f ? 176 : (i32)darkeningTimeLeft;
    } else if (player->bombInfo.timer >= player->bombInfo.duration + -60) {
        darkeningTimeLeft = ((player->bombInfo.duration -
                              player->bombInfo.timer.AsFramesFloat()) *
                             176.0f) /
                            60.0f;
        darknessLevel = darkeningTimeLeft < 0.0f ? 0 : (i32)darkeningTimeLeft;
    } else {
        darknessLevel = 176;
    }

    ScreenEffect::DrawSquare(&viewport, darknessLevel << 24);
}

void BombData::BombReimuBCalc(Player *player) {
    AnmVm *bombSprite;
    i32 i;
    // ZunVec3 unusedVector;

    if (player->bombInfo.timer >= player->bombInfo.duration) {
        g_Gui.EndPlayerSpellcard();
        player->bombInfo.isInUse = 0;
        return;
    }

    if (player->bombInfo.timer.HasTicked() && player->bombInfo.timer == 0) {
        g_ItemManager.RemoveAllItems();
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_ENEMY_SPELLCARD_PORTRAIT,
                                   TH_REIMU_B_BOMB_NAME);
        player->bombInfo.duration = 140;
        player->invulnerabilityTimer.SetCurrent(200);
        bombSprite = player->bombInfo.sprites[0];

        for (i = 0; i < 4; i++, bombSprite++) {
            g_AnmManager->ExecuteAnmIdx(
                bombSprite, ANM_SCRIPT_PLAYER_REIMU_B_BOMB_ARRAY + i);
        }

        g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_REIMARI);
        player->bombInfo.bombRegionPositions[0].x = player->positionCenter.x;
        player->bombInfo.bombRegionPositions[0].y = 224.0f;
        player->bombInfo.bombRegionPositions[0].z = 0.42f;
        player->bombInfo.bombRegionPositions[1].x = 192.0f;
        player->bombInfo.bombRegionPositions[1].y = player->positionCenter.y;
        player->bombInfo.bombRegionPositions[1].z = 0.415f;
        player->bombInfo.bombRegionPositions[2].x = player->positionCenter.x;
        player->bombInfo.bombRegionPositions[2].y = 224.0f;
        player->bombInfo.bombRegionPositions[2].z = 0.41f;
        player->bombInfo.bombRegionPositions[3].x = 192.0f;
        player->bombInfo.bombRegionPositions[3].y = player->positionCenter.y;
        player->bombInfo.bombRegionPositions[3].z = 0.405f;
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 60, 2, 6, 0);
    } else {
        if (player->bombInfo.timer == 60) {
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 80, 20, 0, 0);
        }

        player->bombProjectiles[0].sizeX = 62.0f;
        player->bombProjectiles[0].sizeY = 448.0f;
        player->bombProjectiles[1].sizeX = 384.0f;
        player->bombProjectiles[1].sizeY = 62.0f;
        player->bombProjectiles[2].sizeX = 62.0f;
        player->bombProjectiles[2].sizeY = 448.0f;
        player->bombProjectiles[3].sizeX = 384.0f;
        player->bombProjectiles[3].sizeY = 62.0f;

        for (i = 0; i < 4; i++) {
            g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i]);
            if (player->bombInfo.timer.HasTicked() &&
                player->bombInfo.timer.AsFrames() % 2 != 0) {
                player->bombProjectiles[i].posX =
                    player->bombInfo.bombRegionPositions[i].x +
                    player->bombInfo.sprites[0][i].posOffset.x;
                player->bombProjectiles[i].posY =
                    player->bombInfo.bombRegionPositions[i].y +
                    player->bombInfo.sprites[0][i].posOffset.y;
                player->bombRegionSizes[i].x = player->bombProjectiles[i].sizeX;
                player->bombRegionSizes[i].y = player->bombProjectiles[i].sizeY;
                player->bombRegionPositions[i] =
                    player->bombInfo.bombRegionPositions[i] +
                    player->bombInfo.sprites[0][i].posOffset;
                player->bombRegionDamages[i] = 8;
            }
        }
    }

    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bombInfo.timer.Tick();
}

void BombData::BombReimuBDraw(Player *player) {
    AnmVm *bombSprite;
    i32 i;

    BombData::DarkenViewport(player);
    bombSprite = player->bombInfo.sprites[0];
    for (i = 0; i < 4; i++, bombSprite++) {
        bombSprite->pos =
            player->bombInfo.bombRegionPositions[i] + bombSprite->posOffset;
        bombSprite->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        bombSprite->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        bombSprite->pos.z = 0.0f;
        g_AnmManager->Draw(bombSprite);
    }
}

void BombData::BombMarisaACalc(Player *player) {

    f32 starAngle;
    // i32 unused[3];
    AnmVm *starSprite;
    i32 i;

    if (player->bombInfo.timer >= player->bombInfo.duration) {
        g_Gui.EndPlayerSpellcard();
        player->bombInfo.isInUse = 0;
        return;
    }

    if (player->bombInfo.timer.HasTicked() && player->bombInfo.timer == 0) {
        g_ItemManager.RemoveAllItems();
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_ENEMY_SPELLCARD_PORTRAIT,
                                   TH_MARISA_A_BOMB_NAME);
        player->bombInfo.duration = 250;
        player->invulnerabilityTimer.SetCurrent(300);

        starSprite = player->bombInfo.sprites[0];
        for (i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.sprites);
             i++, starSprite++) {
            g_AnmManager->ExecuteAnmIdx(
                starSprite, ANM_SCRIPT_PLAYER_MARISA_A_BLUE_STAR + i % 3);
            player->bombInfo.bombRegionPositions[i] = player->positionCenter;

            starAngle = i * ZUN_2PI / 8.0f;

            player->bombInfo.bombRegionVelocities[i].x =
                ZUN_COSF(starAngle) * 2;

            player->bombInfo.bombRegionVelocities[i].y =
                ZUN_SINF(starAngle) * 2;
            player->bombInfo.bombRegionVelocities[i].z = 0.0f;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_REIMARI);
        ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 120, 4, 1, 0);
    } else {
        for (i = 0; i < ARRAY_SIZE_SIGNED(player->bombInfo.sprites); i++) {
            player->bombInfo.bombRegionPositions[i] +=
                player->bombInfo.bombRegionVelocities[i] *
                g_Supervisor.effectiveFramerateMultiplier;

            if (player->bombInfo.timer.HasTicked() &&
                player->bombInfo.timer.AsFrames() % 3 != 0) {
                player->bombProjectiles[i].posX =
                    player->bombInfo.bombRegionPositions[i].x;
                player->bombProjectiles[i].posY =
                    player->bombInfo.bombRegionPositions[i].y;
                player->bombProjectiles[i].sizeX = 128.0f;
                player->bombProjectiles[i].sizeY = 128.0f;
                player->bombRegionSizes[i].x = 128.0f;
                player->bombRegionSizes[i].y = 128.0f;

                player->bombRegionPositions[i] =
                    player->bombInfo.bombRegionPositions[i];
                player->bombRegionDamages[i] = 8;
            }
            g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][i]);
        }
    }
    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bombInfo.timer.Tick();

    return;
}

void BombData::BombMarisaADraw(Player *player) {

    AnmVm *bombSprite;
    i32 idx;

    BombData::DarkenViewport(player);
    bombSprite = &player->bombInfo.sprites[0][0];
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(player->bombInfo.sprites); idx++) {

        bombSprite->pos = player->bombInfo.bombRegionPositions[idx];
        bombSprite->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        bombSprite->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        bombSprite->pos.z = 0.0f;
        bombSprite->scaleX = 3.2f;
        bombSprite->scaleY = 3.2f;
        g_AnmManager->Draw(bombSprite);

        bombSprite->pos -= player->bombInfo.bombRegionVelocities[idx] * 6.0f;
        bombSprite->pos.x += -32.0f;
        bombSprite->pos.y += -32.0f;
        bombSprite->pos.z = 0.0f;
        bombSprite->scaleX = 2.2f;
        bombSprite->scaleY = 2.2f;
        g_AnmManager->Draw(bombSprite);

        bombSprite->pos -= player->bombInfo.bombRegionVelocities[idx] * 2.0f;
        bombSprite->pos.x += 64.0f;
        bombSprite->pos.y += 64.0f;
        bombSprite->pos.z = 0.0f;

        bombSprite->pos -= player->bombInfo.bombRegionVelocities[idx] * 2.0f;
        bombSprite->pos.x += -32.0f;
        bombSprite->pos.y += -32.0f;
        bombSprite->pos.z = 0.0f;
        bombSprite->scaleX = 1.0f;
        bombSprite->scaleY = 1.0f;
        g_AnmManager->Draw(bombSprite);
        bombSprite++;
    }
}

void BombData::BombMarisaBCalc(Player *player) {
    AnmVm *bombSprite;
    i32 i;

    if (player->bombInfo.timer >= player->bombInfo.duration) {
        g_Gui.EndPlayerSpellcard();
        player->bombInfo.isInUse = 0;
        player->verticalMovementSpeedMultiplierDuringBomb = 1.0f;
        player->horizontalMovementSpeedMultiplierDuringBomb = 1.0f;
        return;
    }

    if (player->bombInfo.timer.HasTicked() && player->bombInfo.timer == 0) {
        g_ItemManager.RemoveAllItems();
        g_Gui.ShowBombNamePortrait(ANM_SCRIPT_FACE_BOMB_PORTRAIT,
                                   TH_MARISA_B_BOMB_NAME);
        player->bombInfo.duration = 300;
        player->invulnerabilityTimer.SetCurrent(360);
        bombSprite = player->bombInfo.sprites[0];
        for (i = 0; i < 4; i++, bombSprite++) {
            g_AnmManager->ExecuteAnmIdx(
                bombSprite, ANM_SCRIPT_PLAYER_MARISA_B_MASTER_SPARK + i);
            player->bombInfo.bombRegionPositions[i] = player->positionCenter;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB_MARISA_B);
        player->verticalMovementSpeedMultiplierDuringBomb = 0.3f;
        player->horizontalMovementSpeedMultiplierDuringBomb = 0.3f;
    } else {
        if (player->bombInfo.timer == 60) {
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 60, 1, 7, 0);
        } else if (player->bombInfo.timer == 120) {
            ScreenEffect::RegisterChain(SCREEN_EFFECT_SHAKE, 200, 24, 0, 0);
        }

        if (player->bombInfo.timer.HasTicked() &&
            player->bombInfo.timer.AsFrames() % 4 != 0) {
            player->bombProjectiles[0].posX = 192.0f;
            player->bombProjectiles[0].posY = player->positionCenter.y / 2.0f;
            player->bombProjectiles[0].sizeX = 384.0f;
            player->bombProjectiles[0].sizeY = player->positionCenter.y;
            player->bombRegionSizes[0].x = 384.0f;
            player->bombRegionSizes[0].y = player->positionCenter.y;
            player->bombRegionPositions[0].x = player->bombProjectiles[0].posX;
            player->bombRegionPositions[0].y = player->bombProjectiles[0].posY;
            player->bombRegionDamages[0] = 12;
        }

        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][0]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][1]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][2]);
        g_AnmManager->ExecuteScript(&player->bombInfo.sprites[0][3]);
    }

    player->playerState = PLAYER_STATE_INVULNERABLE;
    player->bombInfo.timer.Tick();
}

void BombData::BombMarisaBDraw(Player *player) {
    AnmVm *bombSprite;
    i32 i;
    f32 spriteAngle;

    BombData::DarkenViewport(player);
    bombSprite = player->bombInfo.sprites[0];
    for (i = 0; i < 4; i++) {
        spriteAngle = (((ZUN_PI / 5) * i) / 3.0f - ZUN_PI) + ((2 * ZUN_PI) / 5);
        bombSprite->pos = player->positionCenter;
        bombSprite->pos.x +=
            (ZUN_COSF(spriteAngle) * bombSprite->sprite->heightPx *
             bombSprite->scaleY) /
            2.0f;
        bombSprite->pos.y +=
            (ZUN_SINF(spriteAngle) * bombSprite->sprite->heightPx *
             bombSprite->scaleY) /
            2.0f;
        spriteAngle = (ZUN_PI / 2) - spriteAngle;
        bombSprite->rotation.z = utils::AddNormalizeAngle(spriteAngle, ZUN_PI);
        bombSprite->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        bombSprite->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        bombSprite->pos.z = 0.0f;
        g_AnmManager->Draw(bombSprite);
        bombSprite++;
    }
}
}; // namespace th06
