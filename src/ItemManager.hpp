#pragma once

#include "AnmVm.hpp"
#include "ZunTimer.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"

namespace th06
{
enum ItemType : i16 // This enum is 1 byte in size on Enemy
{
    ITEM_POWER_SMALL,
    ITEM_POINT,
    ITEM_POWER_BIG,
    ITEM_BOMB,
    ITEM_FULL_POWER,
    ITEM_LIFE,
    ITEM_POINT_BULLET,
    // Might need to be 0xffffff? That causes zed to freak out though..
    ITEM_NO_ITEM = -1,
};

struct Item
{
    AnmVm sprite;
    ZunVec3 currentPosition;
    ZunVec3 startPosition;
    ZunVec3 targetPosition;
    ZunTimer timer;
    i8 itemType;
    i8 isInUse;
    i8 unk_142;
    i8 state;
};
ZUN_ASSERT_SIZE(Item, 0x144);

struct ItemManager
{
    ItemManager();
    void SpawnItem(ZunVec3 *position, ItemType type, i32 state);
    void OnUpdate();
    void OnDraw();
    void RemoveAllItems();

    Item items[513];
    i32 nextIndex;
    u32 itemCount;
};
ZUN_ASSERT_SIZE(ItemManager, 0x2894c);

extern ItemManager g_ItemManager;
}; // namespace th06
