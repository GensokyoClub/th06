#include "GameErrorContext.hpp"
#include <SDL2/SDL_messagebox.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace th06
{
GameErrorContext g_GameErrorContext;

const char *GameErrorContext::Log(GameErrorContext *ctx, const char *fmt, ...)
{
    char tmpBuffer[512];
    size_t tmpBufferSize;
    va_list args;

    va_start(args, fmt);
    std::vsprintf(tmpBuffer, fmt, args);

    tmpBufferSize = std::strlen(tmpBuffer);

    if (ctx->m_BufferEnd + tmpBufferSize < &ctx->m_Buffer[sizeof(ctx->m_Buffer) - 1])
    {
        std::strcpy(ctx->m_BufferEnd, tmpBuffer);

        ctx->m_BufferEnd += tmpBufferSize;
        *ctx->m_BufferEnd = '\0';
    }

    va_end(args);

    return fmt;
}

const char *GameErrorContext::Fatal(GameErrorContext *ctx, const char *fmt, ...)
{
    char tmpBuffer[512];
    size_t tmpBufferSize;
    va_list args;

    va_start(args, fmt);
    std::vsprintf(tmpBuffer, fmt, args);

    tmpBufferSize = std::strlen(tmpBuffer);

    if (ctx->m_BufferEnd + tmpBufferSize < &ctx->m_Buffer[sizeof(ctx->m_Buffer) - 1])
    {
        std::strcpy(ctx->m_BufferEnd, tmpBuffer);

        ctx->m_BufferEnd += tmpBufferSize;
        *ctx->m_BufferEnd = '\0';
    }

    va_end(args);

    ctx->m_ShowMessageBox = true;

    return fmt;
}

void GameErrorContext::Flush()
{
    FILE *logFile;

    if (m_BufferEnd != m_Buffer)
    {
        GameErrorContext::Log(this, TH_ERR_LOGGER_END);

        if (m_ShowMessageBox)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "log", m_Buffer, NULL);
        }

        logFile = std::fopen("./log.txt", "w");

        std::fprintf(logFile, "%s", m_Buffer);
        std::fclose(logFile);
    }
}
}; // namespace th06
