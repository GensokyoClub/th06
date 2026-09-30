#include "ItemManager.hpp"

#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"

#include <d3dx8math.h>

namespace th06
{
FILE_BSS_SORT(K1); // This is necessary to position the guard variable for g_ItemSize

DIFFABLE_STATIC_SORTED(K2, i32, g_ItemManagerPad);
DIFFABLE_STATIC_SORTED(K4, ItemManager, g_ItemManager);
DIFFABLE_STATIC_SORTED(K5, ChainElem, g_ItemManagerCalcChain); // unused
DIFFABLE_STATIC_SORTED(K3, ChainElem, g_ItemManagerDrawChain); // unused

void ItemManager::SpawnItem(D3DXVECTOR3 *position, ItemType itemType, ItemState state)
{
    Item *item;
    i32 idx;

    item = &this->items[this->nextIndex];
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->items) - 1; idx++)
    {
        this->nextIndex++;
        if (item->isInUse)
        {
            if (this->nextIndex >= ARRAY_SIZE_SIGNED(this->items) - 1)
            {
                this->nextIndex = 0;
                item = &this->items[0];
            }
            else
            {
                item++;
            }
            continue;
        }
        if (this->nextIndex >= ARRAY_SIZE_SIGNED(this->items) - 1)
        {
            this->nextIndex = 0;
        }
        item->isInUse = true;
        item->currentPosition = *position;
        item->startPositionVelocity.x = 0.0f;
        item->startPositionVelocity.y = -2.2f;
        item->startPositionVelocity.z = 0.0f;
        item->itemType = itemType;
        item->state = state;
        item->timer = 0;
        if (state == ITEM_STATE_SPAWNED_BY_PLAYER_DEATH)
        {
            // From 48.0f to 336.0f
            item->targetPosition.x = g_Rng.GetRandomF32ZeroToOne() * 288.0f + 48.0f;
            // From -64.0 to 128.0f
            item->targetPosition.y = g_Rng.GetRandomF32ZeroToOne() * 192.0f - 64.0f;
            item->targetPosition.z = 0.0f;
            // start position
            item->startPositionVelocity = item->currentPosition;
        }
        g_AnmManager->SetAndExecuteScriptIdx(&item->sprite, ANM_SCRIPT_BULLET3_ITEMS_START + itemType);
        item->sprite.color = COLOR_WHITE;
        item->unk_142 = 1;
        return;
    }
}

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 31, g_PowerItemScore) = {
    10,  20,  30,   40,   50,   60,   70,   80,   90,   100,  200,  300,   400,   500,   600,  700,
    800, 900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 11000, 12000, 51200};
DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 11, g_PowerUpThresholds) = {8, 16, 32, 48, 64, 80, 96, 128, 999, 1, 0};

i32 __inline calculatePointScore(Item *curItem, i32 scoreAcquiredItemTop, i32 scoreAcquiredItemBottom,
                                 i32 posMultiplier)
{
    return ((i32)curItem->currentPosition.y < 128)
               ? scoreAcquiredItemTop
               : (scoreAcquiredItemBottom - (((i32)curItem->currentPosition.y - 128) * posMultiplier));
}

#pragma var_order(idx, itemScore, playerAngle, itemAcquired, curItem, fVar5, powerLevelA, prevPowerLevelA,             \
                  powerLevelB, prevPowerLevelB)
void ItemManager::OnUpdate()
{
    i32 prevPowerLevelB;
    i32 prevPowerLevelA;
    i32 itemScore;
    i32 powerLevelB;
    i32 powerLevelA;
    i32 idx;
    Item *curItem;
    f32 fVar5;
    f32 playerAngle;
    ZunBool itemAcquired;

    curItem = &this->items[0];

    static D3DXVECTOR3 g_ItemSize(16.0f, 16.0f, 16.0f);

    itemAcquired = false;
    this->itemCount = 0;
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->items) - 1; idx++, curItem++)
    {
        if (!curItem->isInUse)
        {
            continue;
        }
        this->itemCount++;
        if (curItem->state == ITEM_STATE_SPAWNED_BY_PLAYER_DEATH)
        {
            if (curItem->timer < 60)
            {
                fVar5 = curItem->timer.AsFramesFloat() / 60.0f;
                // start position
                curItem->currentPosition =
                    fVar5 * curItem->targetPosition + curItem->startPositionVelocity * (1.0f - fVar5);
                goto yolo;
            }
            else if (curItem->timer == 60)
            {
                curItem->startPositionVelocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            if (curItem->state == ITEM_STATE_MAGNETED ||
                (g_GameManager.currentPower >= MAX_POWER && g_Player.positionCenter.y < 128.0f))
            {
                playerAngle = g_Player.AngleToPlayer(&curItem->currentPosition);
                sincosmul(&curItem->startPositionVelocity, playerAngle, 8.0f);
                curItem->state = ITEM_STATE_MAGNETED;
            }
            else
            {
                curItem->startPositionVelocity.x = 0.0f;
                curItem->startPositionVelocity.z = 0.0f;
                if (curItem->startPositionVelocity.y < -2.2f)
                {
                    curItem->startPositionVelocity.y = -2.2f;
                }
            }
        }
        curItem->currentPosition += curItem->startPositionVelocity * g_Supervisor.effectiveFramerateMultiplier;
        if (g_GameManager.arcadeRegionSize.y + GAME_REGION_TOP <= curItem->currentPosition.y)
        {
            curItem->isInUse = false;
            g_GameManager.DecreaseSubrank(3);
            continue;
        }
        if (curItem->startPositionVelocity.y < 3.0f)
        {
            curItem->startPositionVelocity.y += 0.03f * g_Supervisor.effectiveFramerateMultiplier;
        }
        else
        {
            curItem->startPositionVelocity.y = 3.0f;
        }
    yolo:
        if (g_Player.CalcItemBoxCollision(&curItem->currentPosition, &g_ItemSize))
        {
            switch (curItem->itemType)
            {
            case ITEM_POWER_SMALL:
                if (g_GameManager.currentPower >= MAX_POWER)
                {
                    g_GameManager.powerItemCountForScore++;
                    if (g_GameManager.powerItemCountForScore >= 31u)
                    {
                        g_GameManager.powerItemCountForScore = 30;
                    }
                    itemScore = g_PowerItemScore[g_GameManager.powerItemCountForScore];
                    g_GameManager.AddScore(itemScore);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 12800 ? COLOR_YELLOW : COLOR_WHITE);
                }
                else
                {
                    powerLevelA = 0;
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[powerLevelA])
                    {
                        powerLevelA++;
                    }
                    prevPowerLevelA = powerLevelA;
                    g_GameManager.powerItemCountForScore = 0;
                    g_GameManager.currentPower++;
                    if (g_GameManager.currentPower >= MAX_POWER)
                    {
                        g_GameManager.currentPower = MAX_POWER;
                        g_BulletManager.TurnAllBulletsIntoPoints();
                        g_Gui.ShowFullPowerMode(0);
                    }
                    g_GameManager.AddScore(10);
                    g_Gui.flags.flag2 = 2;
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[powerLevelA])
                    {
                        powerLevelA++;
                    }
                    if (powerLevelA != prevPowerLevelA)
                    {
                        g_AsciiManager.CreatePopup1(&curItem->currentPosition, -1, 0xff80c0ff);
                        g_SoundPlayer.PlaySoundByIdx(SOUND_POWERUP);
                    }
                    else
                    {
                        g_AsciiManager.CreatePopup1(&curItem->currentPosition, 10, COLOR_WHITE);
                    }
                }
                g_GameManager.IncreaseSubrank(1);
                break;
            case ITEM_POINT:
                switch (g_GameManager.difficulty)
                {
                case EASY:
                case NORMAL:
                    itemScore = calculatePointScore(curItem, 100000, 60000, 100);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 100000 ? COLOR_YELLOW : COLOR_WHITE);
                    break;
                case HARD:
                    itemScore = calculatePointScore(curItem, 150000, 100000, 180);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 150000 ? COLOR_YELLOW : COLOR_WHITE);
                    break;
                case LUNATIC:
                    itemScore = calculatePointScore(curItem, 200000, 150000, 270);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 200000 ? COLOR_YELLOW : COLOR_WHITE);
                    break;
                case EXTRA:
                    itemScore = calculatePointScore(curItem, 300000, 200000, 400);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 300000 ? COLOR_YELLOW : COLOR_WHITE);
                    break;
                }
                g_GameManager.score += itemScore;
                g_GameManager.pointItemsCollectedInStage++;
                g_GameManager.pointItemsCollected++;
                g_Gui.flags.flag4 = 2;
                if (curItem->currentPosition.y < 128.0f)
                {
                    g_GameManager.IncreaseSubrank(30);
                }
                else
                {
                    g_GameManager.IncreaseSubrank(3);
                }
                break;
            case ITEM_POWER_BIG:
                if (g_GameManager.currentPower >= MAX_POWER)
                {
                    g_GameManager.powerItemCountForScore += 8;
                    if (g_GameManager.powerItemCountForScore >= 31u)
                    {
                        g_GameManager.powerItemCountForScore = 30;
                    }
                    itemScore = g_PowerItemScore[g_GameManager.powerItemCountForScore];
                    g_GameManager.score += itemScore;
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 12800 ? COLOR_YELLOW : COLOR_WHITE);
                }
                else
                {
                    powerLevelB = 0;
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[powerLevelB])
                    {
                        powerLevelB++;
                    }
                    prevPowerLevelB = powerLevelB;
                    g_GameManager.currentPower += 8;
                    if (g_GameManager.currentPower >= MAX_POWER)
                    {
                        g_GameManager.currentPower = MAX_POWER;
                        g_BulletManager.TurnAllBulletsIntoPoints();
                        g_Gui.ShowFullPowerMode(0);
                    }
                    g_Gui.flags.flag2 = 2;
                    g_GameManager.AddScore(10);
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[powerLevelB])
                    {
                        powerLevelB++;
                    }
                    if (powerLevelB != prevPowerLevelB)
                    {
                        g_AsciiManager.CreatePopup1(&curItem->currentPosition, -1, 0xff80c0ff);
                        g_SoundPlayer.PlaySoundByIdx(SOUND_POWERUP);
                    }
                    else
                    {
                        g_AsciiManager.CreatePopup1(&curItem->currentPosition, 10, COLOR_WHITE);
                    }
                }
                break;
            case ITEM_BOMB:
                if (g_GameManager.bombsRemaining < 8)
                {
                    g_GameManager.bombsRemaining++;
                    g_Gui.flags.flag1 = 2;
                }
                g_GameManager.IncreaseSubrank(5);
                break;
            case ITEM_LIFE:
                if (g_GameManager.livesRemaining < 8)
                {
                    g_GameManager.livesRemaining++;
                    g_Gui.flags.flag0 = 2;
                }
                g_GameManager.IncreaseSubrank(200);
                g_SoundPlayer.PlaySoundByIdx(SOUND_1UP);
                break;
            case ITEM_FULL_POWER:
                if (g_GameManager.currentPower < MAX_POWER)
                {
                    g_BulletManager.TurnAllBulletsIntoPoints();
                    g_Gui.ShowFullPowerMode(0);
                    g_SoundPlayer.PlaySoundByIdx(SOUND_POWERUP);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, -1, 0xff80c0ff);
                }
                g_GameManager.currentPower = MAX_POWER;
                g_GameManager.AddScore(1000);
                g_AsciiManager.CreatePopup1(&curItem->currentPosition, 1000, COLOR_WHITE);
                g_Gui.flags.flag2 = 2;
                break;
            case ITEM_POINT_BULLET:
                itemScore = (g_GameManager.grazeInStage / 3) * 10 + 500;
                if (g_Player.bombInfo.isInUse)
                {
                    itemScore = 100;
                }
                g_GameManager.score += itemScore;
                g_AsciiManager.CreatePopup2(&curItem->currentPosition, itemScore, COLOR_WHITE);
                break;
            }
            curItem->isInUse = false;
            itemAcquired = true;
            continue;
        }
        curItem->timer++;
        g_AnmManager->ExecuteScript(&curItem->sprite);
    }
    if (itemAcquired)
    {
        g_SoundPlayer.PlaySoundByIdx(SOUND_15);
    }
}

#pragma var_order(idx, cursor)
void ItemManager::MagnetAllItems()
{
    Item *cursor;
    i32 idx;

    for (cursor = &this->items[0], idx = 0; idx < ARRAY_SIZE_SIGNED(this->items) - 1; idx++, cursor++)
    {
        if (!cursor->isInUse)
        {
            continue;
        }
        cursor->state = ITEM_STATE_MAGNETED;
    }
}

#pragma var_order(itemAlpha, idx, curItem)
void ItemManager::OnDraw()
{
    Item *curItem;
    i32 idx;
    i32 itemAlpha;

    curItem = &this->items[0];
    idx = 0;
    for (; idx < ARRAY_SIZE_SIGNED(this->items) - 1; idx++, curItem++)
    {
        if (!curItem->isInUse)
        {
            continue;
        }
        curItem->sprite.pos.x = g_GameManager.arcadeRegionTopLeftPos.x + curItem->currentPosition.x;
        curItem->sprite.pos.y = g_GameManager.arcadeRegionTopLeftPos.y + curItem->currentPosition.y;
        curItem->sprite.pos.z = 0.01f;
        if (curItem->currentPosition.y < -8.0f)
        {
            curItem->sprite.pos.y = 8.0f + g_GameManager.arcadeRegionTopLeftPos.y;
            if (curItem->unk_142 != 0)
            {
                g_AnmManager->SetActiveSprite(&curItem->sprite, curItem->itemType + 519);
                curItem->unk_142 = 0;
            }
            itemAlpha = 255 - (i32)(((8.0f - curItem->currentPosition.y) * 255.0f) / 128.0f);
            if (itemAlpha < 0x40)
            {
                itemAlpha = 0x40;
            }
            curItem->sprite.color = COLOR_SET_ALPHA3(curItem->sprite.color, itemAlpha);
        }
        else
        {
            if (curItem->unk_142 == 0)
            {
                g_AnmManager->SetActiveSprite(&curItem->sprite, curItem->itemType + 512);
                curItem->unk_142 = 1;
                curItem->sprite.color = COLOR_WHITE;
            }
        }
        g_AnmManager->DrawNoRotation(&curItem->sprite);
    }
}

}; // namespace th06
