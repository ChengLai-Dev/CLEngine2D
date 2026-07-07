#include "SceneGraph/DragonBones/CLEngine2DTextureAtlasData.h"

TextureData* CLEngine2DTextureAtlasData::createTexture() const {
    return BaseObject::borrowObject<CLEngine2DTextureData>();
}

void CLEngine2DTextureAtlasData::_onClear() {
    TextureAtlasData::_onClear();
    m_renderTexture = nullptr;
}

void CLEngine2DTextureData::_onClear() {
    TextureData::_onClear();
}
