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
DIFFABLE_STATIC(ChainElem, g_EclManagerCalcChain); // unused
DIFFABLE_STATIC(EclManager, g_EclManager);
DIFFABLE_STATIC(i32, g_PlayerShot);
DIFFABLE_STATIC(f32, g_PlayerDistance);
DIFFABLE_STATIC(f32, g_PlayerAngle);

ZunResult EclManager::Load(char *eclPath)
{
    i32 idx;

    this->eclFile = (EclRawHeader *)FileSystem::OpenPath(eclPath);
    if (this->eclFile == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_ECLMANAGER_ENEMY_DATA_CORRUPT);
        return ZUN_ERROR;
    }
    this->eclFile->timelineOffsets[0] =
        (EclTimelineInstr *)((int)this->eclFile->timelineOffsets[0] + (int)this->eclFile);
    this->subTable = &this->eclFile->subOffsets[0];
    for (idx = 0; idx < this->eclFile->subCount; idx++)
    {
        this->subTable[idx] = (EclRawInstr *)((int)this->subTable[idx] + (int)this->eclFile);
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
    return;
}

ZunResult EclManager::CallEclSub(EnemyEclContext *ctx, i16 subId)
{
    ctx->currentInstr = this->subTable[subId];
    ctx->time = 0;
    ctx->subId = subId;
    return ZUN_SUCCESS;
}

#pragma var_order(local_8, local_14, local_18, args, instruction, local_24, local_28, local_2c, local_30, local_34,    \
                  local_38, local_3c, local_40, local_44, local_48, local_4c, local_50, local_54, local_58, local_5c,  \
                  local_60, local_64, local_68, local_6c, local_70, local_74, csum, scoreIncrease, local_80, local_84, \
                  local_88, local_8c, local_98, local_b0, local_b4, local_b8, local_bc, local_c0)
ZunResult EclManager::RunEcl(Enemy *enemy)
{
    EclRawInstr *instruction;
    EclRawInstrArgs *args;
    ZunVec3 local_8;
    i32 local_14, local_24, local_28, local_2c, *local_3c, *local_40, local_44, local_48, local_68, local_74, csum,
        scoreIncrease, local_84, local_88, local_8c, local_b8, local_c0;
    f32 local_18, local_30, local_34, local_38, local_4c, local_50, local_bc;
    Catk *local_70, *local_80;
    EclRawInstrBulletArgs *local_54;
    EnemyBulletShooter *local_58;
    EclRawInstrAnmSetDeathArgs *local_5c;
    EnemyLaserShooter *local_60;
    EclRawInstrLaserArgs *local_64;
    EclRawInstrSpellcardEffectArgs *local_6c;
    D3DXVECTOR3 local_98;
    EclRawInstrEnemyCreateArgs local_b0;
    Enemy *local_b4;

    for (;;)
    {
        instruction = enemy->currentContext.currentInstr;
        if (0 <= enemy->runInterrupt)
        {
            goto HANDLE_INTERRUPT;
        }

    YOLO:
        if (enemy->currentContext.time == instruction->time)
        {
            if (!(instruction->skipForDifficulty & (1 << g_GameManager.difficulty)))
            {
                goto NEXT_INSN;
            }

            args = &instruction->args;
            switch (instruction->opCode)
            {
            case ECL_OPCODE_UNIMP:
                return ZUN_ERROR;
            case ECL_OPCODE_JUMPDEC:
                local_14 = *EnemyEclInstr::GetVar(enemy, &args->jump.var, NULL);
                local_14--;
                EnemyEclInstr::SetVar(enemy, args->jump.var, &local_14);
                if (local_14 <= 0)
                    break;
            case ECL_OPCODE_JUMP:
            HANDLE_JUMP:
                enemy->currentContext.time.current = instruction->args.jump.time;
                instruction = (EclRawInstr *)((int)instruction + args->jump.offset);
                goto YOLO;
            case ECL_OPCODE_SETINT:
            case ECL_OPCODE_SETFLOAT:
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &args->alu.arg1.i32);
                break;
            case ECL_OPCODE_MATHNORMANGLE:
                local_18 = *(f32 *)EnemyEclInstr::GetVar(enemy, &instruction->args.alu.res, NULL);
                local_18 = utils::AddNormalizeAngle(local_18, 0.0f);
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &local_18);
                break;
            case ECL_OPCODE_SETINTRAND:
                local_24 = *EnemyEclInstr::GetVar(enemy, &args->alu.arg1.id, NULL);
                local_14 = g_Rng.GetRandomU32InRange(local_24);
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &local_14);
                break;
            case ECL_OPCODE_SETINTRANDMIN:
                local_28 = *EnemyEclInstr::GetVar(enemy, &args->alu.arg1.id, NULL);
                local_2c = *EnemyEclInstr::GetVar(enemy, &args->alu.arg2.id, NULL);
                local_14 = g_Rng.GetRandomU32InRange(local_28);
                local_14 += local_2c;
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &local_14);
                break;
            case ECL_OPCODE_SETFLOATRAND:
                local_30 = *EnemyEclInstr::GetVarFloat(enemy, &args->alu.arg1.f32, NULL);
                local_18 = g_Rng.GetRandomF32InRange(local_30);
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &local_18);
                break;
            case ECL_OPCODE_SETFLOATRANDMIN:
                local_34 = *EnemyEclInstr::GetVarFloat(enemy, &args->alu.arg1.f32, NULL);
                local_38 = *EnemyEclInstr::GetVarFloat(enemy, &args->alu.arg2.f32, NULL);
                local_18 = g_Rng.GetRandomF32InRange(local_34);
                local_18 += local_38;
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &local_18);
                break;
            case ECL_OPCODE_SETVARSELFX:
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &enemy->position.x);
                break;
            case ECL_OPCODE_SETVARSELFY:
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &enemy->position.y);
                break;
            case ECL_OPCODE_SETVARSELFZ:
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &enemy->position.z);
                break;
            case ECL_OPCODE_MATHINTADD:
            case ECL_OPCODE_MATHFLOATADD:
                EnemyEclInstr::MathAdd(enemy, instruction->args.alu.res, &args->alu.arg1.id, &args->alu.arg2.id);
                break;
            case ECL_OPCODE_MATHINC:
                local_3c = EnemyEclInstr::GetVar(enemy, &instruction->args.alu.res, NULL);
                *local_3c += 1;
                break;
            case ECL_OPCODE_MATHDEC:
                local_40 = EnemyEclInstr::GetVar(enemy, &instruction->args.alu.res, NULL);
                *local_40 -= 1;
                break;
            case ECL_OPCODE_MATHINTSUB:
            case ECL_OPCODE_MATHFLOATSUB:
                EnemyEclInstr::MathSub(enemy, instruction->args.alu.res, &args->alu.arg1.id, &args->alu.arg2.id);
                break;
            case ECL_OPCODE_MATHINTMUL:
            case ECL_OPCODE_MATHFLOATMUL:
                EnemyEclInstr::MathMul(enemy, instruction->args.alu.res, &args->alu.arg1.id, &args->alu.arg2.id);
                break;
            case ECL_OPCODE_MATHINTDIV:
            case ECL_OPCODE_MATHFLOATDIV:
                EnemyEclInstr::MathDiv(enemy, instruction->args.alu.res, &args->alu.arg1.id, &args->alu.arg2.id);
                break;
            case ECL_OPCODE_MATHINTMOD:
            case ECL_OPCODE_MATHFLOATMOD:
                EnemyEclInstr::MathMod(enemy, instruction->args.alu.res, &args->alu.arg1.id, &args->alu.arg2.id);
                break;
            case ECL_OPCODE_MATHATAN2:
                EnemyEclInstr::MathAtan2(enemy, instruction->args.alu.res, &args->alu.arg1.f32, &args->alu.arg2.f32,
                                         &args->alu.arg3.f32, &args->alu.arg4.f32);
                break;
            case ECL_OPCODE_CMPINT:
                local_48 = *EnemyEclInstr::GetVar(enemy, &instruction->args.cmp.lhs.id, NULL);
                local_44 = *EnemyEclInstr::GetVar(enemy, &instruction->args.cmp.rhs.id, NULL);
                enemy->currentContext.compareRegister = local_48 == local_44 ? 0 : local_48 < local_44 ? -1 : 1;
                break;
            case ECL_OPCODE_CMPFLOAT:
                local_4c = *EnemyEclInstr::GetVarFloat(enemy, &instruction->args.cmp.lhs.f32, NULL);
                local_50 = *EnemyEclInstr::GetVarFloat(enemy, &instruction->args.cmp.rhs.f32, NULL);
                enemy->currentContext.compareRegister = local_4c == local_50 ? 0 : (local_4c < local_50 ? -1 : 1);
                break;
            case ECL_OPCODE_JUMPLSS:
                if (enemy->currentContext.compareRegister < 0)
                    goto HANDLE_JUMP;
                break;
            case ECL_OPCODE_JUMPLEQ:
                if (enemy->currentContext.compareRegister <= 0)
                    goto HANDLE_JUMP;
                break;
            case ECL_OPCODE_JUMPEQU:
                if (enemy->currentContext.compareRegister == 0)
                    goto HANDLE_JUMP;
                break;
            case ECL_OPCODE_JUMPGRE:
                if (enemy->currentContext.compareRegister > 0)
                    goto HANDLE_JUMP;
                break;
            case ECL_OPCODE_JUMPGEQ:
                if (enemy->currentContext.compareRegister >= 0)
                    goto HANDLE_JUMP;
                break;
            case ECL_OPCODE_JUMPNEQ:
                if (enemy->currentContext.compareRegister != 0)
                    goto HANDLE_JUMP;
                break;
            case ECL_OPCODE_CALL:
            HANDLE_CALL:
                local_14 = instruction->args.call.eclSub;
                enemy->currentContext.currentInstr = (EclRawInstr *)((u8 *)instruction + instruction->offsetToNext);
                if (!enemy->flags.disableCallStack)
                {
                    enemy->savedContextStack[enemy->stackDepth] = enemy->currentContext;
                }
                g_EclManager.CallEclSub(&enemy->currentContext, local_14);
                if (!enemy->flags.disableCallStack && enemy->stackDepth < 7)
                {
                    enemy->stackDepth++;
                }
                enemy->currentContext.var0 = instruction->args.call.var0;
                enemy->currentContext.float0 = instruction->args.call.float0;
                continue;
            case ECL_OPCODE_RET:
                if (enemy->flags.disableCallStack)
                {
                    utils::DebugPrint2("error : no Stack Ret\n");
                }
                enemy->stackDepth--;
                enemy->currentContext = enemy->savedContextStack[enemy->stackDepth];
                continue;
            case ECL_OPCODE_CALLLSS:
                local_14 = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
                if (local_14 < args->call.cmpRhs)
                    goto HANDLE_CALL;
                break;
            case ECL_OPCODE_CALLLEQ:
                local_14 = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
                if (local_14 <= args->call.cmpRhs)
                    goto HANDLE_CALL;
                break;
            case ECL_OPCODE_CALLEQU:
                local_14 = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
                if (local_14 == args->call.cmpRhs)
                    goto HANDLE_CALL;
                break;
            case ECL_OPCODE_CALLGRE:
                local_14 = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
                if (local_14 > args->call.cmpRhs)
                    goto HANDLE_CALL;
                break;
            case ECL_OPCODE_CALLGEQ:
                local_14 = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
                if (local_14 >= args->call.cmpRhs)
                    goto HANDLE_CALL;
                break;
            case ECL_OPCODE_CALLNEQ:
                local_14 = *EnemyEclInstr::GetVar(enemy, &args->call.cmpLhs, NULL);
                if (local_14 != args->call.cmpRhs)
                    goto HANDLE_CALL;
                break;
            case ECL_OPCODE_ANMSETMAIN:
                g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                     instruction->args.anmSetMain.scriptIdx + ANM_SCRIPT_ENEMY_START);
                break;
            case ECL_OPCODE_ANMSETSLOT:
                if (ARRAY_SIZE_SIGNED(enemy->vms) <= instruction->args.anmSetSlot.vmIdx)
                {
                    utils::DebugPrint2("error : sub anim overflow\n");
                }
                g_AnmManager->SetAndExecuteScriptIdx(&enemy->vms[instruction->args.anmSetSlot.vmIdx],
                                                     args->anmSetSlot.scriptIdx + ANM_SCRIPT_ENEMY_START);
                break;
            case ECL_OPCODE_MOVEPOSITION:
                enemy->position = *instruction->args.move.pos.AsD3dXVec();
                enemy->position.x = *EnemyEclInstr::GetVarFloat(enemy, &enemy->position.x, NULL);
                enemy->position.y = *EnemyEclInstr::GetVarFloat(enemy, &enemy->position.y, NULL);
                enemy->position.z = *EnemyEclInstr::GetVarFloat(enemy, &enemy->position.z, NULL);
                enemy->ClampPos();
                break;
            case ECL_OPCODE_MOVEAXISVELOCITY:
                enemy->axisSpeed = *instruction->args.move.pos.AsD3dXVec();
                enemy->axisSpeed.x = *EnemyEclInstr::GetVarFloat(enemy, &enemy->axisSpeed.x, NULL);
                enemy->axisSpeed.y = *EnemyEclInstr::GetVarFloat(enemy, &enemy->axisSpeed.y, NULL);
                enemy->axisSpeed.z = *EnemyEclInstr::GetVarFloat(enemy, &enemy->axisSpeed.z, NULL);
                enemy->flags.movementMode = 0;
                break;
            case ECL_OPCODE_MOVEVELOCITY:
                local_8 = instruction->args.move.pos;
                enemy->angle = *EnemyEclInstr::GetVarFloat(enemy, &local_8.x, NULL);
                enemy->speed = *EnemyEclInstr::GetVarFloat(enemy, &local_8.y, NULL);
                enemy->flags.movementMode = 1;
                break;
            case ECL_OPCODE_MOVEANGULARVELOCITY:
                local_8 = instruction->args.move.pos;
                enemy->angularVelocity = *EnemyEclInstr::GetVarFloat(enemy, &local_8.x, NULL);
                enemy->flags.movementMode = 1;
                break;
            case ECL_OPCODE_MOVEATPLAYER:
                local_8 = instruction->args.move.pos;
                enemy->angle = g_Player.AngleToPlayer(&enemy->position) + local_8.x;
                enemy->speed = *EnemyEclInstr::GetVarFloat(enemy, &local_8.y, NULL);
                enemy->flags.movementMode = 1;
                break;
            case ECL_OPCODE_MOVESPEED:
                local_8 = instruction->args.move.pos;
                enemy->speed = *EnemyEclInstr::GetVarFloat(enemy, &local_8.x, NULL);
                enemy->flags.movementMode = 1;
                break;
            case ECL_OPCODE_MOVEACCELERATION:
                local_8 = instruction->args.move.pos;
                enemy->acceleration = *EnemyEclInstr::GetVarFloat(enemy, &local_8.x, NULL);
                enemy->flags.movementMode = 1;
                break;
            case ECL_OPCODE_BULLETFANAIMED:
            case ECL_OPCODE_BULLETFAN:
            case ECL_OPCODE_BULLETCIRCLEAIMED:
            case ECL_OPCODE_BULLETCIRCLE:
            case ECL_OPCODE_BULLETOFFSETCIRCLEAIMED:
            case ECL_OPCODE_BULLETOFFSETCIRCLE:
            case ECL_OPCODE_BULLETRANDOMANGLE:
            case ECL_OPCODE_BULLETRANDOMSPEED:
            case ECL_OPCODE_BULLETRANDOM:
                local_54 = &instruction->args.bullet;
                local_58 = &enemy->bulletProps;
                local_58->sprite = local_54->sprite;
                local_58->aimMode = instruction->opCode - ECL_OPCODE_BULLETFANAIMED;
                local_58->count1 = *EnemyEclInstr::GetVar(enemy, &local_54->count1, NULL);
                local_58->count1 += enemy->BulletRankAmount1(g_GameManager.rank);
                if (local_58->count1 <= 0)
                {
                    local_58->count1 = 1;
                }

                local_58->count2 = *EnemyEclInstr::GetVar(enemy, &local_54->count2, NULL);
                local_58->count2 += enemy->BulletRankAmount2(g_GameManager.rank);
                if (local_58->count2 <= 0)
                {
                    local_58->count2 = 1;
                }
                local_58->position = enemy->position + enemy->shootOffset;
                local_58->angle1 = *EnemyEclInstr::GetVarFloat(enemy, &local_54->angle1, NULL);
                local_58->angle1 = utils::AddNormalizeAngle(local_58->angle1, 0.0f);
                local_58->speed1 = *EnemyEclInstr::GetVarFloat(enemy, &local_54->speed1, NULL);
                if (local_58->speed1 != 0.0f)
                {
                    local_58->speed1 += enemy->BulletRankSpeed(g_GameManager.rank);
                    if (local_58->speed1 < 0.3f)
                    {
                        local_58->speed1 = 0.3;
                    }
                }
                local_58->angle2 = *EnemyEclInstr::GetVarFloat(enemy, &local_54->angle2, NULL);
                local_58->speed2 = *EnemyEclInstr::GetVarFloat(enemy, &local_54->speed2, NULL);
                local_58->speed2 += enemy->BulletRankSpeed(g_GameManager.rank) / 2.0f;
                if (local_58->speed2 < 0.3f)
                {
                    local_58->speed2 = 0.3f;
                }
                local_58->unk_4a = 0;
                local_58->flags = local_54->flags;
                local_14 = local_54->color;
                // TODO: Strict aliasing rule be like.
                local_58->spriteOffset = *EnemyEclInstr::GetVar(enemy, (EclVarId *)&local_14, NULL);
                if (!enemy->flags.shootingDisabled)
                {
                    g_BulletManager.SpawnBulletPattern(local_58);
                }
                break;
            case ECL_OPCODE_BULLETEFFECTS:
                enemy->bulletProps.exInts[0] = *EnemyEclInstr::GetVar(enemy, &args->bulletEffects.ivar1, NULL);
                enemy->bulletProps.exInts[1] = *EnemyEclInstr::GetVar(enemy, &args->bulletEffects.ivar2, NULL);
                enemy->bulletProps.exInts[2] = *EnemyEclInstr::GetVar(enemy, &args->bulletEffects.ivar3, NULL);
                enemy->bulletProps.exInts[3] = *EnemyEclInstr::GetVar(enemy, &args->bulletEffects.ivar4, NULL);
                enemy->bulletProps.exFloats[0] = *EnemyEclInstr::GetVarFloat(enemy, &args->bulletEffects.fvar1, NULL);
                enemy->bulletProps.exFloats[1] = *EnemyEclInstr::GetVarFloat(enemy, &args->bulletEffects.fvar2, NULL);
                enemy->bulletProps.exFloats[2] = *EnemyEclInstr::GetVarFloat(enemy, &args->bulletEffects.fvar3, NULL);
                enemy->bulletProps.exFloats[3] = *EnemyEclInstr::GetVarFloat(enemy, &args->bulletEffects.fvar4, NULL);
                break;
            case ECL_OPCODE_ANMSETDEATH:
                local_5c = &instruction->args.anmSetDeath;
                enemy->deathAnm1 = local_5c->deathAnm1;
                enemy->deathAnm2 = local_5c->deathAnm2;
                enemy->deathAnm3 = local_5c->deathAnm3;
                break;
            case ECL_OPCODE_SHOOTINTERVAL:
                enemy->shootInterval = instruction->args.setInt;
                enemy->shootInterval += enemy->ShootInterval(g_GameManager.rank);
                enemy->shootIntervalTimer = 0;
                break;
            case ECL_OPCODE_SHOOTINTERVALDELAYED:
                enemy->shootInterval = instruction->args.setInt;
                enemy->shootInterval += enemy->ShootInterval(g_GameManager.rank);
                if (enemy->shootInterval != 0)
                {
                    enemy->shootIntervalTimer = g_Rng.GetRandomU32InRange(enemy->shootInterval);
                }
                break;
            case ECL_OPCODE_SHOOTDISABLED:
                enemy->flags.shootingDisabled = true;
                break;
            case ECL_OPCODE_SHOOTENABLED:
                enemy->flags.shootingDisabled = false;
                break;
            case ECL_OPCODE_SHOOTNOW:
                enemy->bulletProps.position = enemy->position + enemy->shootOffset;
                g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
                break;
            case ECL_OPCODE_SHOOTOFFSET:
                enemy->shootOffset.x = *EnemyEclInstr::GetVarFloat(enemy, &args->move.pos.x, NULL);
                enemy->shootOffset.y = *EnemyEclInstr::GetVarFloat(enemy, &args->move.pos.y, NULL);
                enemy->shootOffset.z = *EnemyEclInstr::GetVarFloat(enemy, &args->move.pos.z, NULL);
                break;
            case ECL_OPCODE_LASERCREATE:
            case ECL_OPCODE_LASERCREATEAIMED:
                local_64 = &instruction->args.laser;
                local_60 = &enemy->laserProps;
                local_60->position = enemy->position + enemy->shootOffset;
                local_60->sprite = local_64->sprite;
                local_60->spriteOffset = local_64->color;
                local_60->angle = *EnemyEclInstr::GetVarFloat(enemy, &local_64->angle, NULL);
                local_60->speed = *EnemyEclInstr::GetVarFloat(enemy, &local_64->speed, NULL);
                local_60->startOffset = *EnemyEclInstr::GetVarFloat(enemy, &local_64->startOffset, NULL);
                local_60->endOffset = *EnemyEclInstr::GetVarFloat(enemy, &local_64->endOffset, NULL);
                local_60->startLength = *EnemyEclInstr::GetVarFloat(enemy, &local_64->startLength, NULL);
                local_60->width = local_64->width;
                local_60->startTime = local_64->startTime;
                local_60->duration = local_64->duration;
                local_60->despawnDuration = local_64->despawnDuration;
                local_60->hitboxStartTime = local_64->hitboxStartTime;
                local_60->hitboxEndDelay = local_64->hitboxEndDelay;
                local_60->flags = local_64->flags;
                if (instruction->opCode == ECL_OPCODE_LASERCREATEAIMED)
                {
                    local_60->type = 0;
                }
                else
                {
                    local_60->type = 1;
                }
                enemy->lasers[enemy->laserStore] = g_BulletManager.SpawnLaserPattern(local_60);
                break;
            case ECL_OPCODE_LASERINDEX:
                enemy->laserStore = *EnemyEclInstr::GetVar(enemy, &instruction->args.alu.res, NULL);
                break;
            case ECL_OPCODE_LASERROTATE:
                if (enemy->lasers[instruction->args.laserOp.laserIdx] != NULL)
                {
                    enemy->lasers[instruction->args.laserOp.laserIdx]->angle +=
                        *EnemyEclInstr::GetVarFloat(enemy, &instruction->args.laserOp.arg1.x, NULL);
                }
                break;
            case ECL_OPCODE_LASERROTATEFROMPLAYER:
                if (enemy->lasers[instruction->args.laserOp.laserIdx] != NULL)
                {
                    enemy->lasers[instruction->args.laserOp.laserIdx]->angle =
                        g_Player.AngleToPlayer(&enemy->lasers[instruction->args.laserOp.laserIdx]->pos) +
                        *EnemyEclInstr::GetVarFloat(enemy, &instruction->args.laserOp.arg1.x, NULL);
                }
                break;
            case ECL_OPCODE_LASEROFFSET:
                if (enemy->lasers[instruction->args.laserOp.laserIdx] != NULL)
                {
                    enemy->lasers[instruction->args.laserOp.laserIdx]->pos =
                        enemy->position + *instruction->args.laserOp.arg1.AsD3dXVec();
                }
                break;
            case ECL_OPCODE_LASERTEST:
                if (enemy->lasers[instruction->args.laserOp.laserIdx] != NULL &&
                    enemy->lasers[instruction->args.laserOp.laserIdx]->inUse)
                {
                    enemy->currentContext.compareRegister = 0;
                }
                else
                {
                    enemy->currentContext.compareRegister = 1;
                }
                break;
            case ECL_OPCODE_LASERCANCEL:
                if (enemy->lasers[instruction->args.laserOp.laserIdx] != NULL &&
                    enemy->lasers[instruction->args.laserOp.laserIdx]->inUse &&
                    enemy->lasers[instruction->args.laserOp.laserIdx]->state < 2)
                {
                    enemy->lasers[instruction->args.laserOp.laserIdx]->state = 2;
                    enemy->lasers[instruction->args.laserOp.laserIdx]->timer = 0;
                }
                break;
            case ECL_OPCODE_LASERCLEARALL:
                for (local_68 = 0; local_68 < ARRAY_SIZE_SIGNED(enemy->lasers); local_68++)
                {
                    enemy->lasers[local_68] = NULL;
                }
                break;
            case ECL_OPCODE_BOSSSET:
                if (instruction->args.setInt >= 0)
                {
                    g_EnemyManager.bosses[instruction->args.setInt] = enemy;
                    g_Gui.bossPresent = true;
                    g_Gui.SetBossHealthBar(1.0f);
                    enemy->flags.isBoss = true;
                    enemy->bossId = instruction->args.setInt;
                }
                else
                {
                    g_Gui.bossPresent = false;
                    g_EnemyManager.bosses[enemy->bossId] = NULL;
                    enemy->flags.isBoss = false;
                }
                break;
            case ECL_OPCODE_SPELLCARDEFFECT:
                local_6c = &instruction->args.spellcardEffect;
                enemy->effectArray[enemy->effectIdx] = g_EffectManager.SpawnParticles(
                    0xd, &enemy->position, 1, (ZunColor)g_EffectsColor[local_6c->effectColorId]);
                enemy->effectArray[enemy->effectIdx]->pos2 = *local_6c->pos.AsD3dXVec();
                enemy->effectDistance = local_6c->effectDistance;
                enemy->effectIdx++;
                break;
            case ECL_OPCODE_MOVEDIRTIMEDECELERATE:
                EnemyEclInstr::MoveDirTime(enemy, instruction);
                enemy->flags.movementEaseType = 1;
                break;
            case ECL_OPCODE_MOVEDIRTIMEDECELERATEFAST:
                EnemyEclInstr::MoveDirTime(enemy, instruction);
                enemy->flags.movementEaseType = 2;
                break;
            case ECL_OPCODE_MOVEDIRTIMEACCELERATE:
                EnemyEclInstr::MoveDirTime(enemy, instruction);
                enemy->flags.movementEaseType = 3;
                break;
            case ECL_OPCODE_MOVEDIRTIMEACCELERATEFAST:
                EnemyEclInstr::MoveDirTime(enemy, instruction);
                enemy->flags.movementEaseType = 4;
                break;
            case ECL_OPCODE_MOVEPOSITIONTIMELINEAR:
                EnemyEclInstr::MovePosTime(enemy, instruction);
                enemy->flags.movementEaseType = 0;
                break;
            case ECL_OPCODE_MOVEPOSITIONTIMEDECELERATE:
                EnemyEclInstr::MovePosTime(enemy, instruction);
                enemy->flags.movementEaseType = 1;
                break;
            case ECL_OPCODE_MOVEPOSITIONTIMEDECELERATEFAST:
                EnemyEclInstr::MovePosTime(enemy, instruction);
                enemy->flags.movementEaseType = 2;
                break;
            case ECL_OPCODE_MOVEPOSITIONTIMEACCELERATE:
                EnemyEclInstr::MovePosTime(enemy, instruction);
                enemy->flags.movementEaseType = 3;
                break;
            case ECL_OPCODE_MOVEPOSITIONTIMEACCELERATEFAST:
                EnemyEclInstr::MovePosTime(enemy, instruction);
                enemy->flags.movementEaseType = 4;
                break;
            case ECL_OPCODE_MOVETIMEDECELERATE:
                EnemyEclInstr::MoveTime(enemy, instruction);
                enemy->flags.movementEaseType = 1;
                break;
            case ECL_OPCODE_MOVETIMEDECELERATEFAST:
                EnemyEclInstr::MoveTime(enemy, instruction);
                enemy->flags.movementEaseType = 2;
                break;
            case ECL_OPCODE_MOVETIMEACCELERATE:
                EnemyEclInstr::MoveTime(enemy, instruction);
                enemy->flags.movementEaseType = 3;
                break;
            case ECL_OPCODE_MOVETIMEACCELERATEFAST:
                EnemyEclInstr::MoveTime(enemy, instruction);
                enemy->flags.movementEaseType = 4;
                break;
            case ECL_OPCODE_MOVEBOUNDSSET:
                enemy->lowerMoveLimit.x = instruction->args.moveBoundSet.lowerMoveLimit.x;
                enemy->lowerMoveLimit.y = instruction->args.moveBoundSet.lowerMoveLimit.y;
                enemy->upperMoveLimit.x = instruction->args.moveBoundSet.upperMoveLimit.x;
                enemy->upperMoveLimit.y = instruction->args.moveBoundSet.upperMoveLimit.y;
                enemy->flags.shouldClampPos = true;
                break;
            case ECL_OPCODE_MOVEBOUNDSDISABLE:
                enemy->flags.shouldClampPos = false;
                break;
            case ECL_OPCODE_MOVERAND:
                local_8 = instruction->args.move.pos;
                enemy->angle = g_Rng.GetRandomF32InRange(local_8.y - local_8.x) + local_8.x;
                break;
            case ECL_OPCODE_MOVERANDINBOUND:
                local_8 = instruction->args.move.pos;
                enemy->angle = g_Rng.GetRandomF32InRange(local_8.y - local_8.x) + local_8.x;
                if (enemy->position.x < enemy->lowerMoveLimit.x + 96.0f)
                {
                    if (enemy->angle > ZUN_PI / 2.0f)
                    {
                        enemy->angle = ZUN_PI - enemy->angle;
                    }
                    else if (enemy->angle < -ZUN_PI / 2.0f)
                    {
                        enemy->angle = -ZUN_PI - enemy->angle;
                    }
                }
                if (enemy->position.x > enemy->upperMoveLimit.x - 96.0f)
                {
                    if (enemy->angle < ZUN_PI / 2.0f && enemy->angle >= 0.0f)
                    {
                        enemy->angle = ZUN_PI - enemy->angle;
                    }
                    else if (enemy->angle > -ZUN_PI / 2.0f && enemy->angle <= 0.0f)
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
            case ECL_OPCODE_ANMSETPOSES:
                enemy->anmExDefaults = instruction->args.anmSetPoses.anmExDefault;
                enemy->anmExFarLeft = instruction->args.anmSetPoses.anmExFarLeft;
                enemy->anmExFarRight = instruction->args.anmSetPoses.anmExFarRight;
                enemy->anmExLeft = instruction->args.anmSetPoses.anmExLeft;
                enemy->anmExRight = instruction->args.anmSetPoses.anmExRight;
                enemy->anmExFlags = 0xff;
                break;
            case ECL_OPCODE_ENEMYSETHITBOX:
                enemy->hitboxDimensions.x = instruction->args.move.pos.x;
                enemy->hitboxDimensions.y = instruction->args.move.pos.y;
                enemy->hitboxDimensions.z = instruction->args.move.pos.z;
                break;
            case ECL_OPCODE_ENEMYFLAGCOLLISION:
                enemy->flags.isCollidable = instruction->args.setInt;
                break;
            case ECL_OPCODE_ENEMYFLAGCANTAKEDAMAGE:
                enemy->flags.isDamageable = instruction->args.setInt;
                break;
            case ECL_OPCODE_EFFECTSOUND:
                g_SoundPlayer.PlaySoundByIdx((SoundIdx)instruction->args.setInt, 0);
                break;
            case ECL_OPCODE_ENEMYFLAGDEATH:
                enemy->flags.deathMode = instruction->args.setInt;
                break;
            case ECL_OPCODE_DEATHCALLBACKSUB:
                enemy->deathCallbackSub = instruction->args.setInt;
                break;
            case ECL_OPCODE_ENEMYINTERRUPTSET:
                enemy->interrupts[args->setInterrupt.interruptId] = args->setInterrupt.interruptSub;
                break;
            case ECL_OPCODE_ENEMYINTERRUPT:
                enemy->runInterrupt = instruction->args.setInt;
            HANDLE_INTERRUPT:
                enemy->currentContext.currentInstr = (EclRawInstr *)((u8 *)instruction + instruction->offsetToNext);
                if (!enemy->flags.disableCallStack)
                {
                    enemy->savedContextStack[enemy->stackDepth] = enemy->currentContext;
                }
                g_EclManager.CallEclSub(&enemy->currentContext, enemy->interrupts[enemy->runInterrupt]);
                if (enemy->stackDepth < ARRAY_SIZE_SIGNED(enemy->savedContextStack) - 1)
                {
                    enemy->stackDepth++;
                }
                enemy->runInterrupt = -1;
                continue;
            case ECL_OPCODE_ENEMYLIFESET:
                enemy->life = enemy->maxLife = instruction->args.setInt;
                break;
            case ECL_OPCODE_SPELLCARDSTART:
                g_Gui.ShowSpellcard(instruction->args.spellcardStart.spellcardSprite,
                                    instruction->args.spellcardStart.spellcardName);
                g_EnemyManager.spellcardInfo.isCapturing = 1;
                g_EnemyManager.spellcardInfo.isActive = 1;
                g_EnemyManager.spellcardInfo.idx = instruction->args.spellcardStart.spellcardId;
                g_EnemyManager.spellcardInfo.captureScore = g_SpellcardScore[g_EnemyManager.spellcardInfo.idx];
                g_BulletManager.TurnAllBulletsIntoPoints();
                g_Stage.spellcardState = RUNNING;
                g_Stage.ticksSinceSpellcardStarted = 0;
                enemy->bulletRankSpeedLow = -0.5f;
                enemy->bulletRankSpeedHigh = 0.5f;
                enemy->bulletRankAmount1Low = 0;
                enemy->bulletRankAmount1High = 0;
                enemy->bulletRankAmount2Low = 0;
                enemy->bulletRankAmount2High = 0;
                local_70 = &g_GameManager.catk[g_EnemyManager.spellcardInfo.idx];
                csum = 0;
                if (!g_GameManager.isInReplay)
                {
                    strcpy(local_70->name, instruction->args.spellcardStart.spellcardName);
                    local_74 = strlen(local_70->name);
                    while (0 < local_74)
                    {
                        csum += local_70->name[--local_74];
                    }
                    if (local_70->nameCsum != (u8)csum)
                    {
                        local_70->numSuccess = 0;
                        local_70->numAttempts = 0;
                        local_70->nameCsum = csum;
                    }
                    local_70->captureScore = g_EnemyManager.spellcardInfo.captureScore;
                    if (local_70->numAttempts < 9999)
                    {
                        local_70->numAttempts++;
                    }
                }
                break;
            case ECL_OPCODE_SPELLCARDEND:
                if (g_EnemyManager.spellcardInfo.isActive)
                {
                    g_Gui.EndEnemySpellcard();
                    if (g_EnemyManager.spellcardInfo.isActive == 1)
                    {
                        scoreIncrease = g_BulletManager.DespawnBullets(12800, 1);
                        if (g_EnemyManager.spellcardInfo.isCapturing)
                        {
                            local_80 = &g_GameManager.catk[g_EnemyManager.spellcardInfo.idx];
                            local_88 = g_EnemyManager.spellcardInfo.captureScore >= 500000
                                           ? 500000 / 10
                                           : g_EnemyManager.spellcardInfo.captureScore / 10;
                            scoreIncrease =
                                g_EnemyManager.spellcardInfo.captureScore +
                                g_EnemyManager.spellcardInfo.captureScore * g_Gui.SpellcardSecondsRemaining() / 10;
                            g_Gui.ShowSpellcardBonus(scoreIncrease);
                            g_GameManager.score += scoreIncrease;
                            if (!g_GameManager.isInReplay)
                            {
                                local_80->numSuccess++;
                                for (local_84 = 4; 0 < local_84; local_84--)
                                {
                                    local_80->characterShotType[local_84] = local_80->characterShotType[local_84 - 1];
                                }
                                local_80->characterShotType[0] = g_GameManager.CharacterShotType();
                            }
                            g_GameManager.spellcardsCaptured++;
                        }
                    }
                    g_EnemyManager.spellcardInfo.isActive = 0;
                }
                g_Stage.spellcardState = NOT_RUNNING;
                break;
            case ECL_OPCODE_BOSSTIMERSET:
                enemy->bossTimer = instruction->args.setInt;
                break;
            case ECL_OPCODE_LIFECALLBACKTHRESHOLD:
                enemy->lifeCallbackThreshold = instruction->args.setInt;
                break;
            case ECL_OPCODE_LIFECALLBACKSUB:
                enemy->lifeCallbackSub = instruction->args.setInt;
                break;
            case ECL_OPCODE_TIMERCALLBACKTHRESHOLD:
                enemy->timerCallbackThreshold = instruction->args.setInt;
                enemy->bossTimer = 0;
                break;
            case ECL_OPCODE_TIMERCALLBACKSUB:
                enemy->timerCallbackSub = instruction->args.setInt;
                break;
            case ECL_OPCODE_ENEMYFLAGINTERACTABLE:
                enemy->flags.isInteractable = instruction->args.setInt;
                break;
            case ECL_OPCODE_EFFECTPARTICLE:
                g_EffectManager.SpawnParticles(instruction->args.effectParticle.effectId, &enemy->position,
                                               instruction->args.effectParticle.numParticles,
                                               instruction->args.effectParticle.particleColor);
                break;
            case ECL_OPCODE_DROPITEMS:
                for (local_8c = 0; local_8c < instruction->args.setInt; local_8c++)
                {
                    local_98 = enemy->position;

                    local_98[0] += g_Rng.GetRandomF32InRange(144.0f) - 72.0f;
                    local_98[1] += g_Rng.GetRandomF32InRange(144.0f) - 72.0f;
                    if (g_GameManager.currentPower < 128)
                    {
                        g_ItemManager.SpawnItem(&local_98, local_8c == 0 ? ITEM_POWER_BIG : ITEM_POWER_SMALL, 0);
                    }
                    else
                    {
                        g_ItemManager.SpawnItem(&local_98, ITEM_POINT, 0);
                    }
                }
                break;
            case ECL_OPCODE_ANMFLAGROTATION:
                enemy->flags.rotateAnm = instruction->args.setInt;
                break;
            case ECL_OPCODE_EXINSCALL:
                g_EclExInsn[instruction->args.setInt](enemy, instruction);
                break;
            case ECL_OPCODE_EXINSREPEAT:
                if (instruction->args.setInt >= 0)
                {
                    enemy->currentContext.funcSetFunc = g_EclExInsn[instruction->args.setInt];
                }
                else
                {
                    enemy->currentContext.funcSetFunc = NULL;
                }
                break;
            case ECL_OPCODE_TIMESET:
                enemy->currentContext.time += *EnemyEclInstr::GetVar(enemy, &instruction->args.timeSet.timeToSet, NULL);
                break;
            case ECL_OPCODE_DROPITEMID:
                g_ItemManager.SpawnItem(&enemy->position, instruction->args.dropItem.itemId, 0);
                break;
            case ECL_OPCODE_STDUNPAUSE:
                g_Stage.unpauseFlag = 1;
                break;
            case ECL_OPCODE_BOSSSETLIFECOUNT:
                g_Gui.eclSetLives = instruction->args.GetBossLifeCount();
                g_GameManager.counat += 1800;
                break;
            case ECL_OPCODE_ENEMYCREATE:
                local_b0 = instruction->args.enemyCreate;
                local_b0.pos.x = *EnemyEclInstr::GetVarFloat(enemy, &local_b0.pos.x, NULL);
                local_b0.pos.y = *EnemyEclInstr::GetVarFloat(enemy, &local_b0.pos.y, NULL);
                local_b0.pos.z = *EnemyEclInstr::GetVarFloat(enemy, &local_b0.pos.z, NULL);
                g_EnemyManager.SpawnEnemy(local_b0.subId, local_b0.pos.AsD3dXVec(), local_b0.life, local_b0.itemDrop,
                                          local_b0.score);
                break;
            case ECL_OPCODE_ENEMYKILLALL:
                for (local_b4 = &g_EnemyManager.enemies[0], local_b8 = 0;
                     local_b8 < ARRAY_SIZE_SIGNED(g_EnemyManager.enemies) - 1; local_b8++, local_b4++)
                {
                    if (!local_b4->flags.isSlotOccupied)
                    {
                        continue;
                    }
                    if (local_b4->flags.isBoss)
                    {
                        continue;
                    }

                    local_b4->life = 0;
                    if (!local_b4->flags.isInteractable && 0 <= local_b4->deathCallbackSub)
                    {
                        g_EclManager.CallEclSub(&local_b4->currentContext, local_b4->deathCallbackSub);
                        local_b4->deathCallbackSub = -1;
                    }
                }
                break;
            case ECL_OPCODE_ANMINTERRUPTMAIN:
                enemy->primaryVm.pendingInterrupt = instruction->args.setInt;
                break;
            case ECL_OPCODE_ANMINTERRUPTSLOT:
                enemy->vms[args->anmInterruptSlot.vmId].pendingInterrupt = args->anmInterruptSlot.interruptId;
                break;
            case ECL_OPCODE_BULLETCANCEL:
                g_BulletManager.TurnAllBulletsIntoPoints();
                break;
            case ECL_OPCODE_BULLETSOUND:
                if (instruction->args.bulletSound.bulletSfx >= 0)
                {
                    enemy->bulletProps.sfx = instruction->args.bulletSound.bulletSfx;
                    enemy->bulletProps.flags |= 0x200;
                }
                else
                {
                    enemy->bulletProps.flags &= ~0x200;
                }
                break;
            case ECL_OPCODE_ENEMYFLAGDISABLECALLSTACK:
                enemy->flags.disableCallStack = instruction->args.setInt;
                break;
            case ECL_OPCODE_BULLETRANKINFLUENCE:
                enemy->bulletRankSpeedLow = args->bulletRankInfluence.bulletRankSpeedLow;
                enemy->bulletRankSpeedHigh = args->bulletRankInfluence.bulletRankSpeedHigh;
                enemy->bulletRankAmount1Low = args->bulletRankInfluence.bulletRankAmount1Low;
                enemy->bulletRankAmount1High = args->bulletRankInfluence.bulletRankAmount1High;
                enemy->bulletRankAmount2Low = args->bulletRankInfluence.bulletRankAmount2Low;
                enemy->bulletRankAmount2High = args->bulletRankInfluence.bulletRankAmount2High;
                break;
            case ECL_OPCODE_ENEMYFLAGINVISIBLE:
                enemy->flags.isInvisible = instruction->args.setInt;
                break;
            case ECL_OPCODE_BOSSTIMERCLEAR:
                enemy->timerCallbackSub = enemy->deathCallbackSub;
                enemy->bossTimer = 0;
                break;
            case ECL_OPCODE_SPELLCARDFLAGTIMEOUT:
                enemy->flags.isTimeoutSpell = instruction->args.setInt;
                break;
            }
        NEXT_INSN:
            instruction = (EclRawInstr *)((u8 *)instruction + instruction->offsetToNext);
            goto YOLO;
        }
        else
        {
            switch (enemy->flags.movementMode)
            {
            case 1:
                enemy->angle = utils::AddNormalizeAngle(enemy->angle, g_Supervisor.effectiveFramerateMultiplier *
                                                                          enemy->angularVelocity);
                enemy->speed = g_Supervisor.effectiveFramerateMultiplier * enemy->acceleration + enemy->speed;
                sincosmul(&enemy->axisSpeed, enemy->angle, enemy->speed);
                enemy->axisSpeed.z = 0.0;
                break;
            case 2:
                enemy->moveInterpTimer--;
                local_bc = enemy->moveInterpTimer.AsFramesFloat() / enemy->moveInterpStartTime;
                if (local_bc >= 1.0f)
                {
                    local_bc = 1.0f;
                }
                switch (enemy->flags.movementEaseType)
                {
                case 0:
                    local_bc = 1.0f - local_bc;
                    break;
                case 1:
                    local_bc = 1.0f - local_bc * local_bc;
                    break;
                case 2:
                    local_bc = 1.0f - local_bc * local_bc * local_bc * local_bc;
                    break;
                case 3:
                    local_bc = 1.0f - local_bc;
                    local_bc *= local_bc;
                    break;
                case 4:
                    local_bc = 1.0f - local_bc;
                    local_bc = local_bc * local_bc * local_bc * local_bc;
                }
                enemy->axisSpeed = local_bc * enemy->moveInterp + enemy->moveInterpStartPos - enemy->position;
                enemy->angle = atan2f(enemy->axisSpeed.y, enemy->axisSpeed.x);
                if (enemy->moveInterpTimer <= 0)
                {
                    enemy->flags.movementMode = 0;
                    enemy->position = enemy->moveInterpStartPos + enemy->moveInterp;
                    enemy->axisSpeed = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                }
                break;
            }
            if (0 < enemy->life)
            {
                if (0 < enemy->shootInterval)
                {
                    enemy->shootIntervalTimer++;
                    if (enemy->shootIntervalTimer >= enemy->shootInterval)
                    {
                        enemy->bulletProps.position = enemy->position + enemy->shootOffset;
                        g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
                        enemy->shootIntervalTimer = 0;
                    }
                }
                if (0 <= enemy->anmExLeft)
                {
                    local_c0 = 0;
                    if (enemy->axisSpeed.x < 0.0f)
                    {
                        local_c0 = 1;
                    }
                    else if (enemy->axisSpeed.x > 0.0f)
                    {
                        local_c0 = 2;
                    }
                    if (enemy->anmExFlags != local_c0)
                    {
                        switch (local_c0)
                        {
                        case 0:
                            if (enemy->anmExFlags == 0xff)
                            {
                                g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                                     enemy->anmExDefaults + ANM_OFFSET_ENEMY);
                            }
                            else if (enemy->anmExFlags == 1)
                            {
                                g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                                     enemy->anmExFarLeft + ANM_OFFSET_ENEMY);
                            }
                            else
                            {
                                g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                                     enemy->anmExFarRight + ANM_OFFSET_ENEMY);
                            }
                            break;
                        case 1:
                            g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                                 enemy->anmExLeft + ANM_OFFSET_ENEMY);
                            break;
                        case 2:
                            g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                                 enemy->anmExRight + ANM_OFFSET_ENEMY);
                            break;
                        }
                        enemy->anmExFlags = local_c0;
                    }
                }
                if (enemy->currentContext.funcSetFunc != NULL)
                {
                    enemy->currentContext.funcSetFunc(enemy, NULL);
                }
            }
            enemy->currentContext.currentInstr = instruction;
            enemy->currentContext.time++;
            return ZUN_SUCCESS;
        }
    }
}
namespace EnemyEclInstr
{

#pragma var_order(alu, angle)
void MoveDirTime(Enemy *enemy, EclRawInstr *instr)
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

    enemy->flags.movementMode = 2;
}

void MovePosTime(Enemy *enemy, EclRawInstr *instr)
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

    enemy->flags.movementMode = 2;
    enemy->axisSpeed = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

void MoveTime(Enemy *enemy, EclRawInstr *instr)
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

    enemy->flags.movementMode = 2;
}

i32 *GetVar(Enemy *enemy, EclVarId *eclVarId, EclValueType *valueType)
{
    if (valueType != NULL)
        *valueType = ECL_VALUE_TYPE_UNDEFINED;

    switch (*eclVarId)
    {
    case ECL_VAR_I32_0:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.var0;

    case ECL_VAR_I32_1:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.var1;

    case ECL_VAR_I32_2:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.var2;

    case ECL_VAR_I32_3:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.var3;

    case ECL_VAR_F32_0:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->currentContext.float0;

    case ECL_VAR_F32_1:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->currentContext.float1;

    case ECL_VAR_F32_2:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->currentContext.float2;

    case ECL_VAR_F32_3:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->currentContext.float3;

    case ECL_VAR_I32_4:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.var4;

    case ECL_VAR_I32_5:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.var5;

    case ECL_VAR_I32_6:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.var6;

    case ECL_VAR_I32_7:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_INT;
        return &enemy->currentContext.var7;

    case ECL_VAR_DIFFICULTY:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return (int *)&g_GameManager.difficulty;

    case ECL_VAR_RANK:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return &g_GameManager.rank;

    case ECL_VAR_ENEMY_POS_X:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->position;

    case ECL_VAR_ENEMY_POS_Y:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->position.y;

    case ECL_VAR_ENEMY_POS_Z:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_FLOAT;
        return (i32 *)&enemy->position.z;

    case ECL_VAR_PLAYER_POS_X:
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return (i32 *)&g_Player.positionCenter;

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
        return &enemy->bossTimer.current;

    case ECL_VAR_PLAYER_DISTANCE:
        g_PlayerDistance = D3DXVec3Length(&(g_Player.positionCenter - enemy->position));
        if (valueType != NULL)
            *valueType = ECL_VALUE_TYPE_READONLY;
        return (i32 *)&g_PlayerDistance;

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

f32 *GetVarFloat(Enemy *enemy, f32 *eclVarId, EclValueType *valueType)
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
void SetVar(Enemy *enemy, EclVarId lhs, void *rhs)
{
    i32 *lhsPtr;
    EclValueType lhsType;
    i32 *rhsPtr;

    rhsPtr = GetVar(enemy, (EclVarId *)rhs, NULL);
    lhsPtr = GetVar(enemy, &lhs, &lhsType);
    if (lhsType == ECL_VALUE_TYPE_INT)
    {
        *lhsPtr = *rhsPtr;
    }
    else if (lhsType == ECL_VALUE_TYPE_FLOAT)
    {
        *(f32 *)lhsPtr = *(f32 *)rhsPtr;
    }
    return;
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
void MathAdd(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
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
    return;
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
void MathSub(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
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
    return;
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
void MathMul(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
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
    return;
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
void MathDiv(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
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
    return;
}

#pragma var_order(outPtr, rhsPtr, lhsPtr, outType)
void MathMod(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId, EclVarId *rhsVarId)
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
    return;
}

#pragma var_order(y2Ptr, outPtr, x1Ptr, y1Ptr, outType, x2Ptr)
void MathAtan2(Enemy *enemy, EclVarId outVarId, f32 *x1, f32 *y1, f32 *y2, f32 *x2)
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
    return;
}
}; // namespace EnemyEclInstr
}; // namespace th06
