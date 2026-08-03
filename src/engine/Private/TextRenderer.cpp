#include "TextRenderer.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"
#include "Logger.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <vector>

struct StbttFontInfo {
    stbtt_fontinfo info;
};

// 字形 alpha 对比度增强：提升中间调覆盖率，让小字号文字更实、更醒目
static constexpr float kGlyphAlphaBoost = 0.6f;

static unsigned char EnhanceGlyphAlpha(unsigned char alpha) {
    if (alpha == 0 || alpha == 255) return alpha;
    float boosted = std::pow(static_cast<float>(alpha) / 255.0f, kGlyphAlphaBoost);
    return static_cast<unsigned char>(boosted * 255.0f + 0.5f);
}

static uint32_t DecodeUTF8(const char* text, size_t len, size_t& i) {
    unsigned char c0 = static_cast<unsigned char>(text[i]);
    if (c0 < 0x80) {
        ++i;
        return c0;
    }
    if ((c0 & 0xE0) == 0xC0 && i + 1 < len) {
        uint32_t cp = ((c0 & 0x1Fu) << 6) |
                      (static_cast<unsigned char>(text[i + 1]) & 0x3Fu);
        i += 2;
        return cp;
    }
    if ((c0 & 0xF0) == 0xE0 && i + 2 < len) {
        uint32_t cp = ((c0 & 0x0Fu) << 12) |
                      ((static_cast<unsigned char>(text[i + 1]) & 0x3Fu) << 6) |
                      (static_cast<unsigned char>(text[i + 2]) & 0x3Fu);
        i += 3;
        return cp;
    }
    if ((c0 & 0xF8) == 0xF0 && i + 3 < len) {
        uint32_t cp = ((c0 & 0x07u) << 18) |
                      ((static_cast<unsigned char>(text[i + 1]) & 0x3Fu) << 12) |
                      ((static_cast<unsigned char>(text[i + 2]) & 0x3Fu) << 6) |
                      (static_cast<unsigned char>(text[i + 3]) & 0x3Fu);
        i += 4;
        return cp;
    }
    ++i;
    return 0;
}

TextRenderer::TextRenderer()
    : m_glyphs(96), m_rawBearings(96, 0.0f)
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

    m_fontBuffer = std::move(fontBuffer);
    m_fontInfo = std::make_unique<StbttFontInfo>();
    if (!stbtt_InitFont(&m_fontInfo->info, m_fontBuffer.data(), 0)) {
        Logger::Error("TextRenderer: failed to init font '{}'", filepath);
        return false;
    }

    m_pixelHeight = pixelHeight;
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo->info, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&m_fontInfo->info, pixelHeight);
    m_ascent = ascent * scale;
    m_descent = descent * scale;
    m_lineGap = lineGap * scale;

    for (int i = 0; i < kCharCount; ++i) {
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&m_fontInfo->info, kFirstChar + i, &advanceWidth, &leftSideBearing);
        m_rawBearings[static_cast<size_t>(i)] = static_cast<float>(leftSideBearing) * scale;
    }

    int atlasW = 512;
    int atlasH = 512;
    std::vector<unsigned char> bitmap(static_cast<size_t>(atlasW) * atlasH, 0);

    stbtt_bakedchar bakedChars[96];
    int result = stbtt_BakeFontBitmap(m_fontBuffer.data(), 0, pixelHeight,
                                      bitmap.data(), atlasW, atlasH,
                                      kFirstChar, kCharCount, bakedChars);

    if (result < 0) {
        atlasW = 1024;
        atlasH = 1024;
        bitmap.resize(static_cast<size_t>(atlasW) * atlasH, 0);
        result = stbtt_BakeFontBitmap(m_fontBuffer.data(), 0, pixelHeight,
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
        rgbaBitmap[i * 4 + 3] = EnhanceGlyphAlpha(bitmap[i]);
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
        float totalWidth = MeasureString(text, scale).x;
        if (align == Align::Center) {
            x -= totalWidth * 0.5f;
        } else {
            x -= totalWidth;
        }
    }

    float cursorX = x;
    float cursorY = y;
    size_t i = 0;
    size_t len = text.size();

    while (i < len) {
        unsigned char c0 = static_cast<unsigned char>(text[i]);
        if (c0 == '\n') {
            cursorX = x;
            cursorY += GetLineHeight(scale);
            ++i;
            continue;
        }

        uint32_t codepoint = DecodeUTF8(text.c_str(), len, i);
        if (codepoint == 0) continue;

        const GlyphData* glyph = GetGlyph(codepoint);
        if (!glyph) {
            cursorX += m_glyphs[0].xadvance * scale;
            continue;
        }

        const GlyphData& g = *glyph;

        float charW = (g.x1 - g.x0) * scale;
        float charH = (g.y1 - g.y0) * scale;
        float charX = cursorX + g.xoff * scale;
        float charY = cursorY + g.yoff * scale;

        if (charW > 0.0f && charH > 0.0f) {
            if (g.atlasIndex >= 0 &&
                !m_dynBitmap.empty() &&
                static_cast<size_t>(g.atlasIndex) >= m_dynAtlases.size()) {
                FlushDynAtlas();
            }
            Texture* atlasTex = m_atlas.get();
            if (g.atlasIndex >= 0 &&
                static_cast<size_t>(g.atlasIndex) < m_dynAtlases.size()) {
                atlasTex = m_dynAtlases[static_cast<size_t>(g.atlasIndex)].get();
            }

            Vec3 charCenter(charX + charW * 0.5f, charY + charH * 0.5f, 0.0f);

            float texScaleX = (g.s1 - g.s0);
            float texScaleY = (g.t1 - g.t0);

            renderer.DrawQuad(charCenter, Vec3(charW, charH, 1.0f),
                              finalColor, 0.0f, atlasTex,
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

    size_t i = 0;
    size_t len = text.size();
    while (i < len) {
        unsigned char c0 = static_cast<unsigned char>(text[i]);
        if (c0 == '\n') {
            if (currentWidth > maxWidth) maxWidth = currentWidth;
            currentWidth = 0.0f;
            totalHeight += lineHeight;
            ++i;
            continue;
        }

        uint32_t codepoint = DecodeUTF8(text.c_str(), len, i);
        if (codepoint == 0) continue;

        const GlyphData* glyph = GetGlyph(codepoint);
        if (glyph) {
            currentWidth += glyph->xadvance * scale;
        } else {
            currentWidth += m_glyphs[0].xadvance * scale;
        }
    }
    if (currentWidth > maxWidth) maxWidth = currentWidth;

    return Vec2(maxWidth, totalHeight);
}

std::vector<std::string> TextRenderer::WrapString(const std::string& text,
                                                  float maxWidth, float scale) const
{
    std::vector<std::string> lines;
    if (!m_loaded || maxWidth <= 0.0f) {
        lines.push_back(text);
        return lines;
    }

    std::string currentLine;
    float currentWidth = 0.0f;
    // 最近空格断点的信息：lastBreakCharLen = 空格后位置（断点后已 append 的字符数），
    // lastBreakWidth = 断点处（含空格）累计行宽
    size_t lastBreakCharLen = 0;
    float lastBreakWidth = 0.0f;

    auto resetLine = [&]() {
        currentLine.clear();
        currentWidth = 0.0f;
        lastBreakCharLen = 0;
        lastBreakWidth = 0.0f;
    };

    size_t i = 0;
    size_t len = text.size();
    while (i < len) {
        unsigned char c0 = static_cast<unsigned char>(text[i]);
        if (c0 == '\n') {
            lines.push_back(currentLine);
            resetLine();
            ++i;
            continue;
        }

        size_t charStart = i;
        uint32_t codepoint = DecodeUTF8(text.c_str(), len, i);
        if (codepoint == 0) continue;

        float charWidth = m_glyphs[0].xadvance * scale;
        const GlyphData* glyph = GetGlyph(codepoint);
        if (glyph) {
            charWidth = glyph->xadvance * scale;
        }

        // 放不下时循环断行：优先回退最近空格断词，无断点则逐字断
        while (!currentLine.empty() && currentWidth + charWidth > maxWidth) {
            if (lastBreakCharLen > 0 && lastBreakCharLen < currentLine.size()) {
                lines.push_back(currentLine.substr(0, lastBreakCharLen - 1));
                currentLine.erase(0, lastBreakCharLen);
                currentWidth -= lastBreakWidth;
                lastBreakCharLen = 0;
                lastBreakWidth = 0.0f;
            } else {
                lines.push_back(currentLine);
                resetLine();
            }
        }

        if (currentLine.empty()) {
            if (codepoint == ' ') continue;  // 行首空格忽略
            currentLine.assign(text, charStart, i - charStart);
            currentWidth = charWidth;
            continue;
        }

        currentLine.append(text, charStart, i - charStart);
        currentWidth += charWidth;
        if (codepoint == ' ') {
            lastBreakCharLen = currentLine.size();
            lastBreakWidth = currentWidth;
        }
    }
    lines.push_back(currentLine);

    return lines;
}

const TextRenderer::GlyphData* TextRenderer::GetGlyph(uint32_t codepoint) const {
    if (codepoint >= static_cast<uint32_t>(kFirstChar) &&
        codepoint < static_cast<uint32_t>(kFirstChar) + kCharCount) {
        return &m_glyphs[codepoint - static_cast<uint32_t>(kFirstChar)];
    }

    auto it = m_dynGlyphs.find(codepoint);
    if (it != m_dynGlyphs.end()) {
        return &it->second;
    }

    auto result = m_dynGlyphs.emplace(codepoint, BakeDynamicGlyph(codepoint));
    return &result.first->second;
}

TextRenderer::GlyphData TextRenderer::BakeDynamicGlyph(uint32_t codepoint) const {
    GlyphData g;
    if (!m_loaded || !m_fontInfo) return g;

    float scale = stbtt_ScaleForPixelHeight(&m_fontInfo->info, m_pixelHeight);

    int advanceWidth = 0, leftSideBearing = 0;
    stbtt_GetCodepointHMetrics(&m_fontInfo->info, static_cast<int>(codepoint),
                               &advanceWidth, &leftSideBearing);

    int w = 0, h = 0, xoff = 0, yoff = 0;
    unsigned char* bitmap = stbtt_GetCodepointBitmap(
        &m_fontInfo->info, scale, scale, static_cast<int>(codepoint),
        &w, &h, &xoff, &yoff);
    if (!bitmap) {
        g.xadvance = static_cast<float>(advanceWidth) * scale;
        return g;
    }

    if (m_dynBitmap.empty()) {
        m_dynBitmap.assign(static_cast<size_t>(kDynAtlasWidth) * kDynAtlasHeight, 0);
        m_dynCursorX = 0;
        m_dynCursorY = 0;
        m_dynRowHeight = 0;
    }

    if (m_dynCursorX + w > kDynAtlasWidth) {
        m_dynCursorX = 0;
        m_dynCursorY += m_dynRowHeight + 1;
        m_dynRowHeight = 0;
    }
    if (m_dynCursorY + h > kDynAtlasHeight) {
        FlushDynAtlas();
        m_dynBitmap.assign(static_cast<size_t>(kDynAtlasWidth) * kDynAtlasHeight, 0);
        m_dynCursorX = 0;
        m_dynCursorY = 0;
        m_dynRowHeight = 0;
    }

    for (int yy = 0; yy < h; ++yy) {
        for (int xx = 0; xx < w; ++xx) {
            m_dynBitmap[static_cast<size_t>(m_dynCursorY + yy) * kDynAtlasWidth + (m_dynCursorX + xx)] =
                bitmap[static_cast<size_t>(yy) * w + xx];
        }
    }

    g.x0 = static_cast<float>(m_dynCursorX);
    g.y0 = static_cast<float>(m_dynCursorY);
    g.x1 = static_cast<float>(m_dynCursorX + w);
    g.y1 = static_cast<float>(m_dynCursorY + h);
    g.xoff = static_cast<float>(xoff);
    g.yoff = static_cast<float>(yoff);
    g.xadvance = static_cast<float>(advanceWidth) * scale;
    g.atlasIndex = static_cast<int>(m_dynAtlases.size());

    g.s0 = g.x0 / static_cast<float>(kDynAtlasWidth);
    g.t0 = g.y1 / static_cast<float>(kDynAtlasHeight);
    g.s1 = g.x1 / static_cast<float>(kDynAtlasWidth);
    g.t1 = g.y0 / static_cast<float>(kDynAtlasHeight);

    m_dynCursorX += w + 1;
    m_dynRowHeight = (std::max)(m_dynRowHeight, h);

    stbtt_FreeBitmap(bitmap, nullptr);
    return g;
}

void TextRenderer::FlushDynAtlas() const {
    if (m_dynBitmap.empty()) return;

    std::vector<unsigned char> rgba(static_cast<size_t>(kDynAtlasWidth) * kDynAtlasHeight * 4);
    for (int i = 0; i < kDynAtlasWidth * kDynAtlasHeight; ++i) {
        rgba[static_cast<size_t>(i) * 4 + 0] = 255;
        rgba[static_cast<size_t>(i) * 4 + 1] = 255;
        rgba[static_cast<size_t>(i) * 4 + 2] = 255;
        rgba[static_cast<size_t>(i) * 4 + 3] = EnhanceGlyphAlpha(m_dynBitmap[static_cast<size_t>(i)]);
    }

    m_dynAtlases.push_back(std::unique_ptr<Texture>(new Texture(
        static_cast<unsigned int>(kDynAtlasWidth),
        static_cast<unsigned int>(kDynAtlasHeight),
        rgba.data()
    )));
    m_dynBitmap.clear();
}

float TextRenderer::GetGlyphBearingX(unsigned char c, float scale) const {
    if (!m_loaded) return 0.0f;
    if (c < kFirstChar || c >= kFirstChar + kCharCount) return 0.0f;
    int idx = c - kFirstChar;
    return m_rawBearings[idx] * scale;
}

float TextRenderer::GetLineHeight(float scale) const {
    if (!m_loaded) return 0.0f;
    return (m_ascent - m_descent + m_lineGap) * scale;
}

float TextRenderer::GetBaselineOffset(float scale) const {
    if (!m_loaded) return 0.0f;
    return m_ascent * scale;
}
