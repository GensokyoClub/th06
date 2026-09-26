#pragma once

#include "dxutil.hpp"
#include <cstdarg>
#include <d3d8.h>
#include <d3dx8.h>
#include <stdio.h>
#include <windows.h>

#include "ZunBool.hpp"
#include "ZunColor.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "i18n.hpp"
#include "inttypes.hpp"
#include "pbg3/Pbg3Archive.hpp"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define ARRAY_SIZE_SIGNED(x) ((i32)sizeof(x) / (i32)sizeof(x[0]))

#define ZUN_BIT(a) (1 << (a))
#define ZUN_MASK(a) (ZUN_BIT(a) - 1)
#define ZUN_RANGE(a, count) (ZUN_MASK((a) + (count)) & ~ZUN_MASK(a))
#define ZUN_CLEAR_BITS(a, keep_mask) (a & ~keep_mask)

#define IS_PRESSED(key) (g_CurFrameInput & (key))
#define WAS_PRESSED(key) (IS_PRESSED(key) && (g_CurFrameInput & (key)) != (g_LastFrameInput & (key)))
#define WAS_PRESSED_WEIRD(key) (WAS_PRESSED(key) || (IS_PRESSED(key) && g_IsEigthFrameOfHeldInput))

#define ZUN_ALLOC(size) (u8 *)g_ZunMemory.Alloc(size)
#define ZUN_ALLOC_TYPE(type) (type *)ZUN_ALLOC(sizeof(type))
#define ZUN_ALLOC_ARRAY(type, count) (type *)ZUN_ALLOC(sizeof(type) * (count))
#define ZUN_FREE(ptr) g_ZunMemory.Free(ptr)
#define ZUN_NEW(type) g_ZunMemory.AddToRegistry(new type())
#define ZUN_NEW_ARRAY(type, count) g_ZunMemory.AddToRegistry(new type[count]())
#define ZUN_DELETE(p)                                                                                                  \
    g_ZunMemory.RemoveFromRegistry(p);                                                                                 \
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

namespace utils
{
ZunResult CheckForRunningGameInstance(void);
void DebugPrint(const char *fmt, ...);
void DebugPrint2(const char *fmt, ...);

f32 AddNormalizeAngle(f32 a, f32 b);
void Rotate(D3DXVECTOR3 *outVector, D3DXVECTOR3 *point, f32 angle);
}; // namespace utils

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
u32 SetButtonFromControllerInputs(u16 *outButtons, i16 controllerButtonToTest, TouhouButton touhouButton,
                                  u32 inputButtons);

unsigned int SetButtonFromDirectInputJoystate(u16 *outButtons, i16 controllerButtonToTest, TouhouButton touhouButton,
                                              u8 *inputButtons);

u16 GetControllerInput(u16 buttons);
u8 *GetControllerState();
u16 GetInput(void);
void ResetKeyboard(void);
}; // namespace Controller

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

DIFFABLE_EXTERN(ControllerMapping, g_ControllerMapping);
DIFFABLE_EXTERN(u16, g_LastFrameInput);
DIFFABLE_EXTERN(u16, g_CurFrameInput);
DIFFABLE_EXTERN(u16, g_IsEigthFrameOfHeldInput);
DIFFABLE_EXTERN(u16, g_NumOfFramesInputsWereHeld);

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
    template <typename T> T *AddToRegistry(T *ptr, size_t = sizeof(T), const char * = "")
    {
        return ptr;
    }
    template <typename T> void RemoveFromRegistry(T *ptr)
    {
    }

  private:
    ZunBool bRegistryInUse;
};
DIFFABLE_EXTERN(ZunMemory, g_ZunMemory);

// From FileSystem.hpp
namespace FileSystem
{
// This documents intent better than just true
#define EXTERNAL_FILE true
u8 *OpenPath(const char *filepath, ZunBool isExternalResource = false);
int WriteDataToFile(const char *path, const void *data, size_t size);
} // namespace FileSystem
DIFFABLE_EXTERN(u32, g_LastFileSize);

// From Rng.hpp
struct Rng
{
    u16 seed;
    u32 generationCount;

    u16 GetRandomU16();
    u32 GetRandomU32();
    f32 GetRandomF32ZeroToOne();

    void Initialize(u16 seed)
    {
        this->seed = seed;
        this->generationCount = 0;
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

DIFFABLE_EXTERN(Rng, g_Rng);
DIFFABLE_EXTERN(HANDLE, g_ExclusiveMutex);

// From GameErrorContext.hpp
class GameErrorContext
{
  public:
    char m_Buffer[0x800];
    char *m_BufferEnd;
    i8 m_ShowMessageBox;

    GameErrorContext()
    {
        m_BufferEnd = m_Buffer;
        m_Buffer[0] = '\0';
        // Required to get some mov eax, [m_Buffer_ptr]
        m_ShowMessageBox = false;
        Log(TH_ERR_LOGGER_START);
    }

    ~GameErrorContext()
    {
    }

    void ResetContext()
    {
        m_BufferEnd = m_Buffer;
        m_BufferEnd[0] = '\0';
        // TODO: check if it should be m_Buffer[0] above.
    }

    const char *Fatal(const char *fmt, ...);
    const char *Log(const char *fmt, ...);

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

DIFFABLE_EXTERN(GameErrorContext, g_GameErrorContext);
DIFFABLE_EXTERN(Pbg3Archive **, g_Pbg3Archives);
DIFFABLE_EXTERN(LPDIRECT3DSURFACE8, g_TextBufferSurface);
}; // namespace th06
