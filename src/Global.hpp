#pragma once
#include <d3d8.h>
#include <d3dx8.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#include "ZunBool.hpp"
#include "ZunColor.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"
#include "i18n.hpp"
#include "pbg3/Pbg3Archive.hpp"

#define IS_PRESSED(key) (g_CurFrameInput & (key))
#define WAS_PRESSED(key) (IS_PRESSED(key) && (g_CurFrameInput & (key)) != (g_PreviousFrameKeys & (key)))
#define WAS_PRESSED_REPEATING(key) (WAS_PRESSED(key) || (IS_PRESSED(key) && g_IsEigthFrameInput))

#define ZUN_ALLOC(size) (u8 *)g_ZunMemoryMgr.Alloc(size)
#define ZUN_ALLOC_TYPE(type) (type *)ZUN_ALLOC(sizeof(type))
#define ZUN_ALLOC_ARRAY(type, count) (type *)ZUN_ALLOC(sizeof(type) * (count))
#define ZUN_FREE(ptr) g_ZunMemoryMgr.Free(ptr)
#define ZUN_NEW(type) (type *)g_ZunMemoryMgr.AddToRegistry(new type(), sizeof(type), MACRO_STR(type))
#define ZUN_NEW_ARRAY(type, count)                                                                                     \
    (type *)g_ZunMemoryMgr.AddToRegistry(new type[count](), sizeof(type) * (count), MACRO_STR(type))
#define ZUN_DELETE(p)                                                                                                  \
    g_ZunMemoryMgr.RemoveFromRegistry((void *)p);                                                                      \
    delete (p);                                                                                                        \
    (p) = NULL

// Sometimes this is written manually, pay attention to whether
// the NULL assign is inside the if statement or not.
#define ZUN_SAFE_FREE(p)                                                                                               \
    {                                                                                                                  \
        if ((p) != NULL)                                                                                               \
        {                                                                                                              \
            ZUN_FREE(p);                                                                                               \
            (p) = NULL;                                                                                                \
        }                                                                                                              \
    }

namespace th06
{

#define CHARACTER_COUNT 2
#define SHOTTYPES_PER_CHARACTER 2
#define SHOTTYPE_COUNT (CHARACTER_COUNT * SHOTTYPES_PER_CHARACTER)

#define SHOTTYPE_ID(character, shottype) ((character) * SHOTTYPES_PER_CHARACTER + (shottype))

namespace utils
{
ZunResult CheckForRunningGameInstance(void);
#if BUILT_BEFORE(102g)
ZunBool CheckDirectXVersion(void);
#endif

f32 AddNormalizeAngle(f32 a, f32 b);
void Rotate(D3DXVECTOR3 *outVector, D3DXVECTOR3 *point, f32 angle);
} // namespace utils

static void DebugPrint(const char *fmt, ...)
{
#ifdef DEBUG
    char tmpBuffer[512];
    va_list args;

    va_start(args, fmt);
    vsprintf(tmpBuffer, fmt, args);
    va_end(args);

    printf("DEBUG: %s\n", tmpBuffer);
#endif
}

enum TouhouButton
{
    TH_BUTTON_SHOOT = 1 << 0,
    TH_BUTTON_BOMB = 1 << 1,
    TH_BUTTON_FOCUS = 1 << 2,
    TH_BUTTON_MENU = 1 << 3,
    TH_BUTTON_UP = 1 << 4,
    TH_BUTTON_DOWN = 1 << 5,
    TH_BUTTON_LEFT = 1 << 6,
    TH_BUTTON_RIGHT = 1 << 7,
    TH_BUTTON_SKIP = 1 << 8,
    TH_BUTTON_Q = 1 << 9,
    TH_BUTTON_S = 1 << 10,
    TH_BUTTON_HOME = 1 << 11,
    TH_BUTTON_ENTER = 1 << 12,

    TH_BUTTON_UP_LEFT = TH_BUTTON_UP | TH_BUTTON_LEFT,
    TH_BUTTON_UP_RIGHT = TH_BUTTON_UP | TH_BUTTON_RIGHT,
    TH_BUTTON_DOWN_LEFT = TH_BUTTON_DOWN | TH_BUTTON_LEFT,
    TH_BUTTON_DOWN_RIGHT = TH_BUTTON_DOWN | TH_BUTTON_RIGHT,
    TH_BUTTON_DIRECTION = TH_BUTTON_DOWN | TH_BUTTON_RIGHT | TH_BUTTON_UP | TH_BUTTON_LEFT,

    TH_BUTTON_SELECTMENU = TH_BUTTON_ENTER | TH_BUTTON_SHOOT,
    TH_BUTTON_RETURNMENU = TH_BUTTON_MENU | TH_BUTTON_BOMB,
    TH_BUTTON_WRONG_CHEATCODE = TH_BUTTON_SHOOT | TH_BUTTON_BOMB | TH_BUTTON_MENU | TH_BUTTON_Q | TH_BUTTON_S |
        TH_BUTTON_ENTER,
    TH_BUTTON_ANY = 0xFFFF,
};

namespace Controller
{
u16 GetJoystickCaps(void);
u16 GetControllerInput(u16 buttons);
u8 *GetControllerState();
u16 GetInput(void);
void ResetKeyboard(void);
} // namespace Controller

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
ZUN_ASSERT_TYPE(ControllerMapping, 0x12, 2);

extern ControllerMapping g_ControllerMapping;
extern u16 g_PreviousFrameKeys;
extern u16 g_CurFrameInput;
extern u16 g_IsEigthFrameInput;
extern u16 g_CountFramesInputsWereHeld;

class ZunMemory
{
  public:
    ZunMemory()
    {
        this->bRegistryInUse = false;
    }
    ~ZunMemory()
    {
    }
    void *Alloc(i32 size)
    {
        return malloc(size);
    }
    void Free(void *ptr)
    {
        free(ptr);
    }
    void *AddToRegistry(void *ptr, size_t, const char *)
    {
        return ptr;
    }
    void RemoveFromRegistry(void *ptr)
    {
    }

  private:
    ZunBool bRegistryInUse;
};
extern ZunMemory g_ZunMemoryMgr;

// From FileSystem.hpp
namespace FileSystem
{
// This documents intent better than just true
#define EXTERNAL_FILE true
u8 *OpenPath(const char *filepath, ZunBool isExternalResource = false);
int WriteDataToFile(const char *path, const void *data, size_t size);
} // namespace FileSystem
extern long g_ReadFileSize;

// From Rng.hpp
struct Rng
{
    u16 seed;
    alignment_padding(0x2);
    u32 generationCount;

    u16 GetRandomU16();
    u32 GetRandomU32();
    f32 GetRandomF32ZeroToOne();

    void Initialize(u16 seed)
    {
        this->seed = seed;
        this->generationCount = 0;
    }

    void ResetGenerationCount()
    {
        this->generationCount = 0;
    }

    u16 GetSeed()
    {
        return this->seed;
    }

    u16 GetRandomU16InRange(u16 range)
    {
        return range != 0 ? this->GetRandomU16() % range : 0;
    }

    u32 GetRandomU32InRange(u32 range)
    {
        return range != 0 ? this->GetRandomU32() % range : 0;
    }

    f32 GetRandomF32InRange(f32 range)
    {
        return this->GetRandomF32ZeroToOne() * range;
    }
};

extern Rng g_Rng;

#define MULTIPLE_OF(val, mult) !((i32)(val) % (mult))

// From t-pot's font.h: https://t-pot.com/program/12_font/index.html
class CMyFont
{
  private:
    LPD3DXFONT m_lpFont;

  public:
    CMyFont()
    {
        m_lpFont = NULL;
    }
    virtual void Init(LPDIRECT3DDEVICE8 lpD3DDEV, int w, int h);
    virtual void Print(char *str, int x, int y, D3DCOLOR color = COLOR_WHITE);
    virtual void Clean();
};

// From GameErrorContext.hpp
struct GameErrorContext
{
    char m_Buffer[0x800];
#if BUILT_BEFORE(102g)
    char m_DetailedBuffer[0x800];
#endif
    char *m_BufferEnd;
#if BUILT_BEFORE(102g)
    char *m_DetailedBufferEnd;
#endif
    i8 m_ShowMessageBox;
    alignment_padding(0x3);

    // Defined before the constructor that calls it, so the /O2 trial can compile
    // the constructor in its first pass (ahead of FileSystem::OpenPath).
    void ResetContext()
    {
        m_BufferEnd = m_Buffer;
        m_BufferEnd[0] = '\0';
#if BUILT_BEFORE(102g)
        m_DetailedBufferEnd = m_DetailedBuffer;
        m_DetailedBufferEnd[0] = '\0';
#endif
    }

    GameErrorContext()
    {
        ResetContext();
        m_ShowMessageBox = false;
        Log(TH_ERR_LOGGER_START);
#if BUILT_BEFORE(102g)
        DetailedLog(TH_ERR_DETAILED_LOGGER_START);
#endif
    }

    ~GameErrorContext()
    {
    }

    const char *Fatal(const char *fmt, ...);
    const char *Log(const char *fmt, ...);
#if BUILT_BEFORE(102g)
    const char *DetailedLog(const char *fmd, ...);
#endif

    void Flush()
    {
        FILE *logFile;

        if (m_BufferEnd != m_Buffer)
        {
            this->Log(TH_ERR_LOGGER_END);

            if (m_ShowMessageBox)
            {
                MessageBox(NULL, m_Buffer, "log", MB_ICONERROR);
            }

            logFile = fopen("./log.txt", "wt");

            fprintf(logFile, m_Buffer);
            fclose(logFile);
        }
    }
};
#if BUILT_SINCE(102g)
ZUN_ASSERT_TYPE(GameErrorContext, 0x808, 4);
#else
ZUN_ASSERT_TYPE(GameErrorContext, 0x100C, 4);
#endif

extern GameErrorContext g_GameErrorContext;
extern Pbg3Archive **g_Pbg3ArcList;
} // namespace th06
