#include "stdafx.h"

#include "EffectManager.hpp"

#include "Chain.hpp"
#include "ChainPriorities.hpp"

namespace th06
{
EffectManager g_EffectManager;
ChainElem g_EffectManagerExecChain;
ChainElem g_EffectManagerDrawChain;

void EffectManager::Reset()
{
    memset(this, 0, sizeof(*this));
}

EffectCallbackResult Effect_RandomSplash(Effect *effect)
{
    if (effect->timer == 0 && effect->timer.HasTicked())
    {
        effect->unk_11c[0] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) / 12.0f;
        effect->unk_11c[1] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) / 12.0f;
        effect->unk_11c[2] = 0.0f;

        effect->unk_128 = -effect->unk_11c / 19.0f;
    }

    effect->pos1 += effect->unk_11c * g_Supervisor.effectiveFramerateMultiplier;
    effect->unk_11c += effect->unk_128 * g_Supervisor.effectiveFramerateMultiplier;

    return EFFECT_CALLBACK_RESULT_DONE;
}

EffectCallbackResult Effect_RandomSplashBig(Effect *effect)
{
    if (effect->timer == 0 && effect->timer.HasTicked())
    {
        effect->unk_11c[0] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) * 4.0f / 33.0f;
        effect->unk_11c[1] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) * 4.0f / 33.0f;
        effect->unk_11c[2] = 0.0f;

        effect->unk_128 = -effect->unk_11c / 20.0f;
    }

    effect->pos1 += effect->unk_11c * g_Supervisor.effectiveFramerateMultiplier;
    effect->unk_11c += effect->unk_128 * g_Supervisor.effectiveFramerateMultiplier;

    return EFFECT_CALLBACK_RESULT_DONE;
}

EffectCallbackResult Effect_Still(Effect *effect)
{
    effect->pos1 += effect->unk_11c * g_Supervisor.effectiveFramerateMultiplier;
    effect->unk_11c += effect->unk_128 * g_Supervisor.effectiveFramerateMultiplier;

    return EFFECT_CALLBACK_RESULT_DONE;
}

#pragma var_order(posOffset, verticalAngle, matrix, horizontalAngle, normalizedPos)
EffectCallbackResult Effect_Callback4(Effect *effect)
{
    D3DXVECTOR3 posOffset;
    f32 verticalAngle;
    D3DXMATRIX matrix;
    f32 horizontalAngle;
    D3DXVECTOR3 normalizedPos;

    D3DXVec3Normalize(&normalizedPos, &effect->pos2);

    verticalAngle = sinf(effect->angleRelated);
    horizontalAngle = cosf(effect->angleRelated);

    effect->quaternion.x = normalizedPos.x * verticalAngle;
    effect->quaternion.y = normalizedPos.y * verticalAngle;
    effect->quaternion.z = normalizedPos.z * verticalAngle;
    effect->quaternion.w = horizontalAngle;
    D3DXMatrixRotationQuaternion(&matrix, &effect->quaternion);

    posOffset.x = normalizedPos.y * 1.0f - normalizedPos.z * 0.0f;
    posOffset.y = normalizedPos.z * 0.0f - normalizedPos.x * 1.0f;
    posOffset.z = normalizedPos.x * 0.0f - normalizedPos.y * 0.0f;

    if (D3DXVec3LengthSq(&posOffset) < 0.00001f)
    {
        normalizedPos = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    }
    else
    {
        D3DXVec3Normalize(&posOffset, &posOffset);
    }

    posOffset *= effect->distance;
    D3DXVec3TransformCoord(&posOffset, &posOffset, &matrix);
    posOffset.z *= 6.0f;

    effect->pos1 = posOffset + effect->position;

    if (effect->flag_17a)
    {
        effect->unk_17b++;

        if (effect->unk_17b >= 16)
        {
            return EFFECT_CALLBACK_RESULT_STOP;
        }

        float alpha = 1.0f - effect->unk_17b / 16.0f;
        effect->vm.color = COLOR_SET_ALPHA3(effect->vm.color, (i32)(alpha * 255.0f));

        effect->vm.scaleY = 2.0f - alpha;
        effect->vm.scaleX = effect->vm.scaleY;
    }

    return EFFECT_CALLBACK_RESULT_DONE;
}

EffectCallbackResult Effect_Attract(Effect *effect)
{
    f32 angle;

    if (effect->timer == 0 && effect->timer.HasTicked())
    {
        effect->position = effect->pos1;

        angle = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
        effect->pos2.x = cosf(angle);
        effect->pos2.y = sinf(angle);
        effect->pos2.z = 0.0;
    }

    angle = 256.0f - (f32)effect->timer * 256.0f / 60.0f;

    effect->pos1 = angle * effect->pos2 + effect->position;

    return EFFECT_CALLBACK_RESULT_DONE;
}

EffectCallbackResult Effect_AttractSlow(Effect *effect)
{
    f32 angle;

    if (effect->timer == 0 && effect->timer.HasTicked())
    {
        effect->position = effect->pos1;

        angle = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
        effect->pos2.x = cosf(angle);
        effect->pos2.y = sinf(angle);
        effect->pos2.z = 0.0;
    }

    angle = 256.0f - (f32)effect->timer * 256.0f / 240.0f;

    effect->pos1 = angle * effect->pos2 + effect->position;

    return EFFECT_CALLBACK_RESULT_DONE;
}

EffectInfo g_Effects[] = {
    {ANM_SCRIPT_BULLET4_SPAWN_BUBBLE_EXPLOSION_SMALL, NULL},
    {ANM_SCRIPT_BULLET4_SPAWN_BUBBLE_EXPLOSION_SPIRAL, NULL},
    {ANM_SCRIPT_BULLET4_SPAWN_BUBBLE_EXPLOSION_NORMAL, NULL},
    {ANM_SCRIPT_BULLET4_SPAWN_GLOW_1, Effect_RandomSplashBig},
    {ANM_SCRIPT_BULLET4_SPAWN_WHITE_PARTICLE, Effect_RandomSplash},
    {ANM_SCRIPT_BULLET4_SPAWN_RED_PARTICLE, Effect_RandomSplash},
    {ANM_SCRIPT_BULLET4_SPAWN_GREEN_PARTICLE, Effect_RandomSplash},
    {ANM_SCRIPT_BULLET4_SPAWN_BLUE_PARTICLE, Effect_RandomSplash},
    {ANM_SCRIPT_BULLET4_SPAWN_WHITE_PARTICLE_SMALL, Effect_RandomSplash},
    {ANM_SCRIPT_BULLET4_SPAWN_RED_PARTICLE_SMALL, Effect_RandomSplash},
    {ANM_SCRIPT_BULLET4_SPAWN_GREEN_PARTICLE_SMALL, Effect_RandomSplash},
    {ANM_SCRIPT_BULLET4_SPAWN_BLUE_PARTICLE_SMALL, Effect_RandomSplash},
    {ANM_SCRIPT_BULLET4_SCRIPT_17, NULL},
    {ANM_SCRIPT_BULLET4_SCRIPT_18, Effect_Callback4},
    {ANM_SCRIPT_BULLET4_SCRIPT_18, Effect_Callback4},
    {ANM_SCRIPT_BULLET4_SCRIPT_18, Effect_Callback4},
    {ANM_SCRIPT_EFFECTS_SPELLCARD_BACKGROUND, NULL},
    {ANM_SCRIPT_BULLET4_SPAWN_GLOW_2, Effect_Attract},
    {ANM_SCRIPT_BULLET4_SPAWN_GLOW_3, Effect_AttractSlow},
    {ANM_SCRIPT_BULLET4_SCRIPT_19, Effect_Still},
};

Effect *EffectManager::SpawnParticles(i32 effectIdx, D3DXVECTOR3 *pos, i32 count, ZunColor color)
{
    Effect *effect = &this->effects[this->nextIndex];
    for (i32 i = 0; i < MAX_EFFECT_COUNT; i++)
    {
        this->nextIndex++;
        if (this->nextIndex >= MAX_EFFECT_COUNT)
        {
            this->nextIndex = 0;
        }
        if (effect->inUseFlag)
        {
            if (this->nextIndex == 0)
            {
                effect = &this->effects[0];
            }
            else
            {
                effect++;
            }
            continue;
        }

        effect->inUseFlag = true;
        effect->effectId = effectIdx;
        effect->pos1 = *pos;

        g_AnmManager->SetAndExecuteScriptIdx(&effect->vm, g_Effects[effectIdx].anmIdx);

        effect->vm.color = color;
        effect->updateCallback = g_Effects[effectIdx].updateCallback;
        effect->timer = 0;
        effect->flag_17a = false;
        effect->unk_17b = 0;

        if (--count == 0)
            break;

        if (this->nextIndex == 0)
        {
            effect = &this->effects[0];
        }
        else
        {
            effect++;
        }
    }

    return i >= MAX_EFFECT_COUNT ? &this->effects[MAX_EFFECT_COUNT] : effect;
}

ChainCallbackResult EffectManager_OnUpdate(EffectManager *mgr)
{
    Effect *effect = &mgr->effects[0];
    mgr->activeEffects = 0;
    for (i32 i = 0; i < MAX_EFFECT_COUNT; i++, effect++)
    {
        if (!effect->inUseFlag)
        {
            continue;
        }

        mgr->activeEffects++;
        if (effect->updateCallback != NULL && effect->updateCallback(effect) != EFFECT_CALLBACK_RESULT_DONE)
        {
            effect->inUseFlag = false;
        }

        if (g_AnmManager->ExecuteScript(&effect->vm) != 0)
        {
            effect->inUseFlag = false;
        }

        effect->timer++;
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult EffectManager_OnDraw(EffectManager *mgr)
{
    Effect *effect = &mgr->effects[0];
    for (i32 i = 0; i < MAX_EFFECT_COUNT; i++, effect++)
    {
        if (!effect->inUseFlag)
        {
            continue;
        }

        effect->vm.pos = effect->pos1;
        g_AnmManager->Draw3(&effect->vm);
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ZunResult EffectManager_AddedCallback(EffectManager *mgr)
{
    mgr->Reset();
    switch (g_GameManager.currentStage)
    {
    case DummyStage:
    case Stage1:
        if (g_AnmManager->LoadAnm(ANM_FILE_EFFECTS, "data/eff01.anm", ANM_OFFSET_EFFECTS) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case Stage2:
        if (g_AnmManager->LoadAnm(ANM_FILE_EFFECTS, "data/eff02.anm", ANM_OFFSET_EFFECTS) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case Stage3:
        if (g_AnmManager->LoadAnm(ANM_FILE_EFFECTS, "data/eff03.anm", ANM_OFFSET_EFFECTS) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case Stage4:
        if (g_AnmManager->LoadAnm(ANM_FILE_EFFECTS, "data/eff04.anm", ANM_OFFSET_EFFECTS) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case Stage5:
        if (g_AnmManager->LoadAnm(ANM_FILE_EFFECTS, "data/eff05.anm", ANM_OFFSET_EFFECTS) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case Stage6:
        if (g_AnmManager->LoadAnm(ANM_FILE_EFFECTS, "data/eff05.anm", ANM_OFFSET_EFFECTS) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case ExtraStage:
        if (g_AnmManager->LoadAnm(ANM_FILE_EFFECTS, "data/eff04.anm", ANM_OFFSET_EFFECTS) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    }
    return ZUN_SUCCESS;
}

ZunResult EffectManager_DeletedCallback(EffectManager *p)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_EFFECTS);

    return ZUN_SUCCESS;
}

ZunResult EffectManager_RegisterChain()
{
    EffectManager *mgr = &g_EffectManager;
    mgr->Reset();

    g_EffectManagerExecChain.SetCallback((ChainCallback)EffectManager_OnUpdate);
    g_EffectManagerExecChain.addedCallback = (ChainAddedCallback)EffectManager_AddedCallback;
    g_EffectManagerExecChain.deletedCallback = (ChainAddedCallback)EffectManager_DeletedCallback;
    g_EffectManagerExecChain.arg = mgr;

    if (g_FuncChain.AddToCalcChain(&g_EffectManagerExecChain, TH_CHAIN_PRIO_CALC_EFFECTMANAGER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    g_EffectManagerDrawChain.SetCallback((ChainCallback)EffectManager_OnDraw);
    g_EffectManagerDrawChain.arg = mgr;
    g_FuncChain.AddToDrawChain(&g_EffectManagerDrawChain, TH_CHAIN_PRIO_DRAW_EFFECTMANAGER);

    return ZUN_SUCCESS;
}

void EffectManager_CutChain()
{
    g_FuncChain.Cut(&g_EffectManagerExecChain);
    g_FuncChain.Cut(&g_EffectManagerDrawChain);
}
} // namespace th06
