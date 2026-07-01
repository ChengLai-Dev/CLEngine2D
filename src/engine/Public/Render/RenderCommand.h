#pragma once

#include <glad/glad.h>

struct Mat4;

class RenderCommand {
public:
    static void SetClearColor(float r, float g, float b, float a);
    static void Clear();
    static void SetViewport(int x, int y, int width, int height);
    static void SetDepthTest(bool enabled);
    static void SetBlend(bool enabled);
    static void SetBlendFunc(GLenum sfactor, GLenum dfactor);
};
