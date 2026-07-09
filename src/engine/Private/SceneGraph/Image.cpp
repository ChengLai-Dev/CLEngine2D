#include "SceneGraph/Image.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"

Image::Image() = default;

Image::~Image() = default;

void Image::SetScale9Enabled(bool enabled) {
    m_scale9Enabled = enabled;
}

bool Image::IsScale9Enabled() const {
    return m_scale9Enabled;
}

void Image::SetCapInsets(const Vec4& insets) {
    m_capInsets = insets;
}

const Vec4& Image::GetCapInsets() const {
    return m_capInsets;
}

void Image::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
    if (!m_scale9Enabled || !m_texture) {
        Sprite::OnDraw(renderer, worldTransform, worldOpacity);
        return;
    }

    Color finalColor(m_color.x, m_color.y, m_color.z, m_color.w * worldOpacity);

    float texW = static_cast<float>(m_texture->GetWidth());
    float texH = static_cast<float>(m_texture->GetHeight());

    float left = m_capInsets.x;
    float top = m_capInsets.y;
    float right = m_capInsets.z;
    float bottom = m_capInsets.w;

    float centerSrcW = texW - left - right;
    float centerSrcH = texH - top - bottom;

    float dstW = m_contentSize.x;
    float dstH = m_contentSize.y;

    float centerDstW = dstW - left - right;
    float centerDstH = dstH - top - bottom;

    struct Slice {
        float u, v, w, h;
        float ox, oy, dw, dh;
    };

    Slice slices[9];
    float srcRows[3] = { 0.0f, top, centerSrcH };
    float srcCols[3] = { 0.0f, left, centerSrcW };
    float dstRows[3] = { -dstH * 0.5f, -dstH * 0.5f + top, centerDstH };
    float dstCols[3] = { -dstW * 0.5f, -dstW * 0.5f + left, centerDstW };

    float srcWidths[3] = { left, centerSrcW, right };
    float srcHeights[3] = { top, centerSrcH, bottom };
    float dstWidths[3] = { left, centerDstW, right };
    float dstHeights[3] = { top, centerDstH, bottom };

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            Slice& s = slices[row * 3 + col];
            s.u = srcCols[col] / texW;
            s.v = srcRows[row] / texH;
            s.w = srcWidths[col] / texW;
            s.h = srcHeights[row] / texH;
            s.ox = dstCols[col] + dstWidths[col] * 0.5f;
            s.oy = dstRows[row] + dstHeights[row] * 0.5f;
            s.dw = dstWidths[col];
            s.dh = dstHeights[row];
        }
    }

    for (int i = 0; i < 9; ++i) {
        const Slice& s = slices[i];
        if (s.dw <= 0.0f || s.dh <= 0.0f) continue;

        Mat4 sliceTransform = Mat4::Translate(Vec3(s.ox, s.oy, 0.0f));
        Mat4 worldSlice = worldTransform * sliceTransform;

        renderer.DrawQuad(worldSlice, Vec2(s.dw, s.dh),
                          finalColor, m_texture.get(),
                          s.u, s.v, s.w, s.h);
    }
}
