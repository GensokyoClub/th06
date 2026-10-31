#include "stdafx.h"
#include "DebugPrint.hpp"

#include "EclManager.hpp"
#include "EffectManager.hpp"
#include "EnemyEclInstr.hpp"
#include "EnemyManager.hpp"
#include "Gui.hpp"
#include "Stage.hpp"
#include "ItemManager.hpp"
#include "BulletManager.hpp"

namespace th06
{
static i32 *EclGetVar(Enemy *enemy, EclVarId *varId, EclValueType *valueType);
static f32 *EclGetVarFloat(Enemy *enemy, f32 *varId, EclValueType *valueType);
static void EclSetVar(Enemy *enemy, EclVarId out, void *value);

static void EclMathAdd(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
static void EclMathSub(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
static void EclMathMul(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
static void EclMathDiv(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
static void EclMathMod(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
#if BUILT_SINCE(100)
static void EclMathLineAngle(Enemy *enemy, EclVarId out, f32 *a1, f32 *a2, f32 *b1, f32 *b2);
#endif

static void EclMoveDirTime(Enemy *enemy, EclRawInstr *instr);
static void EclMovePosTime(Enemy *enemy, EclRawInstr *instr);
static void EclMoveTime(Enemy *enemy, EclRawInstr *instr);

// clang-format off
i32 g_SpellcardScore[] = {
#if BUILT_SINCE(100)
    // Stage 1
    200000, 200000, 200000,
    // Stage 2
    200000, 200000, 200000, 200000,
    // Stage 3
    250000, 250000, 250000, 250000, 250000, 250000, 250000,
    // Stage 4
    300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000,
    300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000,
    // Stage 5
    400000, 400000, 400000, 400000, 400000, 400000, 400000, 400000,
    // Stage 6
    500000, 500000, 500000, 500000, 500000, 500000, 600000, 600000, 600000, 600000, 600000,
    // Extra
    700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000,
#else
    // Stage 1
    150000, 150000, 150000,
    // Stage 2
    150000, 150000, 150000, 150000,
    // Stage 3
    200000, 200000, 200000, 200000, 200000, 200000, 200000,
    // Stage 4
    250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000,
    250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000, 250000,
    // Stage 5
    300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000,
    // Stage 6
    500000, 300000, 300000, 300000, 300000, 300000, 400000, 400000, 400000, 400000, 400000,
#endif
};
// clang-format on

typedef void (*ExInsn)(Enemy *, EclRawInstr *);
ExInsn g_EclExInsn[] = {
    ExInsCirnoRainbowBallJank,
    ExInsShootAtRandomArea,
    ExInsShootStarPattern,
    ExInsPatchouliShottypeSetVars,
    ExInsStage56Func4,
    ExInsStage5Func5,
    ExInsBatWingEffect,
    ExInsStage6Func7,
    ExInsStage6Func8,
    ExInsStage6Func9,
    ExInsHandleBatTransformation,
    ExInsStage6Func11,
    ExInsStage4Func12,
#if BUILT_SINCE(100)
    ExInsStageXFunc13,
    ExInsStageXFunc14,
    ExInsStageXFunc15,
    ExInsFlandreFinalContextUpdate,
#endif
};

ChainElem g_EclManagerCalcChain; // unused
EclManager g_EclManager;

ZunResult EclManager::Load(const char *eclPath)
{
    this->eclFile = (EclRawHeader *)FileSystem::OpenPath(eclPath);
    if (this->eclFile == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_ECLMANAGER_ENEMY_DATA_CORRUPT);
        return ZUN_ERROR;
    }
    this->eclFile->timelineOffsets[0] = (TimelineInstr *)((u32)this->eclFile->timelineOffsets[0] + (u32)this->eclFile);
    this->subTable = &this->eclFile->subOffsets[0];
    for (i32 idx = 0; idx < this->eclFile->subCount; idx++)
    {
        this->subTable[idx] = (EclRawInstr *)((u32)this->subTable[idx] + (u32)this->eclFile);
    }
    this->timeline = this->eclFile->timelineOffsets[0];
    return ZUN_SUCCESS;
}

void EclManager::Unload()
{
    if (this->eclFile != NULL)
    {
        ZUN_FREE(this->eclFile);
    }
    this->eclFile = NULL;
}

ZunResult EclManager::CallEclSub(EnemyEclContext *ctx, i16 subId)
{
    ctx->currentInstr = this->subTable[subId];
    ctx->time = 0;
    ctx->subId = subId;
    return ZUN_SUCCESS;
}

#define GET_ARG(type, num) ((type *)args)[num]
#define GET_STRING_ARG(num) &GET_ARG(char, num)
#define GET_INT_ARG(num) GET_ARG(i32, num)
#define GET_SHORT_ARG(num) GET_ARG(i16, num)
#define GET_FLOAT_ARG(num) GET_ARG(float, num)
#define GET_VAR_ARG(num) GET_ARG(EclVarId, num)

#define GET_INSTR_ARG(type, num) ((type *)curInstr->args)[num]
#define GET_INSTR_STRING_ARG(num) &GET_INSTR_ARG(char, num)
#define GET_INSTR_INT_ARG(num) GET_INSTR_ARG(i32, num)
#define GET_INSTR_SHORT_ARG(num) GET_INSTR_ARG(i16, num)
#define GET_INSTR_FLOAT_ARG(num) GET_INSTR_ARG(float, num)
#define GET_INSTR_VAR_ARG(num) GET_INSTR_ARG(EclVarId, num)

#pragma var_order(genericFloat3, genericInt, genericFloat, args, curInstr)
ZunResult EclManager::RunEcl(Enemy *enemy)
{
    EclRawInstr *curInstr;
    void *args;
    ZunVec3 genericFloat3;
    i32 genericInt;
    f32 genericFloat;

restart_sub_changed:
    curInstr = enemy->currentContext.currentInstr;
    if (enemy->runInterrupt >= 0)
    {
        goto run_interrupt;
    }

    while (enemy->currentContext.time == curInstr->time)
    {
        if (!(curInstr->skipForDifficulty & (1 << g_GameManager.difficulty)))
        {
            goto next_instruction;
        }

        args = curInstr->args;
        switch (curInstr->opCode)
        {
        case ECL_OPCODE_ENEMY_DELETE:
            return ZUN_ERROR;
        case ECL_OPCODE_LOOP:
            genericInt = *EclGetVar(enemy, &GET_VAR_ARG(2), NULL);
            genericInt--;
            EclSetVar(enemy, GET_VAR_ARG(2), &genericInt);
            if (genericInt <= 0)
                break;
            // fallthrough
        case ECL_OPCODE_JUMP:
        handle_jump:
            enemy->currentContext.time.current = GET_INSTR_INT_ARG(0);
            curInstr = (EclRawInstr *)((u32)curInstr + GET_INT_ARG(1));
            continue;
        case ECL_OPCODE_SET_INT:
        case ECL_OPCODE_SET_FLOAT:
            EclSetVar(enemy, GET_INSTR_VAR_ARG(0), &GET_VAR_ARG(1));
            break;
        case ECL_OPCODE_MATH_REDUCE_ANGLE:
            genericFloat = *(f32 *)EclGetVar(enemy, &GET_INSTR_VAR_ARG(0), NULL);
            genericFloat = utils::AddNormalizeAngle(genericFloat, 0.0f);
            EclSetVar(enemy, GET_INSTR_VAR_ARG(0), &genericFloat);
            break;
        case ECL_OPCODE_SET_INT_RAND: {
            i32 range = *EclGetVar(enemy, &GET_VAR_ARG(1), NULL);
            genericInt = g_Rng.GetRandomU32InRange(range);
            EclSetVar(enemy, GET_INSTR_VAR_ARG(0), &genericInt);
            break;
        }
#pragma var_order(range, minVal)
        case ECL_OPCODE_SET_INT_RAND_MIN: {
            i32 range = *EclGetVar(enemy, &GET_VAR_ARG(1), NULL);
            i32 minVal = *EclGetVar(enemy, &GET_VAR_ARG(2), NULL);
            genericInt = g_Rng.GetRandomU32InRange(range);
            genericInt += minVal;
            EclSetVar(enemy, GET_INSTR_VAR_ARG(0), &genericInt);
            break;
        }
        case ECL_OPCODE_SET_FLOAT_RAND: {
            f32 range = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(1), NULL);
            genericFloat = g_Rng.GetRandomF32InRange(range);
            EclSetVar(enemy, GET_INSTR_VAR_ARG(0), &genericFloat);
            break;
        }
#pragma var_order(range, minVal)
        case ECL_OPCODE_SET_FLOAT_RAND_MIN: {
            float range = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(1), NULL);
            float minVal = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(2), NULL);
            genericFloat = g_Rng.GetRandomF32InRange(range);
            genericFloat += minVal;
            EclSetVar(enemy, GET_INSTR_VAR_ARG(0), &genericFloat);
            break;
        }
        case ECL_OPCODE_SET_VAR_SELF_X:
            EclSetVar(enemy, GET_INSTR_VAR_ARG(0), &enemy->position.x);
            break;
        case ECL_OPCODE_SET_VAR_SELF_Y:
            EclSetVar(enemy, GET_INSTR_VAR_ARG(0), &enemy->position.y);
            break;
        case ECL_OPCODE_SET_VAR_SELF_Z:
            EclSetVar(enemy, GET_INSTR_VAR_ARG(0), &enemy->position.z);
            break;
        case ECL_OPCODE_MATH_INT_ADD:
        case ECL_OPCODE_MATH_FLOAT_ADD:
            EclMathAdd(enemy, GET_INSTR_VAR_ARG(0), &GET_VAR_ARG(1), &GET_VAR_ARG(2));
            break;
        case ECL_OPCODE_MATH_INC: {
            i32 *var = EclGetVar(enemy, &GET_INSTR_VAR_ARG(0), NULL);
            *var += 1;
            break;
        }
        case ECL_OPCODE_MATH_DEC: {
            i32 *var = EclGetVar(enemy, &GET_INSTR_VAR_ARG(0), NULL);
            *var -= 1;
            break;
        }
        case ECL_OPCODE_MATH_INT_SUB:
        case ECL_OPCODE_MATH_FLOAT_SUB:
            EclMathSub(enemy, GET_INSTR_VAR_ARG(0), &GET_VAR_ARG(1), &GET_VAR_ARG(2));
            break;
        case ECL_OPCODE_MATH_INT_MUL:
        case ECL_OPCODE_MATH_FLOAT_MUL:
            EclMathMul(enemy, GET_INSTR_VAR_ARG(0), &GET_VAR_ARG(1), &GET_VAR_ARG(2));
            break;
        case ECL_OPCODE_MATH_INT_DIV:
        case ECL_OPCODE_MATH_FLOAT_DIV:
            EclMathDiv(enemy, GET_INSTR_VAR_ARG(0), &GET_VAR_ARG(1), &GET_VAR_ARG(2));
            break;
        case ECL_OPCODE_MATH_INT_MOD:
        case ECL_OPCODE_MATH_FLOAT_MOD:
            EclMathMod(enemy, GET_INSTR_VAR_ARG(0), &GET_VAR_ARG(1), &GET_VAR_ARG(2));
            break;
#if BUILT_SINCE(100)
        case ECL_OPCODE_MATH_LINE_ANGLE:
            EclMathLineAngle(enemy, GET_INSTR_VAR_ARG(0), &GET_FLOAT_ARG(1), &GET_FLOAT_ARG(2), &GET_FLOAT_ARG(3),
                             &GET_FLOAT_ARG(4));
            break;
#endif
#pragma var_order(rhs, lhs)
        case ECL_OPCODE_CMP_INT: {
            i32 lhs = *EclGetVar(enemy, &GET_INSTR_VAR_ARG(0), NULL);
            i32 rhs = *EclGetVar(enemy, &GET_INSTR_VAR_ARG(1), NULL);
            enemy->currentContext.compareRegister = lhs == rhs ? 0 : lhs < rhs ? -1 : 1;
            break;
        }
#pragma var_order(lhs, rhs)
        case ECL_OPCODE_CMP_FLOAT: {
            float lhs = *EclGetVarFloat(enemy, &GET_INSTR_FLOAT_ARG(0), NULL);
            float rhs = *EclGetVarFloat(enemy, &GET_INSTR_FLOAT_ARG(1), NULL);
            enemy->currentContext.compareRegister = lhs == rhs ? 0 : lhs < rhs ? -1 : 1;
            break;
        }
        case ECL_OPCODE_JUMP_LSS:
            if (enemy->currentContext.compareRegister < 0)
                goto handle_jump;
            break;
        case ECL_OPCODE_JUMP_LEQ:
            if (enemy->currentContext.compareRegister <= 0)
                goto handle_jump;
            break;
        case ECL_OPCODE_JUMP_EQU:
            if (enemy->currentContext.compareRegister == 0)
                goto handle_jump;
            break;
        case ECL_OPCODE_JUMP_GRE:
            if (enemy->currentContext.compareRegister > 0)
                goto handle_jump;
            break;
        case ECL_OPCODE_JUMP_GEQ:
            if (enemy->currentContext.compareRegister >= 0)
                goto handle_jump;
            break;
        case ECL_OPCODE_JUMP_NEQ:
            if (enemy->currentContext.compareRegister != 0)
                goto handle_jump;
            break;
        case ECL_OPCODE_CALL:
        handle_call:
            genericInt = GET_INSTR_INT_ARG(0);
            enemy->currentContext.currentInstr = (EclRawInstr *)((u8 *)curInstr + curInstr->offsetToNext);
            if (!enemy->flags.disableCallStack)
            {
                enemy->savedContextStack[enemy->stackDepth] = enemy->currentContext;
            }
            g_EclManager.CallEclSub(&enemy->currentContext, genericInt);
            if (!enemy->flags.disableCallStack && enemy->stackDepth < MAX_ECL_STACK_DEPTH)
            {
                enemy->stackDepth++;
            }
            enemy->currentContext.int0 = GET_INSTR_INT_ARG(1);
            enemy->currentContext.float0 = GET_INSTR_FLOAT_ARG(2);
            goto restart_sub_changed;
        case ECL_OPCODE_RET:
#if !TRIALBUILD
            if (enemy->flags.disableCallStack)
            {
                DebugPrint("error : no Stack Ret\n");
            }
#endif
            enemy->stackDepth--;
            enemy->currentContext = enemy->savedContextStack[enemy->stackDepth];
            goto restart_sub_changed;
        case ECL_OPCODE_CALL_LSS:
            genericInt = *EclGetVar(enemy, &GET_VAR_ARG(3), NULL);
            if (genericInt < GET_INT_ARG(4))
                goto handle_call;
            break;
        case ECL_OPCODE_CALL_LEQ:
            genericInt = *EclGetVar(enemy, &GET_VAR_ARG(3), NULL);
            if (genericInt <= GET_INT_ARG(4))
                goto handle_call;
            break;
        case ECL_OPCODE_CALL_EQU:
            genericInt = *EclGetVar(enemy, &GET_VAR_ARG(3), NULL);
            if (genericInt == GET_INT_ARG(4))
                goto handle_call;
            break;
        case ECL_OPCODE_CALL_GRE:
            genericInt = *EclGetVar(enemy, &GET_VAR_ARG(3), NULL);
            if (genericInt > GET_INT_ARG(4))
                goto handle_call;
            break;
        case ECL_OPCODE_CALL_GEQ:
            genericInt = *EclGetVar(enemy, &GET_VAR_ARG(3), NULL);
            if (genericInt >= GET_INT_ARG(4))
                goto handle_call;
            break;
        case ECL_OPCODE_CALL_NEQ:
            genericInt = *EclGetVar(enemy, &GET_VAR_ARG(3), NULL);
            if (genericInt != GET_INT_ARG(4))
                goto handle_call;
            break;
        case ECL_OPCODE_ANM_SET_MAIN:
            g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, GET_INSTR_INT_ARG(0) + ANM_SCRIPT_ENEMY_START);
            break;
        case ECL_OPCODE_ANM_SET_SLOT:
#if !TRIALBUILD
            if (GET_INSTR_INT_ARG(0) >= ENEMY_ANM_SLOTS)
            {
                DebugPrint("error : sub anim overflow\n");
            }
#endif
            g_AnmManager->SetAndExecuteScriptIdx(&enemy->vms[GET_INSTR_INT_ARG(0)],
                                                 GET_INT_ARG(1) + ANM_SCRIPT_ENEMY_START);
            break;
        case ECL_OPCODE_MOVE_POSITION:
            enemy->position = GET_INSTR_ARG(D3DXVECTOR3, 0);
            enemy->position.x = *EclGetVarFloat(enemy, &enemy->position.x, NULL);
            enemy->position.y = *EclGetVarFloat(enemy, &enemy->position.y, NULL);
            enemy->position.z = *EclGetVarFloat(enemy, &enemy->position.z, NULL);
            enemy->ClampPos();
            break;
        case ECL_OPCODE_MOVE_AXIS_SPEED:
            enemy->axisSpeed = GET_INSTR_ARG(D3DXVECTOR3, 0);
            enemy->axisSpeed.x = *EclGetVarFloat(enemy, &enemy->axisSpeed.x, NULL);
            enemy->axisSpeed.y = *EclGetVarFloat(enemy, &enemy->axisSpeed.y, NULL);
            enemy->axisSpeed.z = *EclGetVarFloat(enemy, &enemy->axisSpeed.z, NULL);
            enemy->flags.movementMode = EnemyMove_AxisSpeed;
            break;
        case ECL_OPCODE_MOVE_VELOCITY:
            // BUG: This instruction is encoded with 2 floats, not 3
            genericFloat3 = GET_INSTR_ARG(ZunVec3, 0);
            enemy->angle = *EclGetVarFloat(enemy, &genericFloat3.x, NULL);
            enemy->speed = *EclGetVarFloat(enemy, &genericFloat3.y, NULL);
            enemy->flags.movementMode = EnemyMove_Velocity;
            break;
        case ECL_OPCODE_MOVE_ANGULAR_VELOCITY:
            // BUG: This instruction is encoded with 1 float, not 3
            genericFloat3 = GET_INSTR_ARG(ZunVec3, 0);
            enemy->angularVelocity = *EclGetVarFloat(enemy, &genericFloat3.x, NULL);
            enemy->flags.movementMode = EnemyMove_Velocity;
            break;
        case ECL_OPCODE_MOVE_TOWARDS_PLAYER:
            // BUG: This instruction is encoded with 2 floats, not 3
            genericFloat3 = GET_INSTR_ARG(ZunVec3, 0);
            enemy->angle = g_Player.AngleToPlayer(&enemy->position) + genericFloat3.x;
            enemy->speed = *EclGetVarFloat(enemy, &genericFloat3.y, NULL);
            enemy->flags.movementMode = EnemyMove_Velocity;
            break;
        case ECL_OPCODE_MOVE_SPEED:
            // BUG: This instruction is encoded with 1 float, not 3
            genericFloat3 = GET_INSTR_ARG(ZunVec3, 0);
            enemy->speed = *EclGetVarFloat(enemy, &genericFloat3.x, NULL);
            enemy->flags.movementMode = EnemyMove_Velocity;
            break;
        case ECL_OPCODE_MOVE_ACCELERATION:
            // BUG: This instruction is encoded with 1 float, not 3
            genericFloat3 = GET_INSTR_ARG(ZunVec3, 0);
            enemy->acceleration = *EclGetVarFloat(enemy, &genericFloat3.x, NULL);
            enemy->flags.movementMode = EnemyMove_Velocity;
            break;
        case ECL_OPCODE_BULLET_FAN_AIMED:
        case ECL_OPCODE_BULLET_FAN:
        case ECL_OPCODE_BULLET_CIRCLE_AIMED:
        case ECL_OPCODE_BULLET_CIRCLE:
        case ECL_OPCODE_BULLET_OFFSET_CIRCLE_AIMED:
        case ECL_OPCODE_BULLET_OFFSET_CIRCLE:
        case ECL_OPCODE_BULLET_RANDOM_ANGLE:
        case ECL_OPCODE_BULLET_RANDOM_SPEED:
        case ECL_OPCODE_BULLET_RANDOM:
#pragma var_order(args, shooter)
        {
            EclRawInstrBulletArgs *args = &GET_INSTR_ARG(EclRawInstrBulletArgs, 0);
            EnemyShooter *shooter = &enemy->bulletProps;
            shooter->sprite = args->sprite;
            shooter->aimMode = curInstr->opCode - ECL_OPCODE_BULLET_FAN_AIMED;
            shooter->count1 = *EclGetVar(enemy, &args->count1, NULL);
            shooter->count1 += g_GameManager.RankLerpInt(enemy->bulletRankAmount1Low, enemy->bulletRankAmount1High);
            if (shooter->count1 <= 0)
            {
                shooter->count1 = 1;
            }
            shooter->count2 = *EclGetVar(enemy, &args->count2, NULL);
            shooter->count2 += g_GameManager.RankLerpInt(enemy->bulletRankAmount2Low, enemy->bulletRankAmount2High);
            if (shooter->count2 <= 0)
            {
                shooter->count2 = 1;
            }
            shooter->position = enemy->position + enemy->shootOffset;
            shooter->angle1 = *EclGetVarFloat(enemy, &args->angle1, NULL);
            shooter->angle1 = utils::AddNormalizeAngle(shooter->angle1, 0.0f);
            shooter->speed1 = *EclGetVarFloat(enemy, &args->speed1, NULL);
#if BUILT_SINCE(100)
            if (shooter->speed1 != 0.0f)
#endif
            {
                shooter->speed1 += g_GameManager.RankLerpFloat(enemy->bulletRankSpeedLow, enemy->bulletRankSpeedHigh);
                if (shooter->speed1 < 0.3f)
                {
                    shooter->speed1 = 0.3;
                }
            }
            shooter->angle2 = *EclGetVarFloat(enemy, &args->angle2, NULL);
            shooter->speed2 = *EclGetVarFloat(enemy, &args->speed2, NULL);
            shooter->speed2 +=
                g_GameManager.RankLerpFloat(enemy->bulletRankSpeedLow, enemy->bulletRankSpeedHigh) / 2.0f;
            if (shooter->speed2 < 0.3f)
            {
                shooter->speed2 = 0.3f;
            }
            shooter->unk_4a = 0;
            shooter->flags = args->flags;
            genericInt = args->color;
            shooter->color = *EclGetVar(enemy, (EclVarId *)&genericInt, NULL);
            if (!enemy->flags.shootingDisabled)
            {
                g_BulletManager.SpawnBulletPattern(shooter);
            }
            break;
        }
        case ECL_OPCODE_BULLET_EFFECTS:
            enemy->bulletProps.exInts[0] = *EclGetVar(enemy, &GET_VAR_ARG(0), NULL);
            enemy->bulletProps.exInts[1] = *EclGetVar(enemy, &GET_VAR_ARG(1), NULL);
            enemy->bulletProps.exInts[2] = *EclGetVar(enemy, &GET_VAR_ARG(2), NULL);
            enemy->bulletProps.exInts[3] = *EclGetVar(enemy, &GET_VAR_ARG(3), NULL);
            enemy->bulletProps.exFloats[0] = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(4), NULL);
            enemy->bulletProps.exFloats[1] = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(5), NULL);
            enemy->bulletProps.exFloats[2] = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(6), NULL);
            enemy->bulletProps.exFloats[3] = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(7), NULL);
            break;
        case ECL_OPCODE_ANM_DEATH_EFFECTS: {
            EclRawInstrAnmSetDeathArgs *args = &GET_INSTR_ARG(EclRawInstrAnmSetDeathArgs, 0);
            enemy->deathParticle1 = args->deathParticle1;
            enemy->deathParticle2 = args->deathParticle2;
            enemy->deathAnm3 = args->deathAnm3;
            break;
        }
        case ECL_OPCODE_SHOOT_INTERVAL:
            enemy->shootInterval = GET_INSTR_INT_ARG(0);
            enemy->shootInterval += g_GameManager.RankLerpInt(enemy->shootInterval / 5, -enemy->shootInterval / 5);
            enemy->shootIntervalTimer = 0;
            break;
        case ECL_OPCODE_SHOOT_INTERVAL_DELAYED:
            enemy->shootInterval = GET_INSTR_INT_ARG(0);
            enemy->shootInterval += g_GameManager.RankLerpInt(enemy->shootInterval / 5, -enemy->shootInterval / 5);
            if (enemy->shootInterval != 0)
            {
                enemy->shootIntervalTimer = g_Rng.GetRandomU32InRange(enemy->shootInterval);
            }
            break;
        case ECL_OPCODE_SHOOT_DISABLE:
            enemy->flags.shootingDisabled = true;
            break;
        case ECL_OPCODE_SHOOT_ENABLE:
            enemy->flags.shootingDisabled = false;
            break;
        case ECL_OPCODE_SHOOT_NOW:
            enemy->bulletProps.position = enemy->position + enemy->shootOffset;
            g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
            break;
        case ECL_OPCODE_SHOOT_OFFSET:
            enemy->shootOffset.x = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(0), NULL);
            enemy->shootOffset.y = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(1), NULL);
            enemy->shootOffset.z = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(2), NULL);
            break;
        case ECL_OPCODE_LASER_CREATE:
        case ECL_OPCODE_LASER_CREATE_AIMED:
#pragma var_order(shooter, args)
        {
            EclRawInstrLaserArgs *args = &GET_INSTR_ARG(EclRawInstrLaserArgs, 0);
            EnemyShooter *shooter = &enemy->laserProps;
            shooter->position = enemy->position + enemy->shootOffset;
            shooter->sprite = args->sprite;
            shooter->color = args->color;
            shooter->angle1 = *EclGetVarFloat(enemy, &args->angle, NULL);
            shooter->speed1 = *EclGetVarFloat(enemy, &args->speed, NULL);
            shooter->startOffset = *EclGetVarFloat(enemy, &args->startOffset, NULL);
            shooter->endOffset = *EclGetVarFloat(enemy, &args->endOffset, NULL);
            shooter->startLength = *EclGetVarFloat(enemy, &args->startLength, NULL);
            shooter->width = args->width;
            shooter->startTime = args->startTime;
            shooter->duration = args->duration;
            shooter->despawnDuration = args->despawnDuration;
            shooter->hitboxStartTime = args->hitboxStartTime;
            shooter->hitboxEndDelay = args->hitboxEndDelay;
            shooter->flags = args->flags;
            if (curInstr->opCode == ECL_OPCODE_LASER_CREATE_AIMED)
            {
                shooter->aimMode = LASER_AIMED;
            }
            else
            {
                shooter->aimMode = LASER_UNAIMED;
            }
            enemy->lasers[enemy->laserStore] = g_BulletManager.SpawnLaserPattern(shooter);
            break;
        }
        case ECL_OPCODE_LASER_INDEX:
            enemy->laserStore = *EclGetVar(enemy, &GET_INSTR_VAR_ARG(0), NULL);
            break;
        case ECL_OPCODE_LASER_ROTATE:
            if (enemy->lasers[GET_INSTR_INT_ARG(0)] != NULL)
            {
                enemy->lasers[GET_INSTR_INT_ARG(0)]->angle += *EclGetVarFloat(enemy, &GET_INSTR_FLOAT_ARG(1), NULL);
            }
            break;
        case ECL_OPCODE_LASER_ROTATE_FROM_PLAYER:
            if (enemy->lasers[GET_INSTR_INT_ARG(0)] != NULL)
            {
                enemy->lasers[GET_INSTR_INT_ARG(0)]->angle =
                    g_Player.AngleToPlayer(&enemy->lasers[GET_INSTR_INT_ARG(0)]->pos) +
                    *EclGetVarFloat(enemy, &GET_INSTR_FLOAT_ARG(1), NULL);
            }
            break;
#if BUILT_SINCE(100)
        case ECL_OPCODE_LASER_OFFSET:
            if (enemy->lasers[GET_INSTR_ARG(EclRawInstrLaserOffsetArgs, 0).laserId] != NULL)
            {
                enemy->lasers[GET_INSTR_ARG(EclRawInstrLaserOffsetArgs, 0).laserId]->pos =
                    enemy->position + GET_INSTR_ARG(EclRawInstrLaserOffsetArgs, 0).offset;
            }
            break;
        case ECL_OPCODE_LASER_TEST:
            if (enemy->lasers[GET_INSTR_INT_ARG(0)] != NULL && enemy->lasers[GET_INSTR_INT_ARG(0)]->inUse)
            {
                enemy->currentContext.compareRegister = 0;
            }
            else
            {
                enemy->currentContext.compareRegister = 1;
            }
            break;
#endif
        case ECL_OPCODE_LASER_CANCEL:
            if (enemy->lasers[GET_INSTR_INT_ARG(0)] != NULL && enemy->lasers[GET_INSTR_INT_ARG(0)]->inUse &&
                enemy->lasers[GET_INSTR_INT_ARG(0)]->state < LASER_STATE_DESPAWNING)
            {
                enemy->lasers[GET_INSTR_INT_ARG(0)]->state = LASER_STATE_DESPAWNING;
                enemy->lasers[GET_INSTR_INT_ARG(0)]->timer = 0;
            }
            break;
        case ECL_OPCODE_LASER_CLEAR_ALL: {
            for (i32 idx = 0; idx < MAX_LASERS_PER_ENEMY; idx++)
            {
                enemy->lasers[idx] = NULL;
            }
            break;
        }
        case ECL_OPCODE_BOSS_SET:
            if (GET_INSTR_INT_ARG(0) >= 0)
            {
                g_EnemyManager.bosses[GET_INSTR_INT_ARG(0)] = enemy;
                g_Gui.SetBossPresent(true);
                g_Gui.bossHealthBar1 = 1.0f;
                enemy->flags.isBoss = true;
                enemy->bossId = GET_INSTR_INT_ARG(0);
            }
            else
            {
                g_Gui.SetBossPresent(false);
                g_EnemyManager.bosses[enemy->bossId] = NULL;
                enemy->flags.isBoss = false;
            }
            break;
        case ECL_OPCODE_SPELLCARD_EFFECT: {
            EclRawInstrSpellcardEffectArgs *args = &GET_INSTR_ARG(EclRawInstrSpellcardEffectArgs, 0);
            enemy->effectArray[enemy->effectIdx] = g_EffectManager.SpawnParticles(
                PARTICLE_EFFECT_UNK_13, &enemy->position, 1, g_EffectsColor[args->effectColorId]);
            enemy->effectArray[enemy->effectIdx]->pos2 = args->pos;
            enemy->effectDistance = args->effectDistance;
            enemy->effectIdx++;
            break;
        }
        case ECL_OPCODE_MOVE_VELOCITY_INTERP_DECELERATE:
            EclMoveDirTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Decelerate;
            break;
        case ECL_OPCODE_MOVE_VELOCITY_INTERP_DECELERATE_FAST:
            EclMoveDirTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_DecelerateFast;
            break;
        case ECL_OPCODE_MOVE_VELOCITY_INTERP_ACCELERATE:
            EclMoveDirTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Accelerate;
            break;
        case ECL_OPCODE_MOVE_VELOCITY_INTERP_ACCELERATE_FAST:
            EclMoveDirTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_AccelerateFast;
            break;
        case ECL_OPCODE_MOVE_POSITION_INTERP_LINEAR:
            EclMovePosTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Linear;
            break;
        case ECL_OPCODE_MOVE_POSITION_INTERP_DECELERATE:
            EclMovePosTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Decelerate;
            break;
        case ECL_OPCODE_MOVE_POSITION_INTERP_DECELERATE_FAST:
            EclMovePosTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_DecelerateFast;
            break;
        case ECL_OPCODE_MOVE_POSITION_INTERP_ACCELERATE:
            EclMovePosTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Accelerate;
            break;
        case ECL_OPCODE_MOVE_POSITION_INTERP_ACCELERATE_FAST:
            EclMovePosTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_AccelerateFast;
            break;
        case ECL_OPCODE_MOVE_AS_INTERP_DECELERATE:
            EclMoveTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Decelerate;
            break;
        case ECL_OPCODE_MOVE_AS_INTERP_DECELERATE_FAST:
            EclMoveTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_DecelerateFast;
            break;
        case ECL_OPCODE_MOVE_AS_INTERP_ACCELERATE:
            EclMoveTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Accelerate;
            break;
        case ECL_OPCODE_MOVE_AS_INTERP_ACCELERATE_FAST:
            EclMoveTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_AccelerateFast;
            break;
        case ECL_OPCODE_MOVE_BOUNDS_SET:
            enemy->lowerMoveLimit.x = GET_INSTR_FLOAT_ARG(0);
            enemy->lowerMoveLimit.y = GET_INSTR_FLOAT_ARG(1);
            enemy->upperMoveLimit.x = GET_INSTR_FLOAT_ARG(2);
            enemy->upperMoveLimit.y = GET_INSTR_FLOAT_ARG(3);
            enemy->flags.shouldClampPos = true;
            break;
        case ECL_OPCODE_MOVE_BOUNDS_DISABLE:
            enemy->flags.shouldClampPos = false;
            break;
        case ECL_OPCODE_MOVE_RAND:
            genericFloat3 = GET_INSTR_ARG(ZunVec3, 0);
            enemy->angle = g_Rng.GetRandomF32InRange(genericFloat3.y - genericFloat3.x) + genericFloat3.x;
            break;
        case ECL_OPCODE_MOVE_RAND_IN_BOUNDS:
            genericFloat3 = GET_INSTR_ARG(ZunVec3, 0);
            enemy->angle = g_Rng.GetRandomF32InRange(genericFloat3.y - genericFloat3.x) + genericFloat3.x;
            if (enemy->position.x < enemy->lowerMoveLimit.x + 96.0f)
            {
                if (enemy->angle > ZUN_HALF_PI)
                {
                    enemy->angle = ZUN_PI - enemy->angle;
                }
                else if (enemy->angle < -ZUN_HALF_PI)
                {
                    enemy->angle = -ZUN_PI - enemy->angle;
                }
            }
            if (enemy->position.x > enemy->upperMoveLimit.x - 96.0f)
            {
                if (enemy->angle < ZUN_HALF_PI && enemy->angle >= 0.0f)
                {
                    enemy->angle = ZUN_PI - enemy->angle;
                }
                else if (enemy->angle > -ZUN_HALF_PI && enemy->angle <= 0.0f)
                {
                    enemy->angle = -ZUN_PI - enemy->angle;
                }
            }
            if (enemy->position.y < enemy->lowerMoveLimit.y + 48.0f && enemy->angle < 0.0f)
            {
                enemy->angle = -enemy->angle;
            }
            if (enemy->position.y > enemy->upperMoveLimit.y - 48.0f && enemy->angle > 0.0f)
            {
                enemy->angle = -enemy->angle;
            }
            break;
        case ECL_OPCODE_ANM_SET_POSES:
            enemy->anmPoseDefault = GET_INSTR_SHORT_ARG(0);
            enemy->anmPoseNeutralFromLeft = GET_INSTR_SHORT_ARG(1);
            enemy->anmPoseNeutralFromRight = GET_INSTR_SHORT_ARG(2);
            enemy->anmPoseLeft = GET_INSTR_SHORT_ARG(3);
            enemy->anmPoseRight = GET_INSTR_SHORT_ARG(4);
            enemy->anmPoseCurrent = EnemyPose_Default;
            break;
        case ECL_OPCODE_ENEMY_SET_HITBOX:
            enemy->hitboxDimensions.x = GET_INSTR_FLOAT_ARG(0);
            enemy->hitboxDimensions.y = GET_INSTR_FLOAT_ARG(1);
            enemy->hitboxDimensions.z = GET_INSTR_FLOAT_ARG(2);
            break;
        case ECL_OPCODE_ENEMY_FLAG_COLLISION:
            enemy->flags.isCollidable = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_ENEMY_FLAG_CAN_TAKE_DAMAGE:
            enemy->flags.isDamageable = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_EFFECT_SOUND:
            g_SoundPlayer.PlaySoundByIdx(GET_INSTR_ARG(SoundIdx, 0));
            break;
        case ECL_OPCODE_ENEMY_FLAGS_DEATH:
            enemy->flags.deathMode = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_DEATH_CALLBACK_SUB:
            enemy->deathCallbackSub = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_ENEMY_INTERRUPT_SET:
            enemy->interrupts[GET_INT_ARG(1)] = GET_INT_ARG(0);
            break;
        case ECL_OPCODE_ENEMY_INTERRUPT:
            enemy->runInterrupt = GET_INSTR_INT_ARG(0);
        run_interrupt:
            enemy->currentContext.currentInstr = (EclRawInstr *)((u8 *)curInstr + curInstr->offsetToNext);
            if (!enemy->flags.disableCallStack)
            {
                enemy->savedContextStack[enemy->stackDepth] = enemy->currentContext;
            }
            g_EclManager.CallEclSub(&enemy->currentContext, enemy->interrupts[enemy->runInterrupt]);
            if (enemy->stackDepth < MAX_ECL_STACK_DEPTH)
            {
                enemy->stackDepth++;
            }
            enemy->runInterrupt = -1;
            goto restart_sub_changed;
        case ECL_OPCODE_ENEMY_LIFE_SET:
            enemy->life = enemy->maxLife = GET_INSTR_INT_ARG(0);
            break;
#pragma var_order(catk, length, csum)
        case ECL_OPCODE_SPELLCARD_START: {
            g_Gui.ShowSpellcard(GET_INSTR_SHORT_ARG(0), GET_INSTR_STRING_ARG(4));
            g_EnemyManager.spellcardInfo.isCapturing = true;
            g_EnemyManager.spellcardInfo.isActive = 1;
            g_EnemyManager.spellcardInfo.idx = GET_INSTR_SHORT_ARG(1);
            g_EnemyManager.spellcardInfo.captureScore = g_SpellcardScore[g_EnemyManager.spellcardInfo.idx];
            g_BulletManager.TurnAllBulletsIntoPoints();
            g_Stage.spellcardState = RUNNING;
            g_Stage.ticksSinceSpellcardStarted = 0;
            enemy->ResetRank();
            Catk *catk = &g_GameManager.catk[g_EnemyManager.spellcardInfo.idx];
            i32 csum = 0;
            i32 length;
            if (!g_GameManager.isInReplay)
            {
                strcpy(catk->name, GET_INSTR_STRING_ARG(4));
                length = strlen(catk->name);
                while (length > 0)
                {
                    csum += catk->name[--length];
                }
                if (catk->nameCsum != (u8)csum)
                {
                    catk->numSuccess = 0;
                    catk->numAttempts = 0;
                    catk->nameCsum = csum;
                }
                catk->captureScore = g_EnemyManager.spellcardInfo.captureScore;
                if (catk->numAttempts < 9999)
                {
                    catk->numAttempts++;
                }
            }
            break;
        }
        case ECL_OPCODE_SPELLCARD_END:
            if (g_EnemyManager.spellcardInfo.isActive)
            {
                g_Gui.EndEnemySpellcard();
                if (g_EnemyManager.spellcardInfo.isActive == 1)
                {
                    i32 scoreIncrease = g_BulletManager.DespawnBullets(12800, true);
#pragma var_order(catk, idx, score)
                    if (g_EnemyManager.spellcardInfo.isCapturing)
                    {
                        i32 idx;
                        Catk *catk = &g_GameManager.catk[g_EnemyManager.spellcardInfo.idx];
                        i32 score = g_EnemyManager.spellcardInfo.captureScore >= 500000
                                        ? 500000 / 10
                                        : g_EnemyManager.spellcardInfo.captureScore / 10;
                        scoreIncrease =
                            g_EnemyManager.spellcardInfo.captureScore +
                            g_EnemyManager.spellcardInfo.captureScore * g_Gui.SpellcardSecondsRemaining() / 10;
                        g_Gui.ShowSpellcardBonus(scoreIncrease);
                        g_GameManager.AddScore(scoreIncrease);
                        if (!g_GameManager.isInReplay)
                        {
                            catk->numSuccess++;
                            for (idx = 4; idx > 0; idx--)
                            {
                                catk->characterShotType[idx] = catk->characterShotType[idx - 1];
                            }
                            catk->characterShotType[0] = CURRENT_SHOTTYPE();
                        }
                        g_GameManager.spellcardsCaptured++;
                    }
                }
                g_EnemyManager.spellcardInfo.isActive = 0;
            }
            g_Stage.spellcardState = NOT_RUNNING;
            break;
        case ECL_OPCODE_PHASE_TIMER_SET:
            enemy->phaseTimer = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_LIFE_CALLBACK_THRESHOLD:
            enemy->lifeCallbackThreshold = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_LIFE_CALLBACK_SUB:
            enemy->lifeCallbackSub = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_TIMER_CALLBACK_THRESHOLD:
            enemy->timerCallbackThreshold = GET_INSTR_INT_ARG(0);
            enemy->phaseTimer = 0;
            break;
        case ECL_OPCODE_TIMER_CALLBACK_SUB:
            enemy->timerCallbackSub = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_ENEMY_FLAG_INTERACTABLE:
            enemy->flags.isInteractable = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_EFFECT_PARTICLE:
            g_EffectManager.SpawnParticles(GET_INSTR_INT_ARG(0), &enemy->position, GET_INSTR_INT_ARG(1),
                                           GET_INSTR_ARG(ZunColor, 2));
            break;
        case ECL_OPCODE_DROP_ITEMS: {
            for (i32 idx = 0; idx < GET_INSTR_INT_ARG(0); idx++)
            {
                D3DXVECTOR3 pos = enemy->position;

                pos[0] += g_Rng.GetRandomF32InRange(144.0f) - 72.0f;
                pos[1] += g_Rng.GetRandomF32InRange(144.0f) - 72.0f;
                if (g_GameManager.currentPower < MAX_POWER)
                {
                    g_ItemManager.SpawnItem(&pos, idx == 0 ? ITEM_POWER_BIG : ITEM_POWER_SMALL, ITEM_STATE_FALLING);
                }
                else
                {
                    g_ItemManager.SpawnItem(&pos, ITEM_POINT, ITEM_STATE_FALLING);
                }
            }
            break;
        }
        case ECL_OPCODE_ANM_FLAG_ROTATION:
            enemy->flags.rotateAnm = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_EX_INS_CALL:
            g_EclExInsn[GET_INSTR_INT_ARG(0)](enemy, curInstr);
            break;
        case ECL_OPCODE_EX_INS_REPEAT:
            if (GET_INSTR_INT_ARG(0) >= 0)
            {
                enemy->currentContext.funcSetFunc = g_EclExInsn[GET_INSTR_INT_ARG(0)];
            }
            else
            {
                enemy->currentContext.funcSetFunc = NULL;
            }
            break;
        case ECL_OPCODE_ECL_TIME_ADD:
            enemy->currentContext.time += *EclGetVar(enemy, &GET_INSTR_VAR_ARG(0), NULL);
            break;
        case ECL_OPCODE_DROP_ITEM_ID:
            g_ItemManager.SpawnItem(&enemy->position, GET_INSTR_ARG(ItemType, 0), ITEM_STATE_FALLING);
            break;
        case ECL_OPCODE_STD_UNPAUSE:
            g_Stage.unpauseFlag = 1;
            break;
        case ECL_OPCODE_BOSS_SET_LIFE_COUNT:
            g_Gui.SetLives(GET_INSTR_INT_ARG(0));
            g_GameManager.counat += 1800;
            break;
        case ECL_OPCODE_ENEMY_CREATE: {
            EclRawInstrEnemyCreateArgs args = GET_INSTR_ARG(EclRawInstrEnemyCreateArgs, 0);
            args.pos.x = *EclGetVarFloat(enemy, &args.pos.x, NULL);
            args.pos.y = *EclGetVarFloat(enemy, &args.pos.y, NULL);
            args.pos.z = *EclGetVarFloat(enemy, &args.pos.z, NULL);
            g_EnemyManager.SpawnEnemy(args.subId, &args.pos, args.life, args.itemDrop, args.score);
            break;
        }
        case ECL_OPCODE_ENEMY_KILL_ALL: {
            Enemy *currentEnemy = &g_EnemyManager.enemies[0];
            for (i32 i = 0; i < MAX_ENEMY_COUNT; i++, currentEnemy++)
            {
                if (!currentEnemy->flags.isSlotOccupied)
                {
                    continue;
                }
                if (currentEnemy->flags.isBoss)
                {
                    continue;
                }

                currentEnemy->life = 0;
                if (!currentEnemy->flags.isInteractable && currentEnemy->deathCallbackSub >= 0)
                {
                    g_EclManager.CallEclSub(&currentEnemy->currentContext, currentEnemy->deathCallbackSub);
                    currentEnemy->deathCallbackSub = -1;
                }
            }
            break;
        }
        case ECL_OPCODE_ANM_INTERRUPT_MAIN:
            enemy->primaryVm.pendingInterrupt = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_ANM_INTERRUPT_SLOT:
            enemy->vms[GET_INT_ARG(0)].pendingInterrupt = GET_INT_ARG(1);
            break;
        case ECL_OPCODE_BULLET_CANCEL:
            g_BulletManager.TurnAllBulletsIntoPoints();
            break;
        case ECL_OPCODE_BULLET_SOUND:
            if (GET_INSTR_ARG(SoundIdx, 0) >= 0)
            {
                enemy->bulletProps.sfx = GET_INSTR_ARG(SoundIdx, 0);
                enemy->bulletProps.flags |= EX_SPAWN_SOUND;
            }
            else
            {
                enemy->bulletProps.flags &= ~EX_SPAWN_SOUND;
            }
            break;
        case ECL_OPCODE_ENEMY_FLAG_DISABLE_CALLSTACK:
            enemy->flags.disableCallStack = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_BULLET_RANK_INFLUENCE:
            enemy->bulletRankSpeedLow = GET_FLOAT_ARG(0);
            enemy->bulletRankSpeedHigh = GET_FLOAT_ARG(1);
            enemy->bulletRankAmount1Low = GET_INT_ARG(2);
            enemy->bulletRankAmount1High = GET_INT_ARG(3);
            enemy->bulletRankAmount2Low = GET_INT_ARG(4);
            enemy->bulletRankAmount2High = GET_INT_ARG(5);
            break;
        case ECL_OPCODE_ENEMY_FLAG_INVISIBLE:
            enemy->flags.isInvisible = GET_INSTR_INT_ARG(0);
            break;
        case ECL_OPCODE_PHASE_ADVANCE:
            enemy->timerCallbackSub = enemy->deathCallbackSub;
            enemy->phaseTimer = 0;
            break;
#if BUILT_SINCE(100)
        case ECL_OPCODE_SPELLCARD_FLAG_TIMEOUT:
            enemy->flags.isTimeoutSpell = GET_INSTR_INT_ARG(0);
            break;
#endif
        }
    next_instruction:
        curInstr = (EclRawInstr *)((u8 *)curInstr + curInstr->offsetToNext);
    }
    switch (enemy->flags.movementMode)
    {
    case EnemyMove_Velocity:
        enemy->angle =
            utils::AddNormalizeAngle(enemy->angle, g_Supervisor.effectiveFramerateMultiplier * enemy->angularVelocity);
        enemy->speed = g_Supervisor.effectiveFramerateMultiplier * enemy->acceleration + enemy->speed;
        sincosmul(&enemy->axisSpeed, enemy->angle, enemy->speed);
        enemy->axisSpeed[2] = 0.0f;
        break;
    case EnemyMove_Interp: {
        enemy->moveInterpTimer--;
        f32 interpVal = (f32)enemy->moveInterpTimer / enemy->moveInterpStartTime;
        if (interpVal >= 1.0f)
        {
            interpVal = 1.0f;
        }
        switch (enemy->flags.moveInterpMode)
        {
        case EnemyInterp_Linear:
            interpVal = 1.0f - interpVal;
            break;
        case EnemyInterp_Decelerate:
            interpVal = 1.0f - interpVal * interpVal;
            break;
        case EnemyInterp_DecelerateFast:
            interpVal = 1.0f - interpVal * interpVal * interpVal * interpVal;
            break;
        case EnemyInterp_Accelerate:
            interpVal = 1.0f - interpVal;
            interpVal *= interpVal;
            break;
        case EnemyInterp_AccelerateFast:
            interpVal = 1.0f - interpVal;
            interpVal = interpVal * interpVal * interpVal * interpVal;
        }
        enemy->axisSpeed = interpVal * enemy->moveInterp + enemy->moveInterpStartPos - enemy->position;
        enemy->angle = atan2f(enemy->axisSpeed.y, enemy->axisSpeed.x);
        if (enemy->moveInterpTimer <= 0)
        {
            enemy->flags.movementMode = EnemyMove_AxisSpeed;
            enemy->position = enemy->moveInterpStartPos + enemy->moveInterp;
            enemy->axisSpeed = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        }
        break;
    }
    }
    if (enemy->life > 0)
    {
        if (enemy->shootInterval > 0)
        {
            enemy->shootIntervalTimer++;
            if (enemy->shootIntervalTimer >= enemy->shootInterval)
            {
                enemy->bulletProps.position = enemy->position + enemy->shootOffset;
                g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
                enemy->shootIntervalTimer = 0;
            }
        }
        if (enemy->anmPoseLeft >= 0) // Check if poses are enabled
        {
            EnemyPose newPose = EnemyPose_Neutral;
            if (enemy->axisSpeed.x < 0.0f)
            {
                newPose = EnemyPose_Left;
            }
            else if (enemy->axisSpeed.x > 0.0f)
            {
                newPose = EnemyPose_Right;
            }
            if (enemy->anmPoseCurrent != newPose)
            {
                switch (newPose)
                {
                case EnemyPose_Neutral:
                    if (enemy->anmPoseCurrent == EnemyPose_Default)
                    {
                        g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                             enemy->anmPoseDefault + ANM_OFFSET_ENEMY);
                    }
                    else if (enemy->anmPoseCurrent == EnemyPose_Left)
                    {
                        g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                             enemy->anmPoseNeutralFromLeft + ANM_OFFSET_ENEMY);
                    }
                    else // EnemyPose_Right
                    {
                        g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                             enemy->anmPoseNeutralFromRight + ANM_OFFSET_ENEMY);
                    }
                    break;
                case EnemyPose_Left:
                    g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, enemy->anmPoseLeft + ANM_OFFSET_ENEMY);
                    break;
                case EnemyPose_Right:
                    g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, enemy->anmPoseRight + ANM_OFFSET_ENEMY);
                    break;
                }
                enemy->anmPoseCurrent = newPose;
            }
        }
        if (enemy->currentContext.funcSetFunc != NULL)
        {
            enemy->currentContext.funcSetFunc(enemy, NULL);
        }
    }
    enemy->currentContext.currentInstr = curInstr;
    enemy->currentContext.time++;
    return ZUN_SUCCESS;
}

// TODO: 0.08 codegen diffs
#pragma var_order(args, angle)
static void EclMoveDirTime(Enemy *enemy, EclRawInstr *instr)
{
    void *args = instr->args;

#if BUILT_SINCE(100)
    f32 angle = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(1), NULL);
#else
    f32 angle = GET_FLOAT_ARG(1);
#endif

    enemy->moveInterp.x = cosf(angle) * GET_FLOAT_ARG(2) * GET_INT_ARG(0) / 2.0f;
    enemy->moveInterp.y = sinf(angle) * GET_FLOAT_ARG(2) * GET_INT_ARG(0) / 2.0f;
    enemy->moveInterp.z = 0.0f;

    enemy->moveInterpStartPos = enemy->position;
    enemy->moveInterpStartTime = GET_INT_ARG(0);

    enemy->moveInterpTimer = enemy->moveInterpStartTime;

    enemy->flags.movementMode = EnemyMove_Interp;
}

// TODO: 0.08 codegen diffs
static void EclMovePosTime(Enemy *enemy, EclRawInstr *instr)
{
    void *args = instr->args;

    D3DXVECTOR3 newPos;

#if BUILT_SINCE(100)
    newPos.x = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(1), NULL);
    newPos.y = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(2), NULL);
    newPos.z = *EclGetVarFloat(enemy, &GET_FLOAT_ARG(3), NULL);
    enemy->moveInterp = newPos - enemy->position;
#else
    newPos.z = GET_FLOAT_ARG(3) - enemy->position.z;
    newPos.y = GET_FLOAT_ARG(2) - enemy->position.y;
    newPos.x = GET_FLOAT_ARG(1) - enemy->position.x;
    enemy->moveInterp = newPos;
#endif

    enemy->moveInterpStartPos = enemy->position;
    enemy->moveInterpStartTime = GET_INT_ARG(0);

    enemy->moveInterpTimer = enemy->moveInterpStartTime;

    enemy->flags.movementMode = EnemyMove_Interp;
    enemy->axisSpeed = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

// TODO: 0.08 codegen diffs
static void EclMoveTime(Enemy *enemy, EclRawInstr *instr)
{
    void *args = instr->args;

#if BUILT_SINCE(100)
    f32 angle = *EclGetVarFloat(enemy, &enemy->angle, NULL);
#else
    f32 angle = enemy->angle;
#endif

    enemy->moveInterp.x = cosf(angle) * enemy->speed * GET_INT_ARG(0) / 2.0f;
    enemy->moveInterp.y = sinf(angle) * enemy->speed * GET_INT_ARG(0) / 2.0f;
    enemy->moveInterp.z = 0.0f;

    enemy->moveInterpStartPos = enemy->position;
    enemy->moveInterpStartTime = GET_INT_ARG(0);

    enemy->moveInterpTimer = enemy->moveInterpStartTime;

    enemy->flags.movementMode = EnemyMove_Interp;
}

static i32 *EclGetVar(Enemy *enemy, EclVarId *eclVarId, EclValueType *valueType)
{

    if (valueType != NULL)
        *valueType = ECL_VALUE_TYPE_UNDEFINED;

    switch (*eclVarId)
    {
    case ECL_VAR_I0:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.int0;

    case ECL_VAR_I1:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.int1;

    case ECL_VAR_I2:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.int2;

    case ECL_VAR_I3:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.int3;

    case ECL_VAR_F0:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->currentContext.float0;

    case ECL_VAR_F1:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->currentContext.float1;

    case ECL_VAR_F2:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->currentContext.float2;

    case ECL_VAR_F3:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->currentContext.float3;

    case ECL_VAR_IC0:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.counter0;

    case ECL_VAR_IC1:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.counter1;

    case ECL_VAR_IC2:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.counter2;

    case ECL_VAR_IC3:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.counter3;

    case ECL_VAR_DIFFICULTY:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return (i32 *)&g_GameManager.difficulty;

    case ECL_VAR_RANK:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return &g_GameManager.rank;

    case ECL_VAR_SELF_POS_X:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->position.x;

    case ECL_VAR_SELF_POS_Y:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->position.y;

    case ECL_VAR_SELF_POS_Z:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->position.z;

    case ECL_VAR_PLAYER_POS_X:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return (i32 *)&g_Player.positionCenter.x;

    case ECL_VAR_PLAYER_POS_Y:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return (i32 *)&g_Player.positionCenter.y;

    case ECL_VAR_PLAYER_POS_Z:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return (i32 *)&g_Player.positionCenter.z;

    case ECL_VAR_PLAYER_ANGLE: {
        static f32 g_PlayerAngle;
        g_PlayerAngle = g_Player.AngleToPlayer(&enemy->position);
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return (i32 *)&g_PlayerAngle;
    }

    case ECL_VAR_ENEMY_TIMER:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->phaseTimer.current;

    case ECL_VAR_PLAYER_DISTANCE: {
        static f32 g_PlayerDistance;
        D3DXVECTOR3 distance = g_Player.positionCenter - enemy->position;
        g_PlayerDistance = D3DXVec3Length(&distance);
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return (i32 *)&g_PlayerDistance;
    }

    case ECL_VAR_ENEMY_LIFE:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->life;

    case ECL_VAR_PLAYER_SHOT: {
        static i32 g_PlayerShot;
        g_PlayerShot = CURRENT_SHOTTYPE();
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &g_PlayerShot;
    }
    }
    return (i32 *)eclVarId;
}

static f32 *EclGetVarFloat(Enemy *enemy, f32 *eclVarId, EclValueType *valueType)
{
    i32 varId = *eclVarId;
    i32 *res = EclGetVar(enemy, (EclVarId *)&varId, valueType);
    if (res == &varId)
    {
        return eclVarId;
    }
    else
    {
        return (f32 *)res;
    }
}

#pragma var_order(lhsPtr, rhsPtr, lhsType)
static void EclSetVar(Enemy *enemy, EclVarId out, void *value)
{
    i32 *lhsPtr;
    EclValueType lhsType;
    i32 *rhsPtr;

    rhsPtr = EclGetVar(enemy, (EclVarId *)value, NULL);
    lhsPtr = EclGetVar(enemy, &out, &lhsType);
    if (lhsType == ECL_VALUE_TYPE_INT)
    {
        *lhsPtr = *rhsPtr;
    }
    else if (lhsType == ECL_VALUE_TYPE_FLOAT)
    {
        *(f32 *)lhsPtr = *(f32 *)rhsPtr;
    }
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
static void EclMathAdd(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
{
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    // Get output variable.
    outPtr = EclGetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT)
    {
        lhsPtr = EclGetVar(enemy, lhsVarId, NULL);
        rhsPtr = EclGetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr + *rhsPtr;
    }
    else if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        lhsPtr = (i32 *)EclGetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)EclGetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr + *(f32 *)rhsPtr;
    }
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
static void EclMathSub(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
{
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    outPtr = EclGetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT)
    {
        lhsPtr = EclGetVar(enemy, lhsVarId, NULL);
        rhsPtr = EclGetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr - *rhsPtr;
    }
    else if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        lhsPtr = (i32 *)EclGetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)EclGetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr - *(f32 *)rhsPtr;
    }
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
static void EclMathMul(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
{
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    lhsPtr = EclGetVar(enemy, lhsVarId, NULL);
    rhsPtr = EclGetVar(enemy, rhsVarId, NULL);
    outPtr = EclGetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT)
    {
        lhsPtr = EclGetVar(enemy, lhsVarId, NULL);
        rhsPtr = EclGetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr * *rhsPtr;
    }
    else if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        lhsPtr = (i32 *)EclGetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)EclGetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr * *(f32 *)rhsPtr;
    }
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
static void EclMathDiv(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
{
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    outPtr = EclGetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT)
    {
        lhsPtr = EclGetVar(enemy, lhsVarId, NULL);
        rhsPtr = EclGetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr / *rhsPtr;
    }
    else if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        lhsPtr = (i32 *)EclGetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)EclGetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr / *(f32 *)rhsPtr;
    }
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
static void EclMathMod(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
{
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    outPtr = EclGetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT)
    {
        lhsPtr = EclGetVar(enemy, lhsVarId, NULL);
        rhsPtr = EclGetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr % *rhsPtr;
    }
    else if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        lhsPtr = (i32 *)EclGetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)EclGetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = fmodf(*(f32 *)lhsPtr, *(f32 *)rhsPtr);
    }
}

#if BUILT_SINCE(100)
#pragma var_order(y2Ptr, outPtr, x1Ptr, y1Ptr, outType, x2Ptr)
static void EclMathLineAngle(Enemy *enemy, EclVarId outVarId, f32 *x1, f32 *y1, f32 *y2, f32 *x2)
{
    EclValueType outType;
    f32 *outPtr;
    f32 *y1Ptr, *x1Ptr, *x2Ptr, *y2Ptr;

    outPtr = (f32 *)EclGetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        y1Ptr = EclGetVarFloat(enemy, x1, NULL);
        x1Ptr = EclGetVarFloat(enemy, y1, NULL);
        y2Ptr = EclGetVarFloat(enemy, y2, NULL);
        x2Ptr = EclGetVarFloat(enemy, x2, NULL);
        *outPtr = atan2f(*x2Ptr - *x1Ptr, *y2Ptr - *y1Ptr);
    }
}
#endif
} // namespace th06
