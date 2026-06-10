#pragma once

#include <SDL2/SDL_video.h>

#include "ZunResult.hpp"
#include "graphics/GfxInterface.hpp"
#include "inttypes.hpp"

// The internal resolution EoSD uses. 640x480. I can't think of any reason
// anyone sane
//   would want to change this
#define GAME_WINDOW_WIDTH (640)
#define GAME_WINDOW_HEIGHT (480)

#ifndef GAME_WINDOW_WIDTH_REAL_DEFAULT
#define GAME_WINDOW_WIDTH_REAL_DEFAULT (GAME_WINDOW_WIDTH)
#endif

#ifndef GAME_WINDOW_HEIGHT_REAL_DEFAULT
#define GAME_WINDOW_HEIGHT_REAL_DEFAULT (GAME_WINDOW_HEIGHT)
#endif

struct ViewportScaling {
    i32 realWidth;
    i32 realHeight;
    i32 viewportWidth;
    i32 viewportHeight;
    i32 viewportOffX;
    i32 viewportOffY;
    f32 widthScale;
    f32 heightScale;
    bool maximized;

    void Recompute(i32 realWidth, i32 realHeight);
};

extern ViewportScaling g_ViewportScale;

#define GAME_WINDOW_WIDTH_REAL (g_ViewportScale.realWidth)
#define GAME_WINDOW_HEIGHT_REAL (g_ViewportScale.realHeight)
#define VIEWPORT_WIDTH (g_ViewportScale.viewportWidth)
#define VIEWPORT_HEIGHT (g_ViewportScale.viewportHeight)
#define VIEWPORT_OFF_X (g_ViewportScale.viewportOffX)
#define VIEWPORT_OFF_Y (g_ViewportScale.viewportOffY)
#define WIDTH_RESOLUTION_SCALE (g_ViewportScale.widthScale)
#define HEIGHT_RESOLUTION_SCALE (g_ViewportScale.heightScale)

enum RenderResult {
    RENDER_RESULT_KEEP_RUNNING,
    RENDER_RESULT_EXIT_SUCCESS,
    RENDER_RESULT_EXIT_ERROR,
};

struct GameWindow {
    RenderResult Render();
    static void Present();

    static void CreateGameWindow();
    static ZunResult InitD3dRendering();
    static void InitD3dDevice();

    i32 isAppClosing;
    i32 lastActiveAppValue;
    i32 isAppActive;
    u8 curFrame;
    i32 screenSaveActive;
    i32 lowPowerActive;
    i32 powerOffActive;
    u32 renderBackendIndex;
};

extern GameWindow g_GameWindow;
extern i32 g_TickCountToEffectiveFramerate;
extern double g_LastFrameTime;
extern GfxInterface *g_GfxBackend;