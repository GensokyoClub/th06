#include "stdafx.h"

#include "ChainPriorities.hpp"
#include "EffectManager.hpp"

namespace th06
{
EffectManager g_EffectManager;
ChainElem g_EffectManagerExecChain;
ChainElem g_EffectManagerDrawChain;

void EffectManager::Reset()
{
    memset(this, 0, sizeof(*this));
}

EffectCallbackResult Effect_RandomSpray(Effect *effect)
{
    if (effect->timer == 0 && effect->timer.HasTicked())
    {
        effect->velocity[0] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) / 12.0f;
        effect->velocity[1] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) / 12.0f;
        effect->velocity[2] = 0.0f;

        effect->acceleration = -effect->velocity / 19.0f;
    }

    effect->position += effect->velocity * g_Supervisor.gameSpeed;
    effect->velocity += effect->acceleration * g_Supervisor.gameSpeed;

    return EFFECT_CALLBACK_RESULT_DONE;
}

EffectCallbackResult Effect_RandomSprayLarge(Effect *effect)
{
    if (effect->timer == 0 && effect->timer.HasTicked())
    {
        effect->velocity[0] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) * 4.0f / 33.0f;
        effect->velocity[1] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) * 4.0f / 33.0f;
        effect->velocity[2] = 0.0f;

        effect->acceleration = -effect->velocity / 20.0f;
    }

    effect->position += effect->velocity * g_Supervisor.gameSpeed;
    effect->velocity += effect->acceleration * g_Supervisor.gameSpeed;

    return EFFECT_CALLBACK_RESULT_DONE;
}

EffectCallbackResult Effect_Move(Effect *effect)
{
    effect->position += effect->velocity * g_Supervisor.gameSpeed;
    effect->velocity += effect->acceleration * g_Supervisor.gameSpeed;

    return EFFECT_CALLBACK_RESULT_DONE;
}

#pragma var_order(orbit, sine, rotationMatrix, cosine, orbitAxis)
EffectCallbackResult Effect_SpellOrbit(Effect *effect)
{
    D3DXVECTOR3 orbit;
    f32 sine;
    D3DXMATRIX rotationMatrix;
    f32 cosine;
    D3DXVECTOR3 orbitAxis;

    D3DXVec3Normalize(&orbitAxis, &effect->direction);

    sine = sinf(effect->angle);
    cosine = cosf(effect->angle);

    effect->quaternion.x = orbitAxis.x * sine;
    effect->quaternion.y = orbitAxis.y * sine;
    effect->quaternion.z = orbitAxis.z * sine;
    effect->quaternion.w = cosine;
    D3DXMatrixRotationQuaternion(&rotationMatrix, &effect->quaternion);

    orbit.x = orbitAxis.y * 1.0f - orbitAxis.z * 0.0f;
    orbit.y = orbitAxis.z * 0.0f - orbitAxis.x * 1.0f;
    orbit.z = orbitAxis.x * 0.0f - orbitAxis.y * 0.0f;

    if (D3DXVec3LengthSq(&orbit) < 0.00001f)
    {
        // BUG: This should be setting orbit instead of orbitAxis
        orbitAxis = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    }
    else
    {
        D3DXVec3Normalize(&orbit, &orbit);
    }

    orbit *= effect->radius;
    D3DXVec3TransformCoord(&orbit, &orbit, &rotationMatrix);
    orbit.z *= 6.0f;

    effect->position = orbit + effect->offset;

    if (effect->isFadingOut)
    {
        if (++effect->fadeOutTimer >= 16)
        {
            return EFFECT_CALLBACK_RESULT_STOP;
        }

        float fade = 1.0f - effect->fadeOutTimer / 16.0f;
        effect->vm.color = COLOR_SET_ALPHA2(effect->vm.color, (i32)(fade * 255.0f));
        effect->vm.scaleX = effect->vm.scaleY = 2.0f - fade;
    }

    return EFFECT_CALLBACK_RESULT_DONE;
}

EffectCallbackResult Effect_RandomAttract(Effect *effect)
{
    f32 temp;

    if (effect->timer == 0 && effect->timer.HasTicked())
    {
        effect->offset = effect->position;

        temp = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
        effect->direction.x = cosf(temp);
        effect->direction.y = sinf(temp);
        effect->direction.z = 0.0f;
    }

    temp = RAMP_DOWN(256.0f, (f32)effect->timer, 60.0f);

    effect->position = temp * effect->direction + effect->offset;

    return EFFECT_CALLBACK_RESULT_DONE;
}

EffectCallbackResult Effect_RandomAttractSlow(Effect *effect)
{
    f32 temp;

    if (effect->timer == 0 && effect->timer.HasTicked())
    {
        effect->offset = effect->position;

        temp = g_Rng.GetRandomF32InRange(ZUN_2PI) - ZUN_PI;
        effect->direction.x = cosf(temp);
        effect->direction.y = sinf(temp);
        effect->direction.z = 0.0f;
    }

    temp = RAMP_DOWN(256.0f, (f32)effect->timer, 240.0f);

    effect->position = temp * effect->direction + effect->offset;

    return EFFECT_CALLBACK_RESULT_DONE;
}

EffectInfo g_Effects[] = {
    {ANM_SCRIPT_BULLET4_DEATH_POP, NULL},
    {ANM_SCRIPT_BULLET4_DEATH_SWIRL, NULL},
    {ANM_SCRIPT_BULLET4_DEATH_BIG_POP, NULL},
    {ANM_SCRIPT_BULLET4_FLASH_BIG, Effect_RandomSprayLarge},
    {ANM_SCRIPT_BULLET4_SPARK, Effect_RandomSpray},
    {ANM_SCRIPT_BULLET4_SPARK_RED, Effect_RandomSpray},
    {ANM_SCRIPT_BULLET4_SPARK_GREEN, Effect_RandomSpray},
    {ANM_SCRIPT_BULLET4_SPARK_BLUE, Effect_RandomSpray},
    {ANM_SCRIPT_BULLET4_SMALL_SPARK, Effect_RandomSpray},
    {ANM_SCRIPT_BULLET4_SMALL_SPARK_RED, Effect_RandomSpray},
    {ANM_SCRIPT_BULLET4_SMALL_SPARK_GREEN, Effect_RandomSpray},
    {ANM_SCRIPT_BULLET4_SMALL_SPARK_BLUE, Effect_RandomSpray},
    {ANM_SCRIPT_BULLET4_FLASH, NULL},
    {ANM_SCRIPT_BULLET4_SPELL_ORBIT, Effect_SpellOrbit},
    {ANM_SCRIPT_BULLET4_SPELL_ORBIT, Effect_SpellOrbit},
    {ANM_SCRIPT_BULLET4_SPELL_ORBIT, Effect_SpellOrbit},
    {ANM_SCRIPT_EFFECTS_SPELLCARD_BACKGROUND, NULL},
    {ANM_SCRIPT_BULLET4_GLOW, Effect_RandomAttract},
    {ANM_SCRIPT_BULLET4_DIM_GLOW, Effect_RandomAttractSlow},
    {ANM_SCRIPT_BULLET4_DIM_SPARK, Effect_Move},
};

Effect *EffectManager::SpawnParticles(i32 effectId, D3DXVECTOR3 *pos, i32 count, ZunColor tint)
{
    Effect *effect = &this->effects[this->nextIndex];
    for (i32 i = 0; i < MAX_EFFECT_COUNT; i++)
    {
        if (++this->nextIndex >= MAX_EFFECT_COUNT)
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
        effect->effectId = effectId;
        effect->position = *pos;

        g_AnmManager->SetAndExecuteScriptIdx(&effect->vm, g_Effects[effectId].anmScript);

        effect->vm.color = tint;
        effect->updateCallback = g_Effects[effectId].updateCallback;
        effect->timer = 0;
        effect->isFadingOut = false;
        effect->fadeOutTimer = 0;

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

        if (g_AnmManager->RunAnm(&effect->vm) != SCRIPT_RUNNING)
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

        effect->vm.pos = effect->position;
        g_AnmManager->Draw3DRotated(&effect->vm);
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
