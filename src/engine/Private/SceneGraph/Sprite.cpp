#include "SceneGraph/Sprite.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"

Sprite::Sprite() = default;

void Sprite::SetTexture(std::shared_ptr<Texture> texture) {
    m_texture = std::move(texture);
}

std::shared_ptr<Texture> Sprite::GetTexture() const {
    return m_texture;
}

void Sprite::SetTexOffset(float x, float y) {
    m_texOffsetX = x;
    m_texOffsetY = y;
}

void Sprite::SetTexScale(float x, float y) {
    m_texScaleX = x;
    m_texScaleY = y;
}

void Sprite::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
    Color finalColor(m_color.x, m_color.y, m_color.z, m_color.w * worldOpacity);

    renderer.DrawQuad(worldTransform, m_contentSize,
                      finalColor, m_texture.get(),
                      m_texOffsetX, m_texOffsetY,
                      m_texScaleX, m_texScaleY);
}
