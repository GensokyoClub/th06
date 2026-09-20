#include "Supervisor.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "Ending.hpp"
#include "GameManager.hpp"
#include "GameWindow.hpp"
#include "Global.hpp"
#include "MainMenu.hpp"
#include "MusicRoom.hpp"
#include "ReplayManager.hpp"
#include "ResultScreen.hpp"
#include "SoundPlayer.hpp"
#include "TextHelper.hpp"
#include "i18n.hpp"
#include "inttypes.hpp"

#include <stdio.h>
#include <string.h>

namespace th06
{
DIFFABLE_STATIC(LPDIRECT3DSURFACE8, g_TextBufferSurface)
DIFFABLE_STATIC(Supervisor, g_Supervisor)

ChainCallbackResult Supervisor::OnUpdate(Supervisor *s)
{

    if (g_SoundPlayer.backgroundMusic != NULL)
    {
        g_SoundPlayer.backgroundMusic->UpdateFadeOut();
    }
    g_LastFrameInput = g_CurFrameInput;
    g_CurFrameInput = Controller::GetInput();
    g_IsEigthFrameOfHeldInput = false;
    if (g_LastFrameInput == g_CurFrameInput)
    {
        if (0x1e <= g_NumOfFramesInputsWereHeld)
        {
            if (g_NumOfFramesInputsWereHeld % 8 == 0)
            {
                g_IsEigthFrameOfHeldInput = true;
            }
            if (0x26 <= g_NumOfFramesInputsWereHeld)
            {
                g_NumOfFramesInputsWereHeld = 0x1e;
            }
        }
        g_NumOfFramesInputsWereHeld++;
    }
    else
    {
        g_NumOfFramesInputsWereHeld = 0;
    }

    if (s->wantedState != s->curState)
    {
        s->wantedState2 = s->wantedState;
        switch (s->wantedState)
        {
        case SUPERVISOR_STATE_INIT:
        REINIT_MAINMENU:
            s->curState = SUPERVISOR_STATE_MAINMENU;
            g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);
            if (MainMenu::RegisterChain(0) != ZUN_SUCCESS)
            {
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            }
            break;
        case SUPERVISOR_STATE_MAINMENU:
            switch (s->curState)
            {
            case SUPERVISOR_STATE_EXITSUCCESS:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SUPERVISOR_STATE_GAMEMANAGER:
                if (GameManager::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SUPERVISOR_STATE_EXITERROR:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_ERROR;
            case SUPERVISOR_STATE_RESULTSCREEN:
                if (ResultScreen::RegisterChain(FALSE) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SUPERVISOR_STATE_MUSICROOM:
                if (MusicRoom::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SUPERVISOR_STATE_ENDING:
                GameManager::CutChain();
                if (Ending::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            }
            break;

        case SUPERVISOR_STATE_RESULTSCREEN:
            switch (s->curState)
            {
            case SUPERVISOR_STATE_EXITSUCCESS:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SUPERVISOR_STATE_MAINMENU:
                s->curState = SUPERVISOR_STATE_INIT;
                goto REINIT_MAINMENU;
            }
            break;
        case SUPERVISOR_STATE_GAMEMANAGER:
            switch (s->curState)
            {
            case SUPERVISOR_STATE_EXITSUCCESS:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;

            case SUPERVISOR_STATE_MAINMENU:
            RETURN_TO_MENU_FROM_GAME:
                GameManager::CutChain();
                s->curState = SUPERVISOR_STATE_INIT;
                ReplayManager::SaveReplay(NULL, NULL);
                goto REINIT_MAINMENU;

            case SUPERVISOR_STATE_RESULTSCREEN_FROMGAME:
                GameManager::CutChain();
                if (ResultScreen::RegisterChain(TRUE) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SUPERVISOR_STATE_GAMEMANAGER_REINIT:
                GameManager::CutChain();
                if (GameManager::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                if (s->curState == SUPERVISOR_STATE_MAINMENU)
                {
                    goto RETURN_TO_MENU_FROM_GAME;
                }
                s->curState = SUPERVISOR_STATE_GAMEMANAGER;
                break;
            case SUPERVISOR_STATE_MAINMENU_REPLAY:
                GameManager::CutChain();
                s->curState = SUPERVISOR_STATE_INIT;
                ReplayManager::SaveReplay(NULL, NULL);
                s->curState = SUPERVISOR_STATE_MAINMENU;
                g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);
                if (MainMenu::RegisterChain(1) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;

            case 10:
                GameManager::CutChain();
                if (Ending::RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            }
            break;
        case SUPERVISOR_STATE_RESULTSCREEN_FROMGAME:
            switch (s->curState)
            {
            case SUPERVISOR_STATE_EXITSUCCESS:
                ReplayManager::SaveReplay(NULL, NULL);
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SUPERVISOR_STATE_MAINMENU:
                s->curState = SUPERVISOR_STATE_INIT;
                ReplayManager::SaveReplay(NULL, NULL);
                goto REINIT_MAINMENU;
            }
            break;
        case SUPERVISOR_STATE_MUSICROOM:
            switch (s->curState)
            {
            case SUPERVISOR_STATE_EXITSUCCESS:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;

            case SUPERVISOR_STATE_MAINMENU:
                s->curState = SUPERVISOR_STATE_INIT;
                goto REINIT_MAINMENU;
            }
            break;
        case SUPERVISOR_STATE_ENDING:
            switch (s->curState)
            {
            case SUPERVISOR_STATE_EXITSUCCESS:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SUPERVISOR_STATE_MAINMENU:
                s->curState = SUPERVISOR_STATE_INIT;
                goto REINIT_MAINMENU;
            case SUPERVISOR_STATE_RESULTSCREEN_FROMGAME:
                if (ResultScreen::RegisterChain(TRUE) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
            }
            break;
        }
        g_CurFrameInput = g_LastFrameInput = g_IsEigthFrameOfHeldInput = 0;
    }

    s->wantedState = s->curState;
    s->calcCount++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult Supervisor::OnDraw(Supervisor *s)
{
    g_AnmManager->SetCurrentVertexShader(0xff);
    g_AnmManager->SetCurrentSprite(NULL);
    g_AnmManager->SetCurrentTexture(NULL);
    g_AnmManager->SetCurrentColorOp(0xff);
    g_AnmManager->SetCurrentBlendMode(0xff);
    g_AnmManager->SetCurrentZWriteDisable(0xff);

    Supervisor::DrawFpsCounter();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(diprange, pvRefBackup)
BOOL CALLBACK Supervisor::ControllerCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    LPVOID pvRefBackup;
    DIPROPRANGE diprange;
    pvRefBackup = pvRef;

    if (lpddoi->dwType & DIDFT_AXIS)
    {
        diprange.diph.dwSize = sizeof(diprange);
        diprange.diph.dwHeaderSize = sizeof(diprange.diph);
        diprange.diph.dwHow = DIPH_BYID;
        diprange.diph.dwObj = lpddoi->dwType;
        diprange.lMin = -1000;
        diprange.lMax = 1000;

        if (FAILED(g_Supervisor.controller->SetProperty(DIPROP_RANGE, &diprange.diph)))
        {
            return FALSE;
        }
    }
    return TRUE;
}

#pragma var_order(chain, supervisor)
ZunResult Supervisor::RegisterChain()
{
    ChainElem *chain;
    Supervisor *supervisor = &g_Supervisor;

    supervisor->wantedState = 0;
    supervisor->curState = -1;
    supervisor->calcCount = 0;

    chain = g_Chain.CreateElem((ChainCallback)Supervisor::OnUpdate);
    chain->arg = supervisor;
    chain->addedCallback = (ChainAddedCallback)Supervisor::AddedCallback;
    chain->deletedCallback = (ChainDeletedCallback)Supervisor::DeletedCallback;
    if (g_Chain.AddToCalcChain(chain, TH_CHAIN_PRIO_CALC_SUPERVISOR) != 0)
    {
        return ZUN_ERROR;
    }

    chain = g_Chain.CreateElem((ChainCallback)Supervisor::OnDraw);
    chain->arg = supervisor;
    g_Chain.AddToDrawChain(chain, TH_CHAIN_PRIO_DRAW_SUPERVISOR);

    return ZUN_SUCCESS;
}

ZunResult Supervisor::AddedCallback(Supervisor *s)
{
    i32 i;

    for (i = 0; i < (i32)(sizeof(s->pbg3Archives) / sizeof(s->pbg3Archives[0])); i++)
    {
        s->pbg3Archives[i] = NULL;
    }

    g_Pbg3Archives = s->pbg3Archives;
    if (s->LoadPbg3(IN_PBG3_INDEX, TH_IN_DAT_FILE))
    {
        return ZUN_ERROR;
    }
    g_AnmManager->LoadSurface(0, "data/title/th06logo.jpg");
    g_AnmManager->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    if (FAILED(g_Supervisor.d3dDevice->Present(NULL, NULL, NULL, NULL)))
        g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);

    g_AnmManager->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    if (FAILED(g_Supervisor.d3dDevice->Present(NULL, NULL, NULL, NULL)))
        g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);

    g_AnmManager->ReleaseSurface(0);

    s->startupTimeBeforeMenuMusic = timeGetTime();
    Supervisor::SetupDInput(s);

    s->midiOutput = ZUN_NEW(MidiOutput);

    g_Rng.Initialize(timeGetTime());

    g_SoundPlayer.InitSoundBuffers();
    if (g_AnmManager->LoadAnm(ANM_FILE_TEXT, "data/text.anm", ANM_OFFSET_TEXT) != 0)
    {
        return ZUN_ERROR;
    }

    if (AsciiManager::RegisterChain() != 0)
    {
        g_GameErrorContext.Log(TH_ERR_ASCIIMANAGER_INIT_FAILED);
        return ZUN_ERROR;
    }

    s->unk198 = 0;
    g_AnmManager->SetupVertexBuffer();
    TextHelper::CreateTextBuffer();
    s->ReleasePbg3(IN_PBG3_INDEX);
    if (g_Supervisor.LoadPbg3(MD_PBG3_INDEX, TH_MD_DAT_FILE) != 0)
        return ZUN_ERROR;

    return ZUN_SUCCESS;
}

ZunResult Supervisor::SetupDInput(Supervisor *supervisor)
{
    HINSTANCE hInst;

    hInst = (HINSTANCE)GetWindowLong(supervisor->hwndGameWindow, GWL_HINSTANCE);
    if (supervisor->IsDInputDisabled())
    {
        return ZUN_ERROR;
    }

    if (FAILED(DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *)&supervisor->dinputIface,
                                  NULL)))
    {
        supervisor->dinputIface = NULL;
        g_GameErrorContext.Log(TH_ERR_DIRECTINPUT_NOT_AVAILABLE);
        return ZUN_ERROR;
    }

    if (FAILED(supervisor->dinputIface->CreateDevice(GUID_SysKeyboard, &supervisor->keyboard, NULL)))
    {
        SAFE_RELEASE(supervisor->dinputIface);
        g_GameErrorContext.Log(TH_ERR_DIRECTINPUT_NOT_AVAILABLE);
        return ZUN_ERROR;
    }

    if (FAILED(supervisor->keyboard->SetDataFormat(&c_dfDIKeyboard)))
    {
        SAFE_RELEASE(supervisor->keyboard);

        SAFE_RELEASE(supervisor->dinputIface);

        g_GameErrorContext.Log(TH_ERR_DIRECTINPUT_SETDATAFORMAT_NOT_AVAILABLE);
        return ZUN_ERROR;
    }

    if (FAILED(supervisor->keyboard->SetCooperativeLevel(supervisor->hwndGameWindow,
                                                         DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY)))
    {
        SAFE_RELEASE(supervisor->keyboard);

        SAFE_RELEASE(supervisor->dinputIface);

        g_GameErrorContext.Log(TH_ERR_DIRECTINPUT_SETCOOPERATIVELEVEL_NOT_AVAILABLE);
        return ZUN_ERROR;
    }

    supervisor->keyboard->Acquire();
    g_GameErrorContext.Log(TH_ERR_DIRECTINPUT_INITIALIZED);

    supervisor->dinputIface->EnumDevices(DI8DEVCLASS_GAMECTRL, Supervisor::EnumGameControllersCb, NULL,
                                         DIEDFL_ATTACHEDONLY);
    if (supervisor->controller)
    {
        supervisor->controller->SetDataFormat(&c_dfDIJoystick2);
        supervisor->controller->SetCooperativeLevel(supervisor->hwndGameWindow, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

        g_Supervisor.controllerCaps.dwSize = sizeof(g_Supervisor.controllerCaps);

        supervisor->controller->GetCapabilities(&g_Supervisor.controllerCaps);
        supervisor->controller->EnumObjects(Supervisor::ControllerCallback, NULL, DIDFT_ALL);

        g_GameErrorContext.Log(TH_ERR_PAD_FOUND);
    }
    return ZUN_SUCCESS;
}

BOOL CALLBACK Supervisor::EnumGameControllersCb(LPCDIDEVICEINSTANCE pdidInstance, LPVOID pContext)
{
    HRESULT result;

    if (!g_Supervisor.controller)
    {
        result = g_Supervisor.dinputIface->CreateDevice(pdidInstance->guidInstance, &g_Supervisor.controller, NULL);
        if (FAILED(result))
        {
            return TRUE;
        }
    }
    return FALSE;
}

ZunResult Supervisor::DeletedCallback(Supervisor *s)
{
    i32 pbg3Idx;

    g_AnmManager->ReleaseVertexBuffer();
    for (pbg3Idx = 0; pbg3Idx < ARRAY_SIZE_SIGNED(s->pbg3Archives); pbg3Idx++)
    {
        s->ReleasePbg3(pbg3Idx);
    }
    g_AnmManager->ReleaseAnm(0);
    AsciiManager::CutChain();
    g_SoundPlayer.StopBGM();
    if (s->midiOutput != NULL)
    {
        s->midiOutput->StopPlayback();
        ZUN_DELETE(s->midiOutput);
    }
    ReplayManager::SaveReplay(NULL, NULL);
    TextHelper::ReleaseTextBuffer();
    if (s->keyboard != NULL)
    {
        s->keyboard->Unacquire();
    }
    SAFE_RELEASE(s->keyboard);
    if (s->controller != NULL)
    {
        s->controller->Unacquire();
    }
    SAFE_RELEASE(s->controller);
    SAFE_RELEASE(s->dinputIface);
    return ZUN_SUCCESS;
}

#pragma var_order(curTime, framerate, fps, elapsed, fpsCounterPos)
void Supervisor::DrawFpsCounter()
{
    DWORD curTime;
    float framerate;
    float elapsed;
    float fps;
    D3DXVECTOR3 fpsCounterPos;

    static u32 g_NumFramesSinceLastTime = 0;
    static DWORD g_LastTime = timeGetTime();
    static char g_FpsCounterBuffer[256];

    curTime = timeGetTime();
    g_NumFramesSinceLastTime = g_NumFramesSinceLastTime + 1 + (u32)g_Supervisor.cfg.frameskipConfig;
    if (500 <= curTime - g_LastTime)
    {
        elapsed = (curTime - g_LastTime) / 1000.f;
        fps = g_NumFramesSinceLastTime / elapsed;
        g_LastTime = curTime;
        g_NumFramesSinceLastTime = 0;
        sprintf(g_FpsCounterBuffer, "%.02ffps", fps);
        if (g_GameManager.isInMenu)
        {
            framerate = 60.f / g_Supervisor.framerateMultiplier;
            g_Supervisor.unk1b8 = g_Supervisor.unk1b8 + framerate;

            if (framerate * .89999998f < fps)
                g_Supervisor.unk1b4 = g_Supervisor.unk1b4 + framerate;
            else if (framerate * 0.69999999f < fps)
                g_Supervisor.unk1b4 = framerate * .8f + g_Supervisor.unk1b4;
            else if (framerate * 0.5f < fps)
                g_Supervisor.unk1b4 = framerate * .6f + g_Supervisor.unk1b4;
            else
                g_Supervisor.unk1b4 = framerate * .5f + g_Supervisor.unk1b4;
        }
    }
    if (!g_Supervisor.isInEnding)
    {
        fpsCounterPos.x = 512.0;
        fpsCounterPos.y = 464.0;
        fpsCounterPos.z = 0.0;
        g_AsciiManager.AddString(&fpsCounterPos, g_FpsCounterBuffer);
    }
    return;
}

void ZunTimer::Initialize()
{
    this->current = 0;
    this->previous = -1;
    this->subFrame = 0;
}

void ZunTimer::Increment(i32 value)
{
    if (g_Supervisor.framerateMultiplier > 0.99f)
    {
        this->current += value;

        return;
    }

    if (value < 0)
    {
        Decrement(-value);

        return;
    }

    this->previous = this->current;
    this->subFrame += g_Supervisor.effectiveFramerateMultiplier * (float)value;

    while (this->subFrame >= 1.0f)
    {
        this->current += 1;
        this->subFrame -= 1.0f;
    }
}

void ZunTimer::Decrement(i32 value)
{
    if (g_Supervisor.framerateMultiplier > 0.99f)
    {
        this->current -= value;

        return;
    }

    if (value < 0)
    {
        Increment(-value);

        return;
    }

    this->previous = this->current;
    this->subFrame -= g_Supervisor.effectiveFramerateMultiplier * (float)value;

    while (this->subFrame < 0.0f)
    {
        this->current -= 1;
        this->subFrame += 1.0f;
    }
}

void Supervisor::TickTimer(i32 *frames, f32 *subframes)
{
    if (this->framerateMultiplier <= 0.99f)
    {
        *subframes += this->effectiveFramerateMultiplier;
        if (*subframes >= 1.0f)
        {
            *frames += 1;
            *subframes -= 1.0f;
        }
    }
    else
    {
        *frames += 1;
    }
}

void Supervisor::ReleasePbg3(i32 pbg3FileIdx)
{
    if (this->pbg3Archives[pbg3FileIdx] == NULL)
    {
        return;
    }

    // Double free! Release is called internally by the Pbg3Archive destructor,
    // and as such should not be called directly. By calling it directly here,
    // it ends up being called twice, which will cause the resources owned by
    // Pbg3Archive to be freed multiple times, which can result in crashes.
    //
    // For some reason, this double-free doesn't cause crashes in the original
    // game. However, this can cause problems in dllbuilds of the game. Maybe
    // some accuracy improvements in the PBG3 handling will remove this
    // difference.
    this->pbg3Archives[pbg3FileIdx]->Release();
    ZUN_DELETE(this->pbg3Archives[pbg3FileIdx]);
}

i32 Supervisor::LoadPbg3(i32 pbg3FileIdx, char *filename)
{
    if (this->pbg3Archives[pbg3FileIdx] == NULL || strcmp(filename, this->pbg3ArchiveNames[pbg3FileIdx]) != 0)
    {
        this->ReleasePbg3(pbg3FileIdx);
        this->pbg3Archives[pbg3FileIdx] = ZUN_NEW(Pbg3Archive);
        utils::DebugPrint("%s open ...\n", filename);
        if (this->pbg3Archives[pbg3FileIdx]->Load(filename) != 0)
        {
            strcpy(this->pbg3ArchiveNames[pbg3FileIdx], filename);

            char verPath[128];
            sprintf(verPath, "ver%.4x.dat", GAME_VERSION);
            i32 res = this->pbg3Archives[pbg3FileIdx]->FindEntry(verPath);
            if (res < 0)
            {
                g_GameErrorContext.Fatal("error : データのバージョンが違います\n");
                return 1;
            }
        }
        else
        {
            // Let's really make sure this is null by nulling twice. I assume
            // there's some kind of inline function here, like it's actually
            // calling this->pbg3Archives.delete(pbg3FileIdx), followed by a
            // manual nulling?
            ZUN_DELETE(this->pbg3Archives[pbg3FileIdx]);
            this->pbg3Archives[pbg3FileIdx] = NULL;
        }
    }
    return 0;
}

#pragma var_order(data, wavFile, wavFile2)
ZunResult Supervisor::LoadConfig(char *path)
{
    GameConfiguration *data;
    FILE *wavFile;
    FILE *wavFile2;

    memset(&g_Supervisor.cfg, 0, sizeof(GameConfiguration));
    g_Supervisor.cfg.opts |= 1 << GCOS_USE_D3D_HW_TEXTURE_BLENDING;
    data = (GameConfiguration *)FileSystem::OpenPath(path, EXTERNAL_FILE);
    if (data == NULL)
    {
        g_Supervisor.cfg.lifeCount = 2;
        g_Supervisor.cfg.bombCount = 3;
        g_Supervisor.cfg.colorMode16bit = 0xff;
        g_Supervisor.cfg.version = GAME_VERSION;
        g_Supervisor.cfg.padXAxis = 600;
        g_Supervisor.cfg.padYAxis = 600;
        wavFile = fopen("bgm/th06_01.wav", "rb");
        if (wavFile != NULL)
        {
            g_Supervisor.cfg.musicMode = WAV;
            fclose(wavFile);
        }
        else
        {
            g_Supervisor.cfg.musicMode = MIDI;
            utils::DebugPrint(TH_ERR_NO_WAVE_FILE);
        }
        g_Supervisor.cfg.playSounds = 1;
        g_Supervisor.cfg.defaultDifficulty = NORMAL;
        g_Supervisor.cfg.windowed = false;
        g_Supervisor.cfg.frameskipConfig = 0;
        g_Supervisor.cfg.controllerMapping = g_ControllerMapping;
        g_GameErrorContext.Log(TH_ERR_CONFIG_NOT_FOUND);
    }
    else
    {
        g_Supervisor.cfg = *data;
        if ((g_Supervisor.cfg.lifeCount >= 5) || (g_Supervisor.cfg.bombCount >= 4) ||
            (g_Supervisor.cfg.colorMode16bit >= 2) || (g_Supervisor.cfg.musicMode >= 3) ||
            (g_Supervisor.cfg.defaultDifficulty >= 5) || (g_Supervisor.cfg.playSounds >= 2) ||
            (g_Supervisor.cfg.windowed >= 2) || (g_Supervisor.cfg.frameskipConfig >= 3) ||
            (g_Supervisor.cfg.version != GAME_VERSION) || (g_LastFileSize != 0x38))
        {
            g_Supervisor.cfg.lifeCount = 2;
            g_Supervisor.cfg.bombCount = 3;
            g_Supervisor.cfg.colorMode16bit = 0xff;
            g_Supervisor.cfg.version = GAME_VERSION;
            g_Supervisor.cfg.padXAxis = 600;
            g_Supervisor.cfg.padYAxis = 600;
            wavFile2 = fopen("bgm/th06_01.wav", "rb");
            if (wavFile2 != NULL)
            {
                g_Supervisor.cfg.musicMode = WAV;
                fclose(wavFile2);
            }
            else
            {
                g_Supervisor.cfg.musicMode = MIDI;
                utils::DebugPrint(TH_ERR_NO_WAVE_FILE);
            }
            g_Supervisor.cfg.playSounds = 1;
            g_Supervisor.cfg.defaultDifficulty = NORMAL;
            g_Supervisor.cfg.windowed = false;
            g_Supervisor.cfg.frameskipConfig = 0;
            g_Supervisor.cfg.controllerMapping = g_ControllerMapping;
            memset(&g_Supervisor.cfg.opts, 0, sizeof(GameConfigOptsShifts));
            g_Supervisor.cfg.opts |= 1 << GCOS_USE_D3D_HW_TEXTURE_BLENDING;
            g_GameErrorContext.Log(TH_ERR_CONFIG_CORRUPTED);
        }
        g_ControllerMapping = g_Supervisor.cfg.controllerMapping;
        ZUN_FREE(data);
    }
    if (this->IsVertexBufferDisabled())
    {
        g_GameErrorContext.Log(TH_ERR_NO_VERTEX_BUFFER);
    }
    if (this->IsFogDisabled())
    {
        g_GameErrorContext.Log(TH_ERR_NO_FOG);
    }
    if (this->Is16bitColorMode())
    {
        g_GameErrorContext.Log(TH_ERR_USE_16BIT_TEXTURES);
    }
    if (this->ShouldForceBackbufferClear())
    {
        g_GameErrorContext.Log(TH_ERR_FORCE_BACKBUFFER_CLEAR);
    }
    if (this->IsMinimumGraphicsMode())
    {
        g_GameErrorContext.Log(TH_ERR_DONT_RENDER_ITEMS);
    }
    if (this->IsShadingDisabled())
    {
        g_GameErrorContext.Log(TH_ERR_NO_GOURAUD_SHADING);
    }
    if (this->IsDepthTestDisabled())
    {
        g_GameErrorContext.Log(TH_ERR_NO_DEPTH_TESTING);
    }
    if (this->IsForced60Fps())
    {
        g_GameErrorContext.Log(TH_ERR_FORCE_60FPS_MODE);
        this->vsyncEnabled = false;
    }
    if (this->IsColorCompositingDisabled())
    {
        g_GameErrorContext.Log(TH_ERR_NO_TEXTURE_COLOR_COMPOSITING);
    }
    if (this->IsColorCompositingDisabled())
    {
        g_GameErrorContext.Log(TH_ERR_LAUNCH_WINDOWED);
    }
    if (this->IsReferenceRasterizerMode())
    {
        g_GameErrorContext.Log(TH_ERR_FORCE_REFERENCE_RASTERIZER);
    }
    if (this->IsDInputDisabled())
    {
        g_GameErrorContext.Log(TH_ERR_DO_NOT_USE_DIRECTINPUT);
    }
    if (FileSystem::WriteDataToFile(path, &g_Supervisor.cfg, sizeof(GameConfiguration)) != 0)
    {
        g_GameErrorContext.Fatal(TH_ERR_FILE_CANNOT_BE_EXPORTED, path);
        g_GameErrorContext.Fatal(TH_ERR_FOLDER_HAS_WRITE_PROTECT_OR_DISK_FULL);
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

ZunBool Supervisor::ReadMidiFile(u32 midiFileIdx, char *path)
{
    // Return conventions seem opposite of normal? But they're never used anyway
    if (g_Supervisor.cfg.musicMode == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->ReadFileData(midiFileIdx, path);
        }

        return FALSE;
    }

    return TRUE;
}

i32 Supervisor::PlayMidiFile(i32 midiFileIdx)
{
    MidiOutput *globalMidiController;

    if (g_Supervisor.cfg.musicMode == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            globalMidiController = g_Supervisor.midiOutput;
            globalMidiController->StopPlayback();
            globalMidiController->ParseFile(midiFileIdx);
            globalMidiController->Play();
        }

        return FALSE;
    }

    return TRUE;
}

ZunResult Supervisor::SetupMidiPlayback(char *path)
{
    if (g_Supervisor.cfg.musicMode == MIDI)
        ;
    else if (g_Supervisor.cfg.musicMode == WAV)
        ;
    else
        return ZUN_ERROR;
    return ZUN_SUCCESS;
}

ZunResult Supervisor::PlayAudio(char *path)
{
    char wavName[256];
    char wavPos[256];
    char *pathExtension;

    if (g_Supervisor.cfg.musicMode == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            MidiOutput *midiOutput = g_Supervisor.midiOutput;
            midiOutput->StopPlayback();
            midiOutput->LoadFile(path);
            midiOutput->Play();
        }
    }
    else if (g_Supervisor.cfg.musicMode == WAV)
    {
        strcpy(wavName, path);
        strcpy(wavPos, path);
        pathExtension = strrchr(wavName, L'.');
        pathExtension[1] = 'w';
        pathExtension[2] = 'a';
        pathExtension[3] = 'v';
        pathExtension = strrchr(wavPos, L'.');
        pathExtension[1] = 'p';
        pathExtension[2] = 'o';
        pathExtension[3] = 's';
        g_SoundPlayer.LoadWav(wavName);
        if (g_SoundPlayer.LoadPos(wavPos) < ZUN_SUCCESS)
        {
            g_SoundPlayer.PlayBGM(FALSE);
        }
        else
        {
            g_SoundPlayer.PlayBGM(TRUE);
        }
    }
    else
    {
        return ZUN_ERROR;
    }
    return ZUN_SUCCESS;
}

ZunResult Supervisor::StopAudio()
{
    if (g_Supervisor.cfg.musicMode == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->StopPlayback();
        }
    }
    else if (g_Supervisor.cfg.musicMode == WAV)
    {
        g_SoundPlayer.StopBGM();
    }
    else
    {
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

ZunResult Supervisor::FadeOutMusic(f32 fadeOutSeconds)
{
    if (g_Supervisor.cfg.musicMode == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->SetFadeOut(1000.0f * fadeOutSeconds);
        }
    }
    else if (g_Supervisor.cfg.musicMode == WAV)
    {
        if (this->effectiveFramerateMultiplier == 0.0f)
        {
            g_SoundPlayer.FadeOut(fadeOutSeconds);
        }
        else if (this->effectiveFramerateMultiplier > 1.0f)
        {
            g_SoundPlayer.FadeOut(fadeOutSeconds);
        }
        else
        {
            g_SoundPlayer.FadeOut(fadeOutSeconds / this->effectiveFramerateMultiplier);
        }
    }
    else
    {
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

}; // namespace th06
