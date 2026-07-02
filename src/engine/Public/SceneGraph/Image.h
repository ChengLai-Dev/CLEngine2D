#pragma once

#include "SceneGraph/Widget.h"
#include "SceneGraph/Sprite.h"

class Image : public Sprite {
public:
    Image();
    virtual ~Image();

    void SetScale9Enabled(bool enabled);
    bool IsScale9Enabled() const;

    void SetCapInsets(const Vec4& insets);
    const Vec4& GetCapInsets() const;

protected:
    void OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) override;

private:
    bool m_scale9Enabled = false;
    Vec4 m_capInsets = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
};
