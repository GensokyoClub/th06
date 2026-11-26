#include "stdafx.h"

#include "ScreenEffect.hpp"

namespace th06
{

ScreenEffect g_ScreenEffect; // UNUSED FOREVER

void ScreenEffect_Clear(D3DCOLOR color)
{
    g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
    if (FAILED(g_Supervisor.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
    }
    g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
    if (FAILED(g_Supervisor.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
    }
}

// Why is this not in GameWindow.cpp? Don't ask me...
void ScreenEffect_SetViewport(D3DCOLOR color)
{
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = GAME_WINDOW_WIDTH;
    g_Supervisor.viewport.Height = GAME_WINDOW_HEIGHT;
    g_Supervisor.viewport.MinZ = 0.0f;
    g_Supervisor.viewport.MaxZ = 1.0f;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    ScreenEffect_Clear(color);
}

ChainCallbackResult ScreenEffect_CalcFadeIn(ScreenEffect *effect)
{
    if (effect->duration != 0)
    {
        effect->fadeAlpha = RAMP_DOWN(255.0f, (f32)effect->timer, effect->duration);
        if (effect->fadeAlpha < 0)
        {
            effect->fadeAlpha = 0;
        }
    }

    if (effect->timer >= effect->duration)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    effect->timer++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

void ScreenEffect_DrawRect(FloatRect *rect, D3DCOLOR rectColor)
{
    VertexDiffuseXyzrwh vertices[4];

    vertices[0].position = D3DXVECTOR3(rect->left, rect->top, 0.0f);
    vertices[1].position = D3DXVECTOR3(rect->right, rect->top, 0.0f);
    vertices[2].position = D3DXVECTOR3(rect->left, rect->bottom, 0.0f);
    vertices[3].position = D3DXVECTOR3(rect->right, rect->bottom, 0.0f);
    vertices[0].position_w = vertices[1].position_w = vertices[2].position_w = vertices[3].position_w = 1.0f;
    vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = vertices[3].diffuse = rectColor;

    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    if (!g_Supervisor.IsDepthTestDisabled())
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }

    g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
    g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
    g_AnmManager->ClearVertexShader();
    g_AnmManager->ClearSprite();
    g_AnmManager->ClearTexture();
    g_AnmManager->ClearColorOp();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearZWrite();

    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
}

ChainCallbackResult ScreenEffect_CalcFadeOut(ScreenEffect *effect)
{
    if (effect->duration != 0)
    {
        effect->fadeAlpha = RAMP_UP(255.0f, (f32)effect->timer, effect->duration);
        if (effect->fadeAlpha < 0)
        {
            effect->fadeAlpha = 0;
        }
    }

    if (effect->timer >= effect->duration)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    effect->timer++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult ScreenEffect_DrawFadeIn(ScreenEffect *effect)
{
    FloatRect fadeRect;

    fadeRect.left = 0.0f;
    fadeRect.top = 0.0f;
    fadeRect.right = GAME_WINDOW_WIDTH;
    fadeRect.bottom = GAME_WINDOW_HEIGHT;
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = GAME_WINDOW_WIDTH;
    g_Supervisor.viewport.Height = GAME_WINDOW_HEIGHT;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    ScreenEffect_DrawRect(&fadeRect, COLOR_SET_UNMASKED(effect->genericParam, effect->fadeAlpha));
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult ScreenEffect_DrawFadeOut(ScreenEffect *effect)
{
    FloatRect fadeRect;
    fadeRect.left = GAME_REGION_POS_X;
    fadeRect.top = GAME_REGION_POS_Y;
    fadeRect.right = GAME_REGION_POS_RIGHT;
    fadeRect.bottom = GAME_REGION_POS_BOTTOM;
    ScreenEffect_DrawRect(&fadeRect, COLOR_SET_UNMASKED(effect->genericParam, effect->fadeAlpha));
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult ScreenEffect_ShakeScreen(ScreenEffect *effect)
{
    if (g_GameManager.isTimeStopped)
    {
        g_GameManager.gameRegionPosition.x = GAME_REGION_POS_X;
        g_GameManager.gameRegionPosition.y = GAME_REGION_POS_Y;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT;
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    effect->timer++;
    if (effect->timer >= effect->duration)
    {
        g_GameManager.gameRegionPosition.x = GAME_REGION_POS_X;
        g_GameManager.gameRegionPosition.y = GAME_REGION_POS_Y;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT;
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    f32 screenOffset = (((f32)effect->timer * (effect->shakinessParam - effect->genericParam)) / effect->duration) +
                       effect->genericParam;

    switch (g_Rng.GetRandomU32InRange(3))
    {
    case 0:
        g_GameManager.gameRegionPosition.x = GAME_REGION_POS_X;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH;
        break;
    case 1:
        g_GameManager.gameRegionPosition.x = GAME_REGION_POS_X + screenOffset;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH - screenOffset;
        break;
    case 2:
        g_GameManager.gameRegionPosition.x = GAME_REGION_POS_X;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH - screenOffset;
        break;
    }

    switch (g_Rng.GetRandomU32InRange(3))
    {
    case 0:
        g_GameManager.gameRegionPosition.y = GAME_REGION_POS_Y;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT;
        break;
    case 1:
        g_GameManager.gameRegionPosition.y = GAME_REGION_POS_Y + screenOffset;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT - screenOffset;
        break;
    case 2:
        g_GameManager.gameRegionPosition.y = GAME_REGION_POS_Y;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT - screenOffset;
        break;
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ZunResult ScreenEffect_AddedCallback(ScreenEffect *effect)
{
    effect->timer = 0;
    return ZUN_SUCCESS;
}

static ZunResult ScreenEffect_DeletedCallback(ScreenEffect *effect)
{
    effect->calcChain->deletedCallback = NULL;
    g_FuncChain.Cut(effect->drawChain);
    effect->drawChain = NULL;
    ZUN_DELETE(effect);

    return ZUN_SUCCESS;
}

#pragma var_order(calcChain, drawChain, effect)
ScreenEffect *ScreenEffect_RegisterChain(i32 type, u32 duration, u32 effectParam1, u32 effectParam2,
                                         u32 unusedEffectParam)
{
    ChainElem *calcChain = NULL;
    ChainElem *drawChain = NULL;

    ScreenEffect *effect = ZUN_NEW(ScreenEffect);

    if (effect == NULL)
    {
        return NULL;
    }

    memset(effect, 0, sizeof(*effect));

    switch (type)
    {
    case SCREEN_EFFECT_FADE_IN:
        calcChain = g_FuncChain.CreateElem((ChainCallback)ScreenEffect_CalcFadeIn);
        drawChain = g_FuncChain.CreateElem((ChainCallback)ScreenEffect_DrawFadeIn);
        break;
    case SCREEN_EFFECT_SHAKE:
        calcChain = g_FuncChain.CreateElem((ChainCallback)ScreenEffect_ShakeScreen);
        break;
    case SCREEN_EFFECT_FADE_OUT:
        calcChain = g_FuncChain.CreateElem((ChainCallback)ScreenEffect_CalcFadeOut);
        drawChain = g_FuncChain.CreateElem((ChainCallback)ScreenEffect_DrawFadeOut);
    }

    calcChain->addedCallback = (ChainAddedCallback)ScreenEffect_AddedCallback;
    calcChain->deletedCallback = (ChainAddedCallback)ScreenEffect_DeletedCallback;
    calcChain->arg = effect;
    effect->type = type;
    effect->duration = duration;
    effect->genericParam = effectParam1;
    effect->shakinessParam = effectParam2;
    effect->unusedParam = unusedEffectParam;

    if (g_FuncChain.AddToCalcChain(calcChain, TH_CHAIN_PRIO_CALC_SCREENEFFECT) != ZUN_SUCCESS)
    {
        return NULL;
    }

    if (drawChain != NULL)
    {
        drawChain->arg = effect;
        g_FuncChain.AddToDrawChain(drawChain, TH_CHAIN_PRIO_DRAW_SCREENEFFECT);
    }

    effect->calcChain = calcChain;
    effect->drawChain = drawChain;
    return effect;
}
} // namespace th06
