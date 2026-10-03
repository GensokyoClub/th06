#include "EclManager.hpp"
#include "AnmManager.hpp"
#include "EffectManager.hpp"
#include "Enemy.hpp"
#include "EnemyEclInstr.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "Stage.hpp"

namespace th06
{
namespace EnemyEclInstr
{
static i32 *GetVar(Enemy *enemy, EclVarId *varId, EclValueType *valueType);
static f32 *GetVarFloat(Enemy *enemy, f32 *varId, EclValueType *valueType);
static void SetVar(Enemy *enemy, EclVarId out, void *value);

static void MathAdd(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
static void MathSub(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
static void MathMul(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
static void MathDiv(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
static void MathMod(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
static void MathAtan2(Enemy *enemy, EclVarId out, f32 *a1, f32 *a2, f32 *b1, f32 *b2);

static void MoveDirTime(Enemy *enemy, EclRawInstr *instr);
static void MovePosTime(Enemy *enemy, EclRawInstr *instr);
static void MoveTime(Enemy *enemy, EclRawInstr *instr);

} // namespace EnemyEclInstr

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 64, g_SpellcardScore) = {
    200000, 200000, 200000, 200000, 200000, 200000, 200000, 250000, 250000, 250000, 250000, 250000, 250000,
    250000, 300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000,
    300000, 300000, 300000, 300000, 300000, 300000, 400000, 400000, 400000, 400000, 400000, 400000, 400000,
    400000, 500000, 500000, 500000, 500000, 500000, 500000, 600000, 600000, 600000, 600000, 600000, 700000,
    700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000};
typedef void (*ExInsn)(Enemy *, EclRawInstr *);
DIFFABLE_STATIC_ARRAY_ASSIGN(ExInsn, 17, g_EclExInsn) = {EnemyEclInstr::ExInsCirnoRainbowBallJank,
                                                         EnemyEclInstr::ExInsShootAtRandomArea,
                                                         EnemyEclInstr::ExInsShootStarPattern,
                                                         EnemyEclInstr::ExInsPatchouliShottypeSetVars,
                                                         EnemyEclInstr::ExInsStage56Func4,
                                                         EnemyEclInstr::ExInsStage5Func5,
                                                         EnemyEclInstr::ExInsBatWingEffect,
                                                         EnemyEclInstr::ExInsStage6Func7,
                                                         EnemyEclInstr::ExInsStage6Func8,
                                                         EnemyEclInstr::ExInsStage6Func9,
                                                         EnemyEclInstr::ExInsHandleBatTransformation,
                                                         EnemyEclInstr::ExInsStage6Func11,
                                                         EnemyEclInstr::ExInsStage4Func12,
                                                         EnemyEclInstr::ExInsStageXFunc13,
                                                         EnemyEclInstr::ExInsStageXFunc14,
                                                         EnemyEclInstr::ExInsStageXFunc15,
                                                         EnemyEclInstr::ExInsFlandreFinalContextUpdate};

DIFFABLE_STATIC_SORTED(C5, ChainElem, g_EclManagerCalcChain); // unused
DIFFABLE_STATIC_SORTED(C4, EclManager, g_EclManager);
DIFFABLE_STATIC_SORTED(C1, i32, g_PlayerShot);
DIFFABLE_STATIC_SORTED(C2, f32, g_PlayerDistance);
DIFFABLE_STATIC_SORTED(C3, f32, g_PlayerAngle);

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

#pragma var_order(genericFloat3, genericInt, genericFloat, args, curInstr)
ZunResult EclManager::RunEcl(Enemy *enemy)
{
    EclRawInstr *curInstr;
    EclRawInstrArgs *args;
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

        args = &curInstr->args;
        switch (curInstr->opCode)
        {
        case ECL_OPCODE_ENEMY_DELETE:
            return ZUN_ERROR;
        case ECL_OPCODE_LOOP:
            genericInt = *EnemyEclInstr::GetVar(enemy, &args->jump.var, NULL);
            genericInt--;
            EnemyEclInstr::SetVar(enemy, args->jump.var, &genericInt);
            if (genericInt <= 0)
                break;
            // fallthrough
        case ECL_OPCODE_JUMP:
        handle_jump:
            enemy->currentContext.time.current = curInstr->args.jump.time;
            curInstr = (EclRawInstr *)((u32)curInstr + args->jump.offset);
            continue;
        case ECL_OPCODE_SET_INT:
        case ECL_OPCODE_SET_FLOAT:
            EnemyEclInstr::SetVar(enemy, curInstr->args.alu.res, &args->alu.arg1.i32);
            break;
        case ECL_OPCODE_MATH_REDUCE_ANGLE:
            genericFloat = *(f32 *)EnemyEclInstr::GetVar(enemy, &curInstr->args.alu.res, NULL);
            genericFloat = utils::AddNormalizeAngle(genericFloat, 0.0f);
            EnemyEclInstr::SetVar(enemy, curInstr->args.alu.res, &genericFloat);
            break;
        case ECL_OPCODE_SET_INT_RAND: {
            i32 range = *EnemyEclInstr::GetVar(enemy, &args->alu.arg1.id, NULL);
            genericInt = g_Rng.GetRandomU32InRange(range);
            EnemyEclInstr::SetVar(enemy, curInstr->args.alu.res, &genericInt);
            break;
        }
#pragma var_order(range, minVal)
        case ECL_OPCODE_SET_INT_RAND_MIN: {
            i32 range = *EnemyEclInstr::GetVar(enemy, &args->alu.arg1.id, NULL);
            i32 minVal = *EnemyEclInstr::GetVar(enemy, &args->alu.arg2.id, NULL);
            genericInt = g_Rng.GetRandomU32InRange(range);
            genericInt += minVal;
            EnemyEclInstr::SetVar(enemy, curInstr->args.alu.res, &genericInt);
            break;
        }
        case ECL_OPCODE_SET_FLOAT_RAND: {
            f32 range = *EnemyEclInstr::GetVarFloat(enemy, &args->alu.arg1.f32, NULL);
            genericFloat = g_Rng.GetRandomF32InRange(range);
            EnemyEclInstr::SetVar(enemy, curInstr->args.alu.res, &genericFloat);
            break;
        }
#pragma var_order(range, minVal)
        case ECL_OPCODE_SET_FLOAT_RAND_MIN: {
            float range = *EnemyEclInstr::GetVarFloat(enemy, &args->alu.arg1.f32, NULL);
            float minVal = *EnemyEclInstr::GetVarFloat(enemy, &args->alu.arg2.f32, NULL);
            genericFloat = g_Rng.GetRandomF32InRange(range);
            genericFloat += minVal;
            EnemyEclInstr::SetVar(enemy, curInstr->args.alu.res, &genericFloat);
            break;
        }
        case ECL_OPCODE_SET_VAR_SELF_X:
            EnemyEclInstr::SetVar(enemy, curInstr->args.alu.res, &enemy->position.x);
            break;
        case ECL_OPCODE_SET_VAR_SELF_Y:
            EnemyEclInstr::SetVar(enemy, curInstr->args.alu.res, &enemy->position.y);
            break;
        case ECL_OPCODE_SET_VAR_SELF_Z:
            EnemyEclInstr::SetVar(enemy, curInstr->args.alu.res, &enemy->position.z);
            break;
        case ECL_OPCODE_MATH_INT_ADD:
        case ECL_OPCODE_MATH_FLOAT_ADD:
            EnemyEclInstr::MathAdd(enemy, curInstr->args.alu.res, &args->alu.arg1.id, &args->alu.arg2.id);
            break;
        case ECL_OPCODE_MATH_INC: {
            i32 *var = EnemyEclInstr::GetVar(enemy, &curInstr->args.alu.res, NULL);
            *var += 1;
            break;
        }
        case ECL_OPCODE_MATH_DEC: {
            i32 *var = EnemyEclInstr::GetVar(enemy, &curInstr->args.alu.res, NULL);
            *var -= 1;
            break;
        }
        case ECL_OPCODE_MATH_INT_SUB:
        case ECL_OPCODE_MATH_FLOAT_SUB:
            EnemyEclInstr::MathSub(enemy, curInstr->args.alu.res, &args->alu.arg1.id, &args->alu.arg2.id);
            break;
        case ECL_OPCODE_MATH_INT_MUL:
        case ECL_OPCODE_MATH_FLOAT_MUL:
            EnemyEclInstr::MathMul(enemy, curInstr->args.alu.res, &args->alu.arg1.id, &args->alu.arg2.id);
            break;
        case ECL_OPCODE_MATH_INT_DIV:
        case ECL_OPCODE_MATH_FLOAT_DIV:
            EnemyEclInstr::MathDiv(enemy, curInstr->args.alu.res, &args->alu.arg1.id, &args->alu.arg2.id);
            break;
        case ECL_OPCODE_MATH_INT_MOD:
        case ECL_OPCODE_MATH_FLOAT_MOD:
            EnemyEclInstr::MathMod(enemy, curInstr->args.alu.res, &args->alu.arg1.id, &args->alu.arg2.id);
            break;
        case ECL_OPCODE_MATH_LINE_ANGLE:
            EnemyEclInstr::MathAtan2(enemy, curInstr->args.alu.res, &args->alu.arg1.f32, &args->alu.arg2.f32,
                                     &args->alu.arg3.f32, &args->alu.arg4.f32);
            break;
#pragma var_order(rhs, lhs)
        case ECL_OPCODE_CMP_INT: {
            i32 lhs = *EnemyEclInstr::GetVar(enemy, &curInstr->args.cmp.lhs.id, NULL);
            i32 rhs = *EnemyEclInstr::GetVar(enemy, &curInstr->args.cmp.rhs.id, NULL);
            enemy->currentContext.compareRegister = lhs == rhs ? 0 : lhs < rhs ? -1 : 1;
            break;
        }
#pragma var_order(lhs, rhs)
        case ECL_OPCODE_CMP_FLOAT: {
            float lhs = *EnemyEclInstr::GetVarFloat(enemy, &curInstr->args.cmp.lhs.f32, NULL);
            float rhs = *EnemyEclInstr::GetVarFloat(enemy, &curInstr->args.cmp.rhs.f32, NULL);
            enemy->currentContext.compareRegister = lhs == rhs ? 0 : (lhs < rhs ? -1 : 1);
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
            genericInt = curInstr->args.call.eclSub;
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
            enemy->currentContext.int0 = curInstr->args.call.int0;
            enemy->currentContext.float0 = curInstr->args.call.float0;
            goto restart_sub_changed;
        case ECL_OPCODE_RET:
            if (enemy->flags.disableCallStack)
            {
                utils::DebugPrint2("error : no Stack Ret\n");
            }
            enemy->stackDepth--;
            enemy->currentContext = enemy->savedContextStack[enemy->stackDepth];
            goto restart_sub_changed;
        case ECL_OPCODE_CALL_LSS:
            genericInt = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
            if (genericInt < args->call.cmpRhs)
                goto handle_call;
            break;
        case ECL_OPCODE_CALL_LEQ:
            genericInt = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
            if (genericInt <= args->call.cmpRhs)
                goto handle_call;
            break;
        case ECL_OPCODE_CALL_EQU:
            genericInt = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
            if (genericInt == args->call.cmpRhs)
                goto handle_call;
            break;
        case ECL_OPCODE_CALL_GRE:
            genericInt = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
            if (genericInt > args->call.cmpRhs)
                goto handle_call;
            break;
        case ECL_OPCODE_CALL_GEQ:
            genericInt = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
            if (genericInt >= args->call.cmpRhs)
                goto handle_call;
            break;
        case ECL_OPCODE_CALL_NEQ:
            genericInt = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
            if (genericInt != args->call.cmpRhs)
                goto handle_call;
            break;
        case ECL_OPCODE_ANM_SET_MAIN:
            g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                 curInstr->args.anmMainScriptIdx + ANM_SCRIPT_ENEMY_START);
            break;
        case ECL_OPCODE_ANM_SET_SLOT:
            if (curInstr->args.anmSetSlot.vmIdx >= ENEMY_ANM_SLOTS)
            {
                utils::DebugPrint2("error : sub anim overflow\n");
            }
            g_AnmManager->SetAndExecuteScriptIdx(&enemy->vms[curInstr->args.anmSetSlot.vmIdx],
                                                 args->anmSetSlot.scriptIdx + ANM_SCRIPT_ENEMY_START);
            break;
        case ECL_OPCODE_MOVE_POSITION:
            enemy->position = *curInstr->args.float3.AsD3dXVec();
            enemy->position.x = *EnemyEclInstr::GetVarFloat(enemy, &enemy->position.x, NULL);
            enemy->position.y = *EnemyEclInstr::GetVarFloat(enemy, &enemy->position.y, NULL);
            enemy->position.z = *EnemyEclInstr::GetVarFloat(enemy, &enemy->position.z, NULL);
            enemy->ClampPos();
            break;
        case ECL_OPCODE_MOVE_AXIS_SPEED:
            enemy->axisSpeed = *curInstr->args.float3.AsD3dXVec();
            enemy->axisSpeed.x = *EnemyEclInstr::GetVarFloat(enemy, &enemy->axisSpeed.x, NULL);
            enemy->axisSpeed.y = *EnemyEclInstr::GetVarFloat(enemy, &enemy->axisSpeed.y, NULL);
            enemy->axisSpeed.z = *EnemyEclInstr::GetVarFloat(enemy, &enemy->axisSpeed.z, NULL);
            enemy->flags.movementMode = EnemyMove_AxisSpeed;
            break;
        case ECL_OPCODE_MOVE_VELOCITY:
            // BUG: This instruction is encoded with 2 floats, not 3
            genericFloat3 = curInstr->args.float3;
            enemy->angle = *EnemyEclInstr::GetVarFloat(enemy, &genericFloat3.x, NULL);
            enemy->speed = *EnemyEclInstr::GetVarFloat(enemy, &genericFloat3.y, NULL);
            enemy->flags.movementMode = EnemyMove_Velocity;
            break;
        case ECL_OPCODE_MOVE_ANGULAR_VELOCITY:
            // BUG: This instruction is encoded with 1 float, not 3
            genericFloat3 = curInstr->args.float3;
            enemy->angularVelocity = *EnemyEclInstr::GetVarFloat(enemy, &genericFloat3.x, NULL);
            enemy->flags.movementMode = EnemyMove_Velocity;
            break;
        case ECL_OPCODE_MOVE_TOWARDS_PLAYER:
            // BUG: This instruction is encoded with 2 floats, not 3
            genericFloat3 = curInstr->args.float3;
            enemy->angle = g_Player.AngleToPlayer(&enemy->position) + genericFloat3.x;
            enemy->speed = *EnemyEclInstr::GetVarFloat(enemy, &genericFloat3.y, NULL);
            enemy->flags.movementMode = EnemyMove_Velocity;
            break;
        case ECL_OPCODE_MOVE_SPEED:
            // BUG: This instruction is encoded with 1 float, not 3
            genericFloat3 = curInstr->args.float3;
            enemy->speed = *EnemyEclInstr::GetVarFloat(enemy, &genericFloat3.x, NULL);
            enemy->flags.movementMode = EnemyMove_Velocity;
            break;
        case ECL_OPCODE_MOVE_ACCELERATION:
            // BUG: This instruction is encoded with 1 float, not 3
            genericFloat3 = curInstr->args.float3;
            enemy->acceleration = *EnemyEclInstr::GetVarFloat(enemy, &genericFloat3.x, NULL);
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
            EclRawInstrBulletArgs *args = &curInstr->args.bullet;
            EnemyBulletShooter *shooter = &enemy->bulletProps;
            shooter->sprite = args->sprite;
            shooter->aimMode = curInstr->opCode - ECL_OPCODE_BULLET_FAN_AIMED;
            shooter->count1 = *EnemyEclInstr::GetVar(enemy, &args->count1, NULL);
            shooter->count1 += g_GameManager.RankLerpInt(enemy->bulletRankAmount1Low, enemy->bulletRankAmount1High);
            if (shooter->count1 <= 0)
            {
                shooter->count1 = 1;
            }
            shooter->count2 = *EnemyEclInstr::GetVar(enemy, &args->count2, NULL);
            shooter->count2 += g_GameManager.RankLerpInt(enemy->bulletRankAmount2Low, enemy->bulletRankAmount2High);
            if (shooter->count2 <= 0)
            {
                shooter->count2 = 1;
            }
            shooter->position = enemy->position + enemy->shootOffset;
            shooter->angle1 = *EnemyEclInstr::GetVarFloat(enemy, &args->angle1, NULL);
            shooter->angle1 = utils::AddNormalizeAngle(shooter->angle1, 0.0f);
            shooter->speed1 = *EnemyEclInstr::GetVarFloat(enemy, &args->speed1, NULL);
            if (shooter->speed1 != 0.0f)
            {
                shooter->speed1 += g_GameManager.RankLerpFloat(enemy->bulletRankSpeedLow, enemy->bulletRankSpeedHigh);
                if (shooter->speed1 < 0.3f)
                {
                    shooter->speed1 = 0.3;
                }
            }
            shooter->angle2 = *EnemyEclInstr::GetVarFloat(enemy, &args->angle2, NULL);
            shooter->speed2 = *EnemyEclInstr::GetVarFloat(enemy, &args->speed2, NULL);
            shooter->speed2 +=
                g_GameManager.RankLerpFloat(enemy->bulletRankSpeedLow, enemy->bulletRankSpeedHigh) / 2.0f;
            if (shooter->speed2 < 0.3f)
            {
                shooter->speed2 = 0.3f;
            }
            shooter->unk_4a = 0;
            shooter->flags = args->flags;
            genericInt = args->color;
            shooter->spriteOffset = *EnemyEclInstr::GetVar(enemy, (EclVarId *)&genericInt, NULL);
            if (!enemy->flags.shootingDisabled)
            {
                g_BulletManager.SpawnBulletPattern(shooter);
            }
            break;
        }
        case ECL_OPCODE_BULLET_EFFECTS:
            enemy->bulletProps.exInts[0] = *EnemyEclInstr::GetVar(enemy, &args->bulletEffects.ivar1, NULL);
            enemy->bulletProps.exInts[1] = *EnemyEclInstr::GetVar(enemy, &args->bulletEffects.ivar2, NULL);
            enemy->bulletProps.exInts[2] = *EnemyEclInstr::GetVar(enemy, &args->bulletEffects.ivar3, NULL);
            enemy->bulletProps.exInts[3] = *EnemyEclInstr::GetVar(enemy, &args->bulletEffects.ivar4, NULL);
            enemy->bulletProps.exFloats[0] = *EnemyEclInstr::GetVarFloat(enemy, &args->bulletEffects.fvar1, NULL);
            enemy->bulletProps.exFloats[1] = *EnemyEclInstr::GetVarFloat(enemy, &args->bulletEffects.fvar2, NULL);
            enemy->bulletProps.exFloats[2] = *EnemyEclInstr::GetVarFloat(enemy, &args->bulletEffects.fvar3, NULL);
            enemy->bulletProps.exFloats[3] = *EnemyEclInstr::GetVarFloat(enemy, &args->bulletEffects.fvar4, NULL);
            break;
        case ECL_OPCODE_ANM_DEATH_EFFECTS: {
            EclRawInstrAnmSetDeathArgs *args = &curInstr->args.anmSetDeath;
            enemy->deathParticle1 = args->deathParticle1;
            enemy->deathParticle2 = args->deathParticle2;
            enemy->deathAnm3 = args->deathAnm3;
            break;
        }
        case ECL_OPCODE_SHOOT_INTERVAL:
            enemy->shootInterval = curInstr->args.setInt;
            enemy->shootInterval += g_GameManager.RankLerpInt(enemy->shootInterval / 5, -enemy->shootInterval / 5);
            enemy->shootIntervalTimer = 0;
            break;
        case ECL_OPCODE_SHOOT_INTERVAL_DELAYED:
            enemy->shootInterval = curInstr->args.setInt;
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
            enemy->shootOffset.x = *EnemyEclInstr::GetVarFloat(enemy, &args->float3.x, NULL);
            enemy->shootOffset.y = *EnemyEclInstr::GetVarFloat(enemy, &args->float3.y, NULL);
            enemy->shootOffset.z = *EnemyEclInstr::GetVarFloat(enemy, &args->float3.z, NULL);
            break;
        case ECL_OPCODE_LASER_CREATE:
        case ECL_OPCODE_LASER_CREATE_AIMED:
#pragma var_order(shooter, args)
        {
            EclRawInstrLaserArgs *args = &curInstr->args.laser;
            EnemyLaserShooter *shooter = &enemy->laserProps;
            shooter->position = enemy->position + enemy->shootOffset;
            shooter->sprite = args->sprite;
            shooter->spriteOffset = args->color;
            shooter->angle = *EnemyEclInstr::GetVarFloat(enemy, &args->angle, NULL);
            shooter->speed = *EnemyEclInstr::GetVarFloat(enemy, &args->speed, NULL);
            shooter->startOffset = *EnemyEclInstr::GetVarFloat(enemy, &args->startOffset, NULL);
            shooter->endOffset = *EnemyEclInstr::GetVarFloat(enemy, &args->endOffset, NULL);
            shooter->startLength = *EnemyEclInstr::GetVarFloat(enemy, &args->startLength, NULL);
            shooter->width = args->width;
            shooter->startTime = args->startTime;
            shooter->duration = args->duration;
            shooter->despawnDuration = args->despawnDuration;
            shooter->hitboxStartTime = args->hitboxStartTime;
            shooter->hitboxEndDelay = args->hitboxEndDelay;
            shooter->flags = args->flags;
            if (curInstr->opCode == ECL_OPCODE_LASER_CREATE_AIMED)
            {
                shooter->type = 0;
            }
            else
            {
                shooter->type = 1;
            }
            enemy->lasers[enemy->laserStore] = g_BulletManager.SpawnLaserPattern(shooter);
            break;
        }
        case ECL_OPCODE_LASER_INDEX:
            enemy->laserStore = *EnemyEclInstr::GetVar(enemy, &curInstr->args.alu.res, NULL);
            break;
        case ECL_OPCODE_LASER_ROTATE:
            if (enemy->lasers[curInstr->args.laserOp.laserIdx] != NULL)
            {
                enemy->lasers[curInstr->args.laserOp.laserIdx]->angle +=
                    *EnemyEclInstr::GetVarFloat(enemy, &curInstr->args.laserOp.arg1.x, NULL);
            }
            break;
        case ECL_OPCODE_LASER_ROTATE_FROM_PLAYER:
            if (enemy->lasers[curInstr->args.laserOp.laserIdx] != NULL)
            {
                enemy->lasers[curInstr->args.laserOp.laserIdx]->angle =
                    g_Player.AngleToPlayer(&enemy->lasers[curInstr->args.laserOp.laserIdx]->pos) +
                    *EnemyEclInstr::GetVarFloat(enemy, &curInstr->args.laserOp.arg1.x, NULL);
            }
            break;
        case ECL_OPCODE_LASER_OFFSET:
            if (enemy->lasers[curInstr->args.laserOp.laserIdx] != NULL)
            {
                enemy->lasers[curInstr->args.laserOp.laserIdx]->pos =
                    enemy->position + *curInstr->args.laserOp.arg1.AsD3dXVec();
            }
            break;
        case ECL_OPCODE_LASER_TEST:
            if (enemy->lasers[curInstr->args.laserOp.laserIdx] != NULL &&
                enemy->lasers[curInstr->args.laserOp.laserIdx]->inUse)
            {
                enemy->currentContext.compareRegister = 0;
            }
            else
            {
                enemy->currentContext.compareRegister = 1;
            }
            break;
        case ECL_OPCODE_LASER_CANCEL:
            if (enemy->lasers[curInstr->args.laserOp.laserIdx] != NULL &&
                enemy->lasers[curInstr->args.laserOp.laserIdx]->inUse &&
                enemy->lasers[curInstr->args.laserOp.laserIdx]->state < 2)
            {
                enemy->lasers[curInstr->args.laserOp.laserIdx]->state = 2;
                enemy->lasers[curInstr->args.laserOp.laserIdx]->timer = 0;
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
            if (curInstr->args.setInt >= 0)
            {
                g_EnemyManager.bosses[curInstr->args.setInt] = enemy;
                g_Gui.bossPresent = true;
                g_Gui.SetBossHealthBar(1.0f);
                enemy->flags.isBoss = true;
                enemy->bossId = curInstr->args.setInt;
            }
            else
            {
                g_Gui.bossPresent = false;
                g_EnemyManager.bosses[enemy->bossId] = NULL;
                enemy->flags.isBoss = false;
            }
            break;
        case ECL_OPCODE_SPELLCARD_EFFECT: {
            EclRawInstrSpellcardEffectArgs *args = &curInstr->args.spellcardEffect;
            enemy->effectArray[enemy->effectIdx] = g_EffectManager.SpawnParticles(
                PARTICLE_EFFECT_UNK_13, &enemy->position, 1, g_EffectsColor[args->effectColorId]);
            enemy->effectArray[enemy->effectIdx]->pos2 = *args->pos.AsD3dXVec();
            enemy->effectDistance = args->effectDistance;
            enemy->effectIdx++;
            break;
        }
        case ECL_OPCODE_MOVE_VELOCITY_INTERP_DECELERATE:
            EnemyEclInstr::MoveDirTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Decelerate;
            break;
        case ECL_OPCODE_MOVE_VELOCITY_INTERP_DECELERATE_FAST:
            EnemyEclInstr::MoveDirTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_DecelerateFast;
            break;
        case ECL_OPCODE_MOVE_VELOCITY_INTERP_ACCELERATE:
            EnemyEclInstr::MoveDirTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Accelerate;
            break;
        case ECL_OPCODE_MOVE_VELOCITY_INTERP_ACCELERATE_FAST:
            EnemyEclInstr::MoveDirTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_AccelerateFast;
            break;
        case ECL_OPCODE_MOVE_POSITION_INTERP_LINEAR:
            EnemyEclInstr::MovePosTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Linear;
            break;
        case ECL_OPCODE_MOVE_POSITION_INTERP_DECELERATE:
            EnemyEclInstr::MovePosTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Decelerate;
            break;
        case ECL_OPCODE_MOVE_POSITION_INTERP_DECELERATE_FAST:
            EnemyEclInstr::MovePosTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_DecelerateFast;
            break;
        case ECL_OPCODE_MOVE_POSITION_INTERP_ACCELERATE:
            EnemyEclInstr::MovePosTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Accelerate;
            break;
        case ECL_OPCODE_MOVE_POSITION_INTERP_ACCELERATE_FAST:
            EnemyEclInstr::MovePosTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_AccelerateFast;
            break;
        case ECL_OPCODE_MOVE_AS_INTERP_DECELERATE:
            EnemyEclInstr::MoveTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Decelerate;
            break;
        case ECL_OPCODE_MOVE_AS_INTERP_DECELERATE_FAST:
            EnemyEclInstr::MoveTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_DecelerateFast;
            break;
        case ECL_OPCODE_MOVE_AS_INTERP_ACCELERATE:
            EnemyEclInstr::MoveTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_Accelerate;
            break;
        case ECL_OPCODE_MOVE_AS_INTERP_ACCELERATE_FAST:
            EnemyEclInstr::MoveTime(enemy, curInstr);
            enemy->flags.moveInterpMode = EnemyInterp_AccelerateFast;
            break;
        case ECL_OPCODE_MOVE_BOUNDS_SET:
            enemy->lowerMoveLimit.x = curInstr->args.moveBoundSet.lowerMoveLimit.x;
            enemy->lowerMoveLimit.y = curInstr->args.moveBoundSet.lowerMoveLimit.y;
            enemy->upperMoveLimit.x = curInstr->args.moveBoundSet.upperMoveLimit.x;
            enemy->upperMoveLimit.y = curInstr->args.moveBoundSet.upperMoveLimit.y;
            enemy->flags.shouldClampPos = true;
            break;
        case ECL_OPCODE_MOVE_BOUNDS_DISABLE:
            enemy->flags.shouldClampPos = false;
            break;
        case ECL_OPCODE_MOVE_RAND:
            genericFloat3 = curInstr->args.float3;
            enemy->angle = g_Rng.GetRandomF32InRange(genericFloat3.y - genericFloat3.x) + genericFloat3.x;
            break;
        case ECL_OPCODE_MOVE_RAND_IN_BOUNDS:
            genericFloat3 = curInstr->args.float3;
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
            enemy->anmPoseDefault = curInstr->args.anmSetPoses.anmPoseDefault;
            enemy->anmPoseNeutralFromLeft = curInstr->args.anmSetPoses.anmPoseNeutralFromLeft;
            enemy->anmPoseNeutralFromRight = curInstr->args.anmSetPoses.anmPoseNeutralFromRight;
            enemy->anmPoseLeft = curInstr->args.anmSetPoses.anmPoseLeft;
            enemy->anmPoseRight = curInstr->args.anmSetPoses.anmPoseRight;
            enemy->anmPoseCurrent = EnemyPose_Default;
            break;
        case ECL_OPCODE_ENEMY_SET_HITBOX:
            enemy->hitboxDimensions.x = curInstr->args.float3.x;
            enemy->hitboxDimensions.y = curInstr->args.float3.y;
            enemy->hitboxDimensions.z = curInstr->args.float3.z;
            break;
        case ECL_OPCODE_ENEMY_FLAG_COLLISION:
            enemy->flags.isCollidable = curInstr->args.setInt;
            break;
        case ECL_OPCODE_ENEMY_FLAG_CAN_TAKE_DAMAGE:
            enemy->flags.isDamageable = curInstr->args.setInt;
            break;
        case ECL_OPCODE_EFFECT_SOUND:
            g_SoundPlayer.PlaySoundByIdx((SoundIdx)curInstr->args.setInt);
            break;
        case ECL_OPCODE_ENEMY_FLAGS_DEATH:
            enemy->flags.deathMode = curInstr->args.setInt;
            break;
        case ECL_OPCODE_DEATH_CALLBACK_SUB:
            enemy->deathCallbackSub = curInstr->args.setInt;
            break;
        case ECL_OPCODE_ENEMY_INTERRUPT_SET:
            enemy->interrupts[args->setInterrupt.interruptId] = args->setInterrupt.interruptSub;
            break;
        case ECL_OPCODE_ENEMY_INTERRUPT:
            enemy->runInterrupt = curInstr->args.setInt;
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
            enemy->life = enemy->maxLife = curInstr->args.setInt;
            break;
#pragma var_order(catk, length, csum)
        case ECL_OPCODE_SPELLCARD_START: {
            g_Gui.ShowSpellcard(curInstr->args.spellcardStart.spellcardSprite,
                                curInstr->args.spellcardStart.spellcardName);
            g_EnemyManager.spellcardInfo.isCapturing = true;
            g_EnemyManager.spellcardInfo.isActive = 1;
            g_EnemyManager.spellcardInfo.idx = curInstr->args.spellcardStart.spellcardId;
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
                strcpy(catk->name, curInstr->args.spellcardStart.spellcardName);
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
                        g_GameManager.score += scoreIncrease;
                        if (!g_GameManager.isInReplay)
                        {
                            catk->numSuccess++;
                            for (idx = 4; idx > 0; idx--)
                            {
                                catk->characterShotType[idx] = catk->characterShotType[idx - 1];
                            }
                            catk->characterShotType[0] = g_GameManager.CharacterShotType();
                        }
                        g_GameManager.spellcardsCaptured++;
                    }
                }
                g_EnemyManager.spellcardInfo.isActive = 0;
            }
            g_Stage.spellcardState = NOT_RUNNING;
            break;
        case ECL_OPCODE_PHASE_TIMER_SET:
            enemy->phaseTimer = curInstr->args.setInt;
            break;
        case ECL_OPCODE_LIFE_CALLBACK_THRESHOLD:
            enemy->lifeCallbackThreshold = curInstr->args.setInt;
            break;
        case ECL_OPCODE_LIFE_CALLBACK_SUB:
            enemy->lifeCallbackSub = curInstr->args.setInt;
            break;
        case ECL_OPCODE_TIMER_CALLBACK_THRESHOLD:
            enemy->timerCallbackThreshold = curInstr->args.setInt;
            enemy->phaseTimer = 0;
            break;
        case ECL_OPCODE_TIMER_CALLBACK_SUB:
            enemy->timerCallbackSub = curInstr->args.setInt;
            break;
        case ECL_OPCODE_ENEMY_FLAG_INTERACTABLE:
            enemy->flags.isInteractable = curInstr->args.setInt;
            break;
        case ECL_OPCODE_EFFECT_PARTICLE:
            g_EffectManager.SpawnParticles(curInstr->args.effectParticle.effectId, &enemy->position,
                                           curInstr->args.effectParticle.numParticles,
                                           curInstr->args.effectParticle.particleColor);
            break;
        case ECL_OPCODE_DROP_ITEMS: {
            for (i32 idx = 0; idx < curInstr->args.setInt; idx++)
            {
                D3DXVECTOR3 pos = enemy->position;

                pos[0] += g_Rng.GetRandomF32InRange(144.0f) - 72.0f;
                pos[1] += g_Rng.GetRandomF32InRange(144.0f) - 72.0f;
                if (g_GameManager.currentPower < MAX_POWER)
                {
                    g_ItemManager.SpawnItem(&pos, idx == 0 ? ITEM_POWER_BIG : ITEM_POWER_SMALL, 0);
                }
                else
                {
                    g_ItemManager.SpawnItem(&pos, ITEM_POINT, 0);
                }
            }
            break;
        }
        case ECL_OPCODE_ANM_FLAG_ROTATION:
            enemy->flags.rotateAnm = curInstr->args.setInt;
            break;
        case ECL_OPCODE_EX_INS_CALL:
            g_EclExInsn[curInstr->args.setInt](enemy, curInstr);
            break;
        case ECL_OPCODE_EX_INS_REPEAT:
            if (curInstr->args.setInt >= 0)
            {
                enemy->currentContext.funcSetFunc = g_EclExInsn[curInstr->args.setInt];
            }
            else
            {
                enemy->currentContext.funcSetFunc = NULL;
            }
            break;
        case ECL_OPCODE_ECL_TIME_ADD:
            enemy->currentContext.time += *EnemyEclInstr::GetVar(enemy, &curInstr->args.timeToAdd, NULL);
            break;
        case ECL_OPCODE_DROP_ITEM_ID:
            g_ItemManager.SpawnItem(&enemy->position, curInstr->args.itemId, 0);
            break;
        case ECL_OPCODE_STD_UNPAUSE:
            g_Stage.unpauseFlag = 1;
            break;
        case ECL_OPCODE_BOSS_SET_LIFE_COUNT:
            g_Gui.SetBossLives(curInstr->args.setInt);
            g_GameManager.counat += 1800;
            break;
        case ECL_OPCODE_ENEMY_CREATE: {
            EclRawInstrEnemyCreateArgs args = curInstr->args.enemyCreate;
            args.pos.x = *EnemyEclInstr::GetVarFloat(enemy, &args.pos.x, NULL);
            args.pos.y = *EnemyEclInstr::GetVarFloat(enemy, &args.pos.y, NULL);
            args.pos.z = *EnemyEclInstr::GetVarFloat(enemy, &args.pos.z, NULL);
            g_EnemyManager.SpawnEnemy(args.subId, args.pos.AsD3dXVec(), args.life, args.itemDrop, args.score);
            break;
        }
#pragma var_order(currentEnemy, idx)
        case ECL_OPCODE_ENEMY_KILL_ALL: {
            Enemy *currentEnemy;
            i32 idx;
            for (currentEnemy = &g_EnemyManager.enemies[0], idx = 0; idx < MAX_ENEMY_COUNT; idx++, currentEnemy++)
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
            enemy->primaryVm.pendingInterrupt = curInstr->args.setInt;
            break;
        case ECL_OPCODE_ANM_INTERRUPT_SLOT:
            enemy->vms[args->anmInterruptSlot.vmId].pendingInterrupt = args->anmInterruptSlot.interruptId;
            break;
        case ECL_OPCODE_BULLET_CANCEL:
            g_BulletManager.TurnAllBulletsIntoPoints();
            break;
        case ECL_OPCODE_BULLET_SOUND:
            if (curInstr->args.bulletSound >= 0)
            {
                enemy->bulletProps.sfx = curInstr->args.bulletSound;
                enemy->bulletProps.flags |= 0x200;
            }
            else
            {
                enemy->bulletProps.flags &= ~0x200;
            }
            break;
        case ECL_OPCODE_ENEMY_FLAG_DISABLE_CALLSTACK:
            enemy->flags.disableCallStack = curInstr->args.setInt;
            break;
        case ECL_OPCODE_BULLET_RANK_INFLUENCE:
            enemy->bulletRankSpeedLow = args->bulletRankInfluence.bulletRankSpeedLow;
            enemy->bulletRankSpeedHigh = args->bulletRankInfluence.bulletRankSpeedHigh;
            enemy->bulletRankAmount1Low = args->bulletRankInfluence.bulletRankAmount1Low;
            enemy->bulletRankAmount1High = args->bulletRankInfluence.bulletRankAmount1High;
            enemy->bulletRankAmount2Low = args->bulletRankInfluence.bulletRankAmount2Low;
            enemy->bulletRankAmount2High = args->bulletRankInfluence.bulletRankAmount2High;
            break;
        case ECL_OPCODE_ENEMY_FLAG_INVISIBLE:
            enemy->flags.isInvisible = curInstr->args.setInt;
            break;
        case ECL_OPCODE_PHASE_TIMER_CLEAR:
            enemy->timerCallbackSub = enemy->deathCallbackSub;
            enemy->phaseTimer = 0;
            break;
        case ECL_OPCODE_SPELLCARD_FLAG_TIMEOUT:
            enemy->flags.isTimeoutSpell = curInstr->args.setInt;
            break;
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
        enemy->axisSpeed.z = 0.0f;
        break;
    case EnemyMove_Interp: {
        enemy->moveInterpTimer--;
        f32 interpVal = enemy->moveInterpTimer.AsFramesFloat() / enemy->moveInterpStartTime;
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
namespace EnemyEclInstr
{

#pragma var_order(alu, angle)
static void MoveDirTime(Enemy *enemy, EclRawInstr *instr)
{
    EclRawInstrAluArgs *alu;
    f32 angle;

    alu = &instr->args.alu;
    angle = *GetVarFloat(enemy, &alu->arg1.f32, NULL);

    enemy->moveInterp.x = cosf(angle) * alu->arg2.f32 * alu->res / 2.0f;
    enemy->moveInterp.y = sinf(angle) * alu->arg2.f32 * alu->res / 2.0f;
    enemy->moveInterp.z = 0.0f;

    enemy->moveInterpStartPos = enemy->position;
    enemy->moveInterpStartTime = alu->res;

    enemy->moveInterpTimer = enemy->moveInterpStartTime;

    enemy->flags.movementMode = EnemyMove_Interp;
}

static void MovePosTime(Enemy *enemy, EclRawInstr *instr)
{
    D3DXVECTOR3 newPos;
    EclRawInstrAluArgs *alu = &instr->args.alu;

    newPos.x = *GetVarFloat(enemy, &alu->arg1.f32, NULL);
    newPos.y = *GetVarFloat(enemy, &alu->arg2.f32, NULL);
    newPos.z = *GetVarFloat(enemy, &alu->arg3.f32, NULL);

    enemy->moveInterp = newPos - enemy->position;
    enemy->moveInterpStartPos = enemy->position;
    enemy->moveInterpStartTime = alu->res;

    enemy->moveInterpTimer = enemy->moveInterpStartTime;

    enemy->flags.movementMode = EnemyMove_Interp;
    enemy->axisSpeed = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

static void MoveTime(Enemy *enemy, EclRawInstr *instr)
{
    EclRawInstrAluArgs *alu;
    f32 angle;

    alu = &instr->args.alu;
    angle = *GetVarFloat(enemy, &enemy->angle, NULL);

    enemy->moveInterp.x = cosf(angle) * enemy->speed * alu->res / 2.0f;
    enemy->moveInterp.y = sinf(angle) * enemy->speed * alu->res / 2.0f;
    enemy->moveInterp.z = 0.0f;

    enemy->moveInterpStartPos = enemy->position;
    enemy->moveInterpStartTime = alu->res;

    enemy->moveInterpTimer = enemy->moveInterpStartTime;

    enemy->flags.movementMode = EnemyMove_Interp;
}

static i32 *GetVar(Enemy *enemy, EclVarId *eclVarId, EclValueType *valueType)
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

    case ECL_VAR_PLAYER_ANGLE:
        g_PlayerAngle = g_Player.AngleToPlayer(&enemy->position);
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return (i32 *)&g_PlayerAngle;

    case ECL_VAR_ENEMY_TIMER:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->phaseTimer.current;

    case ECL_VAR_PLAYER_DISTANCE: {
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

    case ECL_VAR_PLAYER_SHOT:
        g_PlayerShot = g_GameManager.CharacterShotType();
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &g_PlayerShot;
    }
    return (i32 *)eclVarId;
}

static f32 *GetVarFloat(Enemy *enemy, f32 *eclVarId, EclValueType *valueType)
{
    i32 varId = *eclVarId;
    i32 *res = GetVar(enemy, (EclVarId *)&varId, valueType);
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
static void SetVar(Enemy *enemy, EclVarId out, void *value)
{
    i32 *lhsPtr;
    EclValueType lhsType;
    i32 *rhsPtr;

    rhsPtr = GetVar(enemy, (EclVarId *)value, NULL);
    lhsPtr = GetVar(enemy, &out, &lhsType);
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
static void MathAdd(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
{
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    // Get output variable.
    outPtr = GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT)
    {
        lhsPtr = GetVar(enemy, lhsVarId, NULL);
        rhsPtr = GetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr + *rhsPtr;
    }
    else if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        lhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr + *(f32 *)rhsPtr;
    }
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
static void MathSub(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
{
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    outPtr = GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT)
    {
        lhsPtr = GetVar(enemy, lhsVarId, NULL);
        rhsPtr = GetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr - *rhsPtr;
    }
    else if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        lhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr - *(f32 *)rhsPtr;
    }
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
static void MathMul(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
{
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    lhsPtr = GetVar(enemy, lhsVarId, NULL);
    rhsPtr = GetVar(enemy, rhsVarId, NULL);
    outPtr = GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT)
    {
        lhsPtr = GetVar(enemy, lhsVarId, NULL);
        rhsPtr = GetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr * *rhsPtr;
    }
    else if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        lhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr * *(f32 *)rhsPtr;
    }
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
static void MathDiv(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
{
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    outPtr = GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT)
    {
        lhsPtr = GetVar(enemy, lhsVarId, NULL);
        rhsPtr = GetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr / *rhsPtr;
    }
    else if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        lhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr / *(f32 *)rhsPtr;
    }
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
static void MathMod(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
{
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    outPtr = GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT)
    {
        lhsPtr = GetVar(enemy, lhsVarId, NULL);
        rhsPtr = GetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr % *rhsPtr;
    }
    else if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        lhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = fmodf(*(f32 *)lhsPtr, *(f32 *)rhsPtr);
    }
}

#pragma var_order(y2Ptr, outPtr, x1Ptr, y1Ptr, outType, x2Ptr)
static void MathAtan2(Enemy *enemy, EclVarId outVarId, f32 *x1, f32 *y1, f32 *y2, f32 *x2)
{
    EclValueType outType;
    f32 *outPtr;
    f32 *y1Ptr, *x1Ptr, *x2Ptr, *y2Ptr;

    outPtr = (f32 *)GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_FLOAT)
    {
        y1Ptr = GetVarFloat(enemy, x1, NULL);
        x1Ptr = GetVarFloat(enemy, y1, NULL);
        y2Ptr = GetVarFloat(enemy, y2, NULL);
        x2Ptr = GetVarFloat(enemy, x2, NULL);
        *outPtr = atan2f(*x2Ptr - *x1Ptr, *y2Ptr - *y1Ptr);
    }
}
} // namespace EnemyEclInstr
} // namespace th06
