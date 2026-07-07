#pragma once

#include "dragonBones/DragonBonesHeaders.h"

DRAGONBONES_USING_NAME_SPACE;

class CLEngine2DFactory : public BaseFactory {
public:
    CLEngine2DFactory();
    ~CLEngine2DFactory() override;

protected:
    TextureAtlasData* _buildTextureAtlasData(
        TextureAtlasData* texAtlasData, void* textureAtlas) const override;

    Armature* _buildArmature(
        const BuildArmaturePackage& dataPack) const override;

    Slot* _buildSlot(
        const BuildArmaturePackage& dataPack,
        const SlotData* slotData, Armature* armature) const override;
};
