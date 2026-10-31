#include "stdafx.h"

#include "Supervisor.hpp"
#include "AsciiManager.hpp"
#include "ChainPriorities.hpp"
#include "Ending.hpp"
#include "MainMenu.hpp"
#include "MidiOutput.hpp"
#include "MusicRoom.hpp"
#include "ReplayManager.hpp"
#include "SoundPlayer.hpp"
#include "TextHelper.hpp"

namespace th06
{

Supervisor g_Supervisor;

ChainCallbackResult Supervisor_OnUpdate(Supervisor *s)
{
    g_SoundPlayer.UpdateFades();
    g_PreviousFrameKeys = g_CurFrameInput;
    g_CurFrameInput = Controller::GetInput();
    g_IsEigthFrameInput = false;
    if (g_PreviousFrameKeys == g_CurFrameInput)
    {
        if (g_CountFramesInputsWereHeld >= 30)
        {
            if (g_CountFramesInputsWereHeld % 8 == 0)
            {
                g_IsEigthFrameInput = true;
            }
            if (g_CountFramesInputsWereHeld >= 38)
            {
                g_CountFramesInputsWereHeld = 30;
            }
        }
        g_CountFramesInputsWereHeld++;
    }
    else
    {
        g_CountFramesInputsWereHeld = 0;
    }

    if (s->wantedState != s->curState)
    {
        s->prevState = s->wantedState;
        switch (s->wantedState)
        {
        case SUPERVISOR_STATE_INIT:
        REINIT_MAINMENU:
            s->curState = SUPERVISOR_STATE_MAINMENU;
            g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);
            if (MainMenu_RegisterChain(FALSE) != ZUN_SUCCESS)
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
                if (GameManager_RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SUPERVISOR_STATE_EXITERROR:
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_ERROR;
            case SUPERVISOR_STATE_RESULTSCREEN:
                if (ResultScreen_RegisterChain(false) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SUPERVISOR_STATE_MUSICROOM:
                if (MusicRoom_RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SUPERVISOR_STATE_ENDING:
                GameManager_CutChain();
                if (Ending_RegisterChain() != ZUN_SUCCESS)
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
                GameManager_CutChain();
                s->curState = SUPERVISOR_STATE_INIT;
                SaveReplay(NULL, NULL);
                goto REINIT_MAINMENU;

            case SUPERVISOR_STATE_RESULTSCREEN_FROMGAME:
                GameManager_CutChain();
                if (ResultScreen_RegisterChain(true) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;
            case SUPERVISOR_STATE_NEXT_STAGE:
                GameManager_CutChain();
                if (GameManager_RegisterChain() != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
#if BUILT_SINCE(102g)
                if (s->curState == SUPERVISOR_STATE_MAINMENU)
                {
                    goto RETURN_TO_MENU_FROM_GAME;
                }
#endif
                s->curState = SUPERVISOR_STATE_GAMEMANAGER;
                break;
            case SUPERVISOR_STATE_MAINMENU_REPLAY:
                GameManager_CutChain();
                s->curState = SUPERVISOR_STATE_INIT;
                SaveReplay(NULL, NULL);
                s->curState = SUPERVISOR_STATE_MAINMENU;
                g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);
                if (MainMenu_RegisterChain(true) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
                break;

            case 10:
                GameManager_CutChain();
                if (Ending_RegisterChain() != ZUN_SUCCESS)
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
                SaveReplay(NULL, NULL);
                return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
            case SUPERVISOR_STATE_MAINMENU:
                s->curState = SUPERVISOR_STATE_INIT;
                SaveReplay(NULL, NULL);
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
                if (ResultScreen_RegisterChain(true) != ZUN_SUCCESS)
                {
                    return CHAIN_CALLBACK_RESULT_EXIT_GAME_SUCCESS;
                }
            }
            break;
        }
        g_CurFrameInput = g_PreviousFrameKeys = g_IsEigthFrameInput = 0;
    }

    s->wantedState = s->curState;
    s->calcCount++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static void DrawFpsCounter();

ChainCallbackResult Supervisor_OnDraw(Supervisor *s)
{
    g_AnmManager->ClearVertexShader();
    g_AnmManager->ClearSprite();
    g_AnmManager->ClearTexture();
    g_AnmManager->ClearColorOp();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearZWrite();

    DrawFpsCounter();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#if BUILT_SINCE(013)
static BOOL CALLBACK EnumGameControllerDevicesCallback(LPCDIDEVICEINSTANCE pdidInstance, LPVOID pContext)
{
    if (!g_Supervisor.controller)
    {
        HRESULT result =
            g_Supervisor.dinputIface->CreateDevice(pdidInstance->guidInstance, &g_Supervisor.controller, NULL);
        if (FAILED(result))
        {
            return TRUE;
        }
    }
    return FALSE;
}
#endif

#if BUILT_SINCE(101)
#pragma var_order(diprange, pvRefBackup)
BOOL CALLBACK EnumGameControllerObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    DIPROPRANGE diprange;
    LPVOID pvRefBackup = pvRef;

    // Until 1.02d every object gets the range, axis or not.
#if BUILT_SINCE(102f)
    if (lpddoi->dwType & DIDFT_AXIS)
#endif
    {
        diprange.diph.dwSize = sizeof(DIPROPRANGE);
        diprange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
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
#endif

static ZunResult SetupDInput(Supervisor *supervisor)
{
    HINSTANCE hInst = (HINSTANCE)GetWindowLong(supervisor->hwndGameWindow, GWL_HINSTANCE);

#if BUILT_SINCE(102g)
    if (supervisor->IsDInputDisabled())
    {
        return ZUN_ERROR;
    }
#endif

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

#if BUILT_SINCE(102)
    supervisor->keyboard->Acquire();
#else
    if (FAILED(supervisor->keyboard->Acquire()))
    {
        SAFE_RELEASE(supervisor->keyboard);

        SAFE_RELEASE(supervisor->dinputIface);

        g_GameErrorContext.Log(TH_ERR_DIRECTINPUT_ACQUIRE_NOT_AVAILABLE);
        return ZUN_ERROR;
    }
#endif
    g_GameErrorContext.Log(TH_ERR_DIRECTINPUT_INITIALIZED);

#if BUILT_SINCE(013)
    supervisor->dinputIface->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumGameControllerDevicesCallback, NULL,
                                         DIEDFL_ATTACHEDONLY);
    if (supervisor->controller)
    {
        supervisor->controller->SetDataFormat(&c_dfDIJoystick2);
        supervisor->controller->SetCooperativeLevel(supervisor->hwndGameWindow, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
#if BUILT_BEFORE(102f)
        supervisor->controller->Acquire();
#endif

#if BUILT_SINCE(101)
#if BUILT_SINCE(102)
        g_Supervisor.controllerCaps.dwSize = sizeof(DIDEVCAPS);
#else
        // 1.01 stores the size in the wrong field, so GetCapabilities sees a dwSize of 0.
        g_Supervisor.controllerCaps.dwAxes = sizeof(DIDEVCAPS);
#endif

        supervisor->controller->GetCapabilities(&g_Supervisor.controllerCaps);
#endif
#if BUILT_SINCE(102f)
        supervisor->controller->EnumObjects(EnumGameControllerObjectsCallback, NULL, DIDFT_ALL);
#elif BUILT_SINCE(101)
        // Until 1.02d only the axes are enumerated; 1.02f enumerates everything and filters in the callback.
        // 1.00 sets no axis ranges and reads the joystick's own limits instead.
        supervisor->controller->EnumObjects(EnumGameControllerObjectsCallback, NULL, DIDFT_AXIS);
#endif

        g_GameErrorContext.Log(TH_ERR_PAD_FOUND);
    }
#endif
    return ZUN_SUCCESS;
}

static ZunResult Supervisor_AddedCallback(Supervisor *s)
{
    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(s->pbg3Archives); i++)
    {
        s->pbg3Archives[i] = NULL;
    }

    g_Pbg3ArcList = s->pbg3Archives;
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
    SetupDInput(s);

    s->midiOutput = ZUN_NEW(MidiOutput);

    g_Rng.Initialize(timeGetTime());

    g_SoundPlayer.InitSoundBuffers();
    if (g_AnmManager->LoadAnm(ANM_FILE_TEXT, "data/text.anm", ANM_OFFSET_TEXT) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    if (AsciiManager_RegisterChain() != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_ASCIIMANAGER_INIT_FAILED);
        return ZUN_ERROR;
    }

    s->forceRedrawFrames = 0;
    g_AnmManager->SetupVertexBuffer();
    TextHelper_CreateTextBuffer();
    s->ReleasePbg3(IN_PBG3_INDEX);
    if (g_Supervisor.LoadPbg3(MD_PBG3_INDEX, TH_MD_DAT_FILE))
    {
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

static ZunResult Supervisor_DeletedCallback(Supervisor *s)
{
    g_AnmManager->ReleaseVertexBuffer();
    for (i32 pbg3Idx = 0; pbg3Idx < ARRAY_SIZE_SIGNED(s->pbg3Archives); pbg3Idx++)
    {
        s->ReleasePbg3(pbg3Idx);
    }
    g_AnmManager->ReleaseAnm(0);
    AsciiManager_CutChain();
    g_SoundPlayer.StopBGM();
#if BUILT_SINCE(102f)
    if (s->midiOutput != NULL)
#endif
    {
        s->midiOutput->StopPlayback();
        ZUN_DELETE(s->midiOutput);
    }
    SaveReplay(NULL, NULL);
    TextHelper_ReleaseTextBuffer();
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

#pragma var_order(chain, supervisor)
ZunResult Supervisor_RegisterChain()
{
    ChainElem *chain;
    Supervisor *supervisor = &g_Supervisor;

    supervisor->wantedState = 0;
    supervisor->curState = -1;
    supervisor->calcCount = 0;

    chain = g_FuncChain.CreateElem((ChainCallback)Supervisor_OnUpdate);
    chain->arg = supervisor;
    chain->addedCallback = (ChainAddedCallback)Supervisor_AddedCallback;
    chain->deletedCallback = (ChainDeletedCallback)Supervisor_DeletedCallback;
    if (g_FuncChain.AddToCalcChain(chain, TH_CHAIN_PRIO_CALC_SUPERVISOR) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    chain = g_FuncChain.CreateElem((ChainCallback)Supervisor_OnDraw);
    chain->arg = supervisor;
    g_FuncChain.AddToDrawChain(chain, TH_CHAIN_PRIO_DRAW_SUPERVISOR);

    return ZUN_SUCCESS;
}

// Adapted from fps_disp: https://t-pot.com/program/13_fps/index.html
#pragma var_order(curTime, framerate, fps, elapsed)
static void DrawFpsCounter()
{
    DWORD curTime;
    float framerate;
    float elapsed;
    float fps;

    static DWORD g_LastTime = timeGetTime();
    static u32 g_NumFramesSinceLastTime = 0;
    static char g_FpsCounterBuffer[256] = "";

    curTime = timeGetTime();
    g_NumFramesSinceLastTime = g_NumFramesSinceLastTime + 1 + (u32)g_Supervisor.cfg.frameskipConfig;
    // Preserve the comparison order from the original sample.
    if (500 <= curTime - g_LastTime)
    {
        elapsed = (curTime - g_LastTime) / 1000.0f;
        fps = g_NumFramesSinceLastTime / elapsed;
        g_LastTime = curTime;
        g_NumFramesSinceLastTime = 0;
        sprintf(g_FpsCounterBuffer, "%.02ffps", fps);
        if (g_GameManager.isInMenu)
        {
            framerate = 60.0f / g_Supervisor.framerateMultiplier;
            g_Supervisor.unk1b8 = g_Supervisor.unk1b8 + framerate;

            if (framerate * 0.89999998f < fps)
                g_Supervisor.unk1b4 = g_Supervisor.unk1b4 + framerate;
            else if (framerate * 0.69999999f < fps)
                g_Supervisor.unk1b4 = framerate * 0.8f + g_Supervisor.unk1b4;
            else if (fps > framerate * 0.5f)
                g_Supervisor.unk1b4 = framerate * 0.6f + g_Supervisor.unk1b4;
            else
                g_Supervisor.unk1b4 = framerate * 0.5f + g_Supervisor.unk1b4;
        }
    }
    if (!g_Supervisor.isInEnding)
    {
        g_AsciiManager.AddString(&D3DXVECTOR3(512.0f, 464.0f, 0.0f), g_FpsCounterBuffer);
    }
}

void AnmManager::ReleaseVertexBuffer()
{
    SAFE_RELEASE(this->vertexBuffer);
}

void ZunTimer::Initialize()
{
    this->current = 0;
    this->previous = -1;
    this->subFrame = 0.0f;
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
    // game. Maybe some accuracy improvements in the PBG3 handling will show
    // why.
    this->pbg3Archives[pbg3FileIdx]->Release();
    ZUN_DELETE(this->pbg3Archives[pbg3FileIdx]);
}

BOOL Supervisor::LoadPbg3(i32 pbg3FileIdx, const char *filename)
{
    if (this->pbg3Archives[pbg3FileIdx] == NULL || strcmp(filename, this->pbg3ArchiveNames[pbg3FileIdx]) != 0)
    {
        this->ReleasePbg3(pbg3FileIdx);
        this->pbg3Archives[pbg3FileIdx] = ZUN_NEW(Pbg3Archive);
        DebugPrint("%s open ...\n", filename);
        if (this->pbg3Archives[pbg3FileIdx]->Load(filename))
        {
            strcpy(this->pbg3ArchiveNames[pbg3FileIdx], filename);

            char verPath[128];
#if BUILD_VERSION == BUILD_VERSION_101
            // 1.01 still uses the 1.00 data archives.
            sprintf(verPath, "ver%.4x.dat", 0x0100);
#else
            sprintf(verPath, "ver%.4x.dat", GAME_VERSION);
#endif
            i32 res = this->pbg3Archives[pbg3FileIdx]->FindEntry(verPath);
            if (res < 0)
            {
                g_GameErrorContext.Fatal(TH_ERR_WRONG_DATA_VERSION);
                return TRUE;
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
    return FALSE;
}

ZunResult Supervisor::LoadConfig(const char *path)
{
    memset(&g_Supervisor.cfg, 0, sizeof(GameConfiguration));
    g_Supervisor.cfg.opts.useSwTextureBlending = true;
#if BUILT_SINCE(100)
    GameConfiguration *data = (GameConfiguration *)FileSystem::OpenPath(path, EXTERNAL_FILE);
#else
    GameConfiguration *data = (GameConfiguration *)FileSystem::OpenPath(path);
#endif
    if (data == NULL)
    {
        g_Supervisor.cfg.lifeCount = 2;
        g_Supervisor.cfg.bombCount = 3;
        g_Supervisor.cfg.colorMode16bit = 0xff;
#if BUILT_SINCE(102)
        g_Supervisor.cfg.version = CONFIG_VERSION;
        g_Supervisor.cfg.padXAxis = 600;
        g_Supervisor.cfg.padYAxis = 600;
#endif
        FILE *wavFile = fopen("bgm/th06_01.wav", "rb");
        if (wavFile != NULL)
        {
            g_Supervisor.cfg.musicMode = WAV;
            fclose(wavFile);
        }
        else
        {
            g_Supervisor.cfg.musicMode = MIDI;
            DebugPrint(TH_ERR_NO_WAVE_FILE);
        }
        g_Supervisor.cfg.playSounds = true;
        g_Supervisor.cfg.defaultDifficulty = NORMAL;
        g_Supervisor.cfg.windowed = false;
        g_Supervisor.cfg.frameskipConfig = 0;
        g_Supervisor.cfg.controllerMapping = g_ControllerMapping;
        g_GameErrorContext.Log(TH_ERR_CONFIG_NOT_FOUND);
    }
    else
    {
        g_Supervisor.cfg = *data;
        if (g_Supervisor.cfg.lifeCount >= 5 || g_Supervisor.cfg.bombCount >= 4 ||
            g_Supervisor.cfg.colorMode16bit >= 2 || g_Supervisor.cfg.musicMode >= 3 ||
            g_Supervisor.cfg.defaultDifficulty >= 5 || g_Supervisor.cfg.playSounds >= 2 ||
            g_Supervisor.cfg.windowed >= 2 || g_Supervisor.cfg.frameskipConfig >= 3 ||
#if BUILT_SINCE(102)
            g_Supervisor.cfg.version != CONFIG_VERSION ||
#endif
            g_ReadFileSize != sizeof(GameConfiguration))
        {
            g_Supervisor.cfg.lifeCount = 2;
            g_Supervisor.cfg.bombCount = 3;
            g_Supervisor.cfg.colorMode16bit = 0xff;
#if BUILT_SINCE(102)
            g_Supervisor.cfg.version = CONFIG_VERSION;
            g_Supervisor.cfg.padXAxis = 600;
            g_Supervisor.cfg.padYAxis = 600;
#endif
            FILE *wavFile = fopen("bgm/th06_01.wav", "rb");
            if (wavFile != NULL)
            {
                g_Supervisor.cfg.musicMode = WAV;
                fclose(wavFile);
            }
            else
            {
                g_Supervisor.cfg.musicMode = MIDI;
                DebugPrint(TH_ERR_NO_WAVE_FILE);
            }
            g_Supervisor.cfg.playSounds = true;
            g_Supervisor.cfg.defaultDifficulty = NORMAL;
            g_Supervisor.cfg.windowed = false;
            g_Supervisor.cfg.frameskipConfig = 0;
            g_Supervisor.cfg.controllerMapping = g_ControllerMapping;
            memset(&g_Supervisor.cfg.opts, 0, sizeof(GameConfigOpts));
            g_Supervisor.cfg.opts.useSwTextureBlending = true;
            g_GameErrorContext.Log(TH_ERR_CONFIG_CORRUPTED);
        }
        g_ControllerMapping = g_Supervisor.cfg.controllerMapping;
        ZUN_FREE(data);
    }
    if (this->cfg.opts.dontUseVertexBuf != false)
    {
        g_GameErrorContext.Log(TH_ERR_NO_VERTEX_BUFFER);
    }
    if (this->cfg.opts.disableFog != false)
    {
        g_GameErrorContext.Log(TH_ERR_NO_FOG);
    }
    if (this->cfg.opts.force16bitColorMode != false)
    {
        g_GameErrorContext.Log(TH_ERR_USE_16BIT_TEXTURES);
    }
    if (this->ShouldForceBackbufferClear())
    {
        g_GameErrorContext.Log(TH_ERR_FORCE_BACKBUFFER_CLEAR);
    }
    if (this->cfg.opts.displayMinimumGraphics != false)
    {
        g_GameErrorContext.Log(TH_ERR_DONT_RENDER_ITEMS);
    }
    if (this->cfg.opts.suppressUseOfGoroudShading != false)
    {
        g_GameErrorContext.Log(TH_ERR_NO_GOURAUD_SHADING);
    }
    if (this->cfg.opts.disableDepthTest != false)
    {
        g_GameErrorContext.Log(TH_ERR_NO_DEPTH_TESTING);
    }
    if (this->cfg.opts.force60Fps != false)
    {
        g_GameErrorContext.Log(TH_ERR_FORCE_60FPS_MODE);
#if BUILT_SINCE(102f)
        this->vsyncEnabled = false;
#endif
    }
    if (this->cfg.opts.disableColorCompositing != false)
    {
        g_GameErrorContext.Log(TH_ERR_NO_TEXTURE_COLOR_COMPOSITING);
    }
    if (this->cfg.opts.disableColorCompositing != false)
    {
        g_GameErrorContext.Log(TH_ERR_LAUNCH_WINDOWED);
    }
    if (this->cfg.opts.referenceRasterizerMode != false)
    {
        g_GameErrorContext.Log(TH_ERR_FORCE_REFERENCE_RASTERIZER);
    }
#if BUILT_SINCE(102g)
    if (this->cfg.opts.dontUseDirectInput != false)
    {
        g_GameErrorContext.Log(TH_ERR_DO_NOT_USE_DIRECTINPUT);
    }
#endif
    if (FileSystem::WriteDataToFile(path, &g_Supervisor.cfg, sizeof(GameConfiguration)) != 0)
    {
        g_GameErrorContext.Fatal(TH_ERR_FILE_CANNOT_BE_EXPORTED, path);
        g_GameErrorContext.Fatal(TH_ERR_FOLDER_HAS_WRITE_PROTECT_OR_DISK_FULL);
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

ZunBool Supervisor::ReadMidiFile(u32 midiFileIdx, const char *path)
{
    // Return conventions seem opposite of normal? But they're never used anyway
    if (g_Supervisor.GetMusicMode() == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->ReadFileData(midiFileIdx, path);
        }

        return FALSE;
    }

    return TRUE;
}

ZunBool Supervisor::PlayMidiFile(i32 midiFileIdx)
{
    if (g_Supervisor.GetMusicMode() == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->PlayFile(midiFileIdx);
        }

        return FALSE;
    }

    return TRUE;
}

ZunResult Supervisor::SetupMidiPlayback(const char *path)
{
    if (g_Supervisor.GetMusicMode() == MIDI)
    {
    }
    else if (g_Supervisor.GetMusicMode() == WAV)
    {
    }
    else
    {
        return ZUN_ERROR;
    }
    return ZUN_SUCCESS;
}

#pragma var_order(pathExtension, audioPaths)
ZunResult Supervisor::PlayAudio(const char *path)
{
    char audioPaths[2][256];
    char *pathExtension;

    if (g_Supervisor.GetMusicMode() == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->LoadAndPlay(path);
        }
    }
    else if (g_Supervisor.GetMusicMode() == WAV)
    {
        strcpy(audioPaths[0], path);
        strcpy(audioPaths[1], path);
        pathExtension = strrchr(audioPaths[0], L'.');
        pathExtension[1] = 'w';
        pathExtension[2] = 'a';
        pathExtension[3] = 'v';
        pathExtension = strrchr(audioPaths[1], L'.');
        pathExtension[1] = 'p';
        pathExtension[2] = 'o';
        pathExtension[3] = 's';
        g_SoundPlayer.LoadWav(audioPaths[0]);
        if (g_SoundPlayer.LoadPos(audioPaths[1]) < ZUN_SUCCESS)
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
    if (g_Supervisor.GetMusicMode() == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->StopPlayback();
        }
    }
    else
    {
        if (g_Supervisor.GetMusicMode() == WAV)
        {
            g_SoundPlayer.StopBGM();
        }
        else
        {
            return ZUN_ERROR;
        }
    }

    return ZUN_SUCCESS;
}

ZunResult Supervisor::FadeOutMusic(f32 fadeOutSeconds)
{
    if (g_Supervisor.GetMusicMode() == MIDI)
    {
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->SetFadeOut(1000.0f * fadeOutSeconds);
        }
    }
    else
    {
        if (g_Supervisor.GetMusicMode() == WAV)
        {
            if (this->effectiveFramerateMultiplier == 0.0f)
            {
                g_SoundPlayer.FadeOut(fadeOutSeconds);
            }
            else
            {
                if (this->effectiveFramerateMultiplier > 1.0f)
                {
                    g_SoundPlayer.FadeOut(fadeOutSeconds);
                }
                else
                {
                    g_SoundPlayer.FadeOut(fadeOutSeconds / this->effectiveFramerateMultiplier);
                }
            }
        }
        else
        {
            return ZUN_ERROR;
        }
    }

    return ZUN_SUCCESS;
}

} // namespace th06
