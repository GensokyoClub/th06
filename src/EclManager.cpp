#include "EclManager.hpp"
#include "AnmManager.hpp"
#include "EffectManager.hpp"
#include "Enemy.hpp"
#include "EnemyEclInstr.hpp"
#include "EnemyManager.hpp"
#include "FileSystem.hpp"
#include "GameErrorContext.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "Rng.hpp"
#include "Stage.hpp"
#include "utils.hpp"
#include <cstdlib>

i32 g_SpellcardScore[64] = {
    200000, 200000, 200000, 200000, 200000, 200000, 200000, 250000,
    250000, 250000, 250000, 250000, 250000, 250000, 300000, 300000,
    300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000,
    300000, 300000, 300000, 300000, 300000, 300000, 300000, 300000,
    400000, 400000, 400000, 400000, 400000, 400000, 400000, 400000,
    500000, 500000, 500000, 500000, 500000, 500000, 600000, 600000,
    600000, 600000, 600000, 700000, 700000, 700000, 700000, 700000,
    700000, 700000, 700000, 700000, 700000, 700000, 700000, 700000
};

EclManager g_EclManager;
typedef void (*ExInsn)(Enemy *, EclRawInstr *);

ExInsn g_EclExInsn[17] = {
    EnemyEclInstr::ExInsCirnoRainbowBallJank,
    EnemyEclInstr::ExInsShootAtRandomArea,
    EnemyEclInstr::ExInsShootStarPattern,
    EnemyEclInstr::ExInsPatchouliShottypeSetVars,
    EnemyEclInstr::ExInsStage56Func4,
    EnemyEclInstr::ExInsStage5Func5,
    EnemyEclInstr::ExInsStage6XFunc6,
    EnemyEclInstr::ExInsStage6Func7,
    EnemyEclInstr::ExInsStage6Func8,
    EnemyEclInstr::ExInsStage6Func9,
    EnemyEclInstr::ExInsStage6XFunc10,
    EnemyEclInstr::ExInsStage6Func11,
    EnemyEclInstr::ExInsStage4Func12,
    EnemyEclInstr::ExInsStageXFunc13,
    EnemyEclInstr::ExInsStageXFunc14,
    EnemyEclInstr::ExInsStageXFunc15,
    EnemyEclInstr::ExInsStageXFunc16
};

bool EclManager::Load(const char *eclPath) {
    i32 idx;

#ifdef TRUTH_FFI_INTEGRATION
    this->truthBuffer = truth_compile_ecl("6", eclPath, "th06.eclm");
    this->eclFile = (EclRawHeader *)this->truthBuffer.data;
#else
    this->eclFile = (EclRawHeader *)FileSystem::OpenPath(eclPath);
#endif

    if (this->eclFile == NULL) {
        GameErrorContext::Log(&g_GameErrorContext, TH_ERR_ECLMANAGER_ENEMY_DATA_CORRUPT);
        return false;
    }

    this->timelinePtrs[0] = (EclTimelineInstr *)(((u8 *)this->eclFile) + this->eclFile->timelineOffsets[0]);

    this->subTable = (EclRawInstr **)malloc(sizeof(EclRawInstr *) * this->eclFile->subCount);

    for (idx = 0; idx < this->eclFile->subCount; idx++) {
        this->subTable[idx] = (EclRawInstr *)(((u8 *)this->eclFile) + this->eclFile->subOffsets[idx]);
    }

    this->timeline = this->timelinePtrs[0];
    return true;
}

void EclManager::Unload() {
    EclRawHeader *file;

    #ifdef TRUTH_FFI_INTEGRATION
        truth_buffer_free(this->truthBuffer);
        this->truthBuffer = {};
        this->eclFile = NULL;
    #else
        free(this->eclFile);
        this->eclFile = NULL;
    #endif

    free(this->subTable);
    this->subTable = NULL;

    return;
}

bool EclManager::CallEclSub(EnemyEclContext *ctx, i16 subId) {
    ctx->currentInstr = this->subTable[subId];
    ctx->time.InitializeForPopup();
    ctx->subId = subId;
    return true;
}

bool EclManager::RunEcl(Enemy *enemy) {
    EclRawInstr *instruction;
    EclRawInstrArgs *args;
    ZunVec3 move_pos;
    i32 local_14, local_44,
        local_48, local_68, local_74, csum, scoreIncrease, local_c0;
    f32 local_18, local_bc;
    Catk *catk1, *catk2;

    for (;;) {
        instruction = enemy->currentContext.currentInstr;
        if (0 <= enemy->runInterrupt) {
            goto HANDLE_INTERRUPT;
        }

    PARSE_INSTRUCTION:
        if (enemy->currentContext.time.current == instruction->time) {
            if (!(instruction->skipForDifficulty &
                  (1 << g_GameManager.difficulty))) {
                goto NEXT_INSN;
            }

            args = &instruction->args;
            switch (instruction->opCode) {
            case ECL_OPCODE_UNIMP:
                return false;
            case ECL_OPCODE_JUMPDEC:
                local_14 = *EnemyEclInstr::GetVar(enemy, &args->jump.var, NULL);
                local_14--;
                EnemyEclInstr::SetVar(enemy, args->jump.var, &local_14);
                if (local_14 <= 0)
                    break;
            case ECL_OPCODE_JUMP:
            HANDLE_JUMP:
                enemy->currentContext.time.current = instruction->args.jump.time;
                instruction = (EclRawInstr *)(((u8 *)instruction) + args->jump.offset);
                goto PARSE_INSTRUCTION;
            case ECL_OPCODE_SETINT:
            case ECL_OPCODE_SETFLOAT:
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &args->alu.arg1.i32Param);
                break;
            case ECL_OPCODE_MATHNORMANGLE:
                local_18 = *(f32 *)EnemyEclInstr::GetVar(enemy, &instruction->args.alu.res, NULL);
                local_18 = utils::AddNormalizeAngle(local_18, 0.0f);
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &local_18);
                break;
            case ECL_OPCODE_SETINTRAND: {
                i32 range = *EnemyEclInstr::GetVar(enemy, &args->alu.arg1.id, NULL);
                local_14 = g_Rng.GetRandomU32InRange(range);
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &local_14);
                break;
            }
            case ECL_OPCODE_SETINTRANDMIN: {
                i32 range = *EnemyEclInstr::GetVar(enemy, &args->alu.arg1.id, NULL);
                i32 idk = *EnemyEclInstr::GetVar(enemy, &args->alu.arg2.id, NULL);
                local_14 = g_Rng.GetRandomU32InRange(range);
                local_14 += idk;
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &local_14);
                break;
            }
            case ECL_OPCODE_SETFLOATRAND: {
                f32 range = *EnemyEclInstr::GetVarFloat(enemy, &args->alu.arg1.f32Param, NULL);
                local_18 = g_Rng.GetRandomF32InRange(range);
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &local_18);
                break;
            }
            case ECL_OPCODE_SETFLOATRANDMIN: {
                f32 range = *EnemyEclInstr::GetVarFloat(enemy, &args->alu.arg1.f32Param, NULL);
                f32 float1 = *EnemyEclInstr::GetVarFloat(enemy, &args->alu.arg2.f32Param, NULL);
                local_18 = g_Rng.GetRandomF32InRange(range);
                local_18 += float1;
                EnemyEclInstr::SetVar(enemy, instruction->args.alu.res, &local_18);
                break;
            }
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
            case ECL_OPCODE_MATHINC: {
                i32 *alu_val = EnemyEclInstr::GetVar(enemy, &instruction->args.alu.res, NULL);
                *alu_val += 1;
                break;
            }
            case ECL_OPCODE_MATHDEC: {
                i32 *alu_val = EnemyEclInstr::GetVar(enemy, &instruction->args.alu.res, NULL);
                *alu_val -= 1;
                break;
            }
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
                EnemyEclInstr::MathAtan2(
                    enemy,
                    instruction->args.alu.res,
                    &args->alu.arg1.f32Param,
                    &args->alu.arg2.f32Param,
                    &args->alu.arg3.f32Param,
                    &args->alu.arg4.f32Param
                );
                break;
            case ECL_OPCODE_CMPINT:
                local_48 = *EnemyEclInstr::GetVar(enemy, &instruction->args.cmp.lhs.id, NULL);
                local_44 = *EnemyEclInstr::GetVar(enemy, &instruction->args.cmp.rhs.id, NULL);
                enemy->currentContext.compareRegister = local_48 == local_44 ? 0 : local_48 < local_44 ? -1 : 1;
                break;
            case ECL_OPCODE_CMPFLOAT: {
                f32 float1 = *EnemyEclInstr::GetVarFloat(enemy, &instruction->args.cmp.lhs.f32Param, NULL);
                f32 float2 = *EnemyEclInstr::GetVarFloat(enemy, &instruction->args.cmp.rhs.f32Param, NULL);
                enemy->currentContext.compareRegister = float1 == float2 ? 0 : (float1 < float2 ? -1 : 1);
                break;
            }
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
                if (enemy->flags.unk14 == 0) {
                    memcpy(&enemy->savedContextStack[enemy->stackDepth], &enemy->currentContext, sizeof(EnemyEclContext));
                }
                g_EclManager.CallEclSub(&enemy->currentContext, (u16)local_14);
                if (enemy->flags.unk14 == 0 && enemy->stackDepth < 7) {
                    enemy->stackDepth++;
                }
                enemy->currentContext.var0 = instruction->args.call.var0;
                enemy->currentContext.float0 = instruction->args.call.float0;
                continue;
            case ECL_OPCODE_RET:
                if (enemy->flags.unk14) {
                    utils::DebugPrint2("error : no Stack Ret\n");
                }
                enemy->stackDepth--;
                memcpy(&enemy->currentContext, &enemy->savedContextStack[enemy->stackDepth], sizeof(EnemyEclContext));
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
                g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, instruction->args.anmSetMain.scriptIdx + ANM_SCRIPT_ENEMY_START);
                break;
            case ECL_OPCODE_ANMSETSLOT:
                if (ARRAY_SIZE_SIGNED(enemy->vms) <=
                    instruction->args.anmSetSlot.vmIdx) {
                    utils::DebugPrint2("error : sub anim overflow\n");
                }
                g_AnmManager->SetAndExecuteScriptIdx(&enemy->vms[instruction->args.anmSetSlot.vmIdx], args->anmSetSlot.scriptIdx + ANM_SCRIPT_ENEMY_START);
                break;
            case ECL_OPCODE_MOVEPOSITION:
                enemy->position = instruction->args.move.pos;
                enemy->position.x = *EnemyEclInstr::GetVarFloat(enemy, &enemy->position.x, NULL);
                enemy->position.y = *EnemyEclInstr::GetVarFloat(enemy, &enemy->position.y, NULL);
                enemy->position.z = *EnemyEclInstr::GetVarFloat(enemy, &enemy->position.z, NULL);
                enemy->ClampPos();
                break;
            case ECL_OPCODE_MOVEAXISVELOCITY:
                enemy->axisSpeed = instruction->args.move.pos;
                enemy->axisSpeed.x = *EnemyEclInstr::GetVarFloat(enemy, &enemy->axisSpeed.x, NULL);
                enemy->axisSpeed.y = *EnemyEclInstr::GetVarFloat(enemy, &enemy->axisSpeed.y, NULL);
                enemy->axisSpeed.z = *EnemyEclInstr::GetVarFloat(enemy, &enemy->axisSpeed.z, NULL);
                enemy->flags.unk1 = 0;
                break;
            case ECL_OPCODE_MOVEVELOCITY:
                move_pos = instruction->args.move.pos;
                enemy->angle = *EnemyEclInstr::GetVarFloat(enemy, &move_pos.x, NULL);
                enemy->speed = *EnemyEclInstr::GetVarFloat(enemy, &move_pos.y, NULL);
                enemy->flags.unk1 = 1;
                break;
            case ECL_OPCODE_MOVEANGULARVELOCITY:
                move_pos = instruction->args.move.pos;
                enemy->angularVelocity = *EnemyEclInstr::GetVarFloat(enemy, &move_pos.x, NULL);
                enemy->flags.unk1 = 1;
                break;
            case ECL_OPCODE_MOVEATPLAYER:
                move_pos = instruction->args.move.pos;
                enemy->angle = g_Player.AngleToPlayer(&enemy->position) + move_pos.x;
                enemy->speed = *EnemyEclInstr::GetVarFloat(enemy, &move_pos.y, NULL);
                enemy->flags.unk1 = 1;
                break;
            case ECL_OPCODE_MOVESPEED:
                move_pos = instruction->args.move.pos;
                enemy->speed = *EnemyEclInstr::GetVarFloat(enemy, &move_pos.x, NULL);
                enemy->flags.unk1 = 1;
                break;
            case ECL_OPCODE_MOVEACCELERATION:
                move_pos = instruction->args.move.pos;
                enemy->acceleration = *EnemyEclInstr::GetVarFloat(enemy, &move_pos.x, NULL);
                enemy->flags.unk1 = 1;
                break;
            case ECL_OPCODE_BULLETFANAIMED:
            case ECL_OPCODE_BULLETFAN:
            case ECL_OPCODE_BULLETCIRCLEAIMED:
            case ECL_OPCODE_BULLETCIRCLE:
            case ECL_OPCODE_BULLETOFFSETCIRCLEAIMED:
            case ECL_OPCODE_BULLETOFFSETCIRCLE:
            case ECL_OPCODE_BULLETRANDOMANGLE:
            case ECL_OPCODE_BULLETRANDOMSPEED:
            case ECL_OPCODE_BULLETRANDOM: {
                EclRawInstrBulletArgs *bullet_args;
                EnemyBulletShooter *bullet_shooter;
                bullet_args = &instruction->args.bullet;
                bullet_shooter = &enemy->bulletProps;
                bullet_shooter->sprite = bullet_args->sprite;
                bullet_shooter->aimMode = instruction->opCode - ECL_OPCODE_BULLETFANAIMED;
                bullet_shooter->count1 = *EnemyEclInstr::GetVar(enemy, &bullet_args->count1, NULL);
                bullet_shooter->count1 += enemy->BulletRankAmount1(g_GameManager.rank);
                if (bullet_shooter->count1 <= 0) {
                    bullet_shooter->count1 = 1;
                }

                bullet_shooter->count2 = *EnemyEclInstr::GetVar(enemy, &bullet_args->count2, NULL);
                bullet_shooter->count2 += enemy->BulletRankAmount2(g_GameManager.rank);
                if (bullet_shooter->count2 <= 0) {
                    bullet_shooter->count2 = 1;
                }
                bullet_shooter->position = enemy->position + enemy->shootOffset;
                bullet_shooter->angle1 = *EnemyEclInstr::GetVarFloat(enemy, &bullet_args->angle1, NULL);
                bullet_shooter->angle1 = utils::AddNormalizeAngle(bullet_shooter->angle1, 0.0f);
                bullet_shooter->speed1 = *EnemyEclInstr::GetVarFloat(enemy, &bullet_args->speed1, NULL);
                if (bullet_shooter->speed1 != 0.0f) {
                    bullet_shooter->speed1 += enemy->BulletRankSpeed(g_GameManager.rank);
                    if (bullet_shooter->speed1 < 0.3f) {
                        bullet_shooter->speed1 = 0.3;
                    }
                }
                bullet_shooter->angle2 = *EnemyEclInstr::GetVarFloat(enemy, &bullet_args->angle2, NULL);
                bullet_shooter->speed2 = *EnemyEclInstr::GetVarFloat(enemy, &bullet_args->speed2, NULL);
                bullet_shooter->speed2 += enemy->BulletRankSpeed(g_GameManager.rank) / 2.0f;
                if (bullet_shooter->speed2 < 0.3f) {
                    bullet_shooter->speed2 = 0.3f;
                }
                bullet_shooter->unk_4a = 0;
                bullet_shooter->flags = bullet_args->flags;
                local_14 = bullet_args->color;
                // TODO: Strict aliasing rule be like.
                bullet_shooter->spriteOffset = *EnemyEclInstr::GetVar(enemy, (EclVarId *)&local_14, NULL);
                if (enemy->flags.unk3 == 0) {
                    g_BulletManager.SpawnBulletPattern(bullet_shooter);
                }
                break;
            }
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
            case ECL_OPCODE_ANMSETDEATH: {
                EclRawInstrAnmSetDeathArgs *death_args = &instruction->args.anmSetDeath;
                enemy->deathAnm1 = death_args->deathAnm1;
                enemy->deathAnm2 = death_args->deathAnm2;
                enemy->deathAnm3 = death_args->deathAnm3;
                break;
            }
            case ECL_OPCODE_SHOOTINTERVAL:
                enemy->shootInterval = instruction->args.setInt;
                enemy->shootInterval += enemy->ShootInterval(g_GameManager.rank);
                enemy->shootIntervalTimer.SetCurrent(0);
                break;
            case ECL_OPCODE_SHOOTINTERVALDELAYED:
                enemy->shootInterval = instruction->args.setInt;
                enemy->shootInterval += enemy->ShootInterval(g_GameManager.rank);
                if (enemy->shootInterval != 0) {
                    enemy->shootIntervalTimer.SetCurrent(g_Rng.GetRandomU32InRange(enemy->shootInterval));
                }
                break;
            case ECL_OPCODE_SHOOTDISABLED:
                enemy->flags.unk3 = 1;
                break;
            case ECL_OPCODE_SHOOTENABLED:
                enemy->flags.unk3 = 0;
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
            case ECL_OPCODE_LASERCREATEAIMED: {
                EnemyLaserShooter *laser_shooter;
                EclRawInstrLaserArgs *laser_args = &instruction->args.laser;

                laser_shooter = &enemy->laserProps;
                laser_shooter->position = enemy->position + enemy->shootOffset;
                laser_shooter->sprite = laser_args->sprite;
                laser_shooter->spriteOffset = laser_args->color;
                laser_shooter->angle = *EnemyEclInstr::GetVarFloat(enemy, &laser_args->angle, NULL);
                laser_shooter->speed = *EnemyEclInstr::GetVarFloat(enemy, &laser_args->speed, NULL);
                laser_shooter->startOffset = *EnemyEclInstr::GetVarFloat(enemy, &laser_args->startOffset, NULL);
                laser_shooter->endOffset = *EnemyEclInstr::GetVarFloat(enemy, &laser_args->endOffset, NULL);
                laser_shooter->startLength = *EnemyEclInstr::GetVarFloat(enemy, &laser_args->startLength, NULL);
                laser_shooter->width = laser_args->width;
                laser_shooter->startTime = laser_args->startTime;
                laser_shooter->duration = laser_args->duration;
                laser_shooter->stopTime = laser_args->stopTime;
                laser_shooter->grazeDelay = laser_args->grazeDelay;
                laser_shooter->grazeDistance = laser_args->grazeDistance;
                laser_shooter->flags = laser_args->flags;
                if (instruction->opCode == ECL_OPCODE_LASERCREATEAIMED) {
                    laser_shooter->type = 0;
                } else {
                    laser_shooter->type = 1;
                }
                enemy->lasers[enemy->laserStore] = g_BulletManager.SpawnLaserPattern(laser_shooter);
                break;
            }
            case ECL_OPCODE_LASERINDEX:
                enemy->laserStore = *EnemyEclInstr::GetVar(enemy, &instruction->args.alu.res, NULL);
                break;
            case ECL_OPCODE_LASERROTATE:
                if (enemy->lasers[instruction->args.laserOp.laserIdx] != NULL) {
                    enemy->lasers[instruction->args.laserOp.laserIdx]->angle += *EnemyEclInstr::GetVarFloat(enemy, &instruction->args.laserOp.arg1.x, NULL);
                }
                break;
            case ECL_OPCODE_LASERROTATEFROMPLAYER:
                if (enemy->lasers[instruction->args.laserOp.laserIdx] != NULL) {
                    enemy->lasers[instruction->args.laserOp.laserIdx]->angle =
                        g_Player.AngleToPlayer(&enemy->lasers[instruction->args.laserOp.laserIdx]->pos) +
                        *EnemyEclInstr::GetVarFloat(enemy, &instruction->args.laserOp.arg1.x, NULL);
                }
                break;
            case ECL_OPCODE_LASEROFFSET:
                if (enemy->lasers[instruction->args.laserOp.laserIdx] != NULL) {
                    enemy->lasers[instruction->args.laserOp.laserIdx]->pos = enemy->position + instruction->args.laserOp.arg1;
                }
                break;
            case ECL_OPCODE_LASERTEST:
                if (enemy->lasers[instruction->args.laserOp.laserIdx] != NULL &&
                    enemy->lasers[instruction->args.laserOp.laserIdx]->inUse) {
                    enemy->currentContext.compareRegister = 0;
                } else {
                    enemy->currentContext.compareRegister = 1;
                }
                break;
            case ECL_OPCODE_LASERCANCEL:
                if (enemy->lasers[instruction->args.laserOp.laserIdx] != NULL &&
                    enemy->lasers[instruction->args.laserOp.laserIdx]->inUse &&
                    enemy->lasers[instruction->args.laserOp.laserIdx]->state < 2) {
                        enemy->lasers[instruction->args.laserOp.laserIdx]->state = 2;
                        enemy->lasers[instruction->args.laserOp.laserIdx]
                            ->timer.SetCurrent(0);
                }
                break;
            case ECL_OPCODE_LASERCLEARALL:
                for (local_68 = 0; local_68 < ARRAY_SIZE_SIGNED(enemy->lasers); local_68++) {
                    enemy->lasers[local_68] = NULL;
                }
                break;
            case ECL_OPCODE_BOSSSET:
                if (instruction->args.setInt >= 0) {
                    g_EnemyManager.bosses[instruction->args.setInt] = enemy;
                    g_Gui.bossPresent = 1;
                    g_Gui.SetBossHealthBar(1.0f);
                    enemy->flags.isBoss = 1;
                    enemy->bossId = instruction->args.setInt;
                } else {
                    g_Gui.bossPresent = 0;
                    g_EnemyManager.bosses[enemy->bossId] = NULL;
                    enemy->flags.isBoss = 0;
                }
                break;
            case ECL_OPCODE_SPELLCARDEFFECT: {
                EclRawInstrSpellcardEffectArgs *spellcard_eff_args = &instruction->args.spellcardEffect;
                enemy->effectArray[enemy->effectIdx] = g_EffectManager.SpawnParticles(0xd, &enemy->position, 1, (ZunColor)g_EffectsColor[spellcard_eff_args->effectColorId]);
                enemy->effectArray[enemy->effectIdx]->pos2 = spellcard_eff_args->pos;
                enemy->effectDistance = spellcard_eff_args->effectDistance;
                enemy->effectIdx++;
                break;
            }
            case ECL_OPCODE_MOVEDIRTIMEDECELERATE:
                EnemyEclInstr::MoveDirTime(enemy, instruction);
                enemy->flags.unk2 = 1;
                break;
            case ECL_OPCODE_MOVEDIRTIMEDECELERATEFAST:
                EnemyEclInstr::MoveDirTime(enemy, instruction);
                enemy->flags.unk2 = 2;
                break;
            case ECL_OPCODE_MOVEDIRTIMEACCELERATE:
                EnemyEclInstr::MoveDirTime(enemy, instruction);
                enemy->flags.unk2 = 3;
                break;
            case ECL_OPCODE_MOVEDIRTIMEACCELERATEFAST:
                EnemyEclInstr::MoveDirTime(enemy, instruction);
                enemy->flags.unk2 = 4;
                break;
            case ECL_OPCODE_MOVEPOSITIONTIMELINEAR:
                EnemyEclInstr::MovePosTime(enemy, instruction);
                enemy->flags.unk2 = 0;
                break;
            case ECL_OPCODE_MOVEPOSITIONTIMEDECELERATE:
                EnemyEclInstr::MovePosTime(enemy, instruction);
                enemy->flags.unk2 = 1;
                break;
            case ECL_OPCODE_MOVEPOSITIONTIMEDECELERATEFAST:
                EnemyEclInstr::MovePosTime(enemy, instruction);
                enemy->flags.unk2 = 2;
                break;
            case ECL_OPCODE_MOVEPOSITIONTIMEACCELERATE:
                EnemyEclInstr::MovePosTime(enemy, instruction);
                enemy->flags.unk2 = 3;
                break;
            case ECL_OPCODE_MOVEPOSITIONTIMEACCELERATEFAST:
                EnemyEclInstr::MovePosTime(enemy, instruction);
                enemy->flags.unk2 = 4;
                break;
            case ECL_OPCODE_MOVETIMEDECELERATE:
                EnemyEclInstr::MoveTime(enemy, instruction);
                enemy->flags.unk2 = 1;
                break;
            case ECL_OPCODE_MOVETIMEDECELERATEFAST:
                EnemyEclInstr::MoveTime(enemy, instruction);
                enemy->flags.unk2 = 2;
                break;
            case ECL_OPCODE_MOVETIMEACCELERATE:
                EnemyEclInstr::MoveTime(enemy, instruction);
                enemy->flags.unk2 = 3;
                break;
            case ECL_OPCODE_MOVETIMEACCELERATEFAST:
                EnemyEclInstr::MoveTime(enemy, instruction);
                enemy->flags.unk2 = 4;
                break;
            case ECL_OPCODE_MOVEBOUNDSSET:
                enemy->lowerMoveLimit.x = instruction->args.moveBoundSet.lowerMoveLimit.x;
                enemy->lowerMoveLimit.y = instruction->args.moveBoundSet.lowerMoveLimit.y;
                enemy->upperMoveLimit.x = instruction->args.moveBoundSet.upperMoveLimit.x;
                enemy->upperMoveLimit.y = instruction->args.moveBoundSet.upperMoveLimit.y;
                enemy->flags.shouldClampPos = 1;
                break;
            case ECL_OPCODE_MOVEBOUNDSDISABLE:
                enemy->flags.shouldClampPos = 0;
                break;
            case ECL_OPCODE_MOVERAND:
                move_pos = instruction->args.move.pos;
                enemy->angle = g_Rng.GetRandomF32InRange(move_pos.y - move_pos.x) + move_pos.x;
                break;
            case ECL_OPCODE_MOVERANDINBOUND:
                move_pos = instruction->args.move.pos;
                enemy->angle = g_Rng.GetRandomF32InRange(move_pos.y - move_pos.x) + move_pos.x;
                if (enemy->position.x < enemy->lowerMoveLimit.x + 96.0f) {
                    if (enemy->angle > ZUN_PI / 2.0f) {
                        enemy->angle = ZUN_PI - enemy->angle;
                    } else if (enemy->angle < -ZUN_PI / 2.0f) {
                        enemy->angle = -ZUN_PI - enemy->angle;
                    }
                }
                if (enemy->position.x > enemy->upperMoveLimit.x - 96.0f) {
                    if (enemy->angle < ZUN_PI / 2.0f && enemy->angle >= 0.0f) {
                        enemy->angle = ZUN_PI - enemy->angle;
                    } else if (enemy->angle > -ZUN_PI / 2.0f && enemy->angle <= 0.0f) {
                        enemy->angle = -ZUN_PI - enemy->angle;
                    }
                }
                if (enemy->position.y < enemy->lowerMoveLimit.y + 48.0f &&
                    enemy->angle < 0.0f) {
                    enemy->angle = -enemy->angle;
                }
                if (enemy->position.y > enemy->upperMoveLimit.y - 48.0f &&
                    enemy->angle > 0.0f) {
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
                enemy->flags.unk7 = instruction->args.setInt;
                break;
            case ECL_OPCODE_ENEMYFLAGCANTAKEDAMAGE:
                enemy->flags.unk10 = instruction->args.setInt;
                break;
            case ECL_OPCODE_EFFECTSOUND:
                g_SoundPlayer.PlaySoundByIdx((SoundIdx)instruction->args.setInt);
                break;
            case ECL_OPCODE_ENEMYFLAGDEATH:
                enemy->flags.unk11 = instruction->args.setInt;
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
                if (enemy->flags.unk14 == 0) {
                    memcpy(&enemy->savedContextStack[enemy->stackDepth], &enemy->currentContext, sizeof(EnemyEclContext));
                }
                g_EclManager.CallEclSub(&enemy->currentContext, enemy->interrupts[enemy->runInterrupt]);
                if (enemy->stackDepth < ARRAY_SIZE_SIGNED(enemy->savedContextStack) - 1) {
                    enemy->stackDepth++;
                }
                enemy->runInterrupt = -1;
                continue;
            case ECL_OPCODE_ENEMYLIFESET:
                enemy->life = enemy->maxLife = instruction->args.setInt;
                break;
            case ECL_OPCODE_SPELLCARDSTART:
                g_Gui.ShowSpellcard(instruction->args.spellcardStart.spellcardSprite, instruction->args.spellcardStart.spellcardName);
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
                catk1 = &g_GameManager.catk[g_EnemyManager.spellcardInfo.idx];
                csum = 0;
                if (!g_GameManager.isInReplay) {
                    strcpy(catk1->name, instruction->args.spellcardStart.spellcardName);
                    local_74 = strlen(catk1->name);
                    while (0 < local_74) {
                        csum += catk1->name[--local_74];
                    }
                    if (catk1->nameCsum != (u8)csum) {
                        catk1->numSuccess = 0;
                        catk1->numAttempts = 0;
                        catk1->nameCsum = csum;
                    }
                    catk1->captureScore = g_EnemyManager.spellcardInfo.captureScore;
                    if (catk1->numAttempts < 9999) {
                        catk1->numAttempts++;
                    }
                }
                break;
            case ECL_OPCODE_SPELLCARDEND: {
                if (g_EnemyManager.spellcardInfo.isActive != 0) {
                    g_Gui.EndEnemySpellcard();
                    if (g_EnemyManager.spellcardInfo.isActive == 1) {
                        scoreIncrease = g_BulletManager.DespawnBullets(12800, 1);
                        if (g_EnemyManager.spellcardInfo.isCapturing) {
                            catk2 = &g_GameManager.catk[g_EnemyManager.spellcardInfo.idx];
                            // This goes completely unused?
                            i32 cap_score = g_EnemyManager.spellcardInfo.captureScore >= 500000
                                    ? 500000 / 10
                                    : g_EnemyManager.spellcardInfo.captureScore / 10;
                            scoreIncrease = g_EnemyManager.spellcardInfo.captureScore + g_EnemyManager.spellcardInfo.captureScore * g_Gui.SpellcardSecondsRemaining() / 10;
                            g_Gui.ShowSpellcardBonus(scoreIncrease);
                            g_GameManager.score += scoreIncrease;
                            if (!g_GameManager.isInReplay) {
                                catk2->numSuccess++;
                                // What. the. fuck?
                                // memmove(&local_80->nameCsum,
                                // &local_80->characterShotType, 4);
                                for (i32 i = 4; 0 < i; i = i + -1) {
                                    ((u8 *)&catk2->nameCsum)[i + 1] = ((u8 *)&catk2->nameCsum)[i];
                                }
                                catk2->characterShotType = g_GameManager.CharacterShotType();
                            }
                            g_GameManager.spellcardsCaptured++;
                        }
                    }
                    g_EnemyManager.spellcardInfo.isActive = 0;
                }
                g_Stage.spellcardState = NOT_RUNNING;
                break;
            }
            case ECL_OPCODE_BOSSTIMERSET:
                enemy->bossTimer.SetCurrent(instruction->args.setInt);
                break;
            case ECL_OPCODE_LIFECALLBACKTHRESHOLD:
                enemy->lifeCallbackThreshold = instruction->args.setInt;
                break;
            case ECL_OPCODE_LIFECALLBACKSUB:
                enemy->lifeCallbackSub = instruction->args.setInt;
                break;
            case ECL_OPCODE_TIMERCALLBACKTHRESHOLD:
                enemy->timerCallbackThreshold = instruction->args.setInt;
                enemy->bossTimer.SetCurrent(0);
                break;
            case ECL_OPCODE_TIMERCALLBACKSUB:
                enemy->timerCallbackSub = instruction->args.setInt;
                break;
            case ECL_OPCODE_ENEMYFLAGINTERACTABLE:
                enemy->flags.unk6 = instruction->args.setInt;
                break;
            case ECL_OPCODE_EFFECTPARTICLE:
                g_EffectManager.SpawnParticles(
                    instruction->args.effectParticle.effectId, &enemy->position,
                    instruction->args.effectParticle.numParticles,
                    instruction->args.effectParticle.particleColor
                );
                break;
            case ECL_OPCODE_DROPITEMS: {
                for (i32 i = 0; i < instruction->args.setInt; i++) {
                    ZunVec3 vec = enemy->position;

                    g_Rng.GetRandomF32InBounds(&vec.x, -72.0f, 72.0f);
                    g_Rng.GetRandomF32InBounds(&vec.y, -72.0f, 72.0f);
                    if (g_GameManager.currentPower < 128) {
                        g_ItemManager.SpawnItem(&vec, i == 0 ? ITEM_POWER_BIG : ITEM_POWER_SMALL, 0);
                    } else {
                        g_ItemManager.SpawnItem(&vec, ITEM_POINT, 0);
                    }
                }
                break;
            }
            case ECL_OPCODE_ANMFLAGROTATION:
                enemy->flags.unk13 = instruction->args.setInt;
                break;
            case ECL_OPCODE_EXINSCALL:
                g_EclExInsn[instruction->args.setInt](enemy, instruction);
                break;
            case ECL_OPCODE_EXINSREPEAT:
                if (instruction->args.setInt >= 0) {
                    enemy->currentContext.funcSetFunc = g_EclExInsn[instruction->args.setInt];
                } else {
                    enemy->currentContext.funcSetFunc = NULL;
                }
                break;
            case ECL_OPCODE_TIMESET:
                enemy->currentContext.time.IncrementInline(*EnemyEclInstr::GetVar(enemy, &instruction->args.timeSet.timeToSet, NULL));
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
            case ECL_OPCODE_ENEMYCREATE: {
                EclRawInstrEnemyCreateArgs enemy_create_args = instruction->args.enemyCreate;
                enemy_create_args.pos.x = *EnemyEclInstr::GetVarFloat(enemy, &enemy_create_args.pos.x, NULL);
                enemy_create_args.pos.y = *EnemyEclInstr::GetVarFloat(enemy, &enemy_create_args.pos.y, NULL);
                enemy_create_args.pos.z = *EnemyEclInstr::GetVarFloat(enemy, &enemy_create_args.pos.z, NULL);
                g_EnemyManager.SpawnEnemy(
                    enemy_create_args.subId,
                    &enemy_create_args.pos,
                    enemy_create_args.life,
                    enemy_create_args.itemDrop,
                    enemy_create_args.score
                );
                break;
            }
            case ECL_OPCODE_ENEMYKILLALL: {
                // since arrays decay into pointers, I think this name makes sense.
                Enemy *enemy_iter;
                i32 i;
                for (enemy_iter = &g_EnemyManager.enemies[0], i = 0; i < ARRAY_SIZE_SIGNED(g_EnemyManager.enemies) - 1; i++, enemy_iter++) {
                    if (!enemy_iter->flags.active) {
                        continue;
                    }
                    if (enemy_iter->flags.isBoss) {
                        continue;
                    }

                    enemy_iter->life = 0;
                    if (enemy_iter->flags.unk6 == 0 && 0 <= enemy_iter->deathCallbackSub) {
                        g_EclManager.CallEclSub(&enemy_iter->currentContext, enemy_iter->deathCallbackSub);
                        enemy_iter->deathCallbackSub = -1;
                    }
                }
                break;
            }
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
                if (instruction->args.bulletSound.bulletSfx >= 0) {
                    enemy->bulletProps.sfx = instruction->args.bulletSound.bulletSfx;
                    enemy->bulletProps.flags |= 0x200;
                } else {
                    enemy->bulletProps.flags &= 0xfffffdff;
                }
                break;
            case ECL_OPCODE_ENEMYFLAGDISABLECALLSTACK:
                enemy->flags.unk14 = instruction->args.setInt;
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
                enemy->flags.unk15 = instruction->args.setInt;
                break;
            case ECL_OPCODE_BOSSTIMERCLEAR:
                enemy->timerCallbackSub = enemy->deathCallbackSub;
                enemy->bossTimer.SetCurrent(0);
                break;
            case ECL_OPCODE_SPELLCARDFLAGTIMEOUT:
                enemy->flags.unk16 = instruction->args.setInt;
                break;
            }
        NEXT_INSN:
            instruction = (EclRawInstr *)((u8 *)instruction + instruction->offsetToNext);
            goto PARSE_INSTRUCTION;
        } else {
            switch (enemy->flags.unk1) {
            case 1:
                enemy->angle = utils::AddNormalizeAngle(enemy->angle, g_Supervisor.effectiveFramerateMultiplier * enemy->angularVelocity);
                enemy->speed = g_Supervisor.effectiveFramerateMultiplier * enemy->acceleration + enemy->speed;
                sincosmul(&enemy->axisSpeed, enemy->angle, enemy->speed);
                enemy->axisSpeed.z = 0.0;
                break;
            case 2:
                enemy->moveInterpTimer.Decrement(1);
                local_bc = enemy->moveInterpTimer.AsFramesFloat() / enemy->moveInterpStartTime;
                if (local_bc >= 1.0f) {
                    local_bc = 1.0f;
                }
                switch (enemy->flags.unk2) {
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
                enemy->axisSpeed = enemy->moveInterp * local_bc + enemy->moveInterpStartPos - enemy->position;
                enemy->angle = ZUN_ATAN2F(enemy->axisSpeed.y, enemy->axisSpeed.x);
                if (enemy->moveInterpTimer.current <= 0) {
                    enemy->flags.unk1 = 0;
                    enemy->position = enemy->moveInterpStartPos + enemy->moveInterp;
                    enemy->axisSpeed = ZunVec3(0.0f, 0.0f, 0.0f);
                }
                break;
            }
            if (0 < enemy->life) {
                if (0 < enemy->shootInterval) {
                    enemy->shootIntervalTimer.Tick();
                    if (enemy->shootIntervalTimer.current >= enemy->shootInterval) {
                        enemy->bulletProps.position = enemy->position + enemy->shootOffset;
                        g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
                        enemy->shootIntervalTimer.InitializeForPopup();
                    }
                }
                if (0 <= enemy->anmExLeft) {
                    local_c0 = 0;
                    if (enemy->axisSpeed.x < 0.0f) {
                        local_c0 = 1;
                    } else if (enemy->axisSpeed.x > 0.0f) {
                        local_c0 = 2;
                    }
                    if (enemy->anmExFlags != local_c0) {
                        switch (local_c0) {
                        case 0:
                            if (enemy->anmExFlags == 0xff) {
                                g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, enemy->anmExDefaults + ANM_OFFSET_ENEMY);
                            } else if (enemy->anmExFlags == 1) {
                                g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, enemy->anmExFarLeft + ANM_OFFSET_ENEMY);
                            } else {
                                g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, enemy->anmExFarRight + ANM_OFFSET_ENEMY);
                            }
                            break;
                        case 1:
                            g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, enemy->anmExLeft + ANM_OFFSET_ENEMY);
                            break;
                        case 2:
                            g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm, enemy->anmExRight + ANM_OFFSET_ENEMY);
                            break;
                        }
                        enemy->anmExFlags = local_c0;
                    }
                }
                if (enemy->currentContext.funcSetFunc != NULL) {
                    enemy->currentContext.funcSetFunc(enemy, NULL);
                }
            }
            enemy->currentContext.currentInstr = instruction;
            enemy->currentContext.time.Tick();
            return true;
        }
    }
}
