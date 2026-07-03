#pragma once

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "Math/Vec4.h"

#include <memory>
#include <string>
#include <vector>

class Texture;
class Renderer;

class TextRenderer {
public:
    enum class Align {
        Left,
        Center,
        Right
    };

    TextRenderer();
    ~TextRenderer();

    bool LoadFont(const std::string& filepath, float pixelHeight);
    bool IsLoaded() const;

    void RenderString(Renderer& renderer, const std::string& text,
                      float x, float y, float scale = 1.0f,
                      const float color[4] = nullptr,
                      Align align = Align::Left);

    Vec2 MeasureString(const std::string& text, float scale = 1.0f) const;

    float GetLineHeight(float scale = 1.0f) const;
    float GetBaselineOffset(float scale = 1.0f) const;

private:
    struct GlyphData {
        float x0, y0, x1, y1;
        float s0, t0, s1, t1;
        float xadvance;
        float xoff, yoff;
    };

    static constexpr int kFirstChar = 32;
    static constexpr int kCharCount = 96;

    std::unique_ptr<Texture> m_atlas = nullptr;
    std::vector<GlyphData> m_glyphs;
    int m_atlasWidth = 0;
    int m_atlasHeight = 0;
    float m_pixelHeight = 0.0f;
    float m_ascent = 0.0f;
    float m_descent = 0.0f;
    float m_lineGap = 0.0f;
    bool m_loaded = false;
};
