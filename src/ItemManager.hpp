#pragma once

#include "AnmVm.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"

#include <d3dx8math.h>

namespace th06
{
enum ItemType // This enum is 1 byte in size on Enemy
{
    ITEM_NO_ITEM = -2,
    ITEM_RANDOM_ITEM = -1,
    ITEM_POWER_SMALL,
    ITEM_POINT,
    ITEM_POWER_BIG,
    ITEM_BOMB,
    ITEM_FULL_POWER,
    ITEM_LIFE,
    ITEM_POINT_BULLET,
};

enum ItemState
{
    ITEM_STATE_FALLING,
    ITEM_STATE_MAGNETED,
    ITEM_STATE_SPAWNED_BY_PLAYER_DEATH,
};

struct Item
{
    AnmVm sprite;
    D3DXVECTOR3 currentPosition;
    // For some reason ZUN reused the same field for both startPosition and velocity
    D3DXVECTOR3 startPositionVelocity;
    D3DXVECTOR3 targetPosition;
    ZunTimer timer;
    i8 itemType;
    i8 isInUse;
    i8 unk_142;
    i8 state;
};
ZUN_ASSERT_SIZE(Item, 0x144);

struct ItemManager
{
    void SpawnItem(D3DXVECTOR3 *position, ItemType type, ItemState state);
    void OnUpdate();
    void OnDraw();
    void MagnetAllItems();

    Item items[513];
    i32 nextIndex;
    u32 itemCount;
};
ZUN_ASSERT_SIZE(ItemManager, 0x2894c);

DIFFABLE_EXTERN(ItemManager, g_ItemManager);
}; // namespace th06
