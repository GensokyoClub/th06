#pragma once

#include "Enemy.hpp"

namespace th06
{
void ExInsCirnoRainbowBallJank(Enemy *enemy, EclRawInstr *curInstr);
void ExInsShootAtRandomArea(Enemy *enemy, EclRawInstr *curInstr);
void ExInsShootStarPattern(Enemy *enemy, EclRawInstr *curInstr);
void ExInsPatchouliShottypeSetVars(Enemy *enemy, EclRawInstr *curInstr);
void ExInsStage56Func4(Enemy *enemy, EclRawInstr *curInstr);
void ExInsStage5Func5(Enemy *enemy, EclRawInstr *curInstr);
void ExInsBatWingEffect(Enemy *enemy, EclRawInstr *curInstr);
void ExInsStage6Func7(Enemy *enemy, EclRawInstr *curInstr);
void ExInsStage6Func8(Enemy *enemy, EclRawInstr *curInstr);
void ExInsStage6Func9(Enemy *enemy, EclRawInstr *curInstr);
void ExInsStage6Func11(Enemy *enemy, EclRawInstr *curInstr);
void ExInsHandleBatTransformation(Enemy *enemy, EclRawInstr *curInstr);
void ExInsStage4Func12(Enemy *enemy, EclRawInstr *curInstr);
#if BUILT_SINCE(100)
void ExInsStageXFunc13(Enemy *enemy, EclRawInstr *curInstr);
void ExInsStageXFunc14(Enemy *enemy, EclRawInstr *curInstr);
void ExInsStageXFunc15(Enemy *enemy, EclRawInstr *curInstr);
void ExInsFlandreFinalContextUpdate(Enemy *enemy, EclRawInstr *curInstr);
#endif
} // namespace th06
