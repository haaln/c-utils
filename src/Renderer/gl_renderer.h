#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include <GL/gl.h>

/**
 *
 */
void debug_callback(
    GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char *message,
    const void *userParam);

/**
 *
 */
void enableReportGlErrors();

#endif // GL_RENDERER_H
