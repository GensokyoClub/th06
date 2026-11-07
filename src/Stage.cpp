#include "stdafx.h"

#include "ChainPriorities.hpp"
#include "ScreenEffect.hpp"
#include "Stage.hpp"

namespace th06
{
ChainCallbackResult Stage_OnUpdate(Stage *stage);
ChainCallbackResult Stage_OnDrawHighPrio(Stage *stage);
ChainCallbackResult Stage_OnDrawLowPrio(Stage *stage);
ZunResult Stage_AddedCallback(Stage *stage);
ZunResult Stage_DeletedCallback(Stage *stage);

Stage g_Stage;
ChainElem g_StageCalcChain;
ChainElem g_StageDrawHighPrioChain;
ChainElem g_StageOnDrawLowPrioChain;

#define GET_ARG(type, num) ((type *)curInstr->args)[num]
#define GET_INT_ARG(num) GET_ARG(i32, num)
#define GET_FLOAT_ARG(num) GET_ARG(float, num)
#define GET_FLOAT3_ARG() GET_ARG(D3DXVECTOR3, 0)

#pragma var_order(posInterpRatio, curInstr, pos)
ChainCallbackResult Stage_OnUpdate(Stage *stage)
{
    f32 posInterpRatio;
    D3DXVECTOR3 pos;
    StdRawInstr *curInstr;

    if (stage->stdData == NULL)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (g_GameManager.isTimeStopped)
    {
        // When Sakuya uses her time stop ability, we want to darken her
        // spellcard background a bit, to give a visual indication of what's
        // going on.
        ALPHA(stage->spellcardBackground.color) = 0x60;
        RED(stage->spellcardBackground.color) = 0x80;
        GREEN(stage->spellcardBackground.color) = 0x30;
        BLUE(stage->spellcardBackground.color) = 0x30;
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    for (;;)
    {
        curInstr = stage->beginningOfScript + stage->instructionIndex;
        switch (curInstr->opcode)
        {
        case STD_OPCODE_CAMERA_POSITION_KEY:
            if (curInstr->time == -1)
            {
                stage->positionInterpInitial = GET_FLOAT3_ARG();
                stage->position.x = stage->positionInterpInitial.x;
                stage->position.y = stage->positionInterpInitial.y;
                stage->position.z = stage->positionInterpInitial.z;
            }
            else if (stage->scriptTime >= curInstr->time)
            {
                pos = GET_FLOAT3_ARG();
                stage->position.x = pos.x;
                stage->position.y = pos.y;
                stage->position.z = pos.z;
                stage->positionInterpInitial = pos;
                stage->positionInterpStartTime = curInstr->time;
                stage->instructionIndex++;
                curInstr++;
                while (curInstr->opcode != 0)
                {
                    curInstr++;
                }
                stage->positionInterpEndTime = curInstr->time;
                stage->positionInterpFinal = GET_FLOAT3_ARG();
            }
            break;
        case STD_OPCODE_FOG:
            if (stage->scriptTime >= curInstr->time)
            {
                stage->sky.fogColor = GET_INT_ARG(0);
                stage->sky.nearPlane = GET_FLOAT_ARG(1);
                stage->sky.farPlane = GET_FLOAT_ARG(2);
                if (stage->skyInterpDuration == 0)
                {
                    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, stage->sky.fogColor);
                    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)&stage->sky.nearPlane);
                    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)&stage->sky.farPlane);
                }
                stage->instructionIndex++;
                stage->skyInterpFinal = stage->sky;
                continue;
            }
            break;
        case STD_OPCODE_FOG_INTERP:
            if (stage->scriptTime >= curInstr->time)
            {
                stage->skyInterpInitial = stage->sky;
                stage->skyInterpDuration = GET_INT_ARG(0);
                stage->skyInterpTime = 0;
                stage->instructionIndex++;
                continue;
            }
            break;
        case STD_OPCODE_CAMERA_FACING:
            if (stage->scriptTime >= curInstr->time)
            {
                stage->facingInterpInitial = stage->facingInterpFinal;
                stage->facingInterpFinal = GET_FLOAT3_ARG();
                stage->instructionIndex++;
                continue;
            }
            break;
        case STD_OPCODE_CAMERA_FACING_INTERP_LINEAR:
            if (stage->scriptTime >= curInstr->time)
            {
                stage->facingInterpDuration = GET_INT_ARG(0);
                stage->facingInterpTime = 0;
                stage->instructionIndex++;
                continue;
            }
            break;
        case STD_OPCODE_STD_PAUSE:
            if (stage->unpauseFlag)
            {
                stage->instructionIndex++;
                stage->unpauseFlag = 0;
                continue;
            }
            break;
        }
        if (curInstr->time != -1)
        {
            posInterpRatio = ((f32)stage->scriptTime - stage->positionInterpStartTime) /
                             (stage->positionInterpEndTime - stage->positionInterpStartTime);
            pos = stage->positionInterpFinal;
            stage->position.x = ZUN_LERP(stage->positionInterpInitial.x, pos.x, posInterpRatio);
            stage->position.y = ZUN_LERP(stage->positionInterpInitial.y, pos.y, posInterpRatio);
            stage->position.z = ZUN_LERP(stage->positionInterpInitial.z, pos.z, posInterpRatio);
        }
        if (stage->facingInterpDuration != 0)
        {
            if (stage->facingInterpTime < stage->facingInterpDuration)
            {
                stage->facingInterpTime++;
            }
            else
            {
                stage->facingInterpTime = stage->facingInterpDuration;
            }
            pos = stage->facingInterpFinal - stage->facingInterpInitial;
            f32 facingInterpRatio = (f32)stage->facingInterpTime / stage->facingInterpDuration;
            g_GameManager.stageCameraFacingDir.x = pos.x * facingInterpRatio + stage->facingInterpInitial.x;
            g_GameManager.stageCameraFacingDir.y = pos.y * facingInterpRatio + stage->facingInterpInitial.y;
            g_GameManager.stageCameraFacingDir.z = pos.z * facingInterpRatio + stage->facingInterpInitial.z;
        }
        if (stage->skyInterpDuration != 0)
        {
            stage->skyInterpTime++;
            f32 skyInterpRatio = (f32)stage->skyInterpTime / stage->skyInterpDuration;
            if (skyInterpRatio >= 1.0f)
            {
                skyInterpRatio = 1.0f;
            }
            for (i32 i = 0; i < 4; i++)
            {
                COLOR_CHANNEL(stage->sky.fogColor, i) =
                    ZUN_LERP((f32)COLOR_CHANNEL(stage->skyInterpInitial.fogColor, i),
                             (f32)COLOR_CHANNEL(stage->skyInterpFinal.fogColor, i), skyInterpRatio);
            }
            stage->sky.nearPlane =
                ZUN_LERP(stage->skyInterpInitial.nearPlane, stage->skyInterpFinal.nearPlane, skyInterpRatio);
            stage->sky.farPlane =
                ZUN_LERP(stage->skyInterpInitial.farPlane, stage->skyInterpFinal.farPlane, skyInterpRatio);
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, stage->sky.fogColor);
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)&stage->sky.nearPlane);
            g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)&stage->sky.farPlane);
            if (stage->skyInterpTime >= stage->skyInterpDuration)
            {
                stage->skyInterpDuration = 0;
            }
        }
        if (curInstr->opcode != STD_OPCODE_STD_PAUSE)
        {
            stage->scriptTime++;
        }
        stage->UpdateObjects();
        if (stage->spellcardState >= SPELL_STATE_FADING_IN)
        {
            if (stage->spellcardTicks == SPELL_BACKGROUND_FADE_IN_DURATION)
            {
                stage->spellcardState++; // SPELL_STATE_ACTIVE
            }
            stage->spellcardTicks++;
            g_AnmManager->ExecuteScript(&stage->spellcardBackground);
        }
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
}

ChainCallbackResult Stage_OnDrawHighPrio(Stage *stage)
{
    if (stage->skyNeedsSetup)
    {
        stage->skyNeedsSetup = false;
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, stage->sky.fogColor);
    }
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)&stage->sky.nearPlane);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)&stage->sky.farPlane);
    
    // Stop rendering the background once a spell background
    // has finished fading in since it wouldn't be visible anyway
    if (stage->spellcardState <= SPELL_STATE_FADING_IN)
    {
        if (!g_Gui.IsStageFinished())
        {
            stage->RenderObjects(0);
            stage->RenderObjects(1);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult Stage_OnDrawLowPrio(Stage *stage)
{
    // Stop rendering the background once a spell background
    // has finished fading in since it wouldn't be visible anyway
    if (stage->spellcardState <= SPELL_STATE_FADING_IN)
    {
        if (!g_Gui.IsStageFinished())
        {
            stage->RenderObjects(2);
            stage->RenderObjects(3);

            i32 spellBackgroundAlpha;
            if (stage->spellcardState == SPELL_STATE_FADING_IN)
            {
                ZunRect gameRegion;
                gameRegion.left = GAME_REGION_POS_X;
                gameRegion.top = GAME_REGION_POS_Y;
                gameRegion.right = GAME_REGION_POS_RIGHT;
                gameRegion.bottom = GAME_REGION_POS_BOTTOM;
                spellBackgroundAlpha = (stage->spellcardTicks * 255) / SPELL_BACKGROUND_FADE_IN_DURATION;
                ScreenEffect_DrawSquare(&gameRegion, COLOR_SET_ALPHA(COLOR_BLACK, spellBackgroundAlpha));
            }
        }
    }
    if (stage->spellcardState >= SPELL_STATE_FADING_IN)
    {
        // Why does this need to check the frameskip like
        // this, won't that just restart the script?
        if (stage->spellcardTicks <= g_Supervisor.cfg.frameskipConfig)
        {
            g_AnmManager->SetAndExecuteScriptIdx(&stage->spellcardBackground, ANM_SCRIPT_EFFECTS_SPELLCARD_BACKGROUND);
        }
        g_AnmManager->Draw(&stage->spellcardBackground);
    }
    g_Supervisor.viewport.MinZ = 0.0f;
    g_Supervisor.viewport.MaxZ = 0.5f;
    SetupCameraStageBackground(0.0f);
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

    f32 plane = 1000.0f;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)&plane);
    plane = 2000.0f;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)&plane);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ZunResult Stage_AddedCallback(Stage *stage)
{
    static StageFile g_StageFiles[] = {
        {"dummy", "dummy"},
        {"data/stg1bg.anm", "data/stage1.std"},
        {"data/stg2bg.anm", "data/stage2.std"},
        {"data/stg3bg.anm", "data/stage3.std"},
        {"data/stg4bg.anm", "data/stage4.std"},
        {"data/stg5bg.anm", "data/stage5.std"},
        {"data/stg6bg.anm", "data/stage6.std"},
        {"data/stg7bg.anm", "data/stage7.std"},
    };

    stage->scriptTime = 0;

    stage->instructionIndex = 0;
    stage->position.x = 0.0f;
    stage->position.y = 0.0f;
    stage->position.z = 0.0f;
    stage->spellcardState = SPELL_STATE_NOT_ACTIVE;
    stage->skyInterpDuration = 0;

    if (stage->LoadStageData(g_StageFiles[g_GameManager.currentStage].anmFile,
                             g_StageFiles[g_GameManager.currentStage].stdFile) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    stage->sky.fogColor = COLOR_BLACK;
    stage->sky.nearPlane = 200.0f;
    stage->sky.farPlane = 500.0f;
    stage->facingInterpFinal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    stage->facingInterpInitial = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

    stage->facingInterpDuration = 1;
    stage->facingInterpTime = 0;
    stage->unpauseFlag = 0;

    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, stage->sky.fogColor);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)&stage->sky.nearPlane);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)&stage->sky.farPlane);
    return ZUN_SUCCESS;
}

ZunResult Stage_RegisterChain(u32 stage)
{
    Stage *stg = &g_Stage;

    memset(stg, 0, sizeof(Stage));
    stg->stdData = NULL;

    stg->timer = 0;

    stg->stage = stage;
    g_StageCalcChain.SetCallback((ChainCallback)Stage_OnUpdate);
    g_StageCalcChain.addedCallback = (ChainAddedCallback)Stage_AddedCallback;
    g_StageCalcChain.deletedCallback = (ChainDeletedCallback)Stage_DeletedCallback;
    g_StageCalcChain.arg = stg;

    if (g_FuncChain.AddToCalcChain(&g_StageCalcChain, TH_CHAIN_PRIO_CALC_STAGE) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    g_StageDrawHighPrioChain.SetCallback((ChainCallback)Stage_OnDrawHighPrio);
    g_StageDrawHighPrioChain.arg = stg;
    g_FuncChain.AddToDrawChain(&g_StageDrawHighPrioChain, TH_CHAIN_PRIO_DRAW_HIGH_PRIO_STAGE);
    g_StageOnDrawLowPrioChain.SetCallback((ChainCallback)Stage_OnDrawLowPrio);
    g_StageOnDrawLowPrioChain.arg = stg;
    g_FuncChain.AddToDrawChain(&g_StageOnDrawLowPrioChain, TH_CHAIN_PRIO_DRAW_LOW_PRIO_STAGE);

    return ZUN_SUCCESS;
}

ZunResult Stage_DeletedCallback(Stage *s)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_STAGEBG);
    ZUN_SAFE_FREE(s->quadVms);
    ZUN_SAFE_FREE(s->stdData);
    return ZUN_SUCCESS;
}

void Stage_CutChain()
{
    g_FuncChain.Cut(&g_StageCalcChain);
    g_FuncChain.Cut(&g_StageDrawHighPrioChain);
    g_FuncChain.Cut(&g_StageOnDrawLowPrioChain);
}

#pragma var_order(vmIdx, idx, curObj, curQuad)
ZunResult Stage::LoadStageData(const char *anmpath, const char *stdpath)
{
    RawStageObject *curObj;
    RawStageQuadBasic *curQuad;
    i32 idx;
    i32 vmIdx;

    if (g_AnmManager->LoadAnm(ANM_FILE_STAGEBG, anmpath, ANM_OFFSET_STAGEBG) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    this->stdData = (RawStageHeader *)FileSystem::OpenPath(stdpath);
    if (this->stdData == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_STAGE_DATA_CORRUPTED);
        return ZUN_ERROR;
    }
    this->objectsCount = this->stdData->nbObjects;
    this->quadCount = this->stdData->nbFaces;
    this->objectInstances = (RawStageObjectInstance *)(this->stdData->facesOffset + (u32)this->stdData);
    this->beginningOfScript = (StdRawInstr *)(this->stdData->scriptOffset + (u32)this->stdData);
    this->objects = (RawStageObject **)(this->stdData + 1);
    for (idx = 0; idx < this->objectsCount; idx++)
    {
        this->objects[idx] = (RawStageObject *)((u32)this->objects[idx] + (u32)this->stdData);
    }
    this->quadVms = ZUN_ALLOC_ARRAY(AnmVm, this->quadCount);
    for (idx = 0, vmIdx = 0; idx < this->objectsCount; idx++)
    {
        curObj = this->objects[idx];
        curObj->flags = 1;
        curQuad = curObj->quads;
        while (curQuad->type >= 0)
        {
            g_AnmManager->ExecuteAnmIdx(&this->quadVms[vmIdx], curQuad->anmScript + ANM_OFFSET_STAGEBG);
            curQuad->vmIdx = vmIdx++;
            curQuad = (RawStageQuadBasic *)((u8 *)curQuad + curQuad->byteSize);
        }
    }
    return ZUN_SUCCESS;
}

#pragma var_order(objQuadType1, vmsNotFinished, objIdx, vm)
ZunResult Stage::UpdateObjects()
{
    AnmVm *vm;
    RawStageQuadBasic *objQuadType1;
    i32 objIdx;
    i32 vmsNotFinished;

    for (objIdx = 0; objIdx < this->objectsCount; objIdx++)
    {
        RawStageObject *obj = this->objects[objIdx];
        if (obj->flags & 1)
        {
            vmsNotFinished = 0;
            RawStageQuadBasic *objQuad = obj->quads;
            while (objQuad->type >= 0)
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
                obj->flags &= ~1;
            }
        }
    }
    return ZUN_SUCCESS;
}

#pragma var_order(unk8, curQuadVm, instancesDrawn, instance, worldMatrix, obj, quadScaledPos, quadPos, curQuad,        \
                  didDraw, projectSrc)
ZunResult Stage::RenderObjects(i32 zLevel)
{
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
    didDraw = false;
    projectSrc.x = 0.0f;
    projectSrc.y = 0.0f;
    projectSrc.z = 0.0f;
    D3DXMatrixIdentity(&worldMatrix);
    while (instance->id >= 0)
    {
        obj = this->objects[instance->id];
        if (obj->zLevel == zLevel)
        {
            curQuad = obj->quads;
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
            worldMatrix.m[3][2] = worldMatrix.m[3][2] - obj->size.z;
            D3DXVec3Project(&quadPos, &projectSrc, &g_Supervisor.viewport, &g_Supervisor.projectionMatrix,
                            &g_Supervisor.viewMatrix, &worldMatrix);
            if (quadPos.y >= g_Supervisor.viewport.Y &&
                quadPos.y <= g_Supervisor.viewport.Y + g_Supervisor.viewport.Height)
            {
                goto render;
            }

            // And finally B
            worldMatrix.m[3][1] = worldMatrix.m[3][1] + obj->size.y;
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
            didDraw = true;
            while (curQuad->type >= 0)
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
                        float quadWidth;
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
                        curQuadVm->scaleY = curQuadVm->scaleX = (quadScaledPos.x - quadPos.x) / quadWidth;
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
} // namespace th06
