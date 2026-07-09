#include "SceneGraph/Scale9Sprite.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"

Scale9Sprite::Scale9Sprite() = default;

Scale9Sprite::~Scale9Sprite() = default;

void Scale9Sprite::SetCapInsets(float left, float top, float right, float bottom) {
    m_capInsets = Vec4(left, top, right, bottom);
}

const Vec4& Scale9Sprite::GetCapInsets() const {
    return m_capInsets;
}

void Scale9Sprite::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
    if (!m_texture) {
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

    float dstW = m_contentSize.x;
    float dstH = m_contentSize.y;

    float centerSrcW = texW - left - right;
    float centerSrcH = texH - top - bottom;
    float centerDstW = dstW - left - right;
    float centerDstH = dstH - top - bottom;

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
            float dw = dstWidths[col];
            float dh = dstHeights[row];
            if (dw <= 0.0f || dh <= 0.0f) continue;

            float ox = dstCols[col] + dw * 0.5f;
            float oy = dstRows[row] + dh * 0.5f;

            float u = srcCols[col] / texW;
            float v = srcRows[row] / texH;
            float uw = srcWidths[col] / texW;
            float vh = srcHeights[row] / texH;

            Mat4 sliceTransform = Mat4::Translate(Vec3(ox, oy, 0.0f));
            Mat4 worldSlice = worldTransform * sliceTransform;

            renderer.DrawQuad(worldSlice, Vec2(dw, dh),
                              finalColor, m_texture.get(),
                              u, v, uw, vh);
        }
    }
}
