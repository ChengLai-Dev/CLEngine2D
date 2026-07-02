#pragma once

#include "SceneGraph/Sprite.h"

class Scale9Sprite : public Sprite {
public:
    Scale9Sprite();
    virtual ~Scale9Sprite();

    void SetCapInsets(float left, float top, float right, float bottom);
    const Vec4& GetCapInsets() const;

protected:
    void OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) override;

private:
    Vec4 m_capInsets = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
};
