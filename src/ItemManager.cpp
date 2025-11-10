#include "ItemManager.hpp"

#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "Rng.hpp"
#include "SoundPlayer.hpp"
#include "utils.hpp"

namespace th06
{
ItemManager g_ItemManager;

ItemManager::ItemManager() {

};

void ItemManager::SpawnItem(ZunVec3 *position, ItemType itemType, int state)
{
    Item *item;
    i32 idx;

    item = &this->items[this->nextIndex];
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->items); idx++)
    {
        this->nextIndex++;
        if (item->isInUse)
        {
            if (this->nextIndex >= ARRAY_SIZE_SIGNED(this->items))
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
        if (this->nextIndex >= ARRAY_SIZE_SIGNED(this->items))
        {
            this->nextIndex = 0;
        }
        item->isInUse = 1;
        item->currentPosition = *position;
        item->startPosition.x = 0.0f;
        item->startPosition.y = -2.2f;
        item->startPosition.z = 0.0f;
        item->itemType = itemType;
        item->state = state;
        item->timer.InitializeForPopup();
        if (state == 2)
        {
            // From 48.0f to 336.0f
            item->targetPosition.x = g_Rng.GetRandomF32ZeroToOne() * 288.0f + 48.0f;
            // From -64.0 to 128.0f
            item->targetPosition.y = g_Rng.GetRandomF32ZeroToOne() * 192.0f - 64.0f;
            item->targetPosition.z = 0.0;
            item->startPosition = item->currentPosition;
        }
        g_AnmManager->SetAndExecuteScriptIdx(&item->sprite, ANM_SCRIPT_BULLET3_ITEMS_START + itemType);
        item->sprite.color = COLOR_WHITE;
        item->unk_142 = 1;
        return;
    }
    return;
}

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 11, g_PowerUpThresholds) = {8, 16, 32, 48, 64, 80, 96, 128, 999, 1, 0};
DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 31, g_PowerItemScore) = {
    10,  20,  30,   40,   50,   60,   70,   80,   90,   100,  200,  300,   400,   500,   600,  700,
    800, 900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 11000, 12000, 51200};

i32 inline calculatePointScore(Item *curItem, i32 scoreAcquiredItemTop, i32 scoreAcquiredItemBottom, i32 posMultiplier)
{
    return ((i32)curItem->currentPosition.y < 128)
               ? scoreAcquiredItemTop
               : (scoreAcquiredItemBottom - (((i32)curItem->currentPosition.y - 128) * posMultiplier));
}

void ItemManager::OnUpdate()
{
    i32 iVar9;
    i32 iVar8;
    i32 itemScore;
    i32 idx3;
    i32 idx2;
    i32 idx;
    Item *curItem;
    f32 fVar5;
    f32 playerAngle;
    i32 itemAcquired;

    curItem = &this->items[0];
    static ZunVec3 g_ItemSize(16.0f, 16.0f, 16.0f);
    itemAcquired = false;
    this->itemCount = 0;
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->items); idx++, curItem++)
    {
        if (!curItem->isInUse)
        {
            continue;
        }
        this->itemCount++;
        if (curItem->state == 2)
        {
            if ((i32)(60 > curItem->timer.current))
            {
                fVar5 = curItem->timer.AsFramesFloat() / 60.0f;
                curItem->currentPosition = curItem->targetPosition * fVar5 + curItem->startPosition * (1.0f - fVar5);
                goto yolo;
            }
            else if ((i32)(curItem->timer.current == 60))
            {
                curItem->startPosition = ZunVec3(0.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            if (curItem->state == 1 || (128 <= g_GameManager.currentPower && g_Player.positionCenter.y < 128.0f))
            {
                playerAngle = g_Player.AngleToPlayer(&curItem->currentPosition);
                sincosmul(&curItem->startPosition, playerAngle, 8.0f);
                curItem->state = 1;
            }
            else
            {
                curItem->startPosition.x = 0.0;
                curItem->startPosition.z = 0.0;
                if (curItem->startPosition.y < -2.2f)
                {
                    curItem->startPosition.y = -2.2f;
                }
            }
        }
        curItem->currentPosition += curItem->startPosition * g_Supervisor.effectiveFramerateMultiplier;
        if (g_GameManager.arcadeRegionSize.y + (f32)GAME_REGION_TOP <= curItem->currentPosition.y)
        {
            curItem->isInUse = 0;
            g_GameManager.DecreaseSubrank(3);
            continue;
        }
        if (curItem->startPosition.y < 3.0f)
        {
            curItem->startPosition.y += g_Supervisor.effectiveFramerateMultiplier * 0.03f;
        }
        else
        {
            curItem->startPosition.y = 3.0f;
        }
    yolo:
        if (g_Player.CalcItemBoxCollision(&curItem->currentPosition, &g_ItemSize))
        {
            switch (curItem->itemType)
            {
            case ITEM_POWER_SMALL:
                if (g_GameManager.currentPower >= 128)
                {
                    g_GameManager.powerItemCountForScore++;
                    if ((u32)g_GameManager.powerItemCountForScore >= 31)
                    {
                        g_GameManager.powerItemCountForScore = 30;
                    }
                    itemScore = g_PowerItemScore[g_GameManager.powerItemCountForScore];
                    g_GameManager.AddScore(itemScore);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore, itemScore >= 12800 ? -256 : -1);
                }
                else
                {
                    idx2 = 0;
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[idx2])
                    {
                        idx2++;
                    }
                    iVar8 = idx2;
                    g_GameManager.powerItemCountForScore = 0;
                    g_GameManager.currentPower++;
                    if (g_GameManager.currentPower >= 128)
                    {
                        g_GameManager.currentPower = 128;
                        g_BulletManager.TurnAllBulletsIntoPoints();
                        g_Gui.ShowFullPowerMode(0);
                    }
                    g_GameManager.AddScore(10);
                    g_Gui.flags.flag2 = 2;
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[idx2])
                    {
                        idx2++;
                    }
                    if (idx2 != iVar8)
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
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore, itemScore >= 100000 ? -256 : -1);
                    break;
                case HARD:
                    itemScore = calculatePointScore(curItem, 150000, 100000, 180);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore, itemScore >= 150000 ? -256 : -1);
                    break;
                case LUNATIC:
                    itemScore = calculatePointScore(curItem, 200000, 150000, 270);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore, itemScore >= 200000 ? -256 : -1);
                    break;
                case EXTRA:
                    itemScore = calculatePointScore(curItem, 300000, 200000, 400);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore, itemScore >= 300000 ? -256 : -1);
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
                if (g_GameManager.currentPower >= 128)
                {
                    g_GameManager.powerItemCountForScore += 8;
                    if (31 <= (u32)g_GameManager.powerItemCountForScore)
                    {
                        g_GameManager.powerItemCountForScore = 30;
                    }
                    itemScore = g_PowerItemScore[g_GameManager.powerItemCountForScore];
                    g_GameManager.score += itemScore;
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, itemScore, itemScore >= 12800 ? -256 : -1);
                }
                else
                {
                    idx3 = 0;
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[idx3])
                    {
                        idx3++;
                    }
                    iVar9 = idx3;
                    g_GameManager.currentPower += 8;
                    if (128 <= g_GameManager.currentPower)
                    {
                        g_GameManager.currentPower = 128;
                        g_BulletManager.TurnAllBulletsIntoPoints();
                        g_Gui.ShowFullPowerMode(0);
                    }
                    g_Gui.flags.flag2 = 2;
                    g_GameManager.AddScore(10);
                    while (g_GameManager.currentPower >= g_PowerUpThresholds[idx3])
                    {
                        idx3++;
                    }
                    if (idx3 != iVar9)
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
                if (g_GameManager.currentPower < 128)
                {
                    g_BulletManager.TurnAllBulletsIntoPoints();
                    g_Gui.ShowFullPowerMode(0);
                    g_SoundPlayer.PlaySoundByIdx(SOUND_POWERUP);
                    g_AsciiManager.CreatePopup1(&curItem->currentPosition, -1, 0xff80c0ff);
                }
                g_GameManager.currentPower = 128;
                g_GameManager.AddScore(1000);
                g_AsciiManager.CreatePopup1(&curItem->currentPosition, 1000, COLOR_WHITE);
                g_Gui.flags.flag2 = 2;
                break;
            case ITEM_POINT_BULLET:
                itemScore = (g_GameManager.grazeInStage / 3) * 10 + 500;
                if (g_Player.bombInfo.isInUse != 0)
                {
                    itemScore = 100;
                }
                g_GameManager.score += itemScore;
                g_AsciiManager.CreatePopup2(&curItem->currentPosition, itemScore, COLOR_WHITE);
                break;
            }
            curItem->isInUse = 0;
            itemAcquired = true;
            continue;
        }
        curItem->timer.Tick();
        g_AnmManager->ExecuteScript(&curItem->sprite);
    }
    if (itemAcquired)
    {
        g_SoundPlayer.PlaySoundByIdx(SOUND_15);
    }
    return;
}

void ItemManager::RemoveAllItems()
{
    Item *cursor;
    i32 idx;

    for (cursor = &this->items[0], idx = 0; idx < ARRAY_SIZE_SIGNED(this->items); idx += 1, cursor += 1)
    {
        if (!cursor->isInUse)
        {
            continue;
        }
        cursor->state = 1;
    }
    return;
}

void ItemManager::OnDraw()
{
    Item *curItem;
    i32 idx;
    i32 itemAlpha;

    curItem = &this->items[0];
    idx = 0;
    for (; idx < ARRAY_SIZE_SIGNED(this->items); idx++, curItem++)
    {
        if (curItem->isInUse == 0)
        {
            continue;
        }
        curItem->sprite.pos.x = g_GameManager.arcadeRegionTopLeftPos.x + curItem->currentPosition.x;
        curItem->sprite.pos.y = g_GameManager.arcadeRegionTopLeftPos.y + curItem->currentPosition.y;
        curItem->sprite.pos.z = 0.01f;
        if (curItem->currentPosition.y < -8.0f)
        {
            curItem->sprite.pos.y = g_GameManager.arcadeRegionTopLeftPos.y + 8.0f;
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
    return;
}

}; // namespace th06
