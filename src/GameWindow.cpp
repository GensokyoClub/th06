#include "GameWindow.hpp"
#include "AnmManager.hpp"
#include "GameErrorContext.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "Supervisor.hpp"
#include "diffbuild.hpp"
#include "i18n.hpp"

namespace th06
{
DIFFABLE_STATIC(GameWindow, g_GameWindow)
DIFFABLE_STATIC(i32, g_TickCountToEffectiveFramerate)
DIFFABLE_STATIC(f64, g_LastFrameTime)

#define FRAME_TIME (1000. / 60.)

#pragma var_order(res, viewport, slowdown, local_34, delta, curtime)
RenderResult GameWindow::Render()
{
    i32 res;
    f64 slowdown;
    D3DVIEWPORT8 viewport;
    f64 delta;
    u32 curtime;
    f64 local_34;

    if (this->lastActiveAppValue == 0)
    {
        return RENDER_RESULT_KEEP_RUNNING;
    }

    if (this->curFrame == 0)
    {
    LOOP_USING_GOTO_BECAUSE_WHY_NOT:
        if (g_Supervisor.cfg.frameskipConfig <= this->curFrame)
        {
            if (g_Supervisor.IsUnknown())
            {
                viewport.X = 0;
                viewport.Y = 0;
                viewport.Width = 640;
                viewport.Height = 480;
                viewport.MinZ = 0.0;
                viewport.MaxZ = 1.0;
                g_Supervisor.d3dDevice->SetViewport(&viewport);
                g_Supervisor.d3dDevice->Clear(0, NULL, 3, g_Stage.skyFog.color, 1.0, 0);
                g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
            }
            g_Supervisor.d3dDevice->BeginScene();
            g_Chain.RunDrawChain();
            g_Supervisor.d3dDevice->EndScene();
            g_Supervisor.d3dDevice->SetTexture(0, NULL);
        }

        g_Supervisor.viewport.X = 0;
        g_Supervisor.viewport.Y = 0;
        g_Supervisor.viewport.Width = 640;
        g_Supervisor.viewport.Height = 480;
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

    if (g_Supervisor.cfg.windowed != false || g_Supervisor.ShouldRunAt60Fps())
    {
        if (this->curFrame != 0)
        {
            g_Supervisor.framerateMultiplier = 1.0;
            timeBeginPeriod(1);
            slowdown = timeGetTime();
            if (slowdown < g_LastFrameTime)
            {
                g_LastFrameTime = slowdown;
            }
            local_34 = fabs(slowdown - g_LastFrameTime);
            timeEndPeriod(1);
            if (local_34 >= FRAME_TIME)
            {
                do
                {
                    g_LastFrameTime += FRAME_TIME;
                    local_34 -= FRAME_TIME;
                } while (local_34 >= FRAME_TIME);

                if (g_Supervisor.cfg.frameskipConfig < this->curFrame)
                    goto I_HAVE_NO_CLUE_WHY_BUT_I_MUST_JUMP_HERE;
                goto LOOP_USING_GOTO_BECAUSE_WHY_NOT;
            }
        }
    }

    if (g_Supervisor.cfg.windowed == false && !g_Supervisor.ShouldRunAt60Fps())
    {

        if (g_Supervisor.cfg.frameskipConfig >= this->curFrame)
        {
            Present();
            goto LOOP_USING_GOTO_BECAUSE_WHY_NOT;
        }

    I_HAVE_NO_CLUE_WHY_BUT_I_MUST_JUMP_HERE:
        Present();
        if (g_Supervisor.framerateMultiplier == 0.f)
        {
            if (2 <= g_TickCountToEffectiveFramerate)
            {
                timeBeginPeriod(1);
                curtime = timeGetTime();
                if (curtime < g_Supervisor.lastFrameTime)
                {
                    g_Supervisor.lastFrameTime = curtime;
                }
                delta = curtime - g_Supervisor.lastFrameTime;
                delta = (delta * 60.) / 2. / 1000.;
                delta /= (g_Supervisor.cfg.frameskipConfig + 1);
                if (delta >= .865)
                {
                    delta = 1.0;
                }
                else if (delta >= .6)
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
        g_TickCountToEffectiveFramerate = g_TickCountToEffectiveFramerate + 1;
    }
    return RENDER_RESULT_KEEP_RUNNING;
}

void GameWindow::Present()
{
    i32 unused;
    if (g_Supervisor.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
    {
        g_AnmManager->ReleaseSurfaces();
        g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
        InitD3dDevice();
        g_Supervisor.unk198 = 2;
    }
    g_AnmManager->TakeScreenshotIfRequested();
    if (g_Supervisor.unk198 != 0)
    {
        g_Supervisor.unk198--;
    }
    return;
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
    g_GameWindow.lastActiveAppValue = 0;
    g_GameWindow.isAppActive = 0;
    base_class.lpszClassName = "BASE";
    RegisterClass(&base_class);
    if (g_Supervisor.cfg.windowed == 0)
    {
        width = GAME_WINDOW_WIDTH;
        height = GAME_WINDOW_HEIGHT;
        g_GameWindow.window =
            CreateWindowEx(0, "BASE", TH_WINDOW_TITLE, WS_OVERLAPPEDWINDOW, 0, 0, width, height, 0, 0, hInstance, 0);
    }
    else
    {
        width = GetSystemMetrics(SM_CXFIXEDFRAME) * 2 + GAME_WINDOW_WIDTH;
        height = GAME_WINDOW_HEIGHT + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);
        g_GameWindow.window = CreateWindowEx(0, "BASE", TH_WINDOW_TITLE, WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU,
                                             CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hInstance, 0);
    }
    g_Supervisor.hwndGameWindow = g_GameWindow.window;
}

LRESULT __stdcall GameWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case 0x3c9:
        if (g_Supervisor.midiOutput != NULL)
        {
            g_Supervisor.midiOutput->UnprepareHeader((LPMIDIHDR)lParam);
        }
        break;
    case WM_ACTIVATEAPP:
        g_GameWindow.lastActiveAppValue = wParam;
        if (g_GameWindow.lastActiveAppValue != 0)
        {
            g_GameWindow.isAppActive = 0;
        }
        else
        {
            g_GameWindow.isAppActive = 1;
        }
        break;
    case WM_SETCURSOR:
        if (!g_Supervisor.cfg.windowed)
        {
            if (g_GameWindow.isAppActive != 0)
            {
                SetCursor(LoadCursorA(NULL, IDC_ARROW));
                ShowCursor(1);
            }
            else
            {
                ShowCursor(0);
                SetCursor((HCURSOR)0x0);
            }
        }
        else
        {
            SetCursor(LoadCursorA(NULL, IDC_ARROW));
            ShowCursor(1);
        }

        return 1;
    case WM_CLOSE:
        g_GameWindow.isAppClosing = 1;
        return 1;
    }
    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

#pragma var_order(using_d3d_hal, display_mode, present_params, camera_distance, half_height, half_width, aspect_ratio, \
                  field_of_view_y, up, at, eye, should_run_at_60_fps)
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

    using_d3d_hal = 1;
    memset(&present_params, 0, sizeof(D3DPRESENT_PARAMETERS));
    g_Supervisor.d3dIface->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &display_mode);
    if (!g_Supervisor.cfg.windowed)
    {
        if ((((g_Supervisor.cfg.opts >> GCOS_FORCE_16BIT_COLOR_MODE) & 1) == 1))
        {
            present_params.BackBufferFormat = D3DFMT_R5G6B5;
            g_Supervisor.cfg.colorMode16bit = 1;
        }
        else if (g_Supervisor.cfg.colorMode16bit == 0xff)
        {
            if ((display_mode.Format == D3DFMT_X8R8G8B8) || (display_mode.Format == D3DFMT_A8R8G8B8))
            {
                present_params.BackBufferFormat = D3DFMT_X8R8G8B8;
                g_Supervisor.cfg.colorMode16bit = 0;
                g_GameErrorContext.Log(TH_ERR_SCREEN_INIT_32BITS);
            }
            else
            {
                present_params.BackBufferFormat = D3DFMT_R5G6B5;
                g_Supervisor.cfg.colorMode16bit = 1;
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
        if (!((g_Supervisor.cfg.opts >> GCOS_FORCE_60FPS) & 1))
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
        present_params.Windowed = 1;
    }
    present_params.BackBufferWidth = GAME_WINDOW_WIDTH;
    present_params.BackBufferHeight = GAME_WINDOW_HEIGHT;
    present_params.EnableAutoDepthStencil = true;
    present_params.AutoDepthStencilFormat = D3DFMT_D16;
    present_params.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    g_Supervisor.lockableBackbuffer = 1;
    memcpy(&g_Supervisor.presentParameters, &present_params, sizeof(D3DPRESENT_PARAMETERS));
    for (;;)
    {
        if (((g_Supervisor.cfg.opts >> GCOS_REFERENCE_RASTERIZER_MODE) & 1) != 0)
        {
            goto REFERENCE_RASTERIZER_MODE;
        }
        else
        {
            if (g_Supervisor.d3dIface->CreateDevice(0, D3DDEVTYPE_HAL, g_GameWindow.window,
                                                    D3DCREATE_HARDWARE_VERTEXPROCESSING, &present_params,
                                                    &g_Supervisor.d3dDevice) < 0)
            {
                g_GameErrorContext.Log(TH_ERR_TL_HAL_UNAVAILABLE);
                if (g_Supervisor.d3dIface->CreateDevice(0, D3DDEVTYPE_HAL, g_GameWindow.window,
                                                        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &present_params,
                                                        &g_Supervisor.d3dDevice) < 0)
                {
                    g_GameErrorContext.Log(TH_ERR_HAL_UNAVAILABLE);
                REFERENCE_RASTERIZER_MODE:
                    if (g_Supervisor.d3dIface->CreateDevice(0, D3DDEVTYPE_REF, g_GameWindow.window,
                                                            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &present_params,
                                                            &g_Supervisor.d3dDevice) < 0)
                    {
                        if (((g_Supervisor.cfg.opts >> GCOS_FORCE_60FPS) & 1) != 0 && !g_Supervisor.vsyncEnabled)
                        {
                            g_GameErrorContext.Log(TH_ERR_CANT_CHANGE_REFRESH_RATE_FORCE_VSYNC);
                            present_params.FullScreen_RefreshRateInHz = 0;
                            g_Supervisor.vsyncEnabled = 1;
                            present_params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
                            continue;
                        }
                        else
                        {
                            if (present_params.Flags == D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)
                            {
                                g_GameErrorContext.Log(TH_ERR_BACKBUFFER_NONLOCKED);
                                present_params.Flags = 0;
                                g_Supervisor.lockableBackbuffer = 0;
                                continue;
                            }
                            else
                            {
                                g_GameErrorContext.Fatal(TH_ERR_D3D_INIT_FAILED);
                                if (g_Supervisor.d3dIface != NULL)
                                {
                                    g_Supervisor.d3dIface->Release();
                                    g_Supervisor.d3dIface = NULL;
                                }
                                return 1;
                            }
                        }
                    }
                    else
                    {
                        g_GameErrorContext.Log(TH_USING_REF_MODE);
                        g_Supervisor.hasD3dHardwareVertexProcessing = 0;
                        using_d3d_hal = 0;
                    }
                }
                else
                {
                    g_GameErrorContext.Log(TH_USING_HAL_MODE);
                    g_Supervisor.hasD3dHardwareVertexProcessing = 0;
                }
            }
            else
            {
                g_GameErrorContext.Log(TH_USING_TL_HAL_MODE);
                g_Supervisor.hasD3dHardwareVertexProcessing = 1;
            }
            break;
        }
    }

    half_width = (float)GAME_WINDOW_WIDTH / 2.0;
    half_height = (float)GAME_WINDOW_HEIGHT / 2.0;
    aspect_ratio = (float)GAME_WINDOW_WIDTH / (float)GAME_WINDOW_HEIGHT;
    field_of_view_y = 0.52359879; // PI / 6.0f
    camera_distance = half_height / tanf(field_of_view_y / 2.0f);
    up.x = 0.0;
    up.y = 1.0;
    up.z = 0.0;
    at.x = half_width;
    at.y = -half_height;
    at.z = 0.0;
    eye.x = half_width;
    eye.y = -half_height;
    eye.z = -camera_distance;
    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix, &eye, &at, &up);
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, field_of_view_y, aspect_ratio, 100.0, 10000.0);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
    g_Supervisor.d3dDevice->GetViewport(&g_Supervisor.viewport);
    g_Supervisor.d3dDevice->GetDeviceCaps(&g_Supervisor.d3dCaps);
    if (((((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0) &&
         ((g_Supervisor.d3dCaps.TextureOpCaps & D3DTEXOPCAPS_ADD) == 0)))
    {
        g_GameErrorContext.Log(TH_ERR_NO_SUPPORT_FOR_D3DTEXOPCAPS_ADD);
        g_Supervisor.cfg.opts = g_Supervisor.cfg.opts | (1 << GCOS_USE_D3D_HW_TEXTURE_BLENDING);
    }
    if (g_Supervisor.ShouldRunAt60Fps() &&
        ((g_Supervisor.d3dCaps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE) == 0))
    {
        g_GameErrorContext.Log(TH_ERR_CANT_FORCE_60FPS_NO_ASYNC_FLIP);
        g_Supervisor.cfg.opts = g_Supervisor.cfg.opts & ~(1 << GCOS_FORCE_60FPS);
    }
    if ((((g_Supervisor.cfg.opts >> GCOS_FORCE_16BIT_COLOR_MODE) & 1) == 0) && (using_d3d_hal != 0))
    {
        if (g_Supervisor.d3dIface->CheckDeviceFormat(0, D3DDEVTYPE_HAL, present_params.BackBufferFormat, 0,
                                                     D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8) == 0)
        {
            g_Supervisor.colorMode16Bits = 1;
        }
        else
        {
            g_Supervisor.colorMode16Bits = 0;
            g_Supervisor.cfg.opts = g_Supervisor.cfg.opts | (1 << GCOS_FORCE_16BIT_COLOR_MODE);
            g_GameErrorContext.Log(TH_ERR_D3DFMT_A8R8G8B8_UNSUPPORTED);
        }
    }
    InitD3dDevice();
    ScreenEffect::SetViewport(0);
    g_GameWindow.isAppClosing = 0;
    g_Supervisor.lastFrameTime = 0;
    g_Supervisor.framerateMultiplier = 0.0;
    return 0;
}

#pragma var_order(fogVal, fogDensity, anm1, anm2, anm3, anm4)
void GameWindow::InitD3dDevice(void)
{
    f32 fogVal;
    f32 fogDensity;
    AnmManager *anm1;
    AnmManager *anm2;
    AnmManager *anm3;
    AnmManager *anm4;

    if (((g_Supervisor.cfg.opts >> GCOS_TURN_OFF_DEPTH_TEST) & 1) == 0)
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
    if (((g_Supervisor.cfg.opts >> GCOS_SUPPRESS_USE_OF_GOROUD_SHADING) & 1) == 0)
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    }
    else
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
    }
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    if (((g_Supervisor.cfg.opts >> GCOS_TURN_OFF_DEPTH_TEST) & 1) == 0)
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
    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_FOG) & 1) == 0)
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }
    else
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }
    fogDensity = 1.0;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGDENSITY, *(u32 *)&fogDensity);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, 0xffa0a0a0);
    fogVal = 1000.0;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(u32 *)&fogVal);
    fogVal = 5000.0;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_FOGEND, *(u32 *)&fogVal);
    if (((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP) & 1) == 0)
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0)
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
    if (((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP) & 1) == 0)
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    else
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0)
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
        anm1 = g_AnmManager;
        anm1->currentBlendMode = 0xff;
        anm2 = g_AnmManager;
        anm2->currentColorOp = 0xff;
        anm3 = g_AnmManager;
        anm3->currentVertexShader = 0xff;
        anm4 = g_AnmManager;
        anm4->currentTexture = NULL;
    }
    g_Stage.skyFogNeedsSetup = 1;
    return;
}
}; // namespace th06
