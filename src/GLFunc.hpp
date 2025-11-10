#pragma once

#include <SDL2/SDL_opengl.h>

namespace th06
{
// Function pointers for OpenGL functions used in EoSD. This is necessary because Windows
//   opengl32 only goes up to OpenGL 1.1 and some of the blending parameters we need are
//   from 1.3. Resolving function addresses at runtime using SDL_GL_GetProcAddress gets
//   around that restriction. Plus not directly linking the GL library is good for flexibility
//   in general, even on UNIX
struct GLFuncTable
{
    void ResolveFunctions(bool glesContext);

    // Functions where arguments use doubles in OpenGL and floats in GLES and therefore need manual dispatch
    void glClearDepthf(GLclampf depth);
    void glDepthRangef(GLclampf near_val, GLclampf far_val);
    void glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val, GLfloat far_val);

    // Function pointers for functions shared between GL and GLES
    void GLAPIENTRY (*glAlphaFunc)(GLenum func, GLclampf ref);
    void GLAPIENTRY (*glBindTexture)(GLenum target, GLuint texture);
    void GLAPIENTRY (*glBlendFunc)(GLenum sfactor, GLenum dfactor);
    void GLAPIENTRY (*glClear)(GLbitfield mask);
    void GLAPIENTRY (*glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    void GLAPIENTRY (*glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
    void GLAPIENTRY (*glDeleteTextures)(GLsizei n, const GLuint *textures);
    void GLAPIENTRY (*glDepthFunc)(GLenum func);
    void GLAPIENTRY (*glDepthMask)(GLboolean flag);
    void GLAPIENTRY (*glDisableClientState)(GLenum cap);
    void GLAPIENTRY (*glDrawArrays)(GLenum mode, GLint first, GLsizei count);
    void GLAPIENTRY (*glEnable)(GLenum cap);
    void GLAPIENTRY (*glEnableClientState)(GLenum cap);
    void GLAPIENTRY (*glFogf)(GLenum pname, GLfloat param);
    void GLAPIENTRY (*glFogfv)(GLenum pname, const GLfloat *params);
    void GLAPIENTRY (*glGenTextures)(GLsizei n, GLuint *textures);
    GLenum GLAPIENTRY (*glGetError)(void);
    void GLAPIENTRY (*glGetFloatv)(GLenum pname, GLfloat *params);
    void GLAPIENTRY (*glGetIntegerv)(GLenum pname, GLint *params);
    void GLAPIENTRY (*glLoadIdentity)(void);
    void GLAPIENTRY (*glLoadMatrixf)(const GLfloat *m);
    void GLAPIENTRY (*glMatrixMode)(GLenum mode);
    void GLAPIENTRY (*glMultMatrixf)(const GLfloat *m);
    void GLAPIENTRY (*glPopMatrix)(void);
    void GLAPIENTRY (*glPushMatrix)(void);
    void GLAPIENTRY (*glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
                                    GLvoid *pixels);
    void GLAPIENTRY (*glScalef)(GLfloat x, GLfloat y, GLfloat z);
    void GLAPIENTRY (*glShadeModel)(GLenum mode);
    void GLAPIENTRY (*glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
    void GLAPIENTRY (*glTexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
    void GLAPIENTRY (*glTexEnvi)(GLenum target, GLenum pname, GLint param);
    void GLAPIENTRY (*glTexImage2D)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void GLAPIENTRY (*glTexParameteri)(GLenum target, GLenum pname, GLint param);
    void GLAPIENTRY (*glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                                       GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    void GLAPIENTRY (*glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
    void GLAPIENTRY (*glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
    void GLAPIENTRY (*glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

  private:
    // GLES forms for cases where they're different
    void GLAPIENTRY (*glClearDepthf_ptr)(GLclampf depth);
    void GLAPIENTRY (*glDepthRangef_ptr)(GLclampf near_val, GLclampf far_val);
    void GLAPIENTRY (*glFrustumf_ptr)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val,
                                      GLfloat far_val);

    // GL forms for cases where they're different
    void GLAPIENTRY (*glClearDepth)(GLclampd depth);
    void GLAPIENTRY (*glDepthRange)(GLclampd near_val, GLclampd far_val);
    void GLAPIENTRY (*glFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val,
                                 GLdouble far_val);

    bool isGlesContext;
};

extern GLFuncTable g_glFuncTable;
}; // namespace th06
