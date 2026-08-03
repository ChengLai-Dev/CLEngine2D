#pragma once

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "Math/Vec4.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Texture;
class Renderer;
struct StbttFontInfo;

class TextRenderer {
public:
    enum class Align { Left, Center, Right };
    enum class VAlign { Top, Middle, Bottom };

    TextRenderer();
    ~TextRenderer();

    bool LoadFont(const std::string& filepath, float pixelHeight);
    bool IsLoaded() const;

    void RenderString(Renderer& renderer, const std::string& text,
                      float x, float y, float scale = 1.0f,
                      const float color[4] = nullptr,
                      Align align = Align::Left);

    void RenderStringInRect(Renderer& renderer, const std::string& text,
                            float rectLeft, float rectTop, float rectW, float rectH,
                            float scale = 1.0f,
                            const float color[4] = nullptr,
                            Align hAlign = Align::Center,
                            VAlign vAlign = VAlign::Middle);

    Vec2 MeasureString(const std::string& text, float scale = 1.0f) const;

    // 按 maxWidth 自动折行，返回折行后的行数组（不含换行符）；
    // 算法基于字形 xadvance 累计（与渲染宽度一致），空格优先断词、无空格逐字断，
    // '\n' 强制断行，单字符超宽时单独成行不溢出
    std::vector<std::string> WrapString(const std::string& text,
                                        float maxWidth, float scale = 1.0f) const;

    float GetGlyphBearingX(unsigned char c, float scale = 1.0f) const;

    float GetLineHeight(float scale = 1.0f) const;
    float GetBaselineOffset(float scale = 1.0f) const;

private:
    struct GlyphData {
        float x0, y0, x1, y1;
        float s0, t0, s1, t1;
        float xadvance;
        float xoff, yoff;
        // -1 表示字形在静态 atlas（m_atlas）中；>= 0 为动态 atlas 索引
        int atlasIndex = -1;
    };

    static constexpr int kFirstChar = 32;
    static constexpr int kCharCount = 96;
    static constexpr int kDynAtlasWidth = 2048;
    static constexpr int kDynAtlasHeight = 1024;

    // 非 ASCII 字形按需生成并缓存（首次遇到某字符时烘焙进动态 atlas）
    const GlyphData* GetGlyph(uint32_t codepoint) const;
    GlyphData BakeDynamicGlyph(uint32_t codepoint) const;
    void FlushDynAtlas() const;

    std::unique_ptr<Texture> m_atlas = nullptr;
    std::vector<GlyphData> m_glyphs;
    std::vector<float> m_rawBearings;
    int m_atlasWidth = 0;
    int m_atlasHeight = 0;
    float m_pixelHeight = 0.0f;
    float m_ascent = 0.0f;
    float m_descent = 0.0f;
    float m_lineGap = 0.0f;
    bool m_loaded = false;

    // 动态字形缓存（mutable：MeasureString/RenderString 按需生成字形是惰性缓存副作用）
    mutable std::vector<unsigned char> m_fontBuffer;
    mutable std::unique_ptr<StbttFontInfo> m_fontInfo;
    mutable std::vector<std::unique_ptr<Texture>> m_dynAtlases;
    mutable std::unordered_map<uint32_t, GlyphData> m_dynGlyphs;
    mutable std::vector<unsigned char> m_dynBitmap;
    mutable int m_dynCursorX = 0;
    mutable int m_dynCursorY = 0;
    mutable int m_dynRowHeight = 0;
};
