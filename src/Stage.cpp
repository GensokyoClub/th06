#include "Stage.hpp"
#include "AnmIdx.hpp"
#include "AnmManager.hpp"
#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "FileSystem.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "ScreenEffect.hpp"
#include "Supervisor.hpp"
#include "ZunColor.hpp"
#include "utils.hpp"
#include <d3d8.h>

namespace th06
{
DIFFABLE_STATIC(ChainElem, g_StageCalcChain)
DIFFABLE_STATIC(ChainElem, g_StageOnDrawHighPrioChain)
DIFFABLE_STATIC(ChainElem, g_StageOnDrawLowPrioChain)

DIFFABLE_STATIC_ARRAY_ASSIGN(StageFile, 8, g_StageFiles) = {
    {"dummy", "dummy"},
    {"data/stg1bg.anm", "data/stage1.std"},
    {"data/stg2bg.anm", "data/stage2.std"},
    {"data/stg3bg.anm", "data/stage3.std"},
    {"data/stg4bg.anm", "data/stage4.std"},
    {"data/stg5bg.anm", "data/stage5.std"},
    {"data/stg6bg.anm", "data/stage6.std"},
    {"data/stg7bg.anm", "data/stage7.std"},
};
DIFFABLE_STATIC(Stage, g_Stage)

Stage::Stage()
{
}

#pragma var_order(posInterpRatio, curInsn, pos, facingDirInterpRatio, skyFogInterpRatio, idx)
ChainCallbackResult Stage::OnUpdate(Stage *stage)
{
    f32 posInterpRatio;
    f32 facingDirInterpRatio;
    D3DXVECTOR3 pos;
    i32 idx;
    f32 skyFogInterpRatio;
    RawStageInstr *curInsn;

    if (stage->stdData == NULL)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (g_GameManager.isTimeStopped)
    {
        // When Sakuya uses her time stop ability, we want to darken her
        // spellcard background a bit, to give a visual indication of what's
        // going on.
        COLOR_SET_COMPONENT(stage->spellcardBackground.color, COLOR_ALPHA_BYTE_IDX, 0x60);
        COLOR_SET_COMPONENT(stage->spellcardBackground.color, COLOR_BLUE_BYTE_IDX, 0x80);
        COLOR_SET_COMPONENT(stage->spellcardBackground.color, COLOR_GREEN_BYTE_IDX, 0x30);
        COLOR_SET_COMPONENT(stage->spellcardBackground.color, COLOR_RED_BYTE_IDX, 0x30);
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    for (;;)
    {
        curInsn = stage->beginningOfScript + stage->instructionIndex;
        switch (curInsn->opcode)
        {
        case STDOP_CAMERA_POSITION_KEY:
            if (curInsn->frame == -1)
            {
                stage->positionInterpInitial = *(D3DXVECTOR3 *)curInsn->args;
                stage->position.x = stage->positionInterpInitial.x;
                stage->position.y = stage->positionInterpInitial.y;
                stage->position.z = stage->positionInterpInitial.z;
            }
            else if ((ZunBool)(stage->scriptTime.current >= curInsn->frame))
            {
                pos = *(D3DXVECTOR3 *)curInsn->args;
                stage->position.x = pos.x;
                stage->position.y = pos.y;
                stage->position.z = pos.z;
                stage->positionInterpInitial = pos;
                stage->positionInterpStartTime = curInsn->frame;
                stage->instructionIndex++;
                curInsn++;
                while (curInsn->opcode != 0)
                {
                    curInsn++;
                }
                stage->positionInterpEndTime = curInsn->frame;
                stage->positionInterpFinal = *(D3DXVECTOR3 *)curInsn->args;
            }
            break;
        case STDOP_FOG:
            if ((ZunBool)(stage->scriptTime.current >= curInsn->frame))
            {
                stage->skyFog.color = curInsn->args[0];
                stage->skyFog.nearPlane = ((f32 *)curInsn->args)[1];
                stage->skyFog.farPlane = ((f32 *)curInsn->args)[2];
                if (stage->skyFogInterpDuration == 0)
                {
                    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, stage->skyFog.color);
                    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(u32 *)&stage->skyFog.nearPlane);
                    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(u32 *)&stage->skyFog.farPlane);
                }
                stage->instructionIndex++;
                stage->skyFogInterpFinal = stage->skyFog;
                continue;
            }
            break;
        case STDOP_FOG_INTERP:
            if ((ZunBool)(stage->scriptTime.current >= curInsn->frame))
            {
                stage->skyFogInterpInitial = stage->skyFog;
                stage->skyFogInterpDuration = curInsn->args[0];
                stage->skyFogInterpTimer.InitializeForPopup();
                stage->instructionIndex++;
                continue;
            }
            break;
        case STDOP_CAMERA_FACING:
            if ((ZunBool)(stage->scriptTime.current >= curInsn->frame))
            {
                stage->facingDirInterpInitial = stage->facingDirInterpFinal;
                stage->facingDirInterpFinal = *(D3DXVECTOR3 *)curInsn->args;
                stage->instructionIndex++;
                continue;
            }
            break;
        case STDOP_CAMERA_FACING_INTERP_LINEAR:
            if ((ZunBool)(stage->scriptTime.current >= curInsn->frame))
            {
                stage->facingDirInterpDuration = curInsn->args[0];
                stage->facingDirInterpTimer.InitializeForPopup();
                stage->instructionIndex++;
                continue;
            }
            break;
        case STDOP_PAUSE:
            if (stage->unpauseFlag)
            {
                stage->instructionIndex++;
                stage->unpauseFlag = '\0';
                continue;
            }
            break;
        }
        if (curInsn->frame != -1)
        {
            posInterpRatio = (stage->scriptTime.AsFramesFloat() - stage->positionInterpStartTime) /
                             (stage->positionInterpEndTime - stage->positionInterpStartTime);
            pos = stage->positionInterpFinal;
            stage->position.x =
                (pos.x - stage->positionInterpInitial.x) * posInterpRatio + stage->positionInterpInitial.x;
            stage->position.y =
                (pos.y - stage->positionInterpInitial.y) * posInterpRatio + stage->positionInterpInitial.y;
            stage->position.z =
                (pos.z - stage->positionInterpInitial.z) * posInterpRatio + stage->positionInterpInitial.z;
        }
        if (stage->facingDirInterpDuration != 0)
        {
            if ((ZunBool)(stage->facingDirInterpTimer.current < stage->facingDirInterpDuration))
            {
                stage->facingDirInterpTimer.Tick();
            }
            else
            {
                stage->facingDirInterpTimer.SetCurrent(stage->facingDirInterpDuration);
            }
            pos = stage->facingDirInterpFinal - stage->facingDirInterpInitial;
            facingDirInterpRatio = stage->facingDirInterpTimer.AsFramesFloat() / stage->facingDirInterpDuration;
            g_GameManager.stageCameraFacingDir.x = pos.x * facingDirInterpRatio + stage->facingDirInterpInitial.x;
            g_GameManager.stageCameraFacingDir.y = pos.y * facingDirInterpRatio + stage->facingDirInterpInitial.y;
            g_GameManager.stageCameraFacingDir.z = pos.z * facingDirInterpRatio + stage->facingDirInterpInitial.z;
        }
        if (stage->skyFogInterpDuration != 0)
        {
            stage->skyFogInterpTimer.Tick();
            skyFogInterpRatio = stage->skyFogInterpTimer.AsFramesFloat() / stage->skyFogInterpDuration;
            if (skyFogInterpRatio >= 1.0f)
            {
                skyFogInterpRatio = 1.0;
            }
            for (idx = 0; idx < 4; idx++)
            {
                COLOR_SET_COMPONENT(stage->skyFog.color, idx,
                                    (u8)(((f32)COLOR_GET_COMPONENT(stage->skyFogInterpFinal.color, idx) -
                                          (f32)COLOR_GET_COMPONENT(stage->skyFogInterpInitial.color, idx)) *
                                             skyFogInterpRatio +
                                         (f32)COLOR_GET_COMPONENT(stage->skyFogInterpInitial.color, idx)));
            }
            stage->skyFog.nearPlane =
                (stage->skyFogInterpFinal.nearPlane - stage->skyFogInterpInitial.nearPlane) * skyFogInterpRatio +
                stage->skyFogInterpInitial.nearPlane;
            stage->skyFog.farPlane =
                (stage->skyFogInterpFinal.farPlane - stage->skyFogInterpInitial.farPlane) * skyFogInterpRatio +
                stage->skyFogInterpInitial.farPlane;
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, stage->skyFog.color);
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(u32 *)&stage->skyFog.nearPlane);
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(u32 *)&stage->skyFog.farPlane);
            if ((ZunBool)(stage->skyFogInterpTimer.current >= stage->skyFogInterpDuration))
            {
                stage->skyFogInterpDuration = 0;
            }
        }
        if (curInsn->opcode != STDOP_PAUSE)
        {
            stage->scriptTime.Tick();
        }
        stage->UpdateObjects();
        if (stage->spellcardState >= RUNNING)
        {
            if (stage->ticksSinceSpellcardStarted == 60)
            {
                stage->spellcardState = static_cast<SpellcardState>(stage->spellcardState + 1);
            }
            stage->ticksSinceSpellcardStarted = stage->ticksSinceSpellcardStarted + 1;
            g_AnmManager->ExecuteScript(&stage->spellcardBackground);
        }
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
}

ChainCallbackResult Stage::OnDrawHighPrio(Stage *stage)
{
    if (stage->skyFogNeedsSetup)
    {
        stage->skyFogNeedsSetup = 0;
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, stage->skyFog.color);
    }
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(u32 *)&stage->skyFog.nearPlane);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(u32 *)&stage->skyFog.farPlane);
    if (stage->spellcardState <= RUNNING)
    {
        if (!g_Gui.IsStageFinished())
        {
            stage->RenderObjects(0);
            stage->RenderObjects(1);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(val, stageToSpellcardBackgroundAlpha, gameRegion)
ChainCallbackResult Stage::OnDrawLowPrio(Stage *stage)
{
    f32 val;
    i32 stageToSpellcardBackgroundAlpha;
    ZunRect gameRegion;

    if (stage->spellcardState <= RUNNING)
    {
        if (!g_Gui.IsStageFinished())
        {
            stage->RenderObjects(2);
            stage->RenderObjects(3);
            if (stage->spellcardState == RUNNING)
            {
                gameRegion.left = GAME_REGION_LEFT;
                gameRegion.top = GAME_REGION_TOP;
                gameRegion.right = GAME_REGION_LEFT + GAME_REGION_WIDTH;
                gameRegion.bottom = GAME_REGION_TOP + GAME_REGION_HEIGHT;
                stageToSpellcardBackgroundAlpha = (stage->ticksSinceSpellcardStarted * 255) / 60;
                ScreenEffect::DrawSquare(&gameRegion, stageToSpellcardBackgroundAlpha << 24);
            }
        }
    }
    if (RUNNING <= stage->spellcardState)
    {
        if (stage->ticksSinceSpellcardStarted <= g_Supervisor.cfg.frameskipConfig)
        {
            g_AnmManager->SetAndExecuteScriptIdx(&stage->spellcardBackground, ANM_SCRIPT_EFFECTS_SPELLCARD_BACKGROUND);
        }
        g_AnmManager->Draw(&stage->spellcardBackground);
    }
    g_Supervisor.viewport.MinZ = 0.0;
    g_Supervisor.viewport.MaxZ = 0.5;
    GameManager::SetupCameraStageBackground(0);
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    val = 1000.0f;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(u32 *)&val);
    val = 2000.0f;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(u32 *)&val);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(interpFinal, interpInitial, scriptTimer, facingDirTimer)
ZunResult Stage::AddedCallback(Stage *stage)
{
    ZunTimer *facingDirTimer;
    ZunTimer *scriptTimer;

    D3DXVECTOR3 interpFinal;
    D3DXVECTOR3 interpInitial;

    scriptTimer = &stage->scriptTime;
    scriptTimer->InitializeForPopup();

    stage->instructionIndex = 0;
    stage->position.x = 0.0;
    stage->position.y = 0.0;
    stage->position.z = 0.0;
    stage->spellcardState = NOT_RUNNING;
    stage->skyFogInterpDuration = 0;

    if (stage->LoadStageData(g_StageFiles[g_GameManager.currentStage].anmFile,
                             g_StageFiles[g_GameManager.currentStage].stdFile) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    stage->skyFog.color = COLOR_BLACK;
    stage->skyFog.nearPlane = 200.0;
    stage->skyFog.farPlane = 500.0;
    interpFinal.x = 0;
    interpFinal.y = 0;
    interpFinal.z = 1.0;
    stage->facingDirInterpFinal = interpFinal;

    interpInitial.x = 0;
    interpInitial.y = 0;
    interpInitial.z = 1.0;
    stage->facingDirInterpInitial = interpInitial;

    stage->facingDirInterpDuration = 1;
    facingDirTimer = &stage->facingDirInterpTimer;
    facingDirTimer->InitializeForPopup();
    stage->unpauseFlag = 0;

    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, stage->skyFog.color);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)&stage->skyFog.nearPlane);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)&stage->skyFog.farPlane);
    return ZUN_SUCCESS;
}

#pragma var_order(stg, timer)
ZunResult Stage::RegisterChain(u32 stage)
{

    Stage *stg = &g_Stage;
    ZunTimer *timer;

    memset(stg, 0, sizeof(Stage));
    stg->stdData = NULL;

    timer = &stg->timer;
    timer->InitializeForPopup();

    stg->stage = stage;
    g_StageCalcChain.callback = (ChainCallback)Stage::OnUpdate;
    g_StageCalcChain.addedCallback = NULL;
    g_StageCalcChain.deletedCallback = NULL;
    g_StageCalcChain.addedCallback = (ChainAddedCallback)Stage::AddedCallback;
    g_StageCalcChain.deletedCallback = (ChainDeletedCallback)Stage::DeletedCallback;
    g_StageCalcChain.arg = stg;

    if (g_Chain.AddToCalcChain(&g_StageCalcChain, TH_CHAIN_PRIO_CALC_STAGE))
    {
        return ZUN_ERROR;
    }
    g_StageOnDrawHighPrioChain.callback = (ChainCallback)OnDrawHighPrio;
    g_StageOnDrawHighPrioChain.addedCallback = NULL;
    g_StageOnDrawHighPrioChain.deletedCallback = NULL;
    g_StageOnDrawHighPrioChain.arg = stg;
    g_Chain.AddToDrawChain(&g_StageOnDrawHighPrioChain, TH_CHAIN_PRIO_DRAW_HIGH_PRIO_STAGE);
    g_StageOnDrawLowPrioChain.callback = (ChainCallback)OnDrawLowPrio;
    g_StageOnDrawLowPrioChain.addedCallback = NULL;
    g_StageOnDrawLowPrioChain.deletedCallback = NULL;
    g_StageOnDrawLowPrioChain.arg = stg;
    g_Chain.AddToDrawChain(&g_StageOnDrawLowPrioChain, TH_CHAIN_PRIO_DRAW_LOW_PRIO_STAGE);

    return ZUN_SUCCESS;
}

ZunResult Stage::DeletedCallback(Stage *s)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_STAGEBG);
    if (s->quadVms != NULL)
    {
        void *quadVms = s->quadVms;
        free(quadVms);
        s->quadVms = NULL;
    }
    if (s->stdData != NULL)
    {
        void *stdData = s->stdData;
        free(stdData);
        s->stdData = NULL;
    }
    return ZUN_SUCCESS;
}

void Stage::CutChain()
{
    g_Chain.Cut(&g_StageCalcChain);
    g_Chain.Cut(&g_StageOnDrawHighPrioChain);
    g_Chain.Cut(&g_StageOnDrawLowPrioChain);
}

#pragma var_order(vmIdx, idx, curObj, curQuad, sizeVmArr)
ZunResult Stage::LoadStageData(char *anmpath, char *stdpath)
{
    RawStageObject *curObj;
    RawStageQuadBasic *curQuad;
    i32 idx;
    i32 vmIdx;
    u32 sizeVmArr;

    if (g_AnmManager->LoadAnm(ANM_FILE_STAGEBG, anmpath, ANM_OFFSET_STAGEBG) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    this->stdData = (RawStageHeader *)FileSystem::OpenPath(stdpath, false);
    if (this->stdData == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_STAGE_DATA_CORRUPTED);
        return ZUN_ERROR;
    }
    this->objectsCount = this->stdData->nbObjects;
    this->quadCount = this->stdData->nbFaces;
    this->objectInstances = (RawStageObjectInstance *)(this->stdData->facesOffset + (i32)this->stdData);
    this->beginningOfScript = (RawStageInstr *)(this->stdData->scriptOffset + (i32)this->stdData);
    this->objects = (RawStageObject **)(this->stdData + 1);
    for (idx = 0; idx < this->objectsCount; idx++)
    {
        this->objects[idx] = (RawStageObject *)((i32)this->objects[idx] + (i32)this->stdData);
    }
    sizeVmArr = this->quadCount * sizeof(AnmVm);
    this->quadVms = (AnmVm *)malloc(sizeVmArr);
    for (idx = 0, vmIdx = 0; idx < this->objectsCount; idx++)
    {
        curObj = this->objects[idx];
        curObj->flags = 1;
        curQuad = &curObj->firstQuad;
        while (0 <= curQuad->type)
        {
            g_AnmManager->ExecuteAnmIdx(&this->quadVms[vmIdx], curQuad->anmScript + ANM_OFFSET_STAGEBG);
            curQuad->vmIdx = vmIdx++;
            curQuad = (RawStageQuadBasic *)((u8 *)curQuad + curQuad->byteSize);
        }
    }
    return ZUN_SUCCESS;
}

#pragma var_order(objQuadType1, vmsNotFinished, objIdx, vm, obj, objQuad)
ZunResult Stage::UpdateObjects()
{
    AnmVm *vm;
    RawStageQuadBasic *objQuad;
    RawStageQuadBasic *objQuadType1;
    i32 objIdx;
    i32 vmsNotFinished;
    RawStageObject *obj;

    for (objIdx = 0; objIdx < this->objectsCount; objIdx++)
    {
        obj = this->objects[objIdx];
        if (obj->flags & 1 != 0)
        {
            vmsNotFinished = 0;
            objQuad = &obj->firstQuad;
            while (0 <= objQuad->type)
            {
                vm = &this->quadVms[objQuad->vmIdx];
                switch (objQuad->type)
                {
                case 0:
                    g_AnmManager->ExecuteScript(vm);
                    break;
                case 1:
                    // I assume this casts it, but this is all dead code
                    // as the engine doesn't contain any other reference
                    // to type 1 quads.
                    objQuadType1 = objQuad;
                    g_AnmManager->ExecuteScript(vm);
                    break;
                }
                if (vm->currentInstruction != NULL)
                {
                    vmsNotFinished++;
                }
                objQuad = (RawStageQuadBasic *)((i32)&objQuad->type + objQuad->byteSize);
            }
            if (vmsNotFinished == 0)
            {
                obj->flags = obj->flags & ~1;
            }
        }
    }
    return ZUN_SUCCESS;
}

#pragma var_order(unk8, curQuadVm, instancesDrawn, instance, worldMatrix, obj, quadScaledPos, quadPos, curQuad,        \
                  didDraw, projectSrc, quadWidth)
ZunResult Stage::RenderObjects(i32 zLevel)
{
    f32 quadWidth;
    D3DXVECTOR3 projectSrc;
    ZunBool didDraw;
    RawStageQuadBasic *curQuad;
    D3DXVECTOR3 quadPos;
    D3DXVECTOR3 quadScaledPos;
    RawStageObject *obj;
    D3DXMATRIX worldMatrix;
    RawStageObjectInstance *instance;
    i32 instancesDrawn;
    AnmVm *curQuadVm;
    i32 unk8;

    instance = &this->objectInstances[0];
    instancesDrawn = 0;
    didDraw = 0;
    projectSrc.x = 0.0;
    projectSrc.y = 0.0;
    projectSrc.z = 0.0;
    D3DXMatrixIdentity(&worldMatrix);
    while (instance->id >= 0)
    {
        obj = this->objects[instance->id];
        if (obj->zLevel == zLevel)
        {
            curQuad = &obj->firstQuad;
            unk8 = 0;

            //  Say hello to helper cube:
            //
            //    ^
            //    |           A------B.
            //  y |           |`.    | `.
            //    |           |  `C--+---D
            //    |  x        |   |  |   |
            //    o----->     E---+--F.  |
            //     `.          `. |    `.|
            //    z  `_          `G------H
            //
            //   It's beautiful, I know. E is at point 0, 0, 0 here.
            //
            // During this process, zun will project the world matrix to each of
            // the 8 corner of the kube that reprents this object, and check if
            // any of them is visible on the viewport.
            //
            // It will check them in the following order: C, G, E, A, D, H, F, B.

            // It first starts by checking point C
            worldMatrix.m[3][0] = obj->position.x + instance->position.x - this->position.x;
            worldMatrix.m[3][1] = -(obj->position.y + instance->position.y - this->position.y);
            worldMatrix.m[3][2] = obj->position.z + instance->position.z - this->position.z + obj->size.z;
            D3DXVec3Project(&quadPos, &projectSrc, &g_Supervisor.viewport, &g_Supervisor.projectionMatrix,
                            &g_Supervisor.viewMatrix, &worldMatrix);

            if (quadPos.y >= g_Supervisor.viewport.Y &&
                quadPos.y <= g_Supervisor.viewport.Y + g_Supervisor.viewport.Height)
            {
                goto render;
            }

            // Then G:
            worldMatrix.m[3][1] = worldMatrix.m[3][1] - obj->size.y;
            D3DXVec3Project(&quadPos, &projectSrc, &g_Supervisor.viewport, &g_Supervisor.projectionMatrix,
                            &g_Supervisor.viewMatrix, &worldMatrix);
            if (quadPos.y >= g_Supervisor.viewport.Y &&
                quadPos.y <= g_Supervisor.viewport.Y + g_Supervisor.viewport.Height)
            {
                goto render;
            }

            // Then E
            worldMatrix.m[3][2] = worldMatrix.m[3][2] - obj->size.z;
            D3DXVec3Project(&quadPos, &projectSrc, &g_Supervisor.viewport, &g_Supervisor.projectionMatrix,
                            &g_Supervisor.viewMatrix, &worldMatrix);
            if (quadPos.y >= g_Supervisor.viewport.Y &&
                quadPos.y <= g_Supervisor.viewport.Y + g_Supervisor.viewport.Height)
            {
                goto render;
            }

            // Then A
            worldMatrix.m[3][1] = worldMatrix.m[3][1] + obj->size.y;
            D3DXVec3Project(&quadPos, &projectSrc, &g_Supervisor.viewport, &g_Supervisor.projectionMatrix,
                            &g_Supervisor.viewMatrix, &worldMatrix);
            if (quadPos.y >= g_Supervisor.viewport.Y &&
                quadPos.y <= g_Supervisor.viewport.Y + g_Supervisor.viewport.Height)
            {
                goto render;
            }

            // Then D
            worldMatrix.m[3][0] = obj->position.x + instance->position.x - this->position.x + obj->size.x;
            worldMatrix.m[3][1] = -(obj->position.y + instance->position.y - this->position.y);
            worldMatrix.m[3][2] = obj->position.z + instance->position.z - this->position.z + obj->size.z;
            D3DXVec3Project(&quadPos, &projectSrc, &g_Supervisor.viewport, &g_Supervisor.projectionMatrix,
                            &g_Supervisor.viewMatrix, &worldMatrix);
            if (quadPos.y >= g_Supervisor.viewport.Y &&
                quadPos.y <= g_Supervisor.viewport.Y + g_Supervisor.viewport.Height)
            {
                goto render;
            }

            // Then H
            worldMatrix.m[3][1] = worldMatrix.m[3][1] - obj->size.y;
            D3DXVec3Project(&quadPos, &projectSrc, &g_Supervisor.viewport, &g_Supervisor.projectionMatrix,
                            &g_Supervisor.viewMatrix, &worldMatrix);
            if (quadPos.y >= g_Supervisor.viewport.Y &&
                quadPos.y <= g_Supervisor.viewport.Y + g_Supervisor.viewport.Height)
            {
                goto render;
            }

            // Then F
            worldMatrix.m[3][2] = worldMatrix.m[3][2] - (obj->size).z;
            D3DXVec3Project(&quadPos, &projectSrc, &g_Supervisor.viewport, &g_Supervisor.projectionMatrix,
                            &g_Supervisor.viewMatrix, &worldMatrix);
            if (quadPos.y >= g_Supervisor.viewport.Y &&
                quadPos.y <= g_Supervisor.viewport.Y + g_Supervisor.viewport.Height)
            {
                goto render;
            }

            // And finally B
            worldMatrix.m[3][1] = worldMatrix.m[3][1] + (obj->size).y;
            D3DXVec3Project(&quadPos, &projectSrc, &g_Supervisor.viewport, &g_Supervisor.projectionMatrix,
                            &g_Supervisor.viewMatrix, &worldMatrix);
            if (quadPos.y >= g_Supervisor.viewport.Y &&
                quadPos.y <= g_Supervisor.viewport.Y + g_Supervisor.viewport.Height)
            {
                goto render;
            }

            // If none of the points were in the viewport, we can skip this object
            // entirely.
            goto skip;

        render:
            didDraw = 1;
            while (0 <= curQuad->type)
            {
                curQuadVm = this->quadVms + curQuad->vmIdx;
                switch (curQuad->type)
                {
                case 0:
                    curQuadVm->pos.x = curQuad->position.x + instance->position.x - this->position.x;
                    curQuadVm->pos.y = curQuad->position.y + instance->position.y - this->position.y;
                    curQuadVm->pos.z = curQuad->position.z + instance->position.z - this->position.z;
                    if (curQuad->size.x != 0.0f)
                    {
                        curQuadVm->scaleX = curQuad->size.x / curQuadVm->sprite->widthPx;
                    }
                    if (curQuad->size.y != 0.0f)
                    {
                        curQuadVm->scaleY = curQuad->size.y / curQuadVm->sprite->heightPx;
                    }
                    if (curQuadVm->autoRotate == 2)
                    {
                        if (curQuad->size.x != 0.0f)
                        {
                            quadWidth = curQuad->size.x;
                        }
                        else
                        {
                            quadWidth = curQuadVm->sprite->widthPx;
                        }
                        worldMatrix.m[3][0] = curQuadVm->pos.x;
                        worldMatrix.m[3][1] = -curQuadVm->pos.y;
                        worldMatrix.m[3][2] = curQuadVm->pos.z;
                        D3DXVec3Project(&quadPos, &projectSrc, &g_Supervisor.viewport, &g_Supervisor.projectionMatrix,
                                        &g_Supervisor.viewMatrix, &worldMatrix);
                        worldMatrix.m[3][0] = quadWidth * curQuadVm->scaleX + worldMatrix.m[3][0];
                        D3DXVec3Project(&quadScaledPos, &projectSrc, &g_Supervisor.viewport,
                                        &g_Supervisor.projectionMatrix, &g_Supervisor.viewMatrix, &worldMatrix);
                        curQuadVm->scaleX = (quadScaledPos.x - quadPos.x) / quadWidth;
                        curQuadVm->scaleY = curQuadVm->scaleX;
                        curQuadVm->pos = quadPos;
                        g_AnmManager->DrawFacingCamera(curQuadVm);
                    }
                    else
                    {
                        g_AnmManager->Draw3(curQuadVm);
                    }
                    break;
                }
                curQuad = (RawStageQuadBasic *)((i32)&curQuad->type + curQuad->byteSize);
            }
            instancesDrawn++;
        }
    skip:
        instance++;
    }
    return ZUN_SUCCESS;
}
}; // namespace th06
