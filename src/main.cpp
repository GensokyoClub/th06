#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <cstdio>

#include "AnmManager.hpp"
#include "Chain.hpp"
#include "FileSystem.hpp"
#include "GameErrorContext.hpp"
#include "GameWindow.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"
#include "i18n.hpp"
#include "utils.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

// Forward declarations
static void main_loop();
static void cleanup();
static void restart_game();

static bool running = false;
static i32 renderResult = 0;

#ifdef __EMSCRIPTEN__
#define EM_TH_CONFIG_FILE "/persistent/" TH_CONFIG_FILE
#endif

// ------------------------------------------------
// Initialization
// ------------------------------------------------

static bool initialize_game() {
#ifdef __EMSCRIPTEN__
    if (g_Supervisor.LoadConfig(EM_TH_CONFIG_FILE) != ZUN_SUCCESS)
#else
    if (g_Supervisor.LoadConfig(TH_CONFIG_FILE) != ZUN_SUCCESS)
#endif
    {
        g_GameErrorContext.Flush();
        return false;
    }

    GameWindow::CreateGameWindow();

    g_AnmManager = new AnmManager();

    if (GameWindow::InitD3dRendering() != ZUN_SUCCESS) {
        g_GameErrorContext.Flush();
        return false;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_element_css_size("#canvas", GAME_WINDOW_WIDTH_REAL, GAME_WINDOW_HEIGHT_REAL);
#endif

    g_SoundPlayer.InitializeDSound();
    Controller::GetJoystickCaps();
    Controller::ResetKeyboard();

    if (Supervisor::RegisterChain() != ZUN_SUCCESS) {
        g_GameErrorContext.Flush();
        return false;
    }

    if (!g_Supervisor.cfg.windowed)
        SDL_ShowCursor(SDL_DISABLE);

    g_GameWindow.curFrame = 0;
    running = true;

    return true;
}

static void main_loop() {
    if (!running)
        return;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            cleanup();
            return;
        } else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            g_ViewportScale.Recompute(e.window.data1, e.window.data2);
        }
    }

    renderResult = g_GameWindow.Render();
    if (renderResult != 0) {
        running = false;
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        cleanup();

        if (renderResult == 2) {
            restart_game();
        }
    }
}

// ------------------------------------------------
// Cleanup & Restart
// ------------------------------------------------

static void cleanup() {
    g_Chain.Release();
    g_SoundPlayer.Release();

    delete g_AnmManager;
    g_AnmManager = NULL;

    if (g_GfxBackend != NULL)
        delete g_GfxBackend;

    SDL_Quit();

#ifdef __EMSCRIPTEN__
    FileSystem::WriteDataToFile(EM_TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(g_Supervisor.cfg));
#else
    FileSystem::WriteDataToFile(TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(g_Supervisor.cfg));
#endif

    SDL_ShowCursor(SDL_ENABLE);
    g_GameErrorContext.Flush();
}

static void restart_game() {
    g_GameErrorContext.ResetContext();
    g_GameErrorContext.Log(TH_ERR_OPTION_CHANGED_RESTART);

    if (!g_Supervisor.cfg.windowed)
        SDL_ShowCursor(SDL_ENABLE);

    // Reinitialize game
    if (initialize_game()) {
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(main_loop, 0, 1);
#else
        while (running)
            main_loop();
#endif
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    if (!initialize_game())
        return -1;

#ifdef __EMSCRIPTEN__
    // Non-blocking browser main loop
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (running)
        main_loop();
#endif

    return 0;
}
