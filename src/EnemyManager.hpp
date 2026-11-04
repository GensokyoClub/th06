#pragma once
#include "Chain.hpp"
#include "EclManager.hpp"
#include "Enemy.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"
#include <Windows.h>

namespace th06
{
struct RunningSpellcardInfo
{
    ZunBool isCapturing;
    ZunBool isActive;
    i32 captureScore;
    u32 idx;
    ZunBool usedBomb;
};
ZUN_ASSERT_TYPE(RunningSpellcardInfo, 0x14, 4);

#define MAX_ENEMY_COUNT 256

struct EnemyManager
{
    void Initialize();
    EnemyManager();

    void RunEclTimeline();
    Enemy *SpawnEnemy(i32 eclSubId, D3DXVECTOR3 *pos, i16 life, i16 itemDrop, i32 score);

    const char *stgEnmAnmFilename;
    const char *stgEnm2AnmFilename;
    Enemy enemyTemplate;
    Enemy enemies[MAX_ENEMY_COUNT + 1]; // +1 dummy slot to avoid null checks for failed spawns
    Enemy *bosses[8];
    u16 randomItemSpawnIndex;
    u16 randomItemTableIndex;
    i32 enemyCount;
    unreferenced_fields(0x4);
    RunningSpellcardInfo spellcardInfo;
    unreferenced_fields(0x4);
    TimelineInstr *timelineInstr;
    ZunTimer timelineTime;
};
#if BUILT_SINCE(100)
ZUN_ASSERT_TYPE(EnemyManager, 0xee5ec, 4);
#else
ZUN_ASSERT_TYPE(EnemyManager, 0xecdbc, 4);
#endif

ZunResult EnemyManager_RegisterChain(const char *stgEnm1, const char *stgEnm2);
void EnemyManager_CutChain();

extern EnemyManager g_EnemyManager;
} // namespace th06
