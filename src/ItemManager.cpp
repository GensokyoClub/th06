#include "stdafx.h"

#include "ItemManager.hpp"

#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "Gui.hpp"
#include "SoundPlayer.hpp"

namespace th06
{
ItemManager g_ItemManager;
ChainElem g_ItemManagerCalcChain; // unused
ChainElem g_ItemMgrOnDrawElem;    // unused

void ItemManager::SpawnItem(D3DXVECTOR3 *position, ItemType itemType, ItemState state)
{
    Item *item;
    i32 idx;

    item = &this->items[this->nextIndex];
    for (idx = 0; idx < MAX_ITEMS; idx++)
    {
        this->nextIndex++;
        if (item->isInUse)
        {
            if (this->nextIndex >= MAX_ITEMS)
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
        if (this->nextIndex >= MAX_ITEMS)
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
            item->targetPosition.x = g_Rng.GetRandomF32InRange(288.0f) + 48.0f;
            // From -64.0 to 128.0f
            item->targetPosition.y = g_Rng.GetRandomF32InRange(192.0f) - 64.0f;
            item->targetPosition.z = 0.0f;
            // start position
            item->startPositionVelocity = item->currentPosition;
        }
        g_AnmManager->SetAndExecuteScriptIdx(&item->sprite, ANM_SCRIPT_BULLET3_ITEMS_START + itemType);
        item->sprite.color = COLOR_WHITE;
        item->isIndicatorHidden = true;
        return;
    }
}

i32 g_PowerItemScore[] = {
    10,  20,  30,   40,   50,   60,   70,   80,   90,   100,  200,  300,   400,   500,   600,   700,
    800, 900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 11000, 12000, 51200,
};
// Why are there a 1 and 0 at the end of this???
i32 g_PowerUpThresholds[] = {8, 16, 32, 48, 64, 80, 96, 128, 999, 1, 0};

#define PIV_LINE 128
#define ITEM_SPRITE_SIZE 16.0f

#define CALCULATE_POINT_SCORE(item, top, bottom, multiplier)                                                           \
    (((i32)(item)->currentPosition.y < PIV_LINE)                                                                       \
         ? (top)                                                                                                       \
         : ((bottom) - (((i32)(item)->currentPosition.y - PIV_LINE) * (multiplier))))

#pragma var_order(idx, itemScore, playerAngle, itemAcquired, curItem)
void ItemManager::OnUpdate()
{
    i32 itemScore;
    i32 idx;
    Item *curItem;
    f32 playerAngle;
    ZunBool itemAcquired;

    curItem = &this->items[0];

    static D3DXVECTOR3 g_ItemHitSize(16.0f, 16.0f, 16.0f);

    itemAcquired = false;
    this->itemCount = 0;
    for (idx = 0; idx < MAX_ITEMS; idx++, curItem++)
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
                float fVar5 = (f32)curItem->timer / 60.0f;
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
                (g_GameManager.currentPower >= MAX_POWER && g_Player.positionCenter.y < PIV_LINE))
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
        if (g_GameManager.gameRegionSize.y + ITEM_SPRITE_SIZE <= curItem->currentPosition.y)
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
        if (g_Player.CalcItemBoxCollision(&curItem->currentPosition, &g_ItemHitSize))
        {
            switch (curItem->itemType)
            {
            case ITEM_POWER_SMALL:
                if (g_GameManager.currentPower >= MAX_POWER)
                {
                    g_GameManager.powerItemCountForScore++;
                    if (g_GameManager.powerItemCountForScore >= ARRAY_SIZE(g_PowerItemScore))
                    {
                        g_GameManager.powerItemCountForScore = ARRAY_SIZE(g_PowerItemScore) - 1;
                    }
                    itemScore = g_PowerItemScore[g_GameManager.powerItemCountForScore];
                    g_GameManager.AddScore(itemScore);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 12800 ? COLOR_YELLOW : COLOR_WHITE);
                }
                else
#pragma var_order(powerLevel, prevPowerLevel)
                {
                    i32 powerLevel = 0;
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[powerLevel])
                    {
                        powerLevel++;
                    }
                    i32 prevPowerLevel = powerLevel;
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
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[powerLevel])
                    {
                        powerLevel++;
                    }
                    if (powerLevel != prevPowerLevel)
                    {
                        g_AsciiManager.CreatePopup1(&curItem->currentPosition, -1, COLOR_BABY_BLUE);
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
                    itemScore = CALCULATE_POINT_SCORE(curItem, 100000, 60000, 100);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 100000 ? COLOR_YELLOW : COLOR_WHITE);
                    break;
                case HARD:
                    itemScore = CALCULATE_POINT_SCORE(curItem, 150000, 100000, 180);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 150000 ? COLOR_YELLOW : COLOR_WHITE);
                    break;
                case LUNATIC:
                    itemScore = CALCULATE_POINT_SCORE(curItem, 200000, 150000, 270);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 200000 ? COLOR_YELLOW : COLOR_WHITE);
                    break;
                case EXTRA:
                    itemScore = CALCULATE_POINT_SCORE(curItem, 300000, 200000, 400);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 300000 ? COLOR_YELLOW : COLOR_WHITE);
                    break;
                }
                g_GameManager.AddScore(itemScore);
                g_GameManager.pointItemsCollectedInStage++;
                g_GameManager.pointItemsCollected++;
                g_Gui.flags.flag4 = 2;
                if (curItem->currentPosition.y < PIV_LINE)
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
                    if (g_GameManager.powerItemCountForScore >= ARRAY_SIZE(g_PowerItemScore))
                    {
                        g_GameManager.powerItemCountForScore = ARRAY_SIZE(g_PowerItemScore) - 1;
                    }
                    itemScore = g_PowerItemScore[g_GameManager.powerItemCountForScore];
                    g_GameManager.AddScore(itemScore);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore,
                                                itemScore >= 12800 ? COLOR_YELLOW : COLOR_WHITE);
                }
                else
#pragma var_order(powerLevel, prevPowerLevel)
                {
                    i32 powerLevel = 0;
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[powerLevel])
                    {
                        powerLevel++;
                    }
                    i32 prevPowerLevel = powerLevel;
                    g_GameManager.currentPower += 8;
                    if (g_GameManager.currentPower >= MAX_POWER)
                    {
                        g_GameManager.currentPower = MAX_POWER;
                        g_BulletManager.TurnAllBulletsIntoPoints();
                        g_Gui.ShowFullPowerMode(0);
                    }
                    g_Gui.flags.flag2 = 2;
                    g_GameManager.AddScore(10);
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[powerLevel])
                    {
                        powerLevel++;
                    }
                    if (powerLevel != prevPowerLevel)
                    {
                        g_AsciiManager.CreatePopup1(&curItem->currentPosition, -1, COLOR_BABY_BLUE);
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
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, -1, COLOR_BABY_BLUE);
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
                g_GameManager.AddScore(itemScore);
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

    for (cursor = &this->items[0], idx = 0; idx < MAX_ITEMS; idx++, cursor++)
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
    i32 idx;
    i32 itemAlpha;

    Item *curItem = &this->items[0];
    for (idx = 0; idx < MAX_ITEMS; idx++, curItem++)
    {
        if (!curItem->isInUse)
        {
            continue;
        }
        curItem->sprite.pos.x = g_GameManager.gameRegionScreenPos.x + curItem->currentPosition.x;
        curItem->sprite.pos.y = g_GameManager.gameRegionScreenPos.y + curItem->currentPosition.y;
        curItem->sprite.pos.z = 0.01f;
        if (curItem->currentPosition[1] < -(ITEM_SPRITE_SIZE / 2.0f))
        {
            curItem->sprite.pos.y = (ITEM_SPRITE_SIZE / 2.0f) + g_GameManager.gameRegionScreenPos.y;
            if (curItem->isIndicatorHidden)
            {
                g_AnmManager->SetActiveSprite(&curItem->sprite, curItem->itemType + ANM_SPRITE_ITEM_INDICATORS_START);
                curItem->isIndicatorHidden = false;
            }
            itemAlpha = 255 - (i32)((((ITEM_SPRITE_SIZE / 2.0f) - curItem->currentPosition[1]) * 255.0f) / 128.0f);
            if (itemAlpha < 0x40)
            {
                itemAlpha = 0x40;
            }
            curItem->sprite.color = COLOR_SET_ALPHA3(curItem->sprite.color, itemAlpha);
        }
        else
        {
            if (!curItem->isIndicatorHidden)
            {
                g_AnmManager->SetActiveSprite(&curItem->sprite, curItem->itemType + ANM_SPRITE_ITEMS_START);
                curItem->isIndicatorHidden = true;
                curItem->sprite.color = COLOR_WHITE;
            }
        }
        g_AnmManager->DrawNoRotation(&curItem->sprite);
    }
}

} // namespace th06
