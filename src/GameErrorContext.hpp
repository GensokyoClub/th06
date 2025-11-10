#pragma once

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
        Log(this, TH_ERR_LOGGER_START);
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

    static const char *Fatal(GameErrorContext *ctx, const char *fmt, ...);
    static const char *Log(GameErrorContext *ctx, const char *fmt, ...);
};

extern GameErrorContext g_GameErrorContext;
}; // namespace th06
