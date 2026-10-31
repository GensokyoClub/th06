#include "stdafx.h"

#include "EnemyManager.hpp"
#include "ChainPriorities.hpp"
#include "EffectManager.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "BulletManager.hpp"

namespace th06
{
#define ITEM_SPAWNS 3
#define ITEM_TABLES 8

// clang-format off
static u8 g_RandomItems[] = {
    ITEM_POWER_SMALL, ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POWER_SMALL,
    ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POWER_SMALL, ITEM_POINT,
    ITEM_POINT,       ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POWER_SMALL,
    ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POINT,       ITEM_POWER_SMALL,
    ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POWER_SMALL,
    ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POINT,       ITEM_POWER_SMALL,
    ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POWER_SMALL, ITEM_POINT,
    ITEM_POINT,       ITEM_POINT,       ITEM_POWER_SMALL, ITEM_POWER_BIG,
};
// clang-format on

ChainElem g_EnemyManagerCalcChain;
ChainElem g_EnemyDrawChain;
EnemyManager g_EnemyManager;

#pragma var_order(i, enemy)
void EnemyManager::Initialize()
{
    i32 i;
    Enemy *enemy;

    enemy = &this->enemies[0];
    memset(this, 0, sizeof(EnemyManager));
    enemy = &this->enemyTemplate;
    memset(enemy, 0, sizeof(Enemy));
    for (i = 0; i < ENEMY_ANM_SLOTS; i++)
    {
        enemy->vms[i].anmFileIndex = -1;
    }
    enemy->flags.isSlotOccupied = true;
    enemy->phaseTimer = 0;
    enemy->flags.isInteractable = true;
    enemy->flags.isCollidable = true;
    enemy->flags.hasBeenInBounds = false;
    enemy->hitboxDimensions = D3DXVECTOR3(12.0f, 12.0f, 12.0f);
    enemy->axisSpeed = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->angularVelocity = 0.0f;
    enemy->angle = 0.0f;
    enemy->acceleration = 0.0f;
    enemy->speed = 0.0f;
    enemy->flags.movementMode = 0;
    enemy->flags.shootingDisabled = false;
    enemy->flags.mirrored = false;
    enemy->flags.isBoss = false;
    enemy->stackDepth = 0;
    enemy->life = 1;
    enemy->score = 100;
    enemy->deathParticle1 = PARTICLE_EFFECT_UNK_0;
    enemy->deathParticle2 = 0;
    enemy->deathAnm3 = 0;
    enemy->shootInterval = 0;
    enemy->shootIntervalTimer = 0;
    enemy->shootOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->anmPoseLeft = -1;
    enemy->anmPoseRight = -1;
    enemy->anmPoseDefault = -1;
    enemy->flags.isDamageable = true;
    enemy->flags.deathMode = EnemyDeath_DespawnNoCallback;
    enemy->deathCallbackSub = -1;
    enemy->flags.shouldClampPos = false;
    enemy->effectIdx = 0;
    enemy->runInterrupt = -1;
    enemy->lifeCallbackThreshold = -1;
    enemy->timerCallbackThreshold = -1;
    enemy->laserStore = 0;
    enemy->unk_e41 = 0;
    enemy->flags.rotateAnm = 0;
    enemy->bulletRankSpeedLow = -0.5f;
    enemy->bulletRankSpeedHigh = 0.5f;
}

EnemyManager::EnemyManager()
{
    this->Initialize();
}

Enemy *EnemyManager::SpawnEnemy(i32 eclSubId, D3DXVECTOR3 *pos, i16 life, i16 itemDrop, i32 score)
{
    Enemy *newEnemy;
    i32 idx;

    newEnemy = this->enemies;
    for (idx = 0; idx < MAX_ENEMY_COUNT; idx++, newEnemy++)
    {
        if (newEnemy->flags.isSlotOccupied)
            continue;

        *newEnemy = this->enemyTemplate;

        if (life >= 0)
            newEnemy->life = life;

        newEnemy->position = *pos;
        g_EclManager.CallEclSub(&newEnemy->currentContext, eclSubId);
        g_EclManager.RunEcl(newEnemy);
        newEnemy->color = newEnemy->primaryVm.color;
        newEnemy->itemDrop = itemDrop;

        if (life >= 0)
            newEnemy->life = life;

        if (score >= 0)
            newEnemy->score = score;

        newEnemy->maxLife = newEnemy->life;
        break;
    }
    return newEnemy;
}

static void UpdateEffects(Enemy *enemy)
{
    Effect *effect;
    for (i32 i = 0; i < enemy->effectIdx; i++)
    {
        effect = enemy->effectArray[i];
        if (!effect)
        {
            continue;
        }

        effect->position = enemy->position;
        if (effect->distance < enemy->effectDistance)
        {
            effect->distance += 0.3f;
        }

        effect->angleRelated = utils::AddNormalizeAngle(effect->angleRelated, ZUN_PI / 100.0f);
    }
}

void Enemy_ResetEffectArray(Enemy *enemy)
{
    for (i32 i = 0; i < enemy->effectIdx; i++)
    {
        if (!enemy->effectArray[i])
        {
            continue;
        }
        enemy->effectArray[i]->flag_17a = true;
        enemy->effectArray[i] = NULL;
    }
    enemy->effectIdx = 0;
}

void EnemyManager::RunEclTimeline()
{
    Enemy *spawnedEnemy;

    if (this->timelineInstr == NULL)
    {
        this->timelineInstr = g_EclManager.timeline;
    }
    if (!g_Gui.IsMsgActive())
    {
        // Unclear what this is? It looks like it increases the subrank at
        // regular intervals, where the interval is made shorter based on the
        // number of lives lost?
        i32 subrankIncreaseFrame = 10 * 4 * 60;
        subrankIncreaseFrame -= g_GameManager.livesRemaining * 4 * 60;
        if (this->timelineTime.HasTicked() && MULTIPLE_OF(this->timelineTime, subrankIncreaseFrame))
        {
            g_GameManager.IncreaseSubrank(100);
        }
    }
    while (this->timelineInstr->time >= 0)
    {
        if (this->timelineTime == (i32)this->timelineInstr->time)
        {
            switch (this->timelineInstr->opCode)
            {
            case TIMELINE_OPCODE_ENEMY_CREATE:
                if (!g_Gui.IsBossPresent())
                {
                    TimelineEnemyCreateArgs *args = (TimelineEnemyCreateArgs *)this->timelineInstr->args;
                    this->SpawnEnemy(this->timelineInstr->arg0, &args->pos, args->subLife, args->dropItem, args->score);
                }
                break;
            case TIMELINE_OPCODE_DUMMY_CREATE:
                if (!g_Gui.IsBossPresent())
                {
                    this->SpawnEnemy(this->timelineInstr->arg0, (D3DXVECTOR3 *)this->timelineInstr->args, -1,
                                     ITEM_RANDOM_ITEM, -1);
                }
                break;
            case TIMELINE_OPCODE_ENEMY_CREATE_MIRROR:
                if (!g_Gui.IsBossPresent())
                {
                    TimelineEnemyCreateArgs *args = (TimelineEnemyCreateArgs *)this->timelineInstr->args;
                    spawnedEnemy = this->SpawnEnemy(this->timelineInstr->arg0, &args->pos, args->subLife,
                                                    args->dropItem, args->score);
                    spawnedEnemy->flags.mirrored = true;
                }
                break;
            case TIMELINE_OPCODE_DUMMY_CREATE_MIRROR:
                if (!g_Gui.IsBossPresent())
                {
                    spawnedEnemy = this->SpawnEnemy(this->timelineInstr->arg0, (D3DXVECTOR3 *)this->timelineInstr->args,
                                                    -1, ITEM_RANDOM_ITEM, -1);
                    spawnedEnemy->flags.mirrored = true;
                }
                break;
            case TIMELINE_OPCODE_ENEMY_CREATE_RANDOM: {
                TimelineEnemyCreateArgs *args;
                if (!g_Gui.IsBossPresent())
                {
                    args = (TimelineEnemyCreateArgs *)this->timelineInstr->args;
                    D3DXVECTOR3 pos = args->pos;
                    if (args->pos.x <= -990.0f)
                    {
                        pos.x = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.x);
                    }
                    if (args->pos.y <= -990.0f)
                    {
                        pos.y = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.y);
                    }
                    if (args->pos.z <= -990.0f)
                    {
                        pos.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    this->SpawnEnemy(this->timelineInstr->arg0, &pos, args->subLife, args->dropItem, args->score);
                }
                break;
            }
            case TIMELINE_OPCODE_DUMMY_CREATE_RANDOM:
                if (!g_Gui.IsBossPresent())
                {
                    D3DXVECTOR3 pos = *(D3DXVECTOR3 *)this->timelineInstr->args;
                    if (pos.x <= -990.0f)
                    {
                        pos.x = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.x);
                    }
                    if (pos.y <= -990.0f)
                    {
                        pos.y = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.y);
                    }
                    if (pos.z <= -990.0f)
                    {
                        pos.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    this->SpawnEnemy(this->timelineInstr->arg0, &pos, -1, ITEM_RANDOM_ITEM, -1);
                }
                break;
            case TIMELINE_OPCODE_ENEMY_CREATE_MIRROR_RANDOM: {
                TimelineEnemyCreateArgs *args;
                if (!g_Gui.IsBossPresent())
                {
                    args = (TimelineEnemyCreateArgs *)this->timelineInstr->args;
                    D3DXVECTOR3 pos = args->pos;
                    if (args->pos.x <= -990.0f)
                    {
                        pos.x = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.x);
                    }
                    if (args->pos.y <= -990.0f)
                    {
                        pos.y = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.y);
                    }
                    if (args->pos.z <= -990.0f)
                    {
                        pos.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    spawnedEnemy =
                        this->SpawnEnemy(this->timelineInstr->arg0, &pos, args->subLife, args->dropItem, args->score);
                    spawnedEnemy->flags.mirrored = true;
                }
                break;
            }
            case TIMELINE_OPCODE_DUMMY_CREATE_MIRROR_RANDOM:
                if (!g_Gui.IsBossPresent())
                {
                    D3DXVECTOR3 pos = *(D3DXVECTOR3 *)this->timelineInstr->args;
                    if (pos.x <= -990.0f)
                    {
                        pos.x = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.x);
                    }
                    if (pos.y <= -990.0f)
                    {
                        pos.y = g_Rng.GetRandomF32InRange(g_GameManager.playerMovementAreaSize.y);
                    }
                    if (pos.z <= -990.0f)
                    {
                        pos.z = g_Rng.GetRandomF32InRange(800.0f);
                    }
                    spawnedEnemy = this->SpawnEnemy(this->timelineInstr->arg0, &pos, -1, ITEM_RANDOM_ITEM, -1);
                    spawnedEnemy->flags.mirrored = true;
                }
                break;
            case TIMELINE_OPCODE_MSG_READ:
                if (g_GameManager.difficulty == EASY && g_GameManager.currentStage == Stage5 &&
                    this->timelineInstr->arg0 == 1)
                {
                    g_Gui.MsgRead(MSG_SCRIPT_ID(g_GameManager.character, 3));
                }
                else
                {
                    g_Gui.MsgRead(MSG_SCRIPT_ID(g_GameManager.character, this->timelineInstr->arg0));
                }
                break;
            case TIMELINE_OPCODE_MSG_WAIT:
                if (g_Gui.MsgWait())
                {
                    this->timelineTime--;
                    return;
                }
                break;
            case TIMELINE_OPCODE_BOSS_INTERRUPT:
                this->bosses[this->timelineInstr->args[0]]->runInterrupt = this->timelineInstr->args[1];
                break;
            case TIMELINE_OPCODE_PLAYER_POWER:
                g_GameManager.currentPower = this->timelineInstr->arg0;
                break;
            case TIMELINE_OPCODE_BOSS_WAIT:
                if (this->bosses[this->timelineInstr->arg0] != NULL &&
                    this->bosses[this->timelineInstr->arg0]->flags.isSlotOccupied)
                {
                    this->timelineTime--;
                    return;
                }
                break;
            }
        }
        else if (this->timelineTime < (i32)this->timelineInstr->time)
        {
            break;
        }
        this->timelineInstr = (TimelineInstr *)((u32)this->timelineInstr + this->timelineInstr->size);
    }
    if (!g_Gui.IsMsgActive())
    {
        g_GameManager.counat++;
    }
}

ZunBool Enemy::HandleLifeCallback()
{
    if (this->life < this->lifeCallbackThreshold)
    {
        this->life = this->lifeCallbackThreshold;
        g_EclManager.CallEclSub(&this->currentContext, this->lifeCallbackSub);
        this->lifeCallbackThreshold = -1;
        this->timerCallbackSub = this->deathCallbackSub;
        this->ResetRank();
        this->stackDepth = 0;

        Enemy *curEnemy = g_EnemyManager.enemies;
        for (i32 i = 0; i < MAX_ENEMY_COUNT; i++, curEnemy++)
        {
            if (!curEnemy->flags.isSlotOccupied)
            {
                continue;
            }
            if (curEnemy->flags.isBoss)
            {
                continue;
            }
            curEnemy->life = 0;

            if (!curEnemy->flags.isInteractable && curEnemy->deathCallbackSub >= 0)
            {
                g_EclManager.CallEclSub(&curEnemy->currentContext, curEnemy->deathCallbackSub);
                curEnemy->deathCallbackSub = -1;
            }
        }
        return true;
    }

    return false;
}

ZunBool Enemy::HandleTimerCallback()
{
    if (this->flags.isBoss)
    {
        g_Gui.SetSpellcardSeconds((this->timerCallbackThreshold - (i32)this->phaseTimer) / 60);
    }

    if (this->phaseTimer >= this->timerCallbackThreshold)
    {
        if (this->lifeCallbackThreshold > 0)
        {
            this->life = this->lifeCallbackThreshold;
            this->lifeCallbackThreshold = -1;
        }
        g_EclManager.CallEclSub(&this->currentContext, this->timerCallbackSub);
        this->timerCallbackThreshold = -1;
        this->timerCallbackSub = this->deathCallbackSub;
        this->phaseTimer = 0;
        if (!this->flags.isTimeoutSpell)
        {
            g_EnemyManager.spellcardInfo.isCapturing = false;
            if (g_EnemyManager.spellcardInfo.isActive)
            {
                g_EnemyManager.spellcardInfo.isActive++;
            }
            g_BulletManager.RemoveAllBullets(0);
        }

        Enemy *curEnemy = g_EnemyManager.enemies;
        for (i32 i = 0; i < MAX_ENEMY_COUNT; i++, curEnemy++)
        {
            if (!curEnemy->flags.isSlotOccupied)
            {
                continue;
            }
            if (curEnemy->flags.isBoss)
            {
                continue;
            }
            curEnemy->life = 0;

            if (!curEnemy->flags.isInteractable && curEnemy->deathCallbackSub >= 0)
            {
                g_EclManager.CallEclSub(&curEnemy->currentContext, curEnemy->deathCallbackSub);
                curEnemy->deathCallbackSub = -1;
            }
        }
        this->ResetRank();
        this->stackDepth = 0;
        return true;
    }
    return false;
}

void Enemy::Despawn()
{
    if (this->flags.deathMode == EnemyDeath_DespawnNoCallback)
    {
        this->flags.isSlotOccupied = false;
    }
    else
    {
        this->flags.isInteractable = false;
    }
    if (this->flags.isBoss)
    {
        g_Gui.SetBossPresent(false);
    }
    if (this->effectIdx != 0)
    {
        Enemy_ResetEffectArray(this);
    }
}

void Enemy::ClampPos()
{
    if (this->flags.shouldClampPos)
    {
        if (this->position[0] < this->lowerMoveLimit.x)
        {
            this->position[0] = this->lowerMoveLimit.x;
        }
        else if (this->position[0] > this->upperMoveLimit.x)
        {
            this->position[0] = this->upperMoveLimit.x;
        }

        if (this->position[1] < this->lowerMoveLimit.y)
        {
            this->position[1] = this->lowerMoveLimit.y;
        }
        else if (this->position[1] > this->upperMoveLimit.y)
        {
            this->position[1] = this->upperMoveLimit.y;
        }
    }
}

#pragma var_order(hitByBomb, damage, enemyIdx, enemyHitbox, enemyVmIdx, enemyLifeBeforeDmg, curEnemy)
static ChainCallbackResult EnemyManager_OnUpdate(EnemyManager *mgr)
{
    Enemy *curEnemy;
    i32 enemyLifeBeforeDmg;
    i32 enemyVmIdx;
    D3DXVECTOR3 enemyHitbox;
    i32 enemyIdx;
    i32 damage;
    ZunBool hitByBomb;

    hitByBomb = false;
    mgr->RunEclTimeline();
    for (curEnemy = &mgr->enemies[0], mgr->enemyCount = 0, enemyIdx = 0; enemyIdx < MAX_ENEMY_COUNT;
         enemyIdx++, curEnemy++)
    {
        if (!curEnemy->flags.isSlotOccupied)
        {
            continue;
        }
        mgr->enemyCount++;
        curEnemy->Move();
        curEnemy->ClampPos();
        if (curEnemy->flags.hasBeenInBounds == FALSE &&
            g_GameManager.IsInBounds(curEnemy->position[0], curEnemy->position[1], curEnemy->primaryVm.sprite->widthPx,
                                     curEnemy->primaryVm.sprite->heightPx))
        {
            curEnemy->flags.hasBeenInBounds = TRUE;
        }
        if (curEnemy->flags.hasBeenInBounds == TRUE &&
            !g_GameManager.IsInBounds(curEnemy->position.x, curEnemy->position.y, curEnemy->primaryVm.sprite->widthPx,
                                      curEnemy->primaryVm.sprite->heightPx))
        {
            curEnemy->flags.isSlotOccupied = false;
            curEnemy->Despawn();
            continue;
        }
        if (curEnemy->lifeCallbackThreshold >= 0)
        {
            curEnemy->HandleLifeCallback();
        }
        if (curEnemy->timerCallbackThreshold >= 0)
        {
            curEnemy->HandleTimerCallback();
        }
        if (g_EclManager.RunEcl(curEnemy) == ZUN_ERROR)
        {
            curEnemy->flags.isSlotOccupied = false;
            curEnemy->Despawn();
            continue;
        }
        curEnemy->ClampPos();
        curEnemy->primaryVm.color = curEnemy->color;
        g_AnmManager->ExecuteScript(&curEnemy->primaryVm);
        curEnemy->color = curEnemy->primaryVm.color;
        for (enemyVmIdx = 0; enemyVmIdx < ENEMY_ANM_SLOTS; enemyVmIdx++)
        {
            if (curEnemy->vms[enemyVmIdx].anmFileIndex >= 0 && g_AnmManager->ExecuteScript(&curEnemy->vms[enemyVmIdx]))
            {
                curEnemy->vms[enemyVmIdx].anmFileIndex = -1;
            }
        }
        hitByBomb = false;
        if (curEnemy->flags.hasBeenInBounds && !curEnemy->flags.isInvisible)
        {
            enemyLifeBeforeDmg = curEnemy->life;
            if (curEnemy->flags.isCollidable && curEnemy->flags.isInteractable)
            {
                enemyHitbox = curEnemy->hitboxDimensions / 1.5f;
                if (g_Player.CheckCollision(&curEnemy->position, &enemyHitbox) == COLLISION_HIT &&
                    curEnemy->flags.isInteractable && !curEnemy->flags.isBoss)
                {
                    curEnemy->life -= 10;
                }
            }
            if (curEnemy->flags.isInteractable)
            {
                damage = g_Player.CalcDamageToEnemy(&curEnemy->position, &curEnemy->hitboxDimensions, &hitByBomb);
                if (damage >= 70)
                {
                    damage = 70;
                }
                g_GameManager.score = (damage / 5) * 10 + g_GameManager.score;
                if (mgr->spellcardInfo.isActive)
                {
                    if (!hitByBomb)
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
                    else if (mgr->spellcardInfo.usedBomb)
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
                if (curEnemy->flags.isDamageable)
                {
                    curEnemy->life -= damage;
                }
                if (g_Player.positionOfLastEnemyHit.y < curEnemy->position.y)
                {
                    g_Player.positionOfLastEnemyHit = curEnemy->position;
                }
            }
            if (curEnemy->life <= 0 && curEnemy->flags.isInteractable)
            {
                curEnemy->lifeCallbackThreshold = -1;
                curEnemy->timerCallbackThreshold = -1;
                switch (curEnemy->flags.deathMode)
                {
                case EnemyDeath_SetHpTo1:
                    curEnemy->life = 1;
                    curEnemy->flags.isDamageable = false;
                    curEnemy->flags.deathMode = EnemyDeath_DespawnNoCallback;
                    g_Gui.SetBossPresent(false);
                    g_EffectManager.SpawnParticles(curEnemy->deathParticle1, &curEnemy->position, 1, COLOR_WHITE);
                    g_EffectManager.SpawnParticles(curEnemy->deathParticle1, &curEnemy->position, 1, COLOR_WHITE);
                    g_EffectManager.SpawnParticles(curEnemy->deathParticle1, &curEnemy->position, 1, COLOR_WHITE);
                    break;
                case EnemyDeath_DisableInteraction:
                    g_GameManager.AddScore(curEnemy->score);
                    curEnemy->flags.isInteractable = false;
                    goto LAB_00412a4d;
                case EnemyDeath_DespawnNoCallback:
                    g_GameManager.AddScore(curEnemy->score);
                    curEnemy->flags.isSlotOccupied = false;
                LAB_00412a4d:
                    if (curEnemy->flags.isBoss)
                    {
                        g_Gui.SetBossPresent(false);
                        Enemy_ResetEffectArray(curEnemy);
                    }
                case EnemyDeath_DropItemsOnly:
                    if (curEnemy->itemDrop >= 0)
                    {
                        g_EffectManager.SpawnParticles(curEnemy->deathParticle2 + 4, &curEnemy->position, 3,
                                                       COLOR_WHITE);
                        g_ItemManager.SpawnItem(&curEnemy->position, (ItemType)curEnemy->itemDrop,
                                                (ItemState)hitByBomb);
                    }
                    else if (curEnemy->itemDrop == ITEM_RANDOM_ITEM)
                    {
                        if (mgr->randomItemSpawnIndex % ITEM_SPAWNS == 0)
                        {
                            g_EffectManager.SpawnParticles(curEnemy->deathParticle2 + 4, &curEnemy->position, 6,
                                                           COLOR_WHITE);
                            g_ItemManager.SpawnItem(&curEnemy->position,
                                                    (ItemType)g_RandomItems[mgr->randomItemTableIndex],
                                                    (ItemState)hitByBomb);
                            mgr->randomItemTableIndex++;
                            if (mgr->randomItemTableIndex >= ARRAY_SIZE_SIGNED(g_RandomItems))
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
                g_EffectManager.SpawnParticles(curEnemy->deathParticle1, &curEnemy->position, 1, COLOR_WHITE);
                g_EffectManager.SpawnParticles(curEnemy->deathParticle2 + 4, &curEnemy->position, 4, COLOR_WHITE);
                if (curEnemy->deathCallbackSub >= 0)
                {
                    curEnemy->ResetRank();
                    curEnemy->stackDepth = 0;
                    g_EclManager.CallEclSub(&curEnemy->currentContext, curEnemy->deathCallbackSub);
                    curEnemy->deathCallbackSub = -1;
                }
            }
            if (curEnemy->flags.isBoss && !g_Gui.IsMsgActive())
            {
                g_Gui.SetBossHealthBar(curEnemy->LifePercent());
            }
            if (curEnemy->unk_e41 != 0)
            {
                curEnemy->unk_e41--;
                curEnemy->primaryVm.flags.colorOp = AnmColorOp_Modulate;
            }
            else if (enemyLifeBeforeDmg > curEnemy->life)
            {
                g_SoundPlayer.PlaySoundByIdx(SOUND_TOTAL_BOSS_DEATH);
                curEnemy->primaryVm.flags.colorOp = AnmColorOp_Add;
                curEnemy->unk_e41 = 4;
            }
            else
            {
                curEnemy->primaryVm.flags.colorOp = AnmColorOp_Modulate;
            }
        }
        UpdateEffects(curEnemy);
        if (!g_GameManager.isTimeStopped)
        {
            curEnemy->phaseTimer++;
        }
    }
    mgr->timelineTime++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(curEnemyIdx, curEnemyVm, curEnemyVmIdx, curEnemy)
static ChainCallbackResult EnemyManager_OnDraw(EnemyManager *mgr)
{
    AnmVm *curEnemyVm;
    Enemy *curEnemy;
    i32 curEnemyVmIdx;
    i32 curEnemyIdx;

    for (curEnemy = &mgr->enemies[0], curEnemyIdx = 0; curEnemyIdx < MAX_ENEMY_COUNT; curEnemyIdx++, curEnemy++)
    {
        if (!curEnemy->flags.isSlotOccupied)
        {
            continue;
        }
        if (curEnemy->flags.isInvisible)
        {
            continue;
        }

        for (curEnemyVm = &curEnemy->vms[0], curEnemyVmIdx = 0; curEnemyVmIdx < ENEMY_ANM_SLOTS / 2;
             curEnemyVmIdx++, curEnemyVm++)
        {
            if (curEnemyVm->anmFileIndex >= 0)
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
        if (curEnemy->flags.rotateAnm)
        {
            curEnemy->primaryVm.rotation.z = curEnemy->angle;
        }
        curEnemy->primaryVm.pos = curEnemy->position + curEnemy->primaryVm.posOffset;
        curEnemy->primaryVm.pos.z = 0.494f;
        g_AnmManager->Draw2(&curEnemy->primaryVm);
        for (curEnemyVmIdx = ENEMY_ANM_SLOTS / 2; curEnemyVmIdx < ENEMY_ANM_SLOTS; curEnemyVmIdx++, curEnemyVm++)
        {
            if (curEnemyVm->anmFileIndex >= 0)
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

static ZunResult EnemyManager_AddedCallback(EnemyManager *enemyManager)
{
    Enemy *enemies = enemyManager->enemies;

    if (enemyManager->stgEnmAnmFilename &&
        g_AnmManager->LoadAnm(ANM_FILE_ENEMY, enemyManager->stgEnmAnmFilename, ANM_OFFSET_ENEMY) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (enemyManager->stgEnm2AnmFilename &&
        g_AnmManager->LoadAnm(ANM_FILE_ENEMY2, enemyManager->stgEnm2AnmFilename, ANM_OFFSET_ENEMY) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    enemyManager->randomItemSpawnIndex = g_Rng.GetRandomU16InRange(ITEM_SPAWNS);
    enemyManager->randomItemTableIndex = g_Rng.GetRandomU16InRange(ITEM_TABLES);

    enemyManager->spellcardInfo.isActive = 0;
    enemyManager->timelineInstr = NULL;

    return ZUN_SUCCESS;
}

static ZunResult EnemyManager_DeletedCallback(EnemyManager *mgr)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_ENEMY2);
    g_AnmManager->ReleaseAnm(ANM_FILE_ENEMY);
    return ZUN_SUCCESS;
}

ZunResult EnemyManager_RegisterChain(const char *stgEnm1, const char *stgEnm2)
{
    EnemyManager *mgr = &g_EnemyManager;
    mgr->Initialize();
    mgr->stgEnmAnmFilename = stgEnm1;
    mgr->stgEnm2AnmFilename = stgEnm2;
    g_EnemyManagerCalcChain.SetCallback((ChainCallback)EnemyManager_OnUpdate);
    g_EnemyManagerCalcChain.addedCallback = (ChainAddedCallback)EnemyManager_AddedCallback;
    g_EnemyManagerCalcChain.deletedCallback = (ChainAddedCallback)EnemyManager_DeletedCallback;
    g_EnemyManagerCalcChain.arg = mgr;
    if (g_FuncChain.AddToCalcChain(&g_EnemyManagerCalcChain, TH_CHAIN_PRIO_CALC_ENEMYMANAGER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    g_EnemyDrawChain.SetCallback((ChainCallback)EnemyManager_OnDraw);
    g_EnemyDrawChain.arg = mgr;
    if (g_FuncChain.AddToDrawChain(&g_EnemyDrawChain, TH_CHAIN_PRIO_DRAW_ENEMYMANAGER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    return ZUN_SUCCESS;
}

void EnemyManager_CutChain()
{
    g_FuncChain.Cut(&g_EnemyManagerCalcChain);
    g_FuncChain.Cut(&g_EnemyDrawChain);
}
} // namespace th06
