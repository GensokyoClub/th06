#include "stdafx.h"

#include "ChainPriorities.hpp"
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
    if (effect->effectLength != 0)
    {
        effect->fadeAlpha = 255.0f - (((f32)effect->timer * 255.0f) / effect->effectLength);
        if (effect->fadeAlpha < 0)
        {
            effect->fadeAlpha = 0;
        }
    }

    if (effect->timer >= effect->effectLength)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    effect->timer++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

void ScreenEffect_DrawSquare(ZunRect *rect, D3DCOLOR rectColor)
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
    if (effect->effectLength != 0)
    {
        effect->fadeAlpha = ((f32)effect->timer * 255.0f) / effect->effectLength;
        if (effect->fadeAlpha < 0)
        {
            effect->fadeAlpha = 0;
        }
    }

    if (effect->timer >= effect->effectLength)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    effect->timer++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult ScreenEffect_DrawFadeIn(ScreenEffect *effect)
{
    ZunRect fadeRect;

    fadeRect.left = 0.0f;
    fadeRect.top = 0.0f;
    fadeRect.right = GAME_WINDOW_WIDTH;
    fadeRect.bottom = GAME_WINDOW_HEIGHT;
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = GAME_WINDOW_WIDTH;
    g_Supervisor.viewport.Height = GAME_WINDOW_HEIGHT;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    ScreenEffect_DrawSquare(&fadeRect, (effect->fadeAlpha << 24) | effect->genericParam);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult ScreenEffect_DrawFadeOut(ScreenEffect *effect)
{
    ZunRect fadeRect;
    fadeRect.left = GAME_REGION_POS_X;
    fadeRect.top = GAME_REGION_POS_Y;
    fadeRect.right = GAME_REGION_POS_RIGHT;
    fadeRect.bottom = GAME_REGION_POS_BOTTOM;
    ScreenEffect_DrawSquare(&fadeRect, (effect->fadeAlpha << 24) | effect->genericParam);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult ScreenEffect_ShakeScreen(ScreenEffect *effect)
{
    if (g_GameManager.isTimeStopped)
    {
        g_GameManager.gameRegionScreenPos.x = GAME_REGION_POS_X;
        g_GameManager.gameRegionScreenPos.y = GAME_REGION_POS_Y;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT;
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    effect->timer++;
    if (effect->timer >= effect->effectLength)
    {
        g_GameManager.gameRegionScreenPos.x = GAME_REGION_POS_X;
        g_GameManager.gameRegionScreenPos.y = GAME_REGION_POS_Y;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT;
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    f32 screenOffset = (((f32)effect->timer * (effect->shakinessParam - effect->genericParam)) / effect->effectLength) +
                       effect->genericParam;

    switch (g_Rng.GetRandomU32InRange(3))
    {
    case 0:
        g_GameManager.gameRegionScreenPos.x = GAME_REGION_POS_X;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH;
        break;
    case 1:
        g_GameManager.gameRegionScreenPos.x = GAME_REGION_POS_X + screenOffset;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH - screenOffset;
        break;
    case 2:
        g_GameManager.gameRegionScreenPos.x = GAME_REGION_POS_X;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH - screenOffset;
        break;
    }

    switch (g_Rng.GetRandomU32InRange(3))
    {
    case 0:
        g_GameManager.gameRegionScreenPos.y = GAME_REGION_POS_Y;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT;
        break;
    case 1:
        g_GameManager.gameRegionScreenPos.y = GAME_REGION_POS_Y + screenOffset;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT - screenOffset;
        break;
    case 2:
        g_GameManager.gameRegionScreenPos.y = GAME_REGION_POS_Y;
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
    effect->calcChainElement->deletedCallback = NULL;
    g_FuncChain.Cut(effect->drawChainElement);
    effect->drawChainElement = NULL;
    ZUN_DELETE(effect);

    return ZUN_SUCCESS;
}

#pragma var_order(calcChainElem, drawChainElem, createdEffect)
ScreenEffect *ScreenEffect_RegisterChain(i32 effect, u32 ticks, u32 effectParam1, u32 effectParam2,
                                         u32 unusedEffectParam)
{
    ChainElem *calcChainElem = NULL;
    ChainElem *drawChainElem = NULL;

    ScreenEffect *createdEffect = ZUN_NEW(ScreenEffect);

    if (createdEffect == NULL)
    {
        return NULL;
    }

    memset(createdEffect, 0, sizeof(*createdEffect));

    switch (effect)
    {
    case SCREEN_EFFECT_FADE_IN:
        calcChainElem = g_FuncChain.CreateElem((ChainCallback)ScreenEffect_CalcFadeIn);
        drawChainElem = g_FuncChain.CreateElem((ChainCallback)ScreenEffect_DrawFadeIn);
        break;
    case SCREEN_EFFECT_SHAKE:
        calcChainElem = g_FuncChain.CreateElem((ChainCallback)ScreenEffect_ShakeScreen);
        break;
    case SCREEN_EFFECT_FADE_OUT:
        calcChainElem = g_FuncChain.CreateElem((ChainCallback)ScreenEffect_CalcFadeOut);
        drawChainElem = g_FuncChain.CreateElem((ChainCallback)ScreenEffect_DrawFadeOut);
    }

    calcChainElem->addedCallback = (ChainAddedCallback)ScreenEffect_AddedCallback;
    calcChainElem->deletedCallback = (ChainAddedCallback)ScreenEffect_DeletedCallback;
    calcChainElem->arg = createdEffect;
    createdEffect->usedEffect = (ScreenEffects)effect;
    createdEffect->effectLength = ticks;
    createdEffect->genericParam = effectParam1;
    createdEffect->shakinessParam = effectParam2;
    createdEffect->unusedParam = unusedEffectParam;

    if (g_FuncChain.AddToCalcChain(calcChainElem, TH_CHAIN_PRIO_CALC_SCREENEFFECT) != ZUN_SUCCESS)
    {
        return NULL;
    }

    if (drawChainElem != NULL)
    {
        drawChainElem->arg = createdEffect;
        g_FuncChain.AddToDrawChain(drawChainElem, TH_CHAIN_PRIO_DRAW_SCREENEFFECT);
    }

    createdEffect->calcChainElement = calcChainElem;
    createdEffect->drawChainElement = drawChainElem;
    return createdEffect;
}
} // namespace th06
