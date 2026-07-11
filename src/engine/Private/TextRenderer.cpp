#include "TextRenderer.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"
#include "Logger.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>

TextRenderer::TextRenderer()
    : m_glyphs(96)
{
}

TextRenderer::~TextRenderer() = default;

bool TextRenderer::LoadFont(const std::string& filepath, float pixelHeight) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        Logger::Error("TextRenderer: failed to open font file '{}'", filepath);
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> fontBuffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(fontBuffer.data()), size)) {
        Logger::Error("TextRenderer: failed to read font file '{}'", filepath);
        return false;
    }

    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, fontBuffer.data(), 0)) {
        Logger::Error("TextRenderer: failed to init font '{}'", filepath);
        return false;
    }

    m_pixelHeight = pixelHeight;
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&fontInfo, pixelHeight);
    m_ascent = ascent * scale;
    m_descent = descent * scale;
    m_lineGap = lineGap * scale;

    int atlasW = 512;
    int atlasH = 512;
    std::vector<unsigned char> bitmap(static_cast<size_t>(atlasW) * atlasH, 0);

    stbtt_bakedchar bakedChars[96];
    int result = stbtt_BakeFontBitmap(fontBuffer.data(), 0, pixelHeight,
                                      bitmap.data(), atlasW, atlasH,
                                      kFirstChar, kCharCount, bakedChars);

    if (result < 0) {
        atlasW = 1024;
        atlasH = 1024;
        bitmap.resize(static_cast<size_t>(atlasW) * atlasH, 0);
        result = stbtt_BakeFontBitmap(fontBuffer.data(), 0, pixelHeight,
                                      bitmap.data(), atlasW, atlasH,
                                      kFirstChar, kCharCount, bakedChars);
    }

    if (result <= 0) {
        Logger::Error("TextRenderer: failed to bake font '{}' (result={})", filepath, result);
        return false;
    }

    m_atlasWidth = atlasW;
    m_atlasHeight = atlasH;

    std::vector<unsigned char> rgbaBitmap(static_cast<size_t>(atlasW) * atlasH * 4);
    for (int i = 0; i < atlasW * atlasH; i++) {
        rgbaBitmap[i * 4 + 0] = 255;
        rgbaBitmap[i * 4 + 1] = 255;
        rgbaBitmap[i * 4 + 2] = 255;
        rgbaBitmap[i * 4 + 3] = bitmap[i];
    }

    m_atlas = std::unique_ptr<Texture>(new Texture(
        static_cast<unsigned int>(atlasW),
        static_cast<unsigned int>(atlasH),
        rgbaBitmap.data()
    ));

    for (int i = 0; i < kCharCount; ++i) {
        stbtt_bakedchar& src = bakedChars[i];
        GlyphData& dst = m_glyphs[i];
        dst.x0 = src.x0;
        dst.y0 = src.y0;
        dst.x1 = src.x1;
        dst.y1 = src.y1;
        dst.xoff = src.xoff;
        dst.yoff = src.yoff;
        dst.xadvance = src.xadvance;
    }

    for (int i = 0; i < kCharCount; ++i) {
        GlyphData& dst = m_glyphs[i];

        dst.s0 = static_cast<float>(dst.x0) / static_cast<float>(atlasW);
        dst.t0 = static_cast<float>(dst.y1) / static_cast<float>(atlasH);
        dst.s1 = static_cast<float>(dst.x1) / static_cast<float>(atlasW);
        dst.t1 = static_cast<float>(dst.y0) / static_cast<float>(atlasH);
    }

    m_loaded = true;
    Logger::Info("TextRenderer: loaded font '{}' ({}px, atlas {}x{})",
                             filepath, static_cast<int>(pixelHeight), atlasW, atlasH);
    return true;
}

bool TextRenderer::IsLoaded() const {
    return m_loaded;
}

void TextRenderer::RenderString(Renderer& renderer, const std::string& text,
                                float x, float y, float scale,
                                const float color[4],
                                Align align)
{
    if (!m_loaded || !m_atlas) return;

    Color finalColor;
    if (color) {
        finalColor = Color(color[0], color[1], color[2], color[3]);
    }

    if (align != Align::Left) {
        float totalWidth = 0.0f;
        for (size_t i = 0; i < text.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(text[i]);
            if (c >= kFirstChar && c < kFirstChar + kCharCount) {
                int idx = c - kFirstChar;
                totalWidth += m_glyphs[idx].xadvance * scale;
            }
        }
        if (align == Align::Center) {
            x -= totalWidth * 0.5f;
        } else {
            x -= totalWidth;
        }
    }

    float cursorX = x;
    float cursorY = y;

    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c == '\n') {
            cursorX = x;
            cursorY += GetLineHeight(scale);
            continue;
        }

        if (c < static_cast<unsigned char>(kFirstChar) ||
            c >= static_cast<unsigned char>(kFirstChar + kCharCount)) {
            cursorX += m_glyphs[0].xadvance * scale;
            continue;
        }

        int idx = c - kFirstChar;
        const GlyphData& g = m_glyphs[idx];

        float charW = (g.x1 - g.x0) * scale;
        float charH = (g.y1 - g.y0) * scale;
        float charX = cursorX + g.xoff * scale;
        float charY = cursorY + g.yoff * scale;

        if (charW > 0.0f && charH > 0.0f) {
            Mat4 transform =
                Mat4::Translate(Vec3(charX + charW * 0.5f, charY + charH * 0.5f, 0.0f)) *
                Mat4::Scale(Vec3(charW, charH, 1.0f));

            float texScaleX = (g.s1 - g.s0);
            float texScaleY = (g.t1 - g.t0);

            renderer.DrawQuad(transform, Vec2(1.0f, 1.0f),
                              finalColor, m_atlas.get(),
                              g.s0, g.t0,
                              texScaleX, texScaleY);
        }

        cursorX += g.xadvance * scale;
    }
}

void TextRenderer::RenderStringInRect(Renderer& renderer, const std::string& text,
                                      float rectLeft, float rectTop, float rectW, float rectH,
                                      float scale, const float color[4],
                                      Align hAlign, VAlign vAlign)
{
    Vec2 textSize = MeasureString(text, scale);
    float ascent = GetBaselineOffset(scale);

    float x = rectLeft;
    if (hAlign == Align::Center) {
        x = rectLeft + (rectW - textSize.x) * 0.5f;
    } else if (hAlign == Align::Right) {
        x = rectLeft + rectW - textSize.x;
    }

    float y = rectTop;
    if (vAlign == VAlign::Middle) {
        y = rectTop + rectH * 0.5f + ascent - textSize.y * 0.5f;
    } else if (vAlign == VAlign::Bottom) {
        y = rectTop + rectH - (textSize.y - ascent);
    } else {
        y = rectTop + ascent;
    }

    RenderString(renderer, text, x, y, scale, color, Align::Left);
}

Vec2 TextRenderer::MeasureString(const std::string& text, float scale) const {
    if (!m_loaded) return Vec2(0.0f, 0.0f);

    float maxWidth = 0.0f;
    float currentWidth = 0.0f;
    float lineHeight = GetLineHeight(scale);
    float totalHeight = lineHeight;

    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c == '\n') {
            if (currentWidth > maxWidth) maxWidth = currentWidth;
            currentWidth = 0.0f;
            totalHeight += lineHeight;
            continue;
        }

        if (c >= kFirstChar && c < kFirstChar + kCharCount) {
            int idx = c - kFirstChar;
            currentWidth += m_glyphs[idx].xadvance * scale;
        }
    }
    if (currentWidth > maxWidth) maxWidth = currentWidth;

    return Vec2(maxWidth, totalHeight);
}

float TextRenderer::GetLineHeight(float scale) const {
    if (!m_loaded) return 0.0f;
    return (m_ascent - m_descent + m_lineGap) * scale;
}

float TextRenderer::GetBaselineOffset(float scale) const {
    if (!m_loaded) return 0.0f;
    return m_ascent * scale;
}
