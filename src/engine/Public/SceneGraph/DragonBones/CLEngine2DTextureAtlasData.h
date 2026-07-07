#pragma once

#include "dragonBones/DragonBonesHeaders.h"

DRAGONBONES_USING_NAME_SPACE;

class Texture;

class CLEngine2DTextureAtlasData : public TextureAtlasData {
    BIND_CLASS_TYPE_A(CLEngine2DTextureAtlasData);

public:
    Texture* GetRenderTexture() const { return m_renderTexture; }
    void SetRenderTexture(Texture* tex) { m_renderTexture = tex; }

protected:
    TextureData* createTexture() const override;
    void _onClear() override;

private:
    Texture* m_renderTexture = nullptr;
};

class CLEngine2DTextureData : public TextureData {
    BIND_CLASS_TYPE_A(CLEngine2DTextureData);

protected:
    void _onClear() override;
};
