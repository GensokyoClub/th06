#include "ScreenEffect.hpp"
#include "AnmManager.hpp"
#include "ChainPriorities.hpp"
#include "GLFunc.hpp"
#include "GameWindow.hpp"
#include "Rng.hpp"
#include "Supervisor.hpp"

#include <SDL2/SDL_video.h>
#include <cstring>

namespace th06
{

void ScreenEffect::Clear(ZunColor color)
{
    f32 a = (color >> 24) / 255.0f;
    f32 r = ((color >> 16) & 0xFF) / 255.0f;
    f32 g = ((color >> 8) & 0xFF) / 255.0f;
    f32 b = (color & 0xFF) / 255.0f;

    g_glFuncTable.glClearColor(r, g, b, a);

    // D3D version clears and presents twice (probably to clear both draw buffers?)
    // For now let's copy that behaviour

    g_glFuncTable.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(g_GameWindow.window);
    g_glFuncTable.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(g_GameWindow.window);

    return;
}

// Why is this not in GameWindow.cpp? Don't ask me...
void ScreenEffect::SetViewport(ZunColor color)
{
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = GAME_WINDOW_WIDTH;
    g_Supervisor.viewport.Height = GAME_WINDOW_HEIGHT;
    g_Supervisor.viewport.MinZ = 0.0;
    g_Supervisor.viewport.MaxZ = 1.0;
    g_Supervisor.viewport.Set();
    ScreenEffect::Clear(color);
}

ChainCallbackResult ScreenEffect::CalcFadeIn(ScreenEffect *effect)
{
    if (effect->effectLength != 0)
    {
        effect->fadeAlpha = 255.0f - ((effect->timer.AsFramesFloat() * 255.0f) / effect->effectLength);
        if (effect->fadeAlpha < 0)
        {
            effect->fadeAlpha = 0;
        }
    }

    if (effect->timer >= effect->effectLength)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    effect->timer.Tick();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

void ScreenEffect::DrawSquare(ZunRect *rect, ZunColor rectColor)
{
    VertexDiffuseXyzrhw vertices[4];

    if (g_AnmManager->currentTextureHandle == 0)
    {
        g_AnmManager->SetCurrentTexture(g_AnmManager->dummyTextureHandle);
    }

    vertices[0].position = ZunVec4(rect->left, rect->top, 0.0f, 1.0f);
    vertices[1].position = ZunVec4(rect->right, rect->top, 0.0f, 1.0f);
    vertices[2].position = ZunVec4(rect->left, rect->bottom, 0.0f, 1.0f);
    vertices[3].position = ZunVec4(rect->right, rect->bottom, 0.0f, 1.0f);

    vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = vertices[3].diffuse = ColorData(rectColor);

    inverseViewportMatrix();

    g_glFuncTable.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    g_glFuncTable.glEnableClientState(GL_COLOR_ARRAY);
    g_glFuncTable.glVertexPointer(4, GL_FLOAT, sizeof(*vertices), &vertices[0].position);
    g_glFuncTable.glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(*vertices), &vertices[0].diffuse);

    if (((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP) & 0x01) == 0)
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
        //        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        //        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }

    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
    //    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    //    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);

    if (((g_Supervisor.cfg.opts >> GCOS_TURN_OFF_DEPTH_TEST) & 0x01) == 0)
    {
        g_glFuncTable.glDepthFunc(GL_ALWAYS);
        g_glFuncTable.glDepthMask(GL_FALSE);
    }

    g_glFuncTable.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    g_glFuncTable.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    g_glFuncTable.glMatrixMode(GL_TEXTURE);
    g_glFuncTable.glPopMatrix();
    g_glFuncTable.glMatrixMode(GL_MODELVIEW);
    g_glFuncTable.glPopMatrix();
    g_glFuncTable.glMatrixMode(GL_PROJECTION);
    g_glFuncTable.glPopMatrix();

    g_AnmManager->SetCurrentVertexShader(0xff);
    g_AnmManager->SetCurrentSprite(NULL);
    g_AnmManager->SetCurrentTexture(0);
    g_AnmManager->SetCurrentColorOp(0xff);
    g_AnmManager->SetCurrentBlendMode(0xff);
    g_AnmManager->SetCurrentZWriteDisable(0xff);

    if (((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP) & 0x01) == 0)
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
        //        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        //        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }

    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
    g_glFuncTable.glDepthFunc(GL_LEQUAL);

    //    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    //    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
}

ChainCallbackResult ScreenEffect::CalcFadeOut(ScreenEffect *effect)
{
    if (effect->effectLength != 0)
    {
        effect->fadeAlpha = (effect->timer.AsFramesFloat() * 255.0f) / effect->effectLength;
        if (effect->fadeAlpha < 0)
        {
            effect->fadeAlpha = 0;
        }
    }

    if (effect->timer >= effect->effectLength)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    effect->timer.Tick();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ScreenEffect *ScreenEffect::RegisterChain(i32 effect, u32 ticks, u32 effectParam1, u32 effectParam2,
                                          u32 unusedEffectParam)
{
    ChainElem *calcChainElem;
    ScreenEffect *createdEffect;
    ChainElem *drawChainElem;

    calcChainElem = NULL;
    drawChainElem = NULL;

    createdEffect = new ScreenEffect;

    if (createdEffect == NULL)
    {
        return NULL;
    }

    std::memset(createdEffect, 0, sizeof(*createdEffect));

    switch (effect)
    {
    case SCREEN_EFFECT_FADE_IN:
        calcChainElem = g_Chain.CreateElem((ChainCallback)ScreenEffect::CalcFadeIn);
        drawChainElem = g_Chain.CreateElem((ChainCallback)ScreenEffect::DrawFadeIn);
        break;
    case SCREEN_EFFECT_SHAKE:
        calcChainElem = g_Chain.CreateElem((ChainCallback)ScreenEffect::ShakeScreen);
        break;
    case SCREEN_EFFECT_FADE_OUT:
        calcChainElem = g_Chain.CreateElem((ChainCallback)ScreenEffect::CalcFadeOut);
        drawChainElem = g_Chain.CreateElem((ChainCallback)ScreenEffect::DrawFadeOut);
    }

    calcChainElem->addedCallback = (ChainAddedCallback)ScreenEffect::AddedCallback;
    calcChainElem->deletedCallback = (ChainAddedCallback)ScreenEffect::DeletedCallback;
    calcChainElem->arg = createdEffect;
    createdEffect->usedEffect = (ScreenEffects)effect;
    createdEffect->effectLength = ticks;
    createdEffect->genericParam = effectParam1;
    createdEffect->shakinessParam = effectParam2;
    createdEffect->unusedParam = unusedEffectParam;

    if (!g_Chain.AddToCalcChain(calcChainElem, TH_CHAIN_PRIO_CALC_SCREENEFFECT))
    {
        return NULL;
    }

    if (drawChainElem != NULL)
    {
        drawChainElem->arg = createdEffect;
        g_Chain.AddToDrawChain(drawChainElem, TH_CHAIN_PRIO_DRAW_SCREENEFFECT);
    }

    createdEffect->calcChainElement = calcChainElem;
    createdEffect->drawChainElement = drawChainElem;
    return createdEffect;
}

ChainCallbackResult ScreenEffect::DrawFadeIn(ScreenEffect *effect)
{
    ZunRect fadeRect;

    fadeRect.left = 0.0f;
    fadeRect.top = 0.0f;
    fadeRect.right = 640.0f;
    fadeRect.bottom = 480.0f;
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = 640;
    g_Supervisor.viewport.Height = 480;
    g_Supervisor.viewport.Set();
    ScreenEffect::DrawSquare(&fadeRect, (effect->fadeAlpha << 24) | effect->genericParam);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ScreenEffect::DrawFadeOut(ScreenEffect *effect)
{
    ZunRect fadeRect;

    fadeRect.left = 32.0f;
    fadeRect.top = 16.0f;
    fadeRect.right = 416.0f;
    fadeRect.bottom = 464.0f;
    ScreenEffect::DrawSquare(&fadeRect, (effect->fadeAlpha << 24) | effect->genericParam);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ScreenEffect::ShakeScreen(ScreenEffect *effect)
{
    f32 screenOffset;

    if (g_GameManager.isTimeStopped)
    {
        g_GameManager.arcadeRegionTopLeftPos.x = 32.0f;
        g_GameManager.arcadeRegionTopLeftPos.y = 16.0f;
        g_GameManager.arcadeRegionSize.x = 384.0f;
        g_GameManager.arcadeRegionSize.y = 448.0f;
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    effect->timer.Tick();
    if (effect->timer >= effect->effectLength)
    {
        g_GameManager.arcadeRegionTopLeftPos.x = 32.0f;
        g_GameManager.arcadeRegionTopLeftPos.y = 16.0f;
        g_GameManager.arcadeRegionSize.x = 384.0f;
        g_GameManager.arcadeRegionSize.y = 448.0f;
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }

    screenOffset =
        ((effect->timer.AsFramesFloat() * (effect->shakinessParam - effect->genericParam)) / effect->effectLength) +
        effect->genericParam;

    switch (g_Rng.GetRandomU32InRange(3))
    {
    case 0:
        g_GameManager.arcadeRegionTopLeftPos.x = 32.0f;
        g_GameManager.arcadeRegionSize.x = 384.0f;
        break;
    case 1:
        g_GameManager.arcadeRegionTopLeftPos.x = 32.0f + screenOffset;
        g_GameManager.arcadeRegionSize.x = 384.0f - screenOffset;
        break;
    case 2:
        g_GameManager.arcadeRegionTopLeftPos.x = 32.0f;
        g_GameManager.arcadeRegionSize.x = 384.0f - screenOffset;
        break;
    }

    switch (g_Rng.GetRandomU32InRange(3))
    {
    case 0:
        g_GameManager.arcadeRegionTopLeftPos.y = 16.0f;
        g_GameManager.arcadeRegionSize.y = 448.0f;
        break;
    case 1:
        g_GameManager.arcadeRegionTopLeftPos.y = 16.0f + screenOffset;
        g_GameManager.arcadeRegionSize.y = 448.0f - screenOffset;
        break;
    case 2:
        g_GameManager.arcadeRegionTopLeftPos.y = 16.0f;
        g_GameManager.arcadeRegionSize.y = 448.0f - screenOffset;
        break;
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

bool ScreenEffect::AddedCallback(ScreenEffect *effect)
{
    effect->timer.InitializeForPopup();
    return true;
}

bool ScreenEffect::DeletedCallback(ScreenEffect *effect)
{
    effect->calcChainElement->deletedCallback = NULL;
    g_Chain.Cut(effect->drawChainElement);
    effect->drawChainElement = NULL;
    delete effect;
    effect = NULL;

    return true;
}
}; // namespace th06
