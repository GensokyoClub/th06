#include "stdafx.h"

#include "EffectManager.hpp"
#include "EnemyManager.hpp"
#include "ItemManager.hpp"

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

    Enemy *enemy = &this->enemies[0];
    memset(this, 0, sizeof(EnemyManager));
    enemy = &this->enemyTemplate;
    memset(enemy, 0, sizeof(Enemy));
    for (i = 0; i < ENEMY_ANM_SLOTS; i++)
    {
        enemy->vms[i].scriptId = -1;
    }
    enemy->flags.isSlotOccupied = true;
    enemy->phaseTimer = 0;
    enemy->flags.isInteractable = true;
    enemy->flags.hasContactHitbox = true;
    enemy->flags.hasBeenInBounds = false;
    enemy->hitboxSize = D3DXVECTOR3(12.0f, 12.0f, 12.0f);
    enemy->axisSpeed = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->angularVelocity = 0.0f;
    enemy->angle = 0.0f;
    enemy->acceleration = 0.0f;
    enemy->speed = 0.0f;
    enemy->flags.movementMode = EnemyMove_AxisSpeed;
    enemy->flags.shootingDisabled = false;
    enemy->flags.mirrored = false;
    enemy->flags.isBoss = false;
    enemy->stackDepth = 0;
    enemy->life = 1;
    enemy->score = 100;
    enemy->deathParticle = PARTICLE_POP;
    enemy->deathSparkParticle = SPARK_TYPE_WHITE;
    enemy->unkParticle = 0;
    enemy->shootInterval = 0;
    enemy->shootIntervalTimer = 0;
    enemy->shootOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->anmPoseLeft = -1;
    enemy->anmPoseRight = -1;
    enemy->anmPoseDefault = -1;
    enemy->flags.isDamageable = true;
    enemy->flags.deathMode = EnemyDeath_DespawnNoCallback;
    enemy->deathCallbackSub = -1;
    enemy->flags.hasMovementBounds = false;
    enemy->spellEffectCount = 0;
    enemy->runInterrupt = -1;
    enemy->lifeCallbackThreshold = -1;
    enemy->timerCallbackThreshold = -1;
    enemy->laserStore = 0;
    enemy->hitEffectDelay = 0;
    enemy->flags.rotateAnm = 0;
    enemy->bulletRankSpeedLow = -0.5f;
    enemy->bulletRankSpeedHigh = 0.5f;
}

EnemyManager::EnemyManager()
{
    this->Initialize();
}

#pragma var_order(i, newEnemy)
Enemy *EnemyManager::SpawnEnemy(i32 eclSubId, D3DXVECTOR3 *pos, i16 life, i16 itemDrop, i32 score)
{
    Enemy *newEnemy = this->enemies;
    i32 i;
    for (i = 0; i < MAX_ENEMY_COUNT; i++, newEnemy++)
    {
        if (newEnemy->flags.isSlotOccupied)
        {
            continue;
        }

        *newEnemy = this->enemyTemplate;

        if (life >= 0)
        {
            newEnemy->life = life;
        }

        newEnemy->position = *pos;
        g_EclManager.CallEclSub(&newEnemy->currentContext, eclSubId);
        g_EclManager.RunEcl(newEnemy);
        newEnemy->color = newEnemy->primaryVm.color;
        newEnemy->itemDrop = itemDrop;

        if (life >= 0)
        {
            newEnemy->life = life;
        }

        if (score >= 0)
        {
            newEnemy->score = score;
        }

        newEnemy->maxLife = newEnemy->life;
        break;
    }
    return newEnemy;
}

static void UpdateSpellEffects(Enemy *enemy)
{
    Effect *effect;
    for (i32 i = 0; i < enemy->spellEffectCount; i++)
    {
        effect = enemy->spellEffects[i];
        if (!effect)
        {
            continue;
        }

        effect->offset = enemy->position;
        if (effect->radius < enemy->spellEffectRadius)
        {
            effect->radius += 0.3f;
        }

        effect->angle = ReduceAngleAdd(effect->angle, ZUN_PI / 100.0f);
    }
}

void ResetSpellEffects(Enemy *enemy)
{
    for (i32 i = 0; i < enemy->spellEffectCount; i++)
    {
        if (!enemy->spellEffects[i])
        {
            continue;
        }
        enemy->spellEffects[i]->isFadingOut = true;
        enemy->spellEffects[i] = NULL;
    }
    enemy->spellEffectCount = 0;
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
        subrankIncreaseFrame -= g_GameManager.GetLives() * 4 * 60;
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
                    this->SpawnEnemy(this->timelineInstr->arg0, &args->pos, args->subLife, args->itemDrop, args->score);
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
                                                    args->itemDrop, args->score);
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
                    this->SpawnEnemy(this->timelineInstr->arg0, &pos, args->subLife, args->itemDrop, args->score);
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
                        this->SpawnEnemy(this->timelineInstr->arg0, &pos, args->subLife, args->itemDrop, args->score);
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
        this->timelineInstr = OFFSET_PTR(TimelineInstr *, this->timelineInstr, this->timelineInstr->size);
    }
    if (!g_Gui.IsMsgActive())
    {
        g_GameManager.gameplayTicks++;
    }
}

BOOL Enemy::HandleLifeCallback()
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

BOOL Enemy::HandleTimerCallback()
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
            if (g_EnemyManager.spellcardInfo.ticksActive != 0)
            {
                g_EnemyManager.spellcardInfo.ticksActive++;
            }
            g_BulletManager.DespawnBullets(BULLET_CLEAR);
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
    if (this->spellEffectCount != 0)
    {
        ResetSpellEffects(this);
    }
}

void Enemy::ClampPos()
{
    if (this->flags.hasMovementBounds)
    {
        if (this->position[0] < this->moveLimit.left)
        {
            this->position[0] = this->moveLimit.left;
        }
        else if (this->position[0] > this->moveLimit.right)
        {
            this->position[0] = this->moveLimit.right;
        }

        if (this->position[1] < this->moveLimit.top)
        {
            this->position[1] = this->moveLimit.top;
        }
        else if (this->position[1] > this->moveLimit.bottom)
        {
            this->position[1] = this->moveLimit.bottom;
        }
    }
}

#pragma var_order(hitByBomb, damage, i, enemyHitbox, j, enemyLifeBeforeDmg, curEnemy)
static ChainCallbackResult EnemyManager_OnUpdate(EnemyManager *mgr)
{
    Enemy *curEnemy;
    i32 enemyLifeBeforeDmg;
    i32 j;
    D3DXVECTOR3 enemyHitbox;
    i32 i;
    i32 damage;
    BOOL hitByBomb;

    hitByBomb = false;
    mgr->RunEclTimeline();
    curEnemy = &mgr->enemies[0];
    mgr->enemyCount = 0;
    for (i = 0; i < MAX_ENEMY_COUNT; i++, curEnemy++)
    {
        if (!curEnemy->flags.isSlotOccupied)
        {
            continue;
        }
        mgr->enemyCount++;
        curEnemy->Move();
        curEnemy->ClampPos();
        if (curEnemy->flags.hasBeenInBounds == FALSE &&
            g_GameManager.IsInBounds(curEnemy->position[0], curEnemy->position[1], curEnemy->primaryVm.sprite->width,
                                     curEnemy->primaryVm.sprite->height))
        {
            curEnemy->flags.hasBeenInBounds = TRUE;
        }
        if (curEnemy->flags.hasBeenInBounds == TRUE &&
            !g_GameManager.IsInBounds(curEnemy->position.x, curEnemy->position.y, curEnemy->primaryVm.sprite->width,
                                      curEnemy->primaryVm.sprite->height))
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
        g_AnmManager->RunAnm(&curEnemy->primaryVm);
        curEnemy->color = curEnemy->primaryVm.color;
        for (j = 0; j < ENEMY_ANM_SLOTS; j++)
        {
            if (curEnemy->vms[j].scriptId >= 0 && g_AnmManager->RunAnm(&curEnemy->vms[j]) != SCRIPT_RUNNING)
            {
                curEnemy->vms[j].scriptId = -1;
            }
        }
        hitByBomb = false;
        if (curEnemy->flags.hasBeenInBounds && !curEnemy->flags.isInvisible)
        {
            enemyLifeBeforeDmg = curEnemy->life;
            if (curEnemy->flags.hasContactHitbox && curEnemy->flags.isInteractable)
            {
                enemyHitbox = curEnemy->hitboxSize / 1.5f;
                if (g_Player.CheckCollision(&curEnemy->position, &enemyHitbox) == COLLISION_HIT &&
                    curEnemy->flags.isInteractable && !curEnemy->flags.isBoss)
                {
                    curEnemy->life -= 10;
                }
            }
            if (curEnemy->flags.isInteractable)
            {
                damage = g_Player.CalcDamageToEnemy(&curEnemy->position, &curEnemy->hitboxSize, &hitByBomb);
                if (damage >= 70)
                {
                    damage = 70;
                }
                g_GameManager.score = (damage / 5) * 10 + g_GameManager.score;
                if (mgr->spellcardInfo.ticksActive != 0)
                {
                    if (!hitByBomb)
                    {
                        if (damage > 7)
                        {
                            damage /= 7;
                        }
                        else if (damage != 0)
                        {
                            damage = 1;
                        }
                    }
                    // Was the bomb used after the current spell started?
                    else if (mgr->spellcardInfo.usedBombTick != 0)
                    {
                        if (damage > 3)
                        {
                            damage /= 3;
                        }
                        else if (damage != 0)
                        {
                            damage = 1;
                        }
                    }
                    else
                    {
                        // Bombs used before a spell starts do no damage during spells
                        // BUG(?): This negates all non-bomb damage simultaneously
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
                    // NOTE: SpawnParticles has a count argument, which could've
                    // been used here instead of repeating the call 3 times.
                    g_EffectManager.SpawnParticles(curEnemy->deathParticle, &curEnemy->position);
                    g_EffectManager.SpawnParticles(curEnemy->deathParticle, &curEnemy->position);
                    g_EffectManager.SpawnParticles(curEnemy->deathParticle, &curEnemy->position);
                    break;
                case EnemyDeath_DisableInteraction:
                    g_GameManager.AddScore(curEnemy->score);
                    curEnemy->flags.isInteractable = false;
                    goto common_boss_logic;
                case EnemyDeath_DespawnNoCallback:
                    g_GameManager.AddScore(curEnemy->score);
                    curEnemy->flags.isSlotOccupied = false;
                common_boss_logic:
                    if (curEnemy->flags.isBoss)
                    {
                        g_Gui.SetBossPresent(false);
                        ResetSpellEffects(curEnemy);
                    }
                    // fallthrough
                case EnemyDeath_DropItemsOnly:
                    if (curEnemy->itemDrop >= 0)
                    {
                        g_EffectManager.SpawnParticles(curEnemy->deathSparkParticle + PARTICLE_SPARK,
                                                       &curEnemy->position, 3);
                        g_ItemManager.SpawnItem(&curEnemy->position, curEnemy->itemDrop, hitByBomb);
                    }
                    else if (curEnemy->itemDrop == ITEM_RANDOM_ITEM)
                    {
                        if (mgr->randomItemSpawnIndex % ITEM_SPAWNS == 0)
                        {
                            g_EffectManager.SpawnParticles(curEnemy->deathSparkParticle + PARTICLE_SPARK,
                                                           &curEnemy->position, 6);
                            g_ItemManager.SpawnItem(&curEnemy->position, g_RandomItems[mgr->randomItemTableIndex],
                                                    hitByBomb);
                            if (++mgr->randomItemTableIndex >= ARRAY_SIZE_SIGNED(g_RandomItems))
                            {
                                mgr->randomItemTableIndex = 0;
                            }
                        }
                        mgr->randomItemSpawnIndex++;
                    }
                    if (curEnemy->flags.isBoss && g_EnemyManager.spellcardInfo.ticksActive == 0)
                    {
                        g_BulletManager.DespawnBulletsForScore(12800, BULLET_CLEAR);
                    }
                    curEnemy->life = 0;
                    break;
                }
                g_SoundPlayer.PlaySoundId((i % 2) + SOUND_2);
                g_EffectManager.SpawnParticles(curEnemy->deathParticle, &curEnemy->position);
                g_EffectManager.SpawnParticles(curEnemy->deathSparkParticle + PARTICLE_SPARK, &curEnemy->position, 4);
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
            if (curEnemy->hitEffectDelay != 0)
            {
                curEnemy->hitEffectDelay--;
                curEnemy->primaryVm.flags.colorOp = AnmColorOp_Modulate;
            }
            else if (enemyLifeBeforeDmg > curEnemy->life)
            {
                g_SoundPlayer.PlaySoundId(SOUND_TOTAL_BOSS_DEATH);
                curEnemy->primaryVm.flags.colorOp = AnmColorOp_Add;
                curEnemy->hitEffectDelay = 4;
            }
            else
            {
                curEnemy->primaryVm.flags.colorOp = AnmColorOp_Modulate;
            }
        }
        UpdateSpellEffects(curEnemy);
        if (!g_GameManager.isTimeStopped)
        {
            curEnemy->phaseTimer++;
        }
    }
    mgr->timelineTime++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(i, curEnemyVm, j, curEnemy)
static ChainCallbackResult EnemyManager_OnDraw(EnemyManager *mgr)
{
    AnmVm *curEnemyVm;
    Enemy *curEnemy;
    i32 j;
    i32 i;

    curEnemy = &mgr->enemies[0];
    for (i = 0; i < MAX_ENEMY_COUNT; i++, curEnemy++)
    {
        if (!curEnemy->flags.isSlotOccupied)
        {
            continue;
        }
        if (curEnemy->flags.isInvisible)
        {
            continue;
        }

        curEnemyVm = &curEnemy->vms[0];
        for (j = 0; j < ENEMY_ANM_SLOTS / 2; j++, curEnemyVm++)
        {
            if (curEnemyVm->scriptId >= 0)
            {
                if (curEnemyVm->autoRotate != AnmAutoRotate_Off)
                {
                    curEnemyVm->rotation.z = curEnemy->angle;
                }
                curEnemyVm->pos = curEnemy->position + curEnemyVm->posOffset;
                curEnemyVm->pos.z = 0.495f;
                g_AnmManager->Draw3D(curEnemyVm);
            }
        }
        if (curEnemy->flags.rotateAnm)
        {
            curEnemy->primaryVm.rotation.z = curEnemy->angle;
        }
        curEnemy->primaryVm.pos = curEnemy->position + curEnemy->primaryVm.posOffset;
        curEnemy->primaryVm.pos.z = 0.494f;
        g_AnmManager->Draw3D(&curEnemy->primaryVm);
        for (j = ENEMY_ANM_SLOTS / 2; j < ENEMY_ANM_SLOTS; j++, curEnemyVm++)
        {
            if (curEnemyVm->scriptId >= 0)
            {
                if (curEnemyVm->autoRotate != AnmAutoRotate_Off)
                {
                    curEnemyVm->rotation.z = curEnemy->angle;
                }
                curEnemyVm->pos = curEnemy->position + curEnemyVm->posOffset;
                curEnemyVm->pos.z = 0.495f;
                g_AnmManager->Draw3D(curEnemyVm);
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

    enemyManager->spellcardInfo.ticksActive = 0;
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
