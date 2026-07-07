#include "SceneGraph/DragonBones/CLEngine2DFactory.h"
#include "SceneGraph/DragonBones/CLEngine2DSlot.h"
#include "SceneGraph/DragonBones/CLEngine2DTextureAtlasData.h"
#include "Render/Texture.h"

CLEngine2DFactory::CLEngine2DFactory()
    : BaseFactory() {
}

CLEngine2DFactory::~CLEngine2DFactory() {
    clear();
}

TextureAtlasData* CLEngine2DFactory::_buildTextureAtlasData(
    TextureAtlasData* texAtlasData, void* textureAtlas) const {

    if (texAtlasData) {
        auto* data = static_cast<CLEngine2DTextureAtlasData*>(texAtlasData);
        data->SetRenderTexture(static_cast<Texture*>(textureAtlas));
        return data;
    }

    return BaseObject::borrowObject<CLEngine2DTextureAtlasData>();
}

Armature* CLEngine2DFactory::_buildArmature(
    const BuildArmaturePackage& dataPack) const {

    return BaseObject::borrowObject<Armature>();
}

Slot* CLEngine2DFactory::_buildSlot(
    const BuildArmaturePackage& dataPack,
    const SlotData* slotData, Armature* armature) const {

    return BaseObject::borrowObject<CLEngine2DSlot>();
}
