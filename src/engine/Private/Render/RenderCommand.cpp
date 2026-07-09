#include "Render/RenderCommand.h"
#include <glad/glad.h>

void RenderCommand::SetClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void RenderCommand::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderCommand::SetViewport(int x, int y, int width, int height) {
    if (width < 0) width = 0;
    if (height < 0) height = 0;
    glViewport(x, y, width, height);
}

void RenderCommand::SetDepthTest(bool enabled) {
    if (enabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void RenderCommand::SetBlend(bool enabled) {
    if (enabled)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
}

void RenderCommand::SetBlendFunc(GLenum sfactor, GLenum dfactor) {
    glBlendFunc(sfactor, dfactor);
}

void RenderCommand::SetScissor(bool enabled) {
    if (enabled)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
}

void RenderCommand::SetScissorRect(int x, int y, int width, int height) {
    if (width < 0) width = 0;
    if (height < 0) height = 0;
    glScissor(x, y, width, height);
}
