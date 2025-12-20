#pragma once

#include "Chain.hpp"
#include "EclManager.hpp"
#include "Enemy.hpp"
#include "inttypes.hpp"

namespace th06 {
struct RunningSpellcardInfo {
    bool isCapturing;
    u32 isActive;
    i32 captureScore;
    u32 idx;
    bool usedBomb;
};

struct EnemyManager {
    void Initialize();
    EnemyManager();
    static bool RegisterChain(const char *stgEnm1, const char *stgEnm2);
    static void CutChain();
    static ChainCallbackResult OnUpdate(EnemyManager *enemyManager);
    static ChainCallbackResult OnDraw(EnemyManager *enemyManager);
    static bool AddedCallback(EnemyManager *enemyManager);
    static bool DeletedCallback(EnemyManager *enemyManager);

    void RunEclTimeline();
    Enemy *SpawnEnemy(i32 eclSubId, ZunVec3 *pos, i16 life, i16 itemDrop,
                      i32 score);

    const char *stgEnmAnmFilename;
    const char *stgEnm2AnmFilename;
    Enemy enemyTemplate;
    Enemy enemies[257];
    Enemy *bosses[8];
    u16 randomItemSpawnIndex;
    u16 randomItemTableIndex;
    i32 enemyCount;
    i8 unk_ee5c0[4];
    RunningSpellcardInfo spellcardInfo;
    i32 unk_ee5d8;
    EclTimelineInstr *timelineInstr;
    ZunTimer timelineTime;
};

extern EnemyManager g_EnemyManager;
}; // namespace th06
