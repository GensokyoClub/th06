#pragma once

#include "Enemy.hpp"

namespace th06 {
namespace EnemyEclInstr {
i32 *GetVar(Enemy *enemy, EclVarId *varId, EclValueType *valueType);
f32 *GetVarFloat(Enemy *enemy, f32 *varId, EclValueType *valueType);
void SetVar(Enemy *enemy, EclVarId lhs, void *rhs);

void MathAdd(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
void MathSub(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
void MathMul(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
void MathDiv(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
void MathMod(Enemy *enemy, EclVarId out, EclVarId *lhs, EclVarId *rhs);
void MathAtan2(Enemy *enemy, EclVarId out, f32 *a1, f32 *a2, f32 *b1, f32 *b2);

void MoveDirTime(Enemy *enemy, EclRawInstr *instr);
void MovePosTime(Enemy *enemy, EclRawInstr *instr);
void MoveTime(Enemy *enemy, EclRawInstr *instr);

void ExInsCirnoRainbowBallJank(Enemy *enemy, EclRawInstr *instr);
void ExInsShootAtRandomArea(Enemy *enemy, EclRawInstr *instr);
void ExInsShootStarPattern(Enemy *enemy, EclRawInstr *instr);
void ExInsPatchouliShottypeSetVars(Enemy *enemy, EclRawInstr *instr);
void ExInsStage56Func4(Enemy *enemy, EclRawInstr *instr);
void ExInsStage5Func5(Enemy *enemy, EclRawInstr *instr);
void ExInsStage6XFunc6(Enemy *enemy, EclRawInstr *instr);
void ExInsStage6Func7(Enemy *enemy, EclRawInstr *instr);
void ExInsStage6Func8(Enemy *enemy, EclRawInstr *instr);
void ExInsStage6Func9(Enemy *enemy, EclRawInstr *instr);
void ExInsStage6Func11(Enemy *enemy, EclRawInstr *instr);
void ExInsStage6XFunc10(Enemy *enemy, EclRawInstr *instr);
void ExInsStage4Func12(Enemy *enemy, EclRawInstr *instr);
void ExInsStageXFunc13(Enemy *enemy, EclRawInstr *instr);
void ExInsStageXFunc14(Enemy *enemy, EclRawInstr *instr);
void ExInsStageXFunc15(Enemy *enemy, EclRawInstr *instr);
void ExInsStageXFunc16(Enemy *enemy, EclRawInstr *instr);
}; // namespace EnemyEclInstr
}; // namespace th06
