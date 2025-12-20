#include "EnemyEclInstr.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "EffectManager.hpp"
#include "Enemy.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "Rng.hpp"
#include "utils.hpp"

namespace th06 {
namespace EnemyEclInstr {
#define MAX_BOSS_TIME 7200

struct PatchouliShottypeVars {
    struct {
        i32 var1;
        i32 var2;
        i32 var3;
    } shotVars[2];
};

PatchouliShottypeVars g_PatchouliShottypeVars[2] = {{{{0, 3, 1}, {2, 3, 4}}},
                                                    {{{1, 4, 0}, {4, 2, 3}}}};
i32 g_PlayerShot;
f32 g_PlayerDistance;
f32 g_PlayerAngle;
f32 g_StarAngleTable[6];
ZunVec3 g_EnemyPosVector;
ZunVec3 g_PlayerPosVector;

void MoveDirTime(Enemy *enemy, EclRawInstr *instr) {
    EclRawInstrAluArgs *alu;
    f32 angle;

    alu = &instr->args.alu;
    angle = *GetVarFloat(enemy, &alu->arg1.f32Param, NULL);

    enemy->moveInterp.x =
        ZUN_COSF(angle) * alu->arg2.f32Param * (f32)alu->res / 2.0f;
    enemy->moveInterp.y =
        ZUN_SINF(angle) * alu->arg2.f32Param * (f32)alu->res / 2.0f;
    enemy->moveInterp.z = 0.0f;

    enemy->moveInterpStartPos = enemy->position;
    enemy->moveInterpStartTime = alu->res;

    enemy->moveInterpTimer.SetCurrent(enemy->moveInterpStartTime);

    enemy->flags.unk1 = 2;
}

void MovePosTime(Enemy *enemy, EclRawInstr *instr) {
    ZunVec3 newPos;
    EclRawInstrAluArgs *alu = &instr->args.alu;

    newPos.x = *GetVarFloat(enemy, &alu->arg1.f32Param, NULL);
    newPos.y = *GetVarFloat(enemy, &alu->arg2.f32Param, NULL);
    newPos.z = *GetVarFloat(enemy, &alu->arg3.f32Param, NULL);

    enemy->moveInterp = newPos - enemy->position;
    enemy->moveInterpStartPos = enemy->position;
    enemy->moveInterpStartTime = alu->res;

    enemy->moveInterpTimer.SetCurrent(enemy->moveInterpStartTime);

    enemy->flags.unk1 = 2;
    enemy->axisSpeed = ZunVec3(0.0f, 0.0f, 0.0f);
}

void MoveTime(Enemy *enemy, EclRawInstr *instr) {
    EclRawInstrAluArgs *alu;
    f32 angle;

    alu = &instr->args.alu;
    angle = *GetVarFloat(enemy, &enemy->angle, NULL);

    enemy->moveInterp.x =
        ZUN_COSF(angle) * enemy->speed * (float)alu->res / 2.0f;
    enemy->moveInterp.y =
        ZUN_SINF(angle) * enemy->speed * (float)alu->res / 2.0f;
    enemy->moveInterp.z = 0.0f;

    enemy->moveInterpStartPos = enemy->position;
    enemy->moveInterpStartTime = alu->res;

    enemy->moveInterpTimer.SetCurrent(enemy->moveInterpStartTime);

    enemy->flags.unk1 = 2;
}

i32 *GetVar(Enemy *enemy, EclVarId *eclVarId, EclValueType *valueType) {
    if (valueType != NULL)
        *valueType = ECL_VALUE_TYPE_UNDEFINED;

    switch (*eclVarId) {
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
        return (i32 *)&g_GameManager.difficulty;

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
        g_PlayerDistance =
            (g_Player.positionCenter - enemy->position).getMagnitude();
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

f32 *GetVarFloat(Enemy *enemy, f32 *eclVarId, EclValueType *valueType) {
    i32 varId = *eclVarId;
    i32 *res = GetVar(enemy, (EclVarId *)&varId, valueType);
    if (res == &varId) {
        return eclVarId;
    } else {
        return (f32 *)res;
    }
}

void SetVar(Enemy *enemy, EclVarId lhs, void *rhs) {
    i32 *lhsPtr;
    EclValueType lhsType;
    i32 *rhsPtr;

    rhsPtr = GetVar(enemy, (EclVarId *)rhs, NULL);
    lhsPtr = GetVar(enemy, &lhs, &lhsType);
    if (lhsType == ECL_VALUE_TYPE_INT) {
        *lhsPtr = *rhsPtr;
    } else if (lhsType == ECL_VALUE_TYPE_FLOAT) {
        *(f32 *)lhsPtr = *(f32 *)rhsPtr;
    }
    return;
}

void MathAdd(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId,
             EclVarId *rhsVarId) {
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    // Get output variable.
    outPtr = GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT) {
        lhsPtr = GetVar(enemy, lhsVarId, NULL);
        rhsPtr = GetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr + *rhsPtr;
    } else if (outType == ECL_VALUE_TYPE_FLOAT) {
        lhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr + *(f32 *)rhsPtr;
    }
    return;
}

void MathSub(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId,
             EclVarId *rhsVarId) {
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    outPtr = GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT) {
        lhsPtr = GetVar(enemy, lhsVarId, NULL);
        rhsPtr = GetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr - *rhsPtr;
    } else if (outType == ECL_VALUE_TYPE_FLOAT) {
        lhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr - *(f32 *)rhsPtr;
    }
    return;
}

void MathMul(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId,
             EclVarId *rhsVarId) {
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    lhsPtr = GetVar(enemy, lhsVarId, NULL);
    rhsPtr = GetVar(enemy, rhsVarId, NULL);
    outPtr = GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT) {
        lhsPtr = GetVar(enemy, lhsVarId, NULL);
        rhsPtr = GetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr * *rhsPtr;
    } else if (outType == ECL_VALUE_TYPE_FLOAT) {
        lhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr * *(f32 *)rhsPtr;
    }
    return;
}

void MathDiv(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId,
             EclVarId *rhsVarId) {
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    outPtr = GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT) {
        lhsPtr = GetVar(enemy, lhsVarId, NULL);
        rhsPtr = GetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr / *rhsPtr;
    } else if (outType == ECL_VALUE_TYPE_FLOAT) {
        lhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = *(f32 *)lhsPtr / *(f32 *)rhsPtr;
    }
    return;
}

void MathMod(Enemy *enemy, EclVarId outVarId, EclVarId *lhsVarId,
             EclVarId *rhsVarId) {
    EclValueType outType;
    i32 *outPtr;
    i32 *lhsPtr;
    i32 *rhsPtr;

    outPtr = GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_INT) {
        lhsPtr = GetVar(enemy, lhsVarId, NULL);
        rhsPtr = GetVar(enemy, rhsVarId, NULL);
        *outPtr = *lhsPtr % *rhsPtr;
    } else if (outType == ECL_VALUE_TYPE_FLOAT) {
        lhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)lhsVarId, NULL);
        rhsPtr = (i32 *)GetVarFloat(enemy, (f32 *)rhsVarId, NULL);
        *(f32 *)outPtr = ZUN_FMODF(*(f32 *)lhsPtr, *(f32 *)rhsPtr);
    }
    return;
}

void MathAtan2(Enemy *enemy, EclVarId outVarId, f32 *x1, f32 *y1, f32 *y2,
               f32 *x2) {
    EclValueType outType;
    f32 *outPtr;
    f32 *y1Ptr, *x1Ptr, *x2Ptr, *y2Ptr;

    outPtr = (f32 *)GetVar(enemy, &outVarId, &outType);
    if (outType == ECL_VALUE_TYPE_FLOAT) {
        y1Ptr = GetVarFloat(enemy, x1, NULL);
        x1Ptr = GetVarFloat(enemy, y1, NULL);
        y2Ptr = GetVarFloat(enemy, y2, NULL);
        x2Ptr = GetVarFloat(enemy, x2, NULL);
        *outPtr = ZUN_ATAN2F(*x2Ptr - *x1Ptr, *y2Ptr - *y1Ptr);
    }
    return;
}

void ExInsCirnoRainbowBallJank(Enemy *enemy, EclRawInstr *instr) {
    f32 accelerationAngle;
    f32 accelerationMultiplier;
    ZunTimer *bulletTimer;
    Bullet *currentBullet;
    i32 effectIndex;
    i32 i;
    ZunVec3 velocityVector;

    currentBullet = g_BulletManager.bullets;
    effectIndex = instr->args.exInstr.i32Param;

    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &enemy->position, 1,
                                   COLOR_WHITE);
    for (i = 0; i < ARRAY_SIZE_SIGNED(g_BulletManager.bullets);
         i++, currentBullet++) {
        if (currentBullet->state == 0 || currentBullet->state == 5) {
            continue;
        }

        currentBullet->spriteOffset = 15;
        g_AnmManager->SetActiveSprite(
            &currentBullet->sprites.spriteBullet,
            currentBullet->sprites.spriteBullet.baseSpriteIndex +
                currentBullet->spriteOffset);
        switch (effectIndex) {
        case 0:
            currentBullet->speed = 0.0;
            velocityVector.x = 0.0;
            velocityVector.y = 0.0;
            velocityVector.z = 0.0;
            currentBullet->velocity = velocityVector;
            break;
        case 1:
            currentBullet->exFlags |= 0x10;
            currentBullet->ex5Int0 = 220;
            bulletTimer = &currentBullet->timer;
            bulletTimer->current = 0;
            bulletTimer->subFrame = 0.0;
            bulletTimer->previous = -999;
            accelerationMultiplier = 0.01;
            accelerationAngle =
                g_Rng.GetRandomF32ZeroToOne() * (2 * ZUN_PI) - ZUN_PI;
            sincosmul(&currentBullet->ex4Acceleration, accelerationAngle,
                      accelerationMultiplier);
            break;
        }
    }
}

void ExInsShootAtRandomArea(Enemy *enemy, EclRawInstr *instr) {
    f32 bulletSpeed;

    bulletSpeed = instr->args.exInstr.i32Param;
    enemy->bulletProps.position = enemy->position + enemy->shootOffset;
    enemy->bulletProps.position.x =
        (g_Rng.GetRandomF32ZeroToOne() * bulletSpeed + (enemy->position).x) -
        bulletSpeed / 2.0f;
    bulletSpeed *= 0.75f;
    enemy->bulletProps.position.y =
        (g_Rng.GetRandomF32ZeroToOne() * bulletSpeed + (enemy->position).y) -
        bulletSpeed / 2.0f;
    g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
}

void ExInsShootStarPattern(Enemy *enemy, EclRawInstr *instr) {
    // Variable names are more quick guesses at functionality than anything
    // else, they should not be trusted
    ZunVec3 baseTargetPosition;
    i32 i;
    f32 propsSpeedBackup;
    f32 patternPosition;
    ZunVec3 starPatternTarget0;
    ZunVec3 starPatterTarget1;
    f32 targetDistance;

    if (enemy->currentContext.var2 >= enemy->currentContext.var3) {
        enemy->currentContext.funcSetFunc = NULL;
        return;
    }

    if (enemy->currentContext.var2 == 0) {
        g_EnemyPosVector = enemy->position;
        g_PlayerPosVector = g_Player.positionCenter;
        g_StarAngleTable[0] =
            g_Rng.GetRandomF32ZeroToOne() * (ZUN_PI * 2) - ZUN_PI;
        g_StarAngleTable[1] =
            utils::AddNormalizeAngle(g_StarAngleTable[0], 4 * ZUN_PI / 5);
    }
    if (enemy->currentContext.var2 % 30 == 0) {
        g_StarAngleTable[0] = g_StarAngleTable[1];
        g_StarAngleTable[1] =
            utils::AddNormalizeAngle(g_StarAngleTable[1], 4 * ZUN_PI / 5);
        g_StarAngleTable[2] =
            utils::AddNormalizeAngle(g_StarAngleTable[1], 4 * ZUN_PI / 5);
        g_StarAngleTable[3] =
            utils::AddNormalizeAngle(g_StarAngleTable[2], 4 * ZUN_PI / 5);
        g_StarAngleTable[4] =
            utils::AddNormalizeAngle(g_StarAngleTable[3], 4 * ZUN_PI / 5);
        g_StarAngleTable[5] =
            utils::AddNormalizeAngle(g_StarAngleTable[4], 4 * ZUN_PI / 5);
    }
    if (enemy->currentContext.var2 % 6 == 0) {
        patternPosition =
            (f32)enemy->currentContext.var2 / (f32)enemy->currentContext.var3;
        targetDistance = patternPosition * 0.1f;

        baseTargetPosition =
            (g_PlayerPosVector - g_EnemyPosVector) * targetDistance +
            g_EnemyPosVector;
        baseTargetPosition.z = 0.0f;

        patternPosition += 0.5f;
        enemy->bulletProps.angle1 = (ZUN_PI / 3) * patternPosition;

        for (i = 0; i < 5; i++) {
            targetDistance = (enemy->currentContext.var2 % 30) / 30.0f;
            sincosmul(&starPatternTarget0, g_StarAngleTable[i],
                      enemy->currentContext.float3);
            sincosmul(&starPatterTarget1, g_StarAngleTable[i + 1],
                      enemy->currentContext.float3);
            starPatternTarget0 =
                (starPatterTarget1 - starPatternTarget0) * targetDistance +
                starPatternTarget0;
            starPatternTarget0.z = 0;
            enemy->bulletProps.position =
                baseTargetPosition + starPatternTarget0;
            propsSpeedBackup = enemy->bulletProps.speed1;
            enemy->bulletProps.speed1 =
                g_Rng.GetRandomF32InRange(enemy->bulletProps.speed2) +
                enemy->bulletProps.speed1;
            g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
            enemy->bulletProps.speed1 = propsSpeedBackup;
            enemy->bulletProps.angle1 -= (ZUN_PI / 6) * patternPosition;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_16);
    }
    enemy->currentContext.var2++;
}

void ExInsPatchouliShottypeSetVars(Enemy *enemy, EclRawInstr *instr) {
    enemy->currentContext.var1 =
        g_PatchouliShottypeVars[g_GameManager.character]
            .shotVars[g_GameManager.shotType]
            .var1;
    enemy->currentContext.var2 =
        g_PatchouliShottypeVars[g_GameManager.character]
            .shotVars[g_GameManager.shotType]
            .var2;
    enemy->currentContext.var3 =
        g_PatchouliShottypeVars[g_GameManager.character]
            .shotVars[g_GameManager.shotType]
            .var3;
}

void ExInsStage56Func4(Enemy *enemy, EclRawInstr *instr) {
    i32 bulletsLeft;
    Bullet *currentBullet;
    i32 i;
    ZunVec2 playerBulletOffset;

    if (instr->args.exInstr.i32Param < 2) {
        g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &enemy->position,
                                       1, COLOR_WHITE);
        g_GameManager.isTimeStopped = instr->args.exInstr.u8Param;
    } else {
        bulletsLeft = 14;
        currentBullet = g_BulletManager.bullets;
        if (g_GameManager.difficulty <= NORMAL) {
            for (i = 0; i < ARRAY_SIZE_SIGNED(g_BulletManager.bullets);
                 i++, currentBullet++) {
                if (currentBullet->state == 0 || currentBullet->state == 5) {
                    continue;
                }

                if (currentBullet->sprites.spriteBullet.sprite != NULL &&
                    currentBullet->sprites.spriteBullet.sprite->heightPx >=
                        30.0f &&
                    currentBullet->spriteOffset != 5 &&
                    (g_Rng.GetRandomU16() % 4 == 0)) {
                    currentBullet->spriteOffset = 5;
                    g_AnmManager->SetActiveSprite(
                        &currentBullet->sprites.spriteBullet,
                        currentBullet->sprites.spriteBullet.baseSpriteIndex +
                            currentBullet->spriteOffset);

                    playerBulletOffset.x =
                        (currentBullet->pos.x) - g_Player.positionCenter.x;
                    playerBulletOffset.y =
                        (currentBullet->pos.y) - g_Player.positionCenter.y;

                    if (playerBulletOffset.VectorLength() > 128.0f) {
                        currentBullet->angle =
                            g_Rng.GetRandomF32ZeroToOne() * ((ZUN_PI * 3) / 4) +
                            (ZUN_PI / 4);
                    } else {
                        currentBullet->angle =
                            g_Player.AngleFromPlayer(&currentBullet->pos) +
                            (ZUN_PI / 2) +
                            g_Rng.GetRandomF32InRange(ZUN_PI * 2);
                    }

                    sincosmul(&currentBullet->velocity, currentBullet->angle,
                              currentBullet->speed);
                    bulletsLeft--;
                    if (bulletsLeft == 0) {
                        break;
                    }
                }
            }
        } else {
            bulletsLeft = 52;
            for (i = 0; i < ARRAY_SIZE_SIGNED(g_BulletManager.bullets);
                 i++, currentBullet++) {
                if (currentBullet->state == 0 || currentBullet->state == 5) {
                    continue;
                }

                if (currentBullet->sprites.spriteBullet.sprite != NULL &&
                    currentBullet->sprites.spriteBullet.sprite->heightPx >=
                        30.0f &&
                    currentBullet->spriteOffset != 5 &&
                    (g_Rng.GetRandomU16() % 4 == 0)) {
                    currentBullet->spriteOffset = 5;
                    g_AnmManager->SetActiveSprite(
                        &currentBullet->sprites.spriteBullet,
                        currentBullet->sprites.spriteBullet.baseSpriteIndex +
                            currentBullet->spriteOffset);

                    playerBulletOffset.x =
                        (currentBullet->pos.x) - g_Player.positionCenter.x;
                    playerBulletOffset.y =
                        (currentBullet->pos.y) - g_Player.positionCenter.y;

                    if (playerBulletOffset.VectorLength() > 128.0f) {
                        currentBullet->angle =
                            g_Rng.GetRandomF32ZeroToOne() * (ZUN_PI * 2);
                    } else {
                        currentBullet->angle =
                            g_Player.AngleFromPlayer(&currentBullet->pos) +
                            (ZUN_PI / 2) +
                            g_Rng.GetRandomF32InRange(ZUN_PI * 2);
                    }

                    sincosmul(&currentBullet->velocity, currentBullet->angle,
                              currentBullet->speed);
                    bulletsLeft--;
                    if (bulletsLeft == 0) {
                        break;
                    }
                }
            }
        }
    }
    enemy->currentContext.var2 = 0;
}

void ExInsStage5Func5(Enemy *enemy, EclRawInstr *instr) {
    if (enemy->currentContext.var2 % 9 == 0) {
        ZunVec3 bpPositionOffset;
        f32 bulletAngle;
        f32 cosOut;
        i32 i;
        ZunVec3 matrixIn;
        f32 matrixInSeed;
        ZunVec3 matrixOut;
        f32 matrixOutSeed; // Later reused to store angles for trig function
                           // calls
        i32 patternPosition;
        f32 sinOut;
        EnemyBulletShooter bulletProps;

        patternPosition = enemy->currentContext.var2 / 9;
        bulletProps.sprite = 8;
        bulletProps.aimMode = 0;
        if (g_GameManager.difficulty <= NORMAL) {
            bulletProps.count1 = 1;
        } else {
            bulletProps.count1 = 3;
        }
        bulletProps.count2 = 1;
        bulletProps.angle2 = ZUN_PI / 6;
        bulletProps.unk_4a = 0;
        bulletProps.flags = 0;

        matrixOutSeed = 0.5f - patternPosition * 0.5f / 9.0f;
        matrixOut = g_Player.positionCenter - enemy->position;
        matrixOut.getNormalized(matrixIn);
        if ((patternPosition & 1) != 0) {
            matrixInSeed = -256.0f;
        } else {
            matrixInSeed = 256.0f;
        }
        matrixIn *= matrixInSeed;
        matrixOut *= matrixOutSeed;
        bpPositionOffset = matrixOut + matrixIn;
        matrixIn = -matrixIn;

        matrixOutSeed = ZUN_PI / 4;
        cosOut = ZUN_COSF(matrixOutSeed);
        sinOut = ZUN_SINF(matrixOutSeed);
        matrixOut = matrixIn;
        matrixIn.x = matrixOut.x * cosOut + matrixOut.y * sinOut;
        matrixIn.y = -matrixOut.x * sinOut + matrixOut.y * cosOut;
        matrixOutSeed = -ZUN_PI / 18;
        cosOut = ZUN_COSF(matrixOutSeed);
        sinOut = ZUN_SINF(matrixOutSeed);
        bulletProps.angle1 = 0.0;
        bulletAngle = -ZUN_PI / 4;

        for (i = 0; i < 9; i++, bulletAngle += ZUN_PI / 18) {
            matrixOut = matrixIn;
            matrixIn.x = matrixOut.x * cosOut + matrixOut.y * sinOut;
            matrixIn.y = -matrixOut.x * sinOut + matrixOut.y * cosOut;

            bulletProps.position =
                matrixIn + enemy->position + bpPositionOffset;
            bulletProps.speed1 = 2.0;
            if ((patternPosition & 1) != 0 &&
                g_GameManager.difficulty <= NORMAL) {
                bulletProps.angle1 = bulletAngle;
            }
            bulletProps.spriteOffset = 3;
            g_BulletManager.SpawnBulletPattern(&bulletProps);
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_7);
    }
    enemy->currentContext.var2++;
}

void ExInsStage6XFunc6(Enemy *enemy, EclRawInstr *instr) {
    i32 baseAngleModifier;
    f32 distanceModifier;
    Effect *effect;
    f32 finalAngle;
    ZunVec3 particlePos;

    if (enemy->flags.unk15 != 0) {
        Enemy::ResetEffectArray(enemy);
        return;
    }
    enemy->exInsFunc6Angle += RADIANS(1.0f);
    if (enemy->exInsFunc6Angle >= RADIANS(45.0f)) {
        enemy->exInsFunc6Angle -= RADIANS(90.0f);
    }

    // Run every 8 frames for first 30 frames, then every 4 for next 30, then
    // every 2 for next 60, then every frame
    if (enemy->exInsFunc6Timer.HasTicked() &&
        (enemy->exInsFunc6Timer > 120 ||
         (enemy->exInsFunc6Timer > 60 &&
          enemy->exInsFunc6Timer.current % 2 == 0) ||
         (enemy->exInsFunc6Timer > 30 &&
          enemy->exInsFunc6Timer.current % 4 == 0) ||
         enemy->exInsFunc6Timer.current % 8 == 0)) {
        baseAngleModifier = enemy->exInsFunc6Timer.current % 16;
        baseAngleModifier = g_Rng.GetRandomU16InRange(baseAngleModifier / 2) +
                            baseAngleModifier / 2;
        distanceModifier = (baseAngleModifier * 160.0f) / 16.0f + 32.0f;
        finalAngle = enemy->exInsFunc6Angle -
                     (baseAngleModifier * RADIANS(180.0f)) / 40.0f;
        if (distanceModifier < RADIANS(-45.0f)) {
            distanceModifier += RADIANS(90.0f);
        }

        particlePos = enemy->position;
        particlePos.x += ZUN_COSF(finalAngle) * distanceModifier;
        particlePos.y += ZUN_SINF(finalAngle) * distanceModifier;
        effect = g_EffectManager.SpawnParticles(
            PARTICLE_EFFECT_UNK_19, &particlePos, 1, COLOR_DEEPBLUE);
        effect->unk_11c.x =
            (g_Rng.GetRandomF32ZeroToOne() * 40.0f - 20.0f) / 60.0f;
        effect->unk_11c.y = (8.0f * baseAngleModifier) / 60.0f - (4.0f / 15.0f);
        effect->unk_11c.z = 0.0;
        effect->unk_128 = -effect->unk_11c / 120.0f;

        particlePos = enemy->position;
        particlePos.x -= ZUN_COSF(finalAngle) * distanceModifier;
        particlePos.y += ZUN_SINF(finalAngle) * distanceModifier;
        effect = g_EffectManager.SpawnParticles(
            PARTICLE_EFFECT_UNK_19, &particlePos, 1, COLOR_DEEPBLUE);
        effect->unk_11c.x =
            (g_Rng.GetRandomF32ZeroToOne() * 40.0f - 20.0f) / 60.0f;
        effect->unk_11c.y = (8.0f * baseAngleModifier) / 60.0f - (4.0f / 15.0f);
        effect->unk_11c.z = 0.0;
        effect->unk_128 = -effect->unk_11c / 120.0f;
    }

    enemy->exInsFunc6Timer.Tick();
}

void ExInsStage6Func7(Enemy *enemy, EclRawInstr *instr) {
    f32 angleDiff;
    i32 attackType;
    i32 i;
    i32 innerLoopCount;
    f32 laserAngle;
    EnemyLaserShooter laserProps;
    f32 lengthMultiplier;
    i32 outerLoopCount;
    f32 randomAngleModifier;

    ZunVec3 positionVectors[8];

    attackType = instr->args.exInstr.i32Param;
    randomAngleModifier = g_Rng.GetRandomF32ZeroToOne() * (ZUN_PI * 2);

    for (outerLoopCount = 0; outerLoopCount < 2; outerLoopCount++) {
        if (outerLoopCount == 0) {
            laserAngle = -ZUN_PI + randomAngleModifier;
            angleDiff = ZUN_PI / 4;
        } else {
            laserAngle = (7 * -ZUN_PI / 8) + randomAngleModifier;
            angleDiff = -ZUN_PI / 4;
        }

        lengthMultiplier = 32.0f;
        for (i = 0; i < 8; i++) {
            positionVectors[i] = enemy->position;
            positionVectors[i].x += ZUN_COSF(laserAngle) * lengthMultiplier;
            positionVectors[i].y += ZUN_SINF(laserAngle) * lengthMultiplier;
            laserAngle += ZUN_PI / 4;
        }

        if (outerLoopCount == 0) {
            laserAngle = -ZUN_PI + randomAngleModifier;
        } else {
            laserAngle = (7 * -ZUN_PI / 8) + randomAngleModifier;
        }

        for (innerLoopCount = 0; innerLoopCount < 3; innerLoopCount++) {
            if (innerLoopCount < 2) {
                lengthMultiplier = 112.0f;
            } else {
                lengthMultiplier = 480.0f;
            }
            for (i = 0; i < 8; i++) {
                laserProps.position = positionVectors[i];
                laserProps.sprite = 1;
                if (attackType == 0) {
                    if (g_GameManager.difficulty <= NORMAL) {
                        laserProps.spriteOffset = 2;
                    } else {
                        laserProps.spriteOffset = 8;
                    }
                    laserProps.angle = laserAngle;
                    laserProps.speed = 0.0f;
                    laserProps.startOffset = 0.0f;
                    if (g_GameManager.difficulty <= NORMAL) {
                        laserProps.endOffset = lengthMultiplier;
                        laserProps.startLength = lengthMultiplier;
                    } else {
                        laserProps.endOffset = 440.0f;
                        laserProps.startLength = 440.0f;
                    }
                    if (g_GameManager.difficulty <= NORMAL) {
                        laserProps.width = 28.0f;
                    } else {
                        laserProps.width = 20.0f;
                    }
                    laserProps.startTime = innerLoopCount * 16 + 60;
                    laserProps.duration = 90 - innerLoopCount * 16;
                    laserProps.stopTime = 16;
                    laserProps.grazeDelay = 50;
                    laserProps.grazeDistance = 16;
                    laserProps.flags = 2;
                    laserProps.type = 1;
                    g_BulletManager.SpawnLaserPattern(&laserProps);
                } else {
                    enemy->bulletProps.position = laserProps.position;
                    g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
                }
                positionVectors[i].x += ZUN_COSF(laserAngle) * lengthMultiplier;
                positionVectors[i].y += ZUN_SINF(laserAngle) * lengthMultiplier;
                laserAngle += ZUN_PI / 4;
            }
            laserAngle += angleDiff - (ZUN_PI * 2);
        }
    }
}

void ExInsStage6Func8(Enemy *enemy, EclRawInstr *instr) {
    i32 changedBullets;
    Bullet *currentBullet;
    i32 i;

    changedBullets = 0;
    currentBullet = g_BulletManager.bullets;

    EnemyBulletShooter bulletProps;

    for (i = 0; i < ARRAY_SIZE_SIGNED(g_BulletManager.bullets);
         i++, currentBullet++) {
        if (currentBullet->state == 0 || currentBullet->state == 5) {
            continue;
        }

        if (currentBullet->sprites.spriteBullet.sprite != NULL &&
            currentBullet->sprites.spriteBullet.sprite->heightPx >= 30.0f) {
            bulletProps.position = currentBullet->pos;
            bulletProps.sprite = 3;
            bulletProps.spriteOffset = 1;
            bulletProps.angle1 = g_Rng.GetRandomF32InRange(ZUN_PI * 2) - ZUN_PI;
            bulletProps.speed1 = 0.0f;
            bulletProps.count1 = 1;
            bulletProps.count2 = 1;
            bulletProps.flags = 8;
            bulletProps.aimMode = 1;
            g_BulletManager.SpawnBulletPattern(&bulletProps);
            changedBullets++;
        }
    }

    enemy->currentContext.var3 = changedBullets;
}

void ExInsStage6Func9(Enemy *enemy, EclRawInstr *instr) {
    Bullet *currentBullet;
    f32 distance;
    i32 i;
    f32 randomAngleModifier;

    currentBullet = g_BulletManager.bullets;
    EnemyBulletShooter unusedBulletProps;

    randomAngleModifier = g_Rng.GetRandomF32InRange(ZUN_PI * 2) - ZUN_PI;
    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &enemy->position, 1,
                                   COLOR_WHITE);

    for (i = 0; i < ARRAY_SIZE_SIGNED(g_BulletManager.bullets);
         i++, currentBullet++) {
        if (currentBullet->state == 0 || currentBullet->state == 5) {
            continue;
        }

        if (currentBullet->sprites.spriteBullet.sprite != NULL &&
            currentBullet->sprites.spriteBullet.sprite->heightPx < 30.0f &&
            currentBullet->speed == 0.0f) {
            currentBullet->exFlags |= 0x10;
            currentBullet->spriteOffset = 2;
            g_AnmManager->SetActiveSprite(
                &currentBullet->sprites.spriteBullet,
                currentBullet->sprites.spriteBullet.baseSpriteIndex +
                    currentBullet->spriteOffset);
            currentBullet->speed = 0.01f;
            currentBullet->timer.InitializeForPopup();
            currentBullet->ex5Int0 = 120;
            distance = (enemy->position.x - currentBullet->pos.x) *
                           (enemy->position.x - currentBullet->pos.x) +
                       (enemy->position.y - currentBullet->pos.y) *
                           (enemy->position.y - currentBullet->pos.y);
            if (distance > 0.1f) {
                distance = ZUN_SQRTF(distance);
            } else {
                distance = 0.0f;
            }

            sincosmul(&currentBullet->ex4Acceleration,
                      (distance * ZUN_PI) / 256.0f + randomAngleModifier,
                      0.01f);
        }
    }
}

void ExInsStage6Func11(Enemy *enemy, EclRawInstr *instr) {
    Bullet *currentBullet;
    i32 i;
    // f32 unusedRandomNumber;

    currentBullet = g_BulletManager.bullets;
    EnemyBulletShooter unusedBulletProps;

    // unusedRandomNumber = g_Rng.GetRandomF32InRange(ZUN_PI * 2) - ZUN_PI;
    g_EffectManager.SpawnParticles(PARTICLE_EFFECT_UNK_12, &enemy->position, 1,
                                   COLOR_WHITE);

    for (i = 0; i < ARRAY_SIZE_SIGNED(g_BulletManager.bullets);
         i++, currentBullet++) {
        if (currentBullet->state == 0 || currentBullet->state == 5) {
            continue;
        }

        if (currentBullet->sprites.spriteBullet.sprite != NULL &&
            currentBullet->sprites.spriteBullet.sprite->heightPx < 30.0f &&
            currentBullet->speed == 0.0f) {
            currentBullet->exFlags |= 0x10;
            currentBullet->spriteOffset = 2;
            g_AnmManager->SetActiveSprite(
                &currentBullet->sprites.spriteBullet,
                currentBullet->sprites.spriteBullet.baseSpriteIndex +
                    currentBullet->spriteOffset);
            currentBullet->speed = 0.01f;
            currentBullet->timer.InitializeForPopup();
            currentBullet->ex5Int0 = 120;

            sincosmul(&currentBullet->ex4Acceleration,
                      g_Rng.GetRandomF32InRange(ZUN_PI * 2) - ZUN_PI, 0.01f);
        }
    }
}

void ExInsStage6XFunc10(Enemy *enemy, EclRawInstr *instr) {
    if (enemy->life <= 0) {
        return;
    }

    ExInsStage6XFunc6(enemy, instr);
    if (g_Player.bombInfo.isInUse != 0) {
        if (enemy->anmExLeft >= 0) {
            g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                 ANM_SCRIPT_ENEMY_END);
            enemy->anmExLeft = -1;
        }

        enemy->flags.unk6 = 0;
        enemy->exInsFunc10Timer.SetCurrent(60);
    } else {
        if (enemy->exInsFunc10Timer > 0 &&
            (enemy->exInsFunc10Timer.Decrement(1),
             enemy->exInsFunc10Timer == 0)) {
            if (enemy->anmExLeft < 0) {
                g_AnmManager->SetAndExecuteScriptIdx(&enemy->primaryVm,
                                                     ANM_OFFSET_ENEMY + 0xa0);
                enemy->anmExLeft = 0xa1;
            }

            enemy->flags.unk6 = 1;
        }
    }
}

void ExInsStage4Func12(Enemy *enemy, EclRawInstr *instr) {
    i32 i;

    for (i = 0; i < 8; i++) {
        if (enemy->lasers[i] != NULL && enemy->lasers[i]->inUse != 0) {
            enemy->bulletProps.position = ZunVec3(64.0, 0.0, 0.0);
            utils::Rotate(&enemy->bulletProps.position,
                          &enemy->bulletProps.position,
                          enemy->lasers[i]->angle);
            enemy->bulletProps.position += enemy->position;
            g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
        }
    }
}

void ExInsStageXFunc13(Enemy *enemy, EclRawInstr *instr) {
    f32 basePatternAngle;
    EnemyBulletShooter bulletProps = enemy->bulletProps;
    i32 i;
    i32 numPatterns;

    numPatterns = instr->args.exInstr.i32Param;
    basePatternAngle = enemy->currentContext.float2;
    if (enemy->currentContext.var3 % 6 == 0) {
        for (i = 0; i < numPatterns;
             i++, basePatternAngle += (ZUN_PI * 2) / numPatterns) {
            sincosmul(&bulletProps.position, basePatternAngle,
                      enemy->currentContext.float3);
            bulletProps.position.x += 192.0f;
            bulletProps.position.y += 224.0f;
            bulletProps.angle1 =
                basePatternAngle + enemy->currentContext.float1;
            g_BulletManager.SpawnBulletPattern(&bulletProps);
        }
    }
    enemy->currentContext.var3++;
}

void ExInsStageXFunc14(Enemy *enemy, EclRawInstr *instr) {
    f32 angleCos;
    f32 angleSin;
    ZunVec3 bulletPosition;
    Laser *currentLaser;
    i32 i;
    f32 positionMultiplier;

    enemy->currentContext.var3 = 0;
    for (i = 0; i < 8; i++) {
        if (enemy->lasers[i] != NULL && enemy->lasers[i]->inUse != 0) {
            currentLaser = enemy->lasers[i];
            positionMultiplier = currentLaser->startOffset;
            fsincos_wrapper(&angleSin, &angleCos, currentLaser->angle);

            while (currentLaser->endOffset > positionMultiplier) {
                bulletPosition.x =
                    angleCos * positionMultiplier + currentLaser->pos.x;
                bulletPosition.y =
                    angleSin * positionMultiplier + currentLaser->pos.y;
                bulletPosition.z = 0.0f;
                enemy->bulletProps.position = bulletPosition;
                g_BulletManager.SpawnBulletPattern(&enemy->bulletProps);
                positionMultiplier += 48.0f;
            }

            enemy->currentContext.var3++;
        }
    }
}

void ExInsStageXFunc15(Enemy *enemy, EclRawInstr *instr) {
    f32 bulletsAngle;
    Bullet *currentBullet;
    f32 distance;
    f32 enemyAngle;
    i32 i;
    Bullet *innerBullet;
    i32 j;
    i32 totalIterations;

    totalIterations = 0;
    currentBullet = g_BulletManager.bullets;
    EnemyBulletShooter unusedBulletProps;

    for (i = 0; i < ARRAY_SIZE_SIGNED(g_BulletManager.bullets);
         i++, currentBullet++) {
        if (currentBullet->state == 0 || currentBullet->state == 5) {
            continue;
        }

        if (currentBullet->sprites.spriteBullet.sprite != NULL &&
            currentBullet->sprites.spriteBullet.sprite->heightPx >= 30.0f) {
            totalIterations++;
            enemyAngle = ZUN_ATAN2F(currentBullet->pos.y - enemy->position.y,
                                    currentBullet->pos.x - enemy->position.x);

            for (j = 0, innerBullet = g_BulletManager.bullets;
                 j < ARRAY_SIZE_SIGNED(g_BulletManager.bullets);
                 j++, innerBullet++) {
                if (innerBullet->state == 0 || innerBullet->state == 5) {
                    continue;
                }

                if (innerBullet->sprites.spriteBullet.sprite != NULL &&
                    innerBullet->sprites.spriteBullet.sprite->heightPx <
                        30.0f &&
                    innerBullet->speed == 0.0f &&
                    (distance = ZUN_SQRTF(
                         (innerBullet->pos.x - currentBullet->pos.x) *
                             (innerBullet->pos.x - currentBullet->pos.x) +
                         (innerBullet->pos.y - currentBullet->pos.y) *
                             (innerBullet->pos.y - currentBullet->pos.y))) <
                        64.0f) {
                    innerBullet->exFlags |= 0x10;
                    innerBullet->speed = 0.01f;
                    innerBullet->timer.InitializeForPopup();
                    innerBullet->ex5Int0 = 120;
                    bulletsAngle =
                        ZUN_ATAN2F(innerBullet->pos.y - enemy->position.y,
                                   innerBullet->pos.x - enemy->position.x);
                    innerBullet->angle =
                        (bulletsAngle - enemyAngle) * 2.2f + enemyAngle;
                    sincosmul(&innerBullet->ex4Acceleration, innerBullet->angle,
                              0.01f);
                    innerBullet->spriteOffset += 1;
                    g_AnmManager->SetActiveSprite(
                        &innerBullet->sprites.spriteBullet,
                        innerBullet->sprites.spriteBullet.baseSpriteIndex +
                            innerBullet->spriteOffset);
                }
            }
        }
    }

    ExInsStage6XFunc10(enemy, instr);
    enemy->currentContext.var3 = totalIterations;
}

void ExInsStageXFunc16(Enemy *enemy, EclRawInstr *instr) {
    f32 rangeModifier;
    i32 remainingLife;

    remainingLife = enemy->life;
    if (enemy->bossTimer >= MAX_BOSS_TIME) {
        remainingLife = 0;
    }

    if (instr->args.exInstr.i32Param == 0) {
        enemy->currentContext.float3 = 2.0f - (remainingLife * 1.0f) / 6000.0f;
        enemy->currentContext.var5 = (remainingLife * 240) / 6000 + 40;
    } else {
        rangeModifier = 320.0f - (remainingLife * 160.0f) / 6000.0f;
        enemy->currentContext.float2 =
            g_Rng.GetRandomF32InRange(rangeModifier) +
            (192.0f - rangeModifier / 2.0f);
        rangeModifier = 128.0f - (remainingLife * 64.0f) / 6000.0f;
        enemy->currentContext.float3 =
            g_Rng.GetRandomF32InRange(rangeModifier) +
            (96.0f - rangeModifier / 2.0f);
    }
}
}; // namespace EnemyEclInstr
}; // namespace th06
