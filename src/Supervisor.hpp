#pragma once

#include <d3d8.h>
#include <d3dx8math.h>
#include <dinput.h>

#include "Chain.hpp"
#include "Global.hpp"
#include "MidiOutput.hpp"
#include "ZunBool.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "pbg3/Pbg3Archive.hpp"

namespace th06
{
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

    ZunBool ReadMidiFile(u32 midiFileIdx, char *path);
    i32 PlayMidiFile(i32 midiFileIdx);
    ZunResult PlayAudio(char *path);
    ZunResult StopAudio();
    ZunResult SetupMidiPlayback(char *path);
    ZunResult FadeOutMusic(f32 fadeOutSeconds);

    static ZunResult SetupDInput(Supervisor *s);
    static BOOL CALLBACK ControllerCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
    static BOOL CALLBACK EnumGameControllersCb(LPCDIDEVICEINSTANCE pdidInstance, LPVOID pContext);

    i32 LoadPbg3(i32 pbg3FileIdx, char *filename);
    void ReleasePbg3(i32 pbg3FileIdx);

    ZunResult LoadConfig(char *path);

    void TickTimer(i32 *frames, f32 *subframes);

    f32 FramerateMultiplier()
    {
        return this->effectiveFramerateMultiplier;
    }

    ZunBool IsHardwareBlendingDisabled()
    {
        return this->cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING & 1;
    }

    ZunBool IsVertexBufferDisabled()
    {
        return this->cfg.opts >> GCOS_DONT_USE_VERTEX_BUF & 1;
    }

    ZunBool Is16bitColorMode()
    {
        return this->cfg.opts >> GCOS_FORCE_16BIT_COLOR_MODE & 1;
    }

    ZunBool IsMinimumGraphicsMode()
    {
        return this->cfg.opts >> GCOS_DISPLAY_MINIMUM_GRAPHICS & 1;
    }

    ZunBool IsShadingDisabled()
    {
        return this->cfg.opts >> GCOS_SUPPRESS_USE_OF_GOROUD_SHADING & 1;
    }

    ZunBool IsDepthTestDisabled()
    {
        return this->cfg.opts >> GCOS_TURN_OFF_DEPTH_TEST & 1;
    }

    ZunBool IsForced60Fps()
    {
        return this->cfg.opts >> GCOS_FORCE_60FPS & 1;
    }

    ZunBool IsColorCompositingDisabled()
    {
        return this->cfg.opts >> GCOS_NO_COLOR_COMP & 1;
    }

    ZunBool IsReferenceRasterizerMode()
    {
        return this->cfg.opts >> GCOS_REFERENCE_RASTERIZER_MODE & 1;
    }

    ZunBool IsFogDisabled()
    {
        return this->cfg.opts >> GCOS_DONT_USE_FOG & 1;
    }

    ZunBool IsDInputDisabled()
    {
        return this->cfg.opts >> GCOS_NO_DIRECTINPUT_PAD & 1;
    }

    ZunBool ShouldForceBackbufferClear()
    {
        return this->cfg.opts >> GCOS_CLEAR_BACKBUFFER_ON_REFRESH & 1 | this->IsMinimumGraphicsMode();
    }

    u32 IsSoftwareTexturing()
    {
        return this->IsColorCompositingDisabled() | this->IsHardwareBlendingDisabled();
    }

    ZunBool ShouldRunAt60Fps()
    {
        return this->IsForced60Fps() && this->vsyncEnabled;
    }

    ZunBool IsWindowed()
    {
        return this->cfg.windowed;
    }

    HINSTANCE hInstance;
    LPDIRECT3D8 d3dIface;
    LPDIRECT3DDEVICE8 d3dDevice;
    LPDIRECTINPUT8 dinputIface;
    LPDIRECTINPUTDEVICE8 keyboard;
    LPDIRECTINPUTDEVICE8 controller;
    DIDEVCAPS controllerCaps;
    HWND hwndGameWindow;
    D3DXMATRIX viewMatrix;
    D3DXMATRIX projectionMatrix;
    D3DVIEWPORT8 viewport;
    D3DPRESENT_PARAMETERS presentParameters;
    GameConfiguration cfg;
    GameConfiguration defaultConfig;
    i32 calcCount;
    i32 wantedState;
    i32 curState;
    i32 wantedState2;

    i32 unk194;
    i32 unk198;
    ZunBool isInEnding;

    ZunBool vsyncEnabled;
    i32 lastFrameTime;
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
    D3DCAPS8 d3dCaps;
};
ZUN_ASSERT_SIZE(Supervisor, 0x4d8);

DIFFABLE_EXTERN(ControllerMapping, g_ControllerMapping)
DIFFABLE_EXTERN(Supervisor, g_Supervisor)
DIFFABLE_EXTERN(LPDIRECT3DSURFACE8, g_TextBufferSurface)

struct ZunTimer
{
    i32 previous;
    f32 subFrame;
    i32 current;

    ZunTimer()
    {
        this->Initialize();
    }

    void Initialize();
    void Increment(i32 value);
    void Decrement(i32 value);

    void operator+=(i32 value)
    {
        this->Increment(value);
    }
    void operator-=(i32 value)
    {
        this->Decrement(value);
    }
    void operator++(int)
    {
        this->Tick();
    }
    void operator--(int)
    {
        this->Decrement(1);
    }

    void SetCurrent(i32 value)
    {
        this->current = value;
        this->subFrame = 0;
        this->previous = -999;
    }
    void operator=(i32 value)
    {
        SetCurrent(value);
    }

    i32 Tick()
    {
        this->previous = this->current;
        g_Supervisor.TickTimer(&this->current, &this->subFrame);
        return this->current;
    }

    // Changing this to operator f32 requires way too many
    // casts on comparison operators to be realistic...
    f32 AsFramesFloat()
    {
        return this->current + this->subFrame;
    }

    operator i32()
    {
        return this->current;
    }

    ZunBool HasTicked()
    {
        return this->current != this->previous;
    }
};
ZUN_ASSERT_SIZE(ZunTimer, 0xc);

}; // namespace th06
