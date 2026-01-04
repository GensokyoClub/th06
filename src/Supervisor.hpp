#pragma once

#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_video.h>

#include "Chain.hpp"
#include "Controller.hpp"
#include "MidiOutput.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "inttypes.hpp"
#include "pbg3/Pbg3Archive.hpp"

#define GAME_VERSION 0x102

enum GameConfigOptsShifts
{
    GCOS_USE_D3D_HW_TEXTURE_BLENDING = 0x0,
    GCOS_DONT_USE_VERTEX_BUF = 0x1,
    GCOS_FORCE_16BIT_COLOR_MODE = 0x2,
    GCOS_CLEAR_BACKBUFFER_ON_REFRESH = 0x3,
    GCOS_DISPLAY_MINIMUM_GRAPHICS = 0x4,
    GCOS_SUPPRESS_USE_OF_GOROUD_SHADING = 0x5,
    GCOS_TURN_OFF_DEPTH_TEST = 0x6,
    GCOS_FORCE_60FPS = 0x7,
    GCOS_NO_COLOR_COMP = 0x8,
    GCOS_REFERENCE_RASTERIZER_MODE = 0x9,
    GCOS_DONT_USE_FOG = 0xa,
    GCOS_NO_DIRECTINPUT_PAD = 0xb,
};

struct ControllerMapping
{
    i16 shootButton;
    i16 bombButton;
    i16 focusButton;
    i16 menuButton;
    i16 upButton;
    i16 downButton;
    i16 leftButton;
    i16 rightButton;
    i16 skipButton;
};

enum MusicMode
{
    OFF = 0,
    WAV = 1,
    MIDI = 2
};

struct GameConfiguration
{
    ControllerMapping controllerMapping;
    // Always 0x102 for 1.02
    i32 version;
    u8 lifeCount;
    u8 bombCount;
    u8 colorMode16bit;
    u8 musicMode;
    u8 playSounds;
    u8 defaultDifficulty;
    u8 windowed;
    // 0 = fullspeed, 1 = 1/2 speed, 2 = 1/4 speed.
    u8 frameskipConfig;
    i16 padXAxis;
    i16 padYAxis;
    i8 unk[16];
    // GameConfigOpts bitfield.
    u32 opts;

    u32 IsSoftwareTexturing()
    {
        return (this->opts >> GCOS_NO_COLOR_COMP & 1) | (this->opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING & 1);
    }
};

#define IN_PBG3_INDEX 0
#define MD_PBG3_INDEX 1
#define ST_PBG3_INDEX 2
#define CM_PBG3_INDEX 4
#define ED_PBG3_INDEX 5

typedef char Pbg3ArchiveName[32];

enum SupervisorState
{
    SUPERVISOR_STATE_INIT,
    SUPERVISOR_STATE_MAINMENU,
    SUPERVISOR_STATE_GAMEMANAGER,
    SUPERVISOR_STATE_GAMEMANAGER_REINIT,
    SUPERVISOR_STATE_EXITSUCCESS,
    SUPERVISOR_STATE_EXITERROR,
    SUPERVISOR_STATE_RESULTSCREEN,
    SUPERVISOR_STATE_RESULTSCREEN_FROMGAME,
    SUPERVISOR_STATE_MAINMENU_REPLAY,
    SUPERVISOR_STATE_MUSICROOM,
    SUPERVISOR_STATE_ENDING,
};

struct Supervisor
{
    static ZunResult RegisterChain();
    static ChainCallbackResult OnUpdate(Supervisor *s);
    static ChainCallbackResult OnDraw(Supervisor *s);
    static ZunResult AddedCallback(Supervisor *s);
    static ZunResult DeletedCallback(Supervisor *s);
    static void DrawFpsCounter();

    bool ReadMidiFile(u32 midiFileIdx, char *path);
    ZunResult PlayMidiFile(i32 midiFileIdx);
    ZunResult PlayAudio(char *path);
    ZunResult StopAudio();
    ZunResult FadeOutMusic(f32 fadeOutSeconds);

    static ZunResult SetupDInput(Supervisor *s);

    i32 LoadPbg3(i32 pbg3FileIdx, char *filename);
    void ReleasePbg3(i32 pbg3FileIdx);

    ZunResult LoadConfig(const char *path);

    void TickTimer(i32 *frames, f32 *subframes);

    f32 FramerateMultiplier()
    {
        return this->effectiveFramerateMultiplier;
    }

    u32 RedrawWholeFrame()
    {
        // SDL makes no guarantees about frame state after buffer swap,
        //   and Wayland will "reuse" old framebuffers in a nondeterministic
        //   way, so we're basically required to always redraw to avoid UI corruption
        return (this->cfg.opts >> GCOS_CLEAR_BACKBUFFER_ON_REFRESH & 1) |
               (this->cfg.opts >> GCOS_DISPLAY_MINIMUM_GRAPHICS & 1) | 1;
    }

    u32 ShouldRunAt60Fps()
    {
        return (this->cfg.opts >> GCOS_FORCE_60FPS & 1) || this->vsyncEnabled;
    }

    //    HINSTANCE hInstance;
    //    PDIRECT3D8 d3dIface;
    //    PDIRECT3DDEVICE8 d3dDevice;
    //    LPDIRECTINPUT8 dinputIface;
    //    LPDIRECTINPUTDEVICE8A keyboard;
    //    LPDIRECTINPUTDEVICE8A controller;
    SDL_GameController *gameController;
    //    DIDEVCAPS controllerCaps;
    SDL_Window *gameWindow;
    ZunMatrix viewMatrix;
    ZunMatrix projectionMatrix;
    ZunViewport viewport;
    //    D3DPRESENT_PARAMETERS presentParameters;
    GameConfiguration cfg;
    GameConfiguration defaultConfig;
    i32 calcCount;
    i32 wantedState;
    i32 curState;
    i32 wantedState2;

    i32 unk194;
    i32 unk198;
    bool isInEnding;

    i32 vsyncEnabled;
    u32 lastFrameTime;
    f32 effectiveFramerateMultiplier;
    f32 framerateMultiplier;

    MidiOutput *midiOutput;

    f32 unk1b4;
    f32 unk1b8;

    Pbg3Archive *pbg3Archives[16];
    Pbg3ArchiveName pbg3ArchiveNames[16];

    u8 hasD3dHardwareVertexProcessing;
    u8 lockableBackbuffer;
    u8 colorMode16Bits;

    u32 startupTimeBeforeMenuMusic;
    //    D3DCAPS8 d3dCaps;
};

extern ControllerMapping g_ControllerMapping;
extern Supervisor g_Supervisor;
extern u16 g_LastFrameInput;
extern u16 g_CurFrameInput;
extern u16 g_IsEigthFrameOfHeldInput;
extern SDL_Surface *g_TextBufferSurface;
extern u16 g_NumOfFramesInputsWereHeld;
