#pragma once

#include "SceneGraph/Node.h"
#include <memory>

class Texture;

class Sprite : public Node {
public:
    Sprite();

    void SetTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture() const;

    void SetTexOffset(float x, float y);
    void SetTexScale(float x, float y);
    float GetTexOffsetX() const { return m_texOffsetX; }
    float GetTexOffsetY() const { return m_texOffsetY; }
    float GetTexScaleX() const { return m_texScaleX; }
    float GetTexScaleY() const { return m_texScaleY; }

protected:
    void OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) override;

    std::shared_ptr<Texture> m_texture = nullptr;
    float m_texOffsetX = 0.0f;
    float m_texOffsetY = 0.0f;
    float m_texScaleX = 1.0f;
    float m_texScaleY = 1.0f;
};
