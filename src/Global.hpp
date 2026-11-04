#pragma once
#include <d3d8.h>
#include <d3dx8.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

extern DWORD GetDXVersion();

#include "DebugPrint.hpp"
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

// 1.00/1.01 store a button bitmask; 1.02 switches to signed button indices.
#if BUILT_SINCE(102)
typedef i16 ControllerButton;
#define CONTROLLER_BUTTON(n) (n)
#define INVALID_BUTTON -1
#define IS_INVALID_BUTTON(button) ((button) < 0)
#else
typedef u32 ControllerButton;
#define CONTROLLER_BUTTON(n) (1 << (n))
#define INVALID_BUTTON 0
#define IS_INVALID_BUTTON(button) ((button) == 0)
#endif

enum TouhouButtonIndex
{
    TH_BUTTON_SHOOT_IDX,
    TH_BUTTON_BOMB_IDX,
    TH_BUTTON_FOCUS_IDX,
    TH_BUTTON_MENU_IDX,
    TH_BUTTON_UP_IDX,
    TH_BUTTON_DOWN_IDX,
    TH_BUTTON_LEFT_IDX,
    TH_BUTTON_RIGHT_IDX,
    TH_BUTTON_SKIP_IDX
};

enum TouhouButton
{
    TH_BUTTON_SHOOT = 1 << TH_BUTTON_SHOOT_IDX,
    TH_BUTTON_BOMB = 1 << TH_BUTTON_BOMB_IDX,
    TH_BUTTON_FOCUS = 1 << TH_BUTTON_FOCUS_IDX,
    TH_BUTTON_MENU = 1 << TH_BUTTON_MENU_IDX,
    TH_BUTTON_UP = 1 << TH_BUTTON_UP_IDX,
    TH_BUTTON_DOWN = 1 << TH_BUTTON_DOWN_IDX,
    TH_BUTTON_LEFT = 1 << TH_BUTTON_LEFT_IDX,
    TH_BUTTON_RIGHT = 1 << TH_BUTTON_RIGHT_IDX,
    TH_BUTTON_SKIP = 1 << TH_BUTTON_SKIP_IDX,
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
    TH_BUTTON_WRONG_CHEATCODE =
        TH_BUTTON_SHOOT | TH_BUTTON_BOMB | TH_BUTTON_MENU | TH_BUTTON_Q | TH_BUTTON_S | TH_BUTTON_ENTER,
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
    ControllerButton shootButton;
    ControllerButton bombButton;
    ControllerButton focusButton;
    ControllerButton menuButton;
    ControllerButton upButton;
    ControllerButton downButton;
    ControllerButton leftButton;
    ControllerButton rightButton;
#if BUILT_SINCE(102)
    ControllerButton skipButton;
#endif
};
#if BUILT_SINCE(102)
ZUN_ASSERT_TYPE(ControllerMapping, 0x12, 2);
#else
ZUN_ASSERT_TYPE(ControllerMapping, 0x20, 4);
#endif

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
#if BUILT_SINCE(100)
u8 *OpenPath(const char *filepath, ZunBool isExternalResource = false);
#else
// 0.08 tries the archives first, then falls back to a loose file.
u8 *OpenPath(const char *filepath);
#endif
int WriteDataToFile(const char *path, const void *data, size_t size);
} // namespace FileSystem
extern long g_ReadFileSize;

// From Rng.hpp
class Rng
{
  public:
    u16 GetRandomU16();
    u32 GetRandomU32();
    f32 GetRandomF32();

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
        return this->GetRandomF32() * range;
    }

  private:
    u16 seed;
    alignment_padding(0x2);
    u32 generationCount;
};

extern Rng g_Rng;

#define MULTIPLE_OF(val, mult) !((i32)(val) % (mult))

// From t-pot's font.h: https://t-pot.com/program/12_font/index.html
class CMyFont
{
  private:
    LPD3DXFONT m_lpFont;

  public:
    virtual void Init(LPDIRECT3DDEVICE8 lpD3DDEV, int w, int h);
    virtual void Print(char *str, int x, int y, D3DCOLOR color = COLOR_WHITE);
    virtual void Clean();
    CMyFont()
    {
        m_lpFont = NULL;
    }
};

// From GameErrorContext.hpp
class GameErrorContext
{
  public:
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

  private:
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
};
#if BUILT_SINCE(102g)
ZUN_ASSERT_TYPE(GameErrorContext, 0x808, 4);
#else
ZUN_ASSERT_TYPE(GameErrorContext, 0x100C, 4);
#endif

extern GameErrorContext g_GameErrorContext;
extern Pbg3Archive **g_Pbg3ArcList;
} // namespace th06
