#define _WIN32_WINNT 0x0500
#include "AnmManager.hpp"
#include "GameWindow.hpp"
#include "Global.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "Supervisor.hpp"
#include "ZunTimer.hpp"
#include "decomp.hpp"
#include "i18n.hpp"
#include <stdio.h>

namespace th06
{
DIFFABLE_STATIC_SORTED(L1, GameWindow, g_GameWindow);
DIFFABLE_STATIC_SORTED(L2, i32, g_TickCountToEffectiveFramerate);
DIFFABLE_STATIC_SORTED(L3, f64, g_LastFrameTime);
DIFFABLE_STATIC_SORTED(L4, HANDLE, g_ExclusiveMutex);
} // namespace th06

DWORD GetDXVersion();

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    using namespace th06;

    RenderResult renderResult = RENDER_RESULT_KEEP_RUNNING;

    if (utils::CheckForRunningGameInstance())
    {
        g_GameErrorContext.Flush();

        return 1;
    }

#if TRIALBUILD
    if (GetDXVersion() != 0x800)
    {
        g_GameErrorContext.Fatal(""); // TODO: string
        g_GameErrorContext.Flush();
        return -1;
    }
#endif

    g_Supervisor.hInstance = hInstance;

    if (g_Supervisor.LoadConfig(TH_CONFIG_FILE) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Flush();
        return -1;
    }

    if (GameWindow::InitD3dInterface())
    {
        g_GameErrorContext.Flush();
        return 1;
    }

    SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &g_GameWindow.screenSaveActive, 0);
    SystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &g_GameWindow.lowPowerActive, 0);
    SystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &g_GameWindow.powerOffActive, 0);
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0, NULL, SPIF_SENDCHANGE);

restart:
    GameWindow::CreateGameWindow(hInstance);

    if (GameWindow::InitD3dRendering())
    {
        g_GameErrorContext.Flush();
        return 1;
    }

    g_SoundPlayer.InitializeDSound(g_GameWindow.window);
    Controller::GetJoystickCaps();
    Controller::ResetKeyboard();

    g_AnmManager = ZUN_NEW(AnmManager);

    if (Supervisor::RegisterChain() != ZUN_SUCCESS)
    {
    }
    else
    {
        if (!g_Supervisor.IsWindowed())
        {
            ShowCursor(FALSE);
        }

        g_GameWindow.curFrame = 0;

        HRESULT testCoopLevelRes;

        while (!g_GameWindow.isAppClosing)
        {
            MSG msg;
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                testCoopLevelRes = g_Supervisor.d3dDevice->TestCooperativeLevel();
                if (testCoopLevelRes == D3D_OK)
                {
                    renderResult = g_GameWindow.Render();
                    if (renderResult != RENDER_RESULT_KEEP_RUNNING)
                    {
                        break;
                    }
                }
                else if (testCoopLevelRes == D3DERR_DEVICENOTRESET)
                {
                    g_AnmManager->ReleaseSurfaces();
                    HRESULT testResetRes = g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
                    if (testResetRes != D3D_OK)
                    {
                        break;
                    }
                    GameWindow::InitD3dDevice();
                    g_Supervisor.forceRedrawFrames = 3;
                }
            }
        }
    }

    g_Chain.Release();
    g_SoundPlayer.Release();

    ZUN_DELETE(g_AnmManager);
    SAFE_RELEASE(g_Supervisor.d3dDevice);

    ShowWindow(g_GameWindow.window, 0);
    MoveWindow(g_GameWindow.window, 0, 0, 0, 0, FALSE);
    DestroyWindow(g_GameWindow.window);

    if (renderResult == RENDER_RESULT_EXIT_ERROR)
    {
        g_GameErrorContext.ResetContext();

        g_GameErrorContext.Log(TH_ERR_OPTION_CHANGED_RESTART);

        if (!g_Supervisor.IsWindowed())
        {
            ShowCursor(TRUE);
        }
        goto restart;
    }

    FileSystem::WriteDataToFile(TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(GameConfiguration));
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);

    SAFE_RELEASE(g_Supervisor.d3dIface);

    ShowCursor(TRUE);
    g_GameErrorContext.Flush();
    return 0;
}

namespace th06
{
#define FRAME_TIME (1000.0 / 60.0)

RenderResult GameWindow::Render()
{
    i32 res;

    if (!this->isAppActive)
    {
        return RENDER_RESULT_KEEP_RUNNING;
    }

    if (this->curFrame == 0)
    {
    LOOP_USING_GOTO_BECAUSE_WHY_NOT:
        if (g_Supervisor.cfg.frameskipConfig <= this->curFrame)
        {
            if (g_Supervisor.ShouldForceBackbufferClear())
            {
                D3DVIEWPORT8 viewport;
                viewport.X = 0;
                viewport.Y = 0;
                viewport.Width = GAME_WINDOW_WIDTH;
                viewport.Height = GAME_WINDOW_HEIGHT;
                viewport.MinZ = 0.0f;
                viewport.MaxZ = 1.0f;
                g_Supervisor.d3dDevice->SetViewport(&viewport);
                g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, g_Stage.skyFog.color, 1.0f,
                                              0);
                g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
            }
            g_Supervisor.d3dDevice->BeginScene();
            g_Chain.RunDrawChain();
            g_Supervisor.d3dDevice->EndScene();
            g_Supervisor.d3dDevice->SetTexture(0, NULL);
        }

        g_Supervisor.viewport.X = 0;
        g_Supervisor.viewport.Y = 0;
        g_Supervisor.viewport.Width = GAME_WINDOW_WIDTH;
        g_Supervisor.viewport.Height = GAME_WINDOW_HEIGHT;
        g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
        res = g_Chain.RunCalcChain();
        g_SoundPlayer.PlaySounds();
        if (res == 0)
        {
            return RENDER_RESULT_EXIT_SUCCESS;
        }
        if (res == -1)
        {
            return RENDER_RESULT_EXIT_ERROR;
        }
        this->curFrame++;
    }

    if (g_Supervisor.IsWindowed() || g_Supervisor.ShouldRunAt60Fps())
    {
#pragma var_order(curtime, delta)
        if (this->curFrame != 0)
        {
            g_Supervisor.framerateMultiplier = 1.0f;
            timeBeginPeriod(1);
            f64 curtime = timeGetTime();
            if (curtime < g_LastFrameTime)
            {
                g_LastFrameTime = curtime;
            }
            f64 delta = fabs(curtime - g_LastFrameTime);
            timeEndPeriod(1);
            if (delta >= FRAME_TIME)
            {
                do
                {
                    g_LastFrameTime += FRAME_TIME;
                    delta -= FRAME_TIME;
                } while (delta >= FRAME_TIME);

                if (g_Supervisor.cfg.frameskipConfig < this->curFrame)
                    goto I_HAVE_NO_CLUE_WHY_BUT_I_MUST_JUMP_HERE;
                goto LOOP_USING_GOTO_BECAUSE_WHY_NOT;
            }
        }
    }

    if (!g_Supervisor.IsWindowed() && !g_Supervisor.ShouldRunAt60Fps())
    {
        if (g_Supervisor.cfg.frameskipConfig >= this->curFrame)
        {
            Present();
            goto LOOP_USING_GOTO_BECAUSE_WHY_NOT;
        }

    I_HAVE_NO_CLUE_WHY_BUT_I_MUST_JUMP_HERE:
        Present();
        if (g_Supervisor.framerateMultiplier == 0.0f)
        {
#pragma var_order(delta, curtime)
            if (g_TickCountToEffectiveFramerate >= 2)
            {
                timeBeginPeriod(1);
                u32 curtime = timeGetTime();
                if (curtime < g_Supervisor.lastFrameTime)
                {
                    g_Supervisor.lastFrameTime = curtime;
                }
                f64 delta = curtime - g_Supervisor.lastFrameTime;
                delta = (delta * 60.0) / 2.0 / 1000.0;
                delta /= g_Supervisor.cfg.frameskipConfig + 1;
                if (delta >= 0.865)
                {
                    delta = 1.0;
                }
                else if (delta >= 0.6)
                {
                    delta = 0.8;
                }
                else
                {
                    delta = 0.5;
                }
                g_Supervisor.effectiveFramerateMultiplier = delta;
                g_Supervisor.lastFrameTime = curtime;
                timeEndPeriod(1);
                g_TickCountToEffectiveFramerate = 0;
            }
        }
        else
        {
            g_Supervisor.effectiveFramerateMultiplier = g_Supervisor.framerateMultiplier;
        }
        this->curFrame = 0;
        g_TickCountToEffectiveFramerate++;
    }
    return RENDER_RESULT_KEEP_RUNNING;
}

void GameWindow::Present()
{
    if (FAILED(g_Supervisor.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_AnmManager->ReleaseSurfaces();
        g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
        InitD3dDevice();
        g_Supervisor.forceRedrawFrames = 2;
    }
    g_AnmManager->TakeScreenshotIfRequested();
    if (g_Supervisor.forceRedrawFrames != 0)
    {
        g_Supervisor.forceRedrawFrames--;
    }
}

i32 GameWindow::InitD3dInterface(void)
{
    g_Supervisor.d3dIface = Direct3DCreate8(D3D_SDK_VERSION);

    if (g_Supervisor.d3dIface == NULL)
    {
        g_GameErrorContext.Fatal(TH_ERR_D3D_ERR_COULD_NOT_CREATE_OBJ);
        return 1;
    }
    return 0;
}

void GameWindow::CreateGameWindow(HINSTANCE hInstance)
{
    WNDCLASS base_class;
    i32 width;
    i32 height;

    memset(&base_class, 0, sizeof(base_class));

    base_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    base_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    base_class.hInstance = hInstance;
    base_class.lpfnWndProc = WindowProc;
    g_GameWindow.isAppActive = false;
    g_GameWindow.showCursor = false;
    base_class.lpszClassName = "BASE";
    RegisterClass(&base_class);
    if (!g_Supervisor.IsWindowed())
    {
        width = GAME_WINDOW_WIDTH;
        height = GAME_WINDOW_HEIGHT;
        g_GameWindow.window = CreateWindow("BASE", TH_WINDOW_TITLE, WS_OVERLAPPEDWINDOW, 0, 0, width, height, NULL,
                                           NULL, hInstance, NULL);
    }
    else
    {
        width = GetSystemMetrics(SM_CXFIXEDFRAME) * 2 + GAME_WINDOW_WIDTH;
        height = GAME_WINDOW_HEIGHT + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);
        g_GameWindow.window = CreateWindow("BASE", TH_WINDOW_TITLE, WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU,
                                           CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);
    }
    g_Supervisor.hwndGameWindow = g_GameWindow.window;
}

LRESULT CALLBACK GameWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case MM_MOM_DONE:
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->UnprepareHeader((LPMIDIHDR)lParam);
        }
        break;
    case WM_ACTIVATEAPP:
        g_GameWindow.isAppActive = wParam;
        if (g_GameWindow.isAppActive)
        {
            g_GameWindow.showCursor = false;
        }
        else
        {
            g_GameWindow.showCursor = true;
        }
        break;
    case WM_SETCURSOR:
        if (!g_Supervisor.IsWindowed())
        {
            if (g_GameWindow.showCursor)
            {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                ShowCursor(TRUE);
            }
            else
            {
                ShowCursor(FALSE);
                SetCursor(NULL);
            }
        }
        else
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            ShowCursor(TRUE);
        }

        return 1;
    case WM_CLOSE:
        g_GameWindow.isAppClosing = true;
        return 1;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#pragma var_order(using_d3d_hal, display_mode, present_params, camera_distance, half_height, half_width, aspect_ratio, \
                  field_of_view_y, up, at, eye)
i32 GameWindow::InitD3dRendering(void)
{
    u8 using_d3d_hal;
    D3DPRESENT_PARAMETERS present_params;
    D3DDISPLAYMODE display_mode;
    D3DXVECTOR3 eye;
    D3DXVECTOR3 at;
    D3DXVECTOR3 up;
    float half_width;
    float half_height;
    float aspect_ratio;
    float field_of_view_y;
    float camera_distance;

    using_d3d_hal = true;
    memset(&present_params, 0, sizeof(D3DPRESENT_PARAMETERS));
    g_Supervisor.d3dIface->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &display_mode);
    if (!g_Supervisor.cfg.windowed)
    {
        if (g_Supervisor.Is16bitColorMode() == TRUE)
        {
            present_params.BackBufferFormat = D3DFMT_R5G6B5;
            g_Supervisor.cfg.colorMode16bit = true;
        }
        else if (g_Supervisor.cfg.colorMode16bit == 0xff)
        {
            if (display_mode.Format == D3DFMT_X8R8G8B8 || display_mode.Format == D3DFMT_A8R8G8B8)
            {
                present_params.BackBufferFormat = D3DFMT_X8R8G8B8;
                g_Supervisor.cfg.colorMode16bit = false;
                g_GameErrorContext.Log(TH_ERR_SCREEN_INIT_32BITS);
            }
            else
            {
                present_params.BackBufferFormat = D3DFMT_R5G6B5;
                g_Supervisor.cfg.colorMode16bit = true;
                g_GameErrorContext.Log(TH_ERR_SCREEN_INIT_16BITS);
            }
        }
        else if (g_Supervisor.cfg.colorMode16bit == 0)
        {
            present_params.BackBufferFormat = D3DFMT_X8R8G8B8;
        }
        else
        {
            present_params.BackBufferFormat = D3DFMT_R5G6B5;
        }
        if (!g_Supervisor.IsForced60Fps())
        {
            present_params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        }
        else
        {
            present_params.FullScreen_RefreshRateInHz = 60;
            present_params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
            g_GameErrorContext.Log(TH_ERR_SET_REFRESH_RATE_60HZ);
        }
        if (g_Supervisor.cfg.frameskipConfig == 0)
        {
            present_params.SwapEffect = D3DSWAPEFFECT_FLIP;
        }
        else
        {
            present_params.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
        }
    }
    else
    {
        present_params.BackBufferFormat = display_mode.Format;
        present_params.SwapEffect = D3DSWAPEFFECT_COPY;
        present_params.Windowed = TRUE;
    }
    present_params.BackBufferWidth = GAME_WINDOW_WIDTH;
    present_params.BackBufferHeight = GAME_WINDOW_HEIGHT;
    present_params.EnableAutoDepthStencil = TRUE;
    present_params.AutoDepthStencilFormat = D3DFMT_D16;
    present_params.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    g_Supervisor.lockableBackbuffer = true;
    g_Supervisor.presentParameters = present_params;
    for (;;)
    {
        if (g_Supervisor.IsReferenceRasterizerMode())
        {
            goto REFERENCE_RASTERIZER_MODE;
        }
        else
        {
            if (FAILED(g_Supervisor.d3dIface->CreateDevice(0, D3DDEVTYPE_HAL, g_GameWindow.window,
                                                           D3DCREATE_HARDWARE_VERTEXPROCESSING, &present_params,
                                                           &g_Supervisor.d3dDevice)))
            {
                g_GameErrorContext.Log(TH_ERR_TL_HAL_UNAVAILABLE);
                if (FAILED(g_Supervisor.d3dIface->CreateDevice(0, D3DDEVTYPE_HAL, g_GameWindow.window,
                                                               D3DCREATE_SOFTWARE_VERTEXPROCESSING, &present_params,
                                                               &g_Supervisor.d3dDevice)))
                {
                    g_GameErrorContext.Log(TH_ERR_HAL_UNAVAILABLE);
                REFERENCE_RASTERIZER_MODE:
                    if (FAILED(g_Supervisor.d3dIface->CreateDevice(0, D3DDEVTYPE_REF, g_GameWindow.window,
                                                                   D3DCREATE_SOFTWARE_VERTEXPROCESSING, &present_params,
                                                                   &g_Supervisor.d3dDevice)))
                    {
                        if (g_Supervisor.IsForced60Fps() && !g_Supervisor.vsyncEnabled)
                        {
                            g_GameErrorContext.Log(TH_ERR_CANT_CHANGE_REFRESH_RATE_FORCE_VSYNC);
                            present_params.FullScreen_RefreshRateInHz = 0;
                            g_Supervisor.vsyncEnabled = true;
                            present_params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
                            continue;
                        }
                        else
                        {
                            if (present_params.Flags == D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)
                            {
                                g_GameErrorContext.Log(TH_ERR_BACKBUFFER_NONLOCKED);
                                present_params.Flags = 0;
                                g_Supervisor.lockableBackbuffer = false;
                                continue;
                            }
                            else
                            {
                                g_GameErrorContext.Fatal(TH_ERR_D3D_INIT_FAILED);
                                SAFE_RELEASE(g_Supervisor.d3dIface);
                                return 1;
                            }
                        }
                    }
                    else
                    {
                        g_GameErrorContext.Log(TH_USING_REF_MODE);
                        g_Supervisor.hasD3dHardwareVertexProcessing = false;
                        using_d3d_hal = false;
                    }
                }
                else
                {
                    g_GameErrorContext.Log(TH_USING_HAL_MODE);
                    g_Supervisor.hasD3dHardwareVertexProcessing = false;
                }
            }
            else
            {
                g_GameErrorContext.Log(TH_USING_TL_HAL_MODE);
                g_Supervisor.hasD3dHardwareVertexProcessing = true;
            }
            break;
        }
    }

    half_width = GAME_WINDOW_WIDTH / 2.0f;
    half_height = GAME_WINDOW_HEIGHT / 2.0f;
    aspect_ratio = (float)GAME_WINDOW_WIDTH / (float)GAME_WINDOW_HEIGHT;
    field_of_view_y = ZUN_PI / 6.0f; // PI / 6.0f
    camera_distance = half_height / tanf(field_of_view_y / 2.0f);
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    at.x = half_width;
    at.y = -half_height;
    at.z = 0.0f;
    eye.x = half_width;
    eye.y = -half_height;
    eye.z = -camera_distance;
    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix, &eye, &at, &up);
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, field_of_view_y, aspect_ratio, 100.0f, 10000.0f);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
    g_Supervisor.d3dDevice->GetViewport(&g_Supervisor.viewport);
    g_Supervisor.d3dDevice->GetDeviceCaps(&g_Supervisor.d3dCaps);
    if (!g_Supervisor.IsHardwareBlendingDisabled() && !(g_Supervisor.d3dCaps.TextureOpCaps & D3DTEXOPCAPS_ADD))
    {
        g_GameErrorContext.Log(TH_ERR_NO_SUPPORT_FOR_D3DTEXOPCAPS_ADD);
        g_Supervisor.cfg.opts.useSwTextureBlending = true;
    }
    if (g_Supervisor.ShouldRunAt60Fps() &&
        !(g_Supervisor.d3dCaps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE))
    {
        g_GameErrorContext.Log(TH_ERR_CANT_FORCE_60FPS_NO_ASYNC_FLIP);
        g_Supervisor.cfg.opts.force60Fps = false;
    }
    if (!g_Supervisor.Is16bitColorMode() && using_d3d_hal)
    {
        if (g_Supervisor.d3dIface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, present_params.BackBufferFormat, 0,
                                                     D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8) == D3D_OK)
        {
            g_Supervisor.colorMode16Bits = true;
        }
        else
        {
            g_Supervisor.colorMode16Bits = false;
            g_Supervisor.cfg.opts.force16bitColorMode = true;
            g_GameErrorContext.Log(TH_ERR_D3DFMT_A8R8G8B8_UNSUPPORTED);
        }
    }
    InitD3dDevice();
    ScreenEffect::SetViewport(0);
    g_GameWindow.isAppClosing = false;
    g_Supervisor.lastFrameTime = 0;
    g_Supervisor.framerateMultiplier = 0.0f;
    return 0;
}

#pragma var_order(fogVal, fogDensity)
void GameWindow::InitD3dDevice(void)
{
    f32 fogVal;
    f32 fogDensity;

    if (!g_Supervisor.IsDepthTestDisabled())
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    }
    else
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    }
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    if (!g_Supervisor.IsShadingDisabled())
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    }
    else
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
    }
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    if (!g_Supervisor.IsDepthTestDisabled())
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    }
    else
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    }
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ALPHAREF, 4);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    if (!g_Supervisor.IsFogDisabled())
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }
    else
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }
    fogDensity = 1.0f;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)&fogDensity);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, 0xffa0a0a0);
    fogVal = 1000.0f;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fogVal);
    fogVal = 5000.0f;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)&fogVal);
    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    if (!g_Supervisor.IsVertexBufferDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    if (!g_Supervisor.IsVertexBufferDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
    if (g_AnmManager != NULL)
    {
        g_AnmManager->SetCurrentBlendMode(AnmBlendMode_NotSet);
        g_AnmManager->SetCurrentColorOp(AnmColorOp_NotSet);
        g_AnmManager->SetCurrentVertexShader(AnmVertexShader_NotSet);
        g_AnmManager->SetCurrentTexture(NULL);
    }
    g_Stage.skyFogNeedsSetup = true;
}

namespace utils
{
ZunResult CheckForRunningGameInstance(void)
{
    g_ExclusiveMutex = CreateMutex(NULL, TRUE, TEXT("Touhou Koumakyou App"));

    if (g_ExclusiveMutex == NULL)
    {
        return ZUN_ERROR;
    }
    else if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        g_GameErrorContext.Fatal(TH_ERR_ALREADY_RUNNING);
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}
} // namespace utils
} // namespace th06
