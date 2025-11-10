#include "EnemyManager.hpp"
#include "AnmManager.hpp"
#include "BulletManager.hpp"
#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "EffectManager.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "Rng.hpp"
#include "diffbuild.hpp"
#include "utils.hpp"

namespace th06
{

#define ITEM_SPAWNS 3
#define ITEM_TABLES 8

EnemyManager g_EnemyManager;
ChainElem g_EnemyManagerCalcChain;
ChainElem g_EnemyManagerDrawChain;
DIFFABLE_STATIC_ARRAY_ASSIGN(u8, 32, g_RandomItems) = {
    ITEM_POWER_SMALL, ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POWER_SMALL,
    ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POINT,       ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POWER_SMALL,
    ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POWER_SMALL,
    ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POWER_SMALL,
    ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POINT,       ITEM_POINT,
    ITEM_POWER_SMALL, ITEM_POWER_BIG
};

void EnemyManager::Initialize()
{
    i32 i;
    Enemy *enemy;

    enemy = &this->enemies[0];
    memset(this, 0, sizeof(EnemyManager));
    enemy = &this->enemyTemplate;
    memset(enemy, 0, sizeof(Enemy));
    for (i = 0; i < ARRAY_SIZE_SIGNED(this->enemyTemplate.vms); i++)
    {
        enemy->vms[i].anmFileIndex = -1;
    }
    enemy->flags.active = 1;
    enemy->bossTimer.InitializeForPopup();
    enemy->flags.unk6 = 1;
    enemy->flags.unk7 = 1;
    enemy->flags.unk8 = 0;
    enemy->hitboxDimensions = ZunVec3(12.0f, 12.0f, 12.0f);
    enemy->axisSpeed = ZunVec3(0.0f, 0.0f, 0.0f);
    enemy->angularVelocity = 0.0f;
    enemy->angle = 0.0f;
    enemy->acceleration = 0.0f;
    enemy->speed = 0.0f;
    enemy->flags.unk1 = 0;
    enemy->flags.unk3 = 0;
    enemy->flags.unk4 = 0;
    enemy->flags.isBoss = 0;
    enemy->stackDepth = 0;
    enemy->life = 1;
    enemy->score = 100;
    enemy->deathAnm1 = 0;
    enemy->deathAnm2 = 0;
    enemy->deathAnm3 = 0;
    enemy->shootInterval = 0;
    enemy->shootIntervalTimer.InitializeForPopup();
    enemy->shootOffset = ZunVec3(0.0f, 0.0f, 0.0f);
    enemy->anmExLeft = -1;
    enemy->anmExRight = -1;
    enemy->anmExDefaults = -1;
    enemy->flags.unk10 = 1;
    enemy->flags.unk11 = 0;
    enemy->deathCallbackSub = -1;
    enemy->flags.shouldClampPos = 0;
    enemy->effectIdx = 0;
    enemy->runInterrupt = -1;
    enemy->lifeCallbackThreshold = -1;
    enemy->timerCallbackThreshold = -1;
    enemy->laserStore = 0;
    enemy->unk_e41 = 0;
    enemy->flags.unk13 = 0;
    enemy->bulletRankSpeedLow = -0.5f;
    enemy->bulletRankSpeedHigh = 0.5f;
}

EnemyManager::EnemyManager()
{
    this->Initialize();
}

Enemy *EnemyManager::SpawnEnemy(i32 eclSubId, ZunVec3 *pos, i16 life, i16 itemDrop, i32 score)
{
    Enemy *newEnemy;
    i32 idx;

    newEnemy = this->enemies;
    idx = 0;
    for (; idx < ARRAY_SIZE_SIGNED(this->enemies) - 1; idx++, newEnemy++)
    {
        if (newEnemy->flags.active)
            continue;

        *newEnemy = this->enemyTemplate;

        if (0 <= life)
            newEnemy->life = life;

        newEnemy->position = *pos;
        g_EclManager.CallEclSub(&newEnemy->currentContext, eclSubId);
        g_EclManager.RunEcl(newEnemy);
        newEnemy->color = newEnemy->primaryVm.color;
        newEnemy->itemDrop = itemDrop;

        if (0 <= life)
            newEnemy->life = life;

        if (0 <= score)
            newEnemy->score = score;

        newEnemy->maxLife = newEnemy->life;
        break;
    }
    return newEnemy;
}

void Enemy::ResetEffectArray(Enemy *enemy)
{
    i32 idx;

    for (idx = 0; idx < (i32)enemy->effectIdx; idx++)
    {
        if (!enemy->effectArray[idx])
        {
            continue;
        }
        enemy->effectArray[idx]->unk_17a = 1;
        enemy->effectArray[idx] = NULL;
    }
    enemy->effectIdx = 0;
}

void EnemyManager::RunEclTimeline()
{
    ZunVec3 pos4;
    ZunVec3 pos3;
    ZunVec3 pos2;
    ZunVec3 pos1;
    EclTimelineInstrArgs *args4;
    EclTimelineInstrArgs *args3;
    EclTimelineInstrArgs *args2;
    EclTimelineInstrArgs *args1;
    i32 subrankIncreaseFrame;
    Enemy *spawnedEnemy;

    if (this->timelineInstr == NULL)
    {
        this->timelineInstr = g_EclManager.timeline;
    }
    if (g_Gui.HasCurrentMsgIdx() == 0)
    {
        // Unclear what this is? It looks like it increases the subrank at
        // regular intervals, where the interval is made shorter based on the
        // number of lives lost?
        subrankIncreaseFrame = 10 * 4 * 60;
        subrankIncreaseFrame -= g_GameManager.livesRemaining * 4 * 60;
        if (this->timelineTime.HasTicked() && this->timelineTime.AsFrames() % subrankIncreaseFrame == 0)
        {
            g_GameManager.IncreaseSubrank(100);
        }
    }
    while (0 <= this->timelineInstr->time)
    {
        if (this->timelineTime.current == this->timelineInstr->time)
        {
            switch (this->timelineInstr->opCode)
            {
            case 0:
                if (!g_Gui.BossPresent())
                {
                    args1 = &this->timelineInstr->args;
                    this->SpawnEnemy(this->timelineInstr->arg0, args1->Var1AsVec(), args1->ushortVar1,
                                     args1->ushortVar2, args1->uintVar4);
                }
                break;
            case 1:
                if (!g_Gui.BossPresent())
                {
                    this->SpawnEnemy(this->timelineInstr->arg0, this->timelineInstr->args.Var1AsVec(), -1, ITEM_NO_ITEM,
                                     -1);
                }
                break;
            case 2:
                if (!g_Gui.BossPresent())
                {
                    args2 = &this->timelineInstr->args;
                    spawnedEnemy = this->SpawnEnemy(this->timelineInstr->arg0, args2->Var1AsVec(), args2->ushortVar1,
                                                    args2->ushortVar2, args2->uintVar4);
                    spawnedEnemy->flags.unk4 = 1;
                }
                break;
            case 3:
                if (!g_Gui.BossPresent())
                {
                    spawnedEnemy = this->SpawnEnemy(this->timelineInstr->arg0, this->timelineInstr->args.Var1AsVec(),
                                                    -1, ITEM_NO_ITEM, -1);
                    spawnedEnemy->flags.unk4 = 1;
                }
                break;
            case 4:
                if (!g_Gui.BossPresent())
                {
                    args3 = &this->timelineInstr->args;
                    pos1 = *args3->Var1AsVec();
                    if (args3->Var1AsVec()->x <= -990.0f)
                    {
                        pos1.x = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.x);
                    }
                    if (args3->Var1AsVec()->y <= -990.0f)
                    {
                        pos1.y = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.y);
                    }
                    if (args3->Var1AsVec()->z <= -990.0f)
                    {
                        pos1.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    this->SpawnEnemy(this->timelineInstr->arg0, &pos1, args3->ushortVar1, args3->ushortVar2,
                                     args3->uintVar4);
                }
                break;
            case 5:
                if (!g_Gui.BossPresent())
                {
                    pos2 = *this->timelineInstr->args.Var1AsVec();
                    if (pos2.x <= -990.0f)
                    {
                        pos2.x = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.x);
                    }
                    if (pos2.y <= -990.0f)
                    {
                        pos2.y = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.y);
                    }
                    if (pos2.z <= -990.0f)
                    {
                        pos2.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    this->SpawnEnemy(this->timelineInstr->arg0, &pos2, -1, ITEM_NO_ITEM, -1);
                }
                break;
            case 6:
                if (!g_Gui.BossPresent())
                {
                    args4 = &this->timelineInstr->args;
                    pos3 = *args4->Var1AsVec();
                    if (args4->Var1AsVec()->x <= -990.0f)
                    {
                        pos3.x = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.x);
                    }
                    if (args4->Var1AsVec()->y <= -990.0f)
                    {
                        pos3.y = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.y);
                    }
                    if (args4->Var1AsVec()->z <= -990.0f)
                    {
                        pos3.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    spawnedEnemy = this->SpawnEnemy(this->timelineInstr->arg0, &pos3, args4->ushortVar1,
                                                    args4->ushortVar2, args4->uintVar4);
                    spawnedEnemy->flags.unk4 = 1;
                }
                break;
            case 7:
                if (!g_Gui.BossPresent())
                {
                    pos4 = *this->timelineInstr->args.Var1AsVec();
                    if (pos4.x <= -990.0f)
                    {
                        pos4.x = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.x);
                    }
                    if (pos4.y <= -990.0f)
                    {
                        pos4.y = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.y);
                    }
                    if (pos4.z <= -990.0f)
                    {
                        pos4.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    spawnedEnemy = this->SpawnEnemy(this->timelineInstr->arg0, &pos4, -1, ITEM_NO_ITEM, -1);
                    spawnedEnemy->flags.unk4 = 1;
                }
                break;
            case 8:
                if (g_GameManager.difficulty == EASY && g_GameManager.currentStage == 5 &&
                    this->timelineInstr->arg0 == 1)
                {
                    g_Gui.MsgRead(g_GameManager.character * 10 + 3);
                }
                else
                {
                    g_Gui.MsgRead(this->timelineInstr->arg0 + g_GameManager.character * 10);
                }
                break;
            case 9:
                if (g_Gui.MsgWait())
                {
                    this->timelineTime.Decrement(1);
                    return;
                }
                break;
            case 10:
                this->bosses[this->timelineInstr->args.uintVar1]->runInterrupt = this->timelineInstr->args.uintVar2;
                break;
            case 0xb:
                g_GameManager.currentPower = this->timelineInstr->arg0;
                break;
            case 0xc:
                if (this->bosses[this->timelineInstr->arg0] != NULL &&
                    this->bosses[this->timelineInstr->arg0]->flags.active)
                {
                    this->timelineTime.Decrement(1);
                    return;
                }
            }
        }
        else if (this->timelineTime.current < this->timelineInstr->time)
        {
            break;
        }

        this->timelineInstr = (EclTimelineInstr *)(((u8 *)this->timelineInstr) + this->timelineInstr->size);
    }
    if (!g_Gui.HasCurrentMsgIdx())
    {
        g_GameManager.counat++;
    }
    return;
}

bool Enemy::HandleLifeCallback()
{

    i32 i;
    Enemy *curEnemy;

    if (this->life < this->lifeCallbackThreshold)
    {
        this->life = this->lifeCallbackThreshold;
        g_EclManager.CallEclSub(&this->currentContext, this->lifeCallbackSub);
        this->lifeCallbackThreshold = -1;
        this->timerCallbackSub = this->deathCallbackSub;
        this->bulletRankSpeedLow = -0.5f;
        this->bulletRankSpeedHigh = 0.5f;
        this->bulletRankAmount1Low = 0;
        this->bulletRankAmount1High = 0;
        this->bulletRankAmount2Low = 0;
        this->bulletRankAmount2High = 0;
        this->stackDepth = 0;

        curEnemy = g_EnemyManager.enemies;
        for (i = 0; i < ARRAY_SIZE_SIGNED(g_EnemyManager.enemies) - 1; i++, curEnemy++)
        {
            if (!curEnemy->flags.active)
            {
                continue;
            }
            if (curEnemy->flags.isBoss)
            {
                continue;
            }
            curEnemy->life = 0;

            if (!curEnemy->flags.unk6 && curEnemy->deathCallbackSub >= 0)
            {
                g_EclManager.CallEclSub(&curEnemy->currentContext, curEnemy->deathCallbackSub);
                curEnemy->deathCallbackSub = -1;
            }
        }
        return true;
    }

    return false;
}

bool Enemy::HandleTimerCallback()
{

    Enemy *curEnemy;
    i32 i;

    if (this->flags.isBoss)
    {
        g_Gui.SetSpellcardSeconds((this->timerCallbackThreshold - this->bossTimer.AsFrames()) / 60);
    }

    if (this->HasBossTimerFinished())
    {
        if (this->lifeCallbackThreshold > 0)
        {
            this->life = this->lifeCallbackThreshold;
            this->lifeCallbackThreshold = -1;
        }
        g_EclManager.CallEclSub(&this->currentContext, this->timerCallbackSub);
        this->timerCallbackThreshold = -1;
        this->timerCallbackSub = this->deathCallbackSub;
        this->bossTimer.InitializeForPopup();
        if (!this->flags.unk16)
        {
            g_EnemyManager.spellcardInfo.isCapturing = false;
            if (g_EnemyManager.spellcardInfo.isActive)
            {
                g_EnemyManager.spellcardInfo.isActive++;
            }
            g_BulletManager.RemoveAllBullets(0);
        }

        curEnemy = g_EnemyManager.enemies;
        for (i = 0; i < ARRAY_SIZE_SIGNED(g_EnemyManager.enemies) - 1; i++, curEnemy++)
        {
            if (!curEnemy->flags.active)
            {
                continue;
            }
            if (curEnemy->flags.isBoss)
            {
                continue;
            }
            curEnemy->life = 0;

            if (!curEnemy->flags.unk6 && curEnemy->deathCallbackSub >= 0)
            {
                g_EclManager.CallEclSub(&curEnemy->currentContext, curEnemy->deathCallbackSub);
                curEnemy->deathCallbackSub = -1;
            }
        }
        this->bulletRankSpeedLow = -0.5f;
        this->bulletRankSpeedHigh = 0.5f;
        this->bulletRankAmount1Low = 0;
        this->bulletRankAmount1High = 0;
        this->bulletRankAmount2Low = 0;
        this->bulletRankAmount2High = 0;
        this->stackDepth = 0;
        return true;
    }
    return false;
}

void Enemy::Despawn()
{
    if (!this->flags.unk11)
    {
        this->flags.active = 0;
    }
    else
    {
        this->flags.unk6 = 0;
    }
    if (this->flags.isBoss)
    {
        g_Gui.bossPresent = false;
    }
    if (this->effectIdx != 0)
    {
        this->ResetEffectArray(this);
    }
}

void Enemy::ClampPos()
{
    if (this->flags.shouldClampPos)
    {
        if (this->position.x < this->lowerMoveLimit.x)
        {
            this->position.x = this->lowerMoveLimit.x;
        }
        else if (this->position.x > this->upperMoveLimit.x)
        {
            this->position.x = this->upperMoveLimit.x;
        }

        if (this->position.y < this->lowerMoveLimit.y)
        {
            this->position.y = this->lowerMoveLimit.y;
        }
        else if (this->position.y > this->upperMoveLimit.y)
        {
            this->position.y = this->upperMoveLimit.y;
        }
    }
}

bool EnemyManager::RegisterChain(const char *stgEnm1, const char *stgEnm2)
{
    EnemyManager *mgr = &g_EnemyManager;
    mgr->Initialize();
    mgr->stgEnmAnmFilename = stgEnm1;
    mgr->stgEnm2AnmFilename = stgEnm2;
    g_EnemyManagerCalcChain.callback = (ChainCallback)mgr->OnUpdate;
    g_EnemyManagerCalcChain.addedCallback = NULL;
    g_EnemyManagerCalcChain.deletedCallback = NULL;
    g_EnemyManagerCalcChain.addedCallback = (ChainAddedCallback)mgr->AddedCallback;
    g_EnemyManagerCalcChain.deletedCallback = (ChainAddedCallback)mgr->DeletedCallback;
    g_EnemyManagerCalcChain.arg = mgr;
    if (!g_Chain.AddToCalcChain(&g_EnemyManagerCalcChain, TH_CHAIN_PRIO_CALC_ENEMYMANAGER))
    {
        return false;
    }
    g_EnemyManagerDrawChain.callback = (ChainCallback)mgr->OnDraw;
    g_EnemyManagerDrawChain.addedCallback = NULL;
    g_EnemyManagerDrawChain.deletedCallback = NULL;
    g_EnemyManagerDrawChain.arg = mgr;
    if (g_Chain.AddToDrawChain(&g_EnemyManagerDrawChain, TH_CHAIN_PRIO_DRAW_ENEMYMANAGER))
    {
        return false;
    }
    return true;
}

ChainCallbackResult EnemyManager::OnUpdate(EnemyManager *mgr)
{
    Enemy *curEnemy;
    i32 enemyLifeBeforeDmg;
    i32 enemyVmIdx;
    ZunVec3 enemyHitbox;
    i32 enemyIdx;
    i32 damage;
    bool local_8;

    local_8 = false;
    mgr->RunEclTimeline();
    for (curEnemy = &mgr->enemies[0], mgr->enemyCount = 0, enemyIdx = 0; enemyIdx < ARRAY_SIZE_SIGNED(mgr->enemies) - 1;
         enemyIdx++, curEnemy++)
    {
        if (!curEnemy->flags.active)
        {
            continue;
        }
        mgr->enemyCount++;
        curEnemy->Move();

        curEnemy->ClampPos();
        if (curEnemy->flags.unk8 == 0 &&
            g_GameManager.IsInBounds(curEnemy->position.x, curEnemy->position.y, curEnemy->primaryVm.sprite->widthPx,
                                     curEnemy->primaryVm.sprite->heightPx))
        {
            curEnemy->flags.unk8 = 1;
        }
        if (curEnemy->flags.unk8 == 1 &&
            !g_GameManager.IsInBounds(curEnemy->position.x, curEnemy->position.y, curEnemy->primaryVm.sprite->widthPx,
                                      curEnemy->primaryVm.sprite->heightPx))
        {
            curEnemy->flags.active = 0;
            curEnemy->Despawn();
            continue;
        }
        if (0 <= curEnemy->lifeCallbackThreshold)
        {
            curEnemy->HandleLifeCallback();
        }
        if (0 <= curEnemy->timerCallbackThreshold)
        {
            curEnemy->HandleTimerCallback();
        }
        if (!g_EclManager.RunEcl(curEnemy))
        {
            curEnemy->flags.active = 0;
            curEnemy->Despawn();
            continue;
        }
        curEnemy->ClampPos();
        curEnemy->primaryVm.color = curEnemy->color;
        g_AnmManager->ExecuteScript(&curEnemy->primaryVm);
        curEnemy->color = curEnemy->primaryVm.color;
        for (enemyVmIdx = 0; enemyVmIdx < 8; enemyVmIdx++)
        {
            if (0 <= curEnemy->vms[enemyVmIdx].anmFileIndex && g_AnmManager->ExecuteScript(&curEnemy->vms[enemyVmIdx]))
            {
                curEnemy->vms[enemyVmIdx].anmFileIndex = -1;
            }
        }
        local_8 = 0;
        if (curEnemy->flags.unk8 != 0 && !curEnemy->flags.unk15)
        {
            enemyLifeBeforeDmg = curEnemy->life;
            if (curEnemy->flags.unk7 && curEnemy->flags.unk6)
            {
                // There's something weird going on here, stack-wise.
                enemyHitbox = curEnemy->HitboxDimensions(1.5f);
                if (g_Player.CalcKillBoxCollision(&curEnemy->position, &enemyHitbox) == 1 && curEnemy->flags.unk6 &&
                    !curEnemy->flags.isBoss)
                {
                    curEnemy->life -= 10;
                }
            }
            if (curEnemy->flags.unk6 != 0)
            {
                damage = g_Player.CalcDamageToEnemy(&curEnemy->position, &curEnemy->hitboxDimensions, &local_8);
                if (70 <= damage)
                {
                    damage = 70;
                }
                g_GameManager.score = (damage / 5) * 10 + g_GameManager.score;
                if (mgr->spellcardInfo.isActive != 0)
                {
                    if (local_8 == 0)
                    {
                        if (damage > 7)
                        {
                            damage = damage / 7;
                        }
                        else if (damage != 0)
                        {
                            damage = 1;
                        }
                    }
                    else if (mgr->spellcardInfo.usedBomb != 0)
                    {
                        if (damage > 3)
                        {
                            damage = damage / 3;
                        }
                        else if (damage != 0)
                        {
                            damage = 1;
                        }
                    }
                    else
                    {
                        damage = 0;
                    }
                }
                if (curEnemy->flags.unk10 != 0)
                {
                    curEnemy->life -= damage;
                }
                if (g_Player.positionOfLastEnemyHit.y < curEnemy->position.y)
                {
                    g_Player.positionOfLastEnemyHit = curEnemy->position;
                }
            }
            if (0 >= curEnemy->life && curEnemy->flags.unk6 != 0)
            {
                curEnemy->lifeCallbackThreshold = -1;
                curEnemy->timerCallbackThreshold = -1;
                switch (curEnemy->flags.unk11)
                {
                case 3:
                    curEnemy->life = 1;
                    curEnemy->flags.unk10 = 0;
                    curEnemy->flags.unk11 = 0;
                    g_Gui.bossPresent = 0;
                    g_EffectManager.SpawnParticles(curEnemy->deathAnm1, &curEnemy->position, 1, COLOR_WHITE);
                    g_EffectManager.SpawnParticles(curEnemy->deathAnm1, &curEnemy->position, 1, COLOR_WHITE);
                    g_EffectManager.SpawnParticles(curEnemy->deathAnm1, &curEnemy->position, 1, COLOR_WHITE);
                    break;
                case 1:
                    g_GameManager.AddScore(curEnemy->score);
                    curEnemy->flags.unk6 = 0;
                    goto LAB_00412a4d;
                case 0:
                    g_GameManager.AddScore(curEnemy->score);
                    curEnemy->flags.active = 0;
                LAB_00412a4d:
                    if (curEnemy->flags.isBoss)
                    {
                        g_Gui.bossPresent = 0;
                        Enemy::ResetEffectArray(curEnemy);
                    }
                case 2:
                    if (curEnemy->itemDrop >= 0)
                    {
                        g_EffectManager.SpawnParticles(curEnemy->deathAnm2 + 4, &curEnemy->position, 3, 0xffffffff);
                        g_ItemManager.SpawnItem(&curEnemy->position, (ItemType)curEnemy->itemDrop, local_8);
                    }
                    else if (curEnemy->itemDrop == ITEM_NO_ITEM)
                    {
                        if (mgr->randomItemSpawnIndex % 3 == 0)
                        {
                            g_EffectManager.SpawnParticles(curEnemy->deathAnm2 + 4, &curEnemy->position, 6,
                                                           COLOR_WHITE);
                            g_ItemManager.SpawnItem(&curEnemy->position,
                                                    (ItemType)g_RandomItems[mgr->randomItemTableIndex], local_8);
                            mgr->randomItemTableIndex++;
                            if (ARRAY_SIZE_SIGNED(g_RandomItems) <= mgr->randomItemTableIndex)
                            {
                                mgr->randomItemTableIndex = 0;
                            }
                        }
                        mgr->randomItemSpawnIndex++;
                    }
                    if (curEnemy->flags.isBoss && !g_EnemyManager.spellcardInfo.isActive)
                    {
                        g_BulletManager.DespawnBullets(12800, false);
                    }
                    curEnemy->life = 0;
                    break;
                }
                g_SoundPlayer.PlaySoundByIdx((SoundIdx)((enemyIdx % 2) + SOUND_2));
                g_EffectManager.SpawnParticles(curEnemy->deathAnm1, &curEnemy->position, 1, 0xffffffff);
                g_EffectManager.SpawnParticles(curEnemy->deathAnm2 + 4, &curEnemy->position, 4, 0xffffffff);
                if (0 <= curEnemy->deathCallbackSub)
                {
                    curEnemy->bulletRankSpeedLow = -0.5;
                    curEnemy->bulletRankSpeedHigh = 0.5;
                    curEnemy->bulletRankAmount1Low = 0;
                    curEnemy->bulletRankAmount1High = 0;
                    curEnemy->bulletRankAmount2Low = 0;
                    curEnemy->bulletRankAmount2High = 0;
                    curEnemy->stackDepth = 0;
                    g_EclManager.CallEclSub(&curEnemy->currentContext, curEnemy->deathCallbackSub);
                    curEnemy->deathCallbackSub = -1;
                }
            }
            if (curEnemy->flags.isBoss != 0 && !g_Gui.HasCurrentMsgIdx())
            {
                g_Gui.SetBossHealthBar(curEnemy->LifePercent());
            }
            if (curEnemy->unk_e41 != 0)
            {
                curEnemy->unk_e41--;
                curEnemy->primaryVm.flags.colorOp = AnmVmColorOp_Modulate;
            }
            else if (enemyLifeBeforeDmg > curEnemy->life)
            {
                g_SoundPlayer.PlaySoundByIdx(SOUND_TOTAL_BOSS_DEATH);
                curEnemy->primaryVm.flags.colorOp = AnmVmColorOp_Add;
                curEnemy->unk_e41 = 4;
            }
            else
            {
                curEnemy->primaryVm.flags.colorOp = AnmVmColorOp_Modulate;
            }
        }
        Enemy::UpdateEffects(curEnemy);
        if (g_GameManager.isTimeStopped == 0)
        {
            curEnemy->bossTimer.Tick();
        }
    }
    mgr->timelineTime.Tick();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

void Enemy::UpdateEffects(Enemy *enemy)
{
    Effect *effect;
    i32 i;

    for (i = 0; i < (i32)enemy->effectIdx; i++)
    {
        effect = enemy->effectArray[i];
        if (!effect)
        {
            continue;
        }

        effect->position = enemy->position;
        if (effect->unk_15c < enemy->effectDistance)
        {
            effect->unk_15c += 0.3f;
        }

        effect->angleRelated = utils::AddNormalizeAngle(effect->angleRelated, ZUN_PI / 100);
    }
}

ChainCallbackResult EnemyManager::OnDraw(EnemyManager *mgr)
{
    AnmVm *curEnemyVm;
    Enemy *curEnemy;
    i32 curEnemyVmIdx;
    i32 curEnemyIdx;

    for (curEnemy = &mgr->enemies[0], curEnemyIdx = 0; curEnemyIdx < ARRAY_SIZE_SIGNED(mgr->enemies) - 1;
         curEnemyIdx++, curEnemy++)
    {
        if (!curEnemy->flags.active)
        {
            continue;
        }
        if (curEnemy->flags.unk15)
        {
            continue;
        }

        for (curEnemyVm = &curEnemy->vms[0], curEnemyVmIdx = 0; curEnemyVmIdx < 4; curEnemyVmIdx++, curEnemyVm++)
        {
            if (0 <= curEnemyVm->anmFileIndex)
            {
                if (curEnemyVm->autoRotate != 0)
                {
                    curEnemyVm->rotation.z = curEnemy->angle;
                }
                curEnemyVm->pos = curEnemy->position + curEnemyVm->posOffset;
                curEnemyVm->pos.z = 0.495f;
                g_AnmManager->Draw2(curEnemyVm);
            }
        }
        if (curEnemy->flags.unk13 != 0)
        {
            curEnemy->primaryVm.rotation.z = curEnemy->angle;
        }
        curEnemy->primaryVm.pos = curEnemy->position + curEnemy->primaryVm.posOffset;
        curEnemy->primaryVm.pos.z = 0.494f;
        g_AnmManager->Draw2(&curEnemy->primaryVm);
        for (curEnemyVmIdx = 4; curEnemyVmIdx < 8; curEnemyVmIdx++, curEnemyVm++)
        {
            if (0 <= curEnemyVm->anmFileIndex)
            {
                if (curEnemyVm->autoRotate != 0)
                {
                    curEnemyVm->rotation.z = curEnemy->angle;
                }
                curEnemyVm->pos = curEnemy->position + curEnemyVm->posOffset;
                curEnemyVm->pos.z = 0.495f;
                g_AnmManager->Draw2(curEnemyVm);
            }
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

bool EnemyManager::AddedCallback(EnemyManager *enemyManager)
{
    Enemy *enemies = enemyManager->enemies;

    if (enemyManager->stgEnmAnmFilename &&
        !g_AnmManager->LoadAnm(ANM_FILE_ENEMY, enemyManager->stgEnmAnmFilename, ANM_OFFSET_ENEMY))
    {
        return false;
    }
    if (enemyManager->stgEnm2AnmFilename &&
        !g_AnmManager->LoadAnm(ANM_FILE_ENEMY2, enemyManager->stgEnm2AnmFilename, ANM_OFFSET_ENEMY))
    {
        return false;
    }

    enemyManager->randomItemSpawnIndex = g_Rng.GetRandomU16InRange(ITEM_SPAWNS);
    enemyManager->randomItemTableIndex = g_Rng.GetRandomU16InRange(ITEM_TABLES);

    enemyManager->spellcardInfo.isActive = 0;
    enemyManager->timelineInstr = NULL;

    return true;
}

bool EnemyManager::DeletedCallback(EnemyManager *mgr)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_ENEMY2);
    g_AnmManager->ReleaseAnm(ANM_FILE_ENEMY);
    return true;
}

void EnemyManager::CutChain()
{
    g_Chain.Cut(&g_EnemyManagerCalcChain);
    g_Chain.Cut(&g_EnemyManagerDrawChain);
    return;
}

void Enemy::Move()
{
    if (!this->flags.unk4)
    {
        this->position.x += g_Supervisor.effectiveFramerateMultiplier * this->axisSpeed.x;
    }
    else
    {
        this->position.x -= g_Supervisor.effectiveFramerateMultiplier * this->axisSpeed.x;
    }
    this->position.y += g_Supervisor.effectiveFramerateMultiplier * this->axisSpeed.y;
    this->position.z += g_Supervisor.effectiveFramerateMultiplier * this->axisSpeed.z;
}
}; // namespace th06
