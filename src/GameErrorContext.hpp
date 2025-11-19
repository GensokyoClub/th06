#pragma once

#include <windows.h>

#include "diffbuild.hpp"
#include "i18n.hpp"
#include "inttypes.hpp"

namespace th06
{
class GameErrorContext;

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

    void Flush();

    const char *Fatal(const char *fmt, ...);
    const char *Log(const char *fmt, ...);
};

DIFFABLE_EXTERN(GameErrorContext, g_GameErrorContext)
}; // namespace th06
