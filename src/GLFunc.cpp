#include "GLFunc.hpp"

#include <SDL2/SDL_video.h>

namespace th06
{
GLFuncTable g_glFuncTable;

#define TRY_RESOLVE_FUNCTION(func) this->func = (decltype(this->func))SDL_GL_GetProcAddress(#func);
#define TRY_RESOLVE_FUNCTION_GLES(func) this->func##_ptr = (decltype(this->func##_ptr))SDL_GL_GetProcAddress(#func);

void GLFuncTable::ResolveFunctions(bool glesContext)
{
    TRY_RESOLVE_FUNCTION(glAlphaFunc)
    TRY_RESOLVE_FUNCTION(glBindTexture)
    TRY_RESOLVE_FUNCTION(glBlendFunc)
    TRY_RESOLVE_FUNCTION(glClear)
    TRY_RESOLVE_FUNCTION(glClearColor)
    TRY_RESOLVE_FUNCTION(glColorPointer)
    TRY_RESOLVE_FUNCTION(glDeleteTextures)
    TRY_RESOLVE_FUNCTION(glDepthFunc)
    TRY_RESOLVE_FUNCTION(glDepthMask)
    TRY_RESOLVE_FUNCTION(glDisableClientState)
    TRY_RESOLVE_FUNCTION(glDrawArrays)
    TRY_RESOLVE_FUNCTION(glEnable)
    TRY_RESOLVE_FUNCTION(glEnableClientState)
    TRY_RESOLVE_FUNCTION(glFogf)
    TRY_RESOLVE_FUNCTION(glFogfv)
    TRY_RESOLVE_FUNCTION(glGenTextures)
    TRY_RESOLVE_FUNCTION(glGetError)
    TRY_RESOLVE_FUNCTION(glGetFloatv)
    TRY_RESOLVE_FUNCTION(glGetIntegerv)
    TRY_RESOLVE_FUNCTION(glLoadIdentity)
    TRY_RESOLVE_FUNCTION(glLoadMatrixf)
    TRY_RESOLVE_FUNCTION(glMatrixMode)
    TRY_RESOLVE_FUNCTION(glMultMatrixf)
    TRY_RESOLVE_FUNCTION(glPopMatrix)
    TRY_RESOLVE_FUNCTION(glPushMatrix)
    TRY_RESOLVE_FUNCTION(glReadPixels)
    TRY_RESOLVE_FUNCTION(glScalef)
    TRY_RESOLVE_FUNCTION(glShadeModel)
    TRY_RESOLVE_FUNCTION(glTexCoordPointer)
    TRY_RESOLVE_FUNCTION(glTexEnvfv)
    TRY_RESOLVE_FUNCTION(glTexEnvi)
    TRY_RESOLVE_FUNCTION(glTexImage2D)
    TRY_RESOLVE_FUNCTION(glTexParameteri)
    TRY_RESOLVE_FUNCTION(glTexSubImage2D)
    TRY_RESOLVE_FUNCTION(glTranslatef)
    TRY_RESOLVE_FUNCTION(glVertexPointer)
    TRY_RESOLVE_FUNCTION(glViewport)

    // Ideally, we'd just check for both the regular GL and GLES version of the function and
    //   use whichever doesn't return NULL, but function resolves on GLX are actually context
    //   independent, meaning we can get a valid function pointer that then throws an error
    //   when we call it because the context doesn't actually match what's needed. So instead
    //   we need to pass a parameter to identify which function version to resolve and use.

    if (glesContext)
    {
        TRY_RESOLVE_FUNCTION_GLES(glClearDepthf)
        TRY_RESOLVE_FUNCTION_GLES(glDepthRangef)
        TRY_RESOLVE_FUNCTION_GLES(glFrustumf)
    }
    else
    {
        TRY_RESOLVE_FUNCTION(glClearDepth)
        TRY_RESOLVE_FUNCTION(glDepthRange)
        TRY_RESOLVE_FUNCTION(glFrustum)
    }

    this->isGlesContext = glesContext;
}

void GLFuncTable::glClearDepthf(GLclampf depth)
{
    if (this->isGlesContext)
    {
        this->glClearDepthf_ptr(depth);
    }
    else
    {
        this->glClearDepth(depth);
    }
}

void GLFuncTable::glDepthRangef(GLclampf near_val, GLclampf far_val)
{
    if (this->isGlesContext)
    {
        this->glDepthRangef_ptr(near_val, far_val);
    }
    else
    {
        this->glDepthRange(near_val, far_val);
    }
}

void GLFuncTable::glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val,
                             GLfloat far_val)
{
    if (this->isGlesContext)
    {
        this->glFrustumf_ptr(left, right, bottom, top, near_val, far_val);
    }
    else
    {
        this->glFrustum(left, right, bottom, top, near_val, far_val);
    }
}

}; // namespace th06
