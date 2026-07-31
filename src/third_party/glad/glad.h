#ifndef __glad_h_
#define __glad_h_

#ifdef __gl_h_
#error OpenGL header already included, remove this include, glad already provides it
#endif
#define __gl_h_

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#endif

#include <glad/khrplatform.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GLAD_API_CALL extern
#define GLAD_API_PTR KHRONOS_APIENTRY

typedef void (*GLADapiproc)(void);
typedef GLADapiproc (*GLADloadproc)(const char *name);

GLAD_API_CALL int gladLoadGLLoader(GLADloadproc);
GLAD_API_CALL int gladLoadGL(void);

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef char GLchar;
typedef khronos_intptr_t GLintptr;
typedef khronos_size_t GLsizeiptr;

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_TRIANGLES 0x0004
#define GL_FRAMEBUFFER 0x8D40
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_TEXTURE_2D 0x0DE1
#define GL_RGBA 0x1908
#define GL_RGBA8 0x8058
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801

typedef void (GLAD_API_PTR *PFNGLCLEARPROC)(GLbitfield mask);
typedef void (GLAD_API_PTR *PFNGLCLEARCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (GLAD_API_PTR *PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAD_API_PTR *PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint *framebuffers);
typedef void (GLAD_API_PTR *PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
typedef void (GLAD_API_PTR *PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei n, const GLuint *framebuffers);
typedef void (GLAD_API_PTR *PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
typedef void (GLAD_API_PTR *PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void (GLAD_API_PTR *PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (GLAD_API_PTR *PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
typedef void (GLAD_API_PTR *PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (GLAD_API_PTR *PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint *textures);
typedef void (GLAD_API_PTR *PFNGLGENRENDERBUFFERSPROC)(GLsizei n, GLuint *renderbuffers);
typedef void (GLAD_API_PTR *PFNGLBINDRENDERBUFFERPROC)(GLenum target, GLuint renderbuffer);
typedef void (GLAD_API_PTR *PFNGLRENDERBUFFERSTORAGEPROC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GLAD_API_PTR *PFNGLFRAMEBUFFERRENDERBUFFERPROC)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (GLAD_API_PTR *PFNGLDELETERENDERBUFFERSPROC)(GLsizei n, const GLuint *renderbuffers);
typedef GLenum (GLAD_API_PTR *PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);

GLAD_API_CALL PFNGLCLEARPROC glad_glClear;
#define glClear glad_glClear
GLAD_API_CALL PFNGLCLEARCOLORPROC glad_glClearColor;
#define glClearColor glad_glClearColor
GLAD_API_CALL PFNGLVIEWPORTPROC glad_glViewport;
#define glViewport glad_glViewport

GLAD_API_CALL PFNGLGENFRAMEBUFFERSPROC glad_glGenFramebuffers;
#define glGenFramebuffers glad_glGenFramebuffers
GLAD_API_CALL PFNGLBINDFRAMEBUFFERPROC glad_glBindFramebuffer;
#define glBindFramebuffer glad_glBindFramebuffer
GLAD_API_CALL PFNGLDELETEFRAMEBUFFERSPROC glad_glDeleteFramebuffers;
#define glDeleteFramebuffers glad_glDeleteFramebuffers

GLAD_API_CALL PFNGLGENTEXTURESPROC glad_glGenTextures;
#define glGenTextures glad_glGenTextures
GLAD_API_CALL PFNGLBINDTEXTUREPROC glad_glBindTexture;
#define glBindTexture glad_glBindTexture
GLAD_API_CALL PFNGLTEXIMAGE2DPROC glad_glTexImage2D;
#define glTexImage2D glad_glTexImage2D
GLAD_API_CALL PFNGLTEXPARAMETERIPROC glad_glTexParameteri;
#define glTexParameteri glad_glTexParameteri
GLAD_API_CALL PFNGLFRAMEBUFFERTEXTURE2DPROC glad_glFramebufferTexture2D;
#define glFramebufferTexture2D glad_glFramebufferTexture2D
GLAD_API_CALL PFNGLDELETETEXTURESPROC glad_glDeleteTextures;
#define glDeleteTextures glad_glDeleteTextures

GLAD_API_CALL PFNGLGENRENDERBUFFERSPROC glad_glGenRenderbuffers;
#define glGenRenderbuffers glad_glGenRenderbuffers
GLAD_API_CALL PFNGLBINDRENDERBUFFERPROC glad_glBindRenderbuffer;
#define glBindRenderbuffer glad_glBindRenderbuffer
GLAD_API_CALL PFNGLRENDERBUFFERSTORAGEPROC glad_glRenderbufferStorage;
#define glRenderbufferStorage glad_glRenderbufferStorage
GLAD_API_CALL PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer;
#define glFramebufferRenderbuffer glad_glFramebufferRenderbuffer
GLAD_API_CALL PFNGLDELETERENDERBUFFERSPROC glad_glDeleteRenderbuffers;
#define glDeleteRenderbuffers glad_glDeleteRenderbuffers

GLAD_API_CALL PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus;
#define glCheckFramebufferStatus glad_glCheckFramebufferStatus

#ifdef __cplusplus
}
#endif

#endif
