#include "SceneGraph/DragonBones/CLEngine2DSlot.h"
#include "SceneGraph/DragonBones/CLEngine2DTextureAtlasData.h"

#include "dragonBones/model/DisplayData.h"
#include "dragonBones/model/TextureAtlasData.h"
#include "dragonBones/armature/Bone.h"
#include "dragonBones/armature/Armature.h"
#include "dragonBones/armature/DeformVertices.h"
#include "dragonBones/core/DragonBones.h"

#include "Render/Texture.h"

void CLEngine2DSlot::_onClear() {
    auto* display = static_cast<DBSlotVertexData*>(_display);

    Slot::_onClear();

    if (display) {
        delete display;
    }
}

void CLEngine2DSlot::_initDisplay(void* value, bool isRetain) {
}

void CLEngine2DSlot::_disposeDisplay(void* value, bool isRelease) {
    auto* data = static_cast<DBSlotVertexData*>(value);
    if (data) {
        delete data;
    }
}

void CLEngine2DSlot::_onUpdateDisplay() {
    _meshDisplay = _display;
}

void CLEngine2DSlot::_addDisplay() {
}

void CLEngine2DSlot::_replaceDisplay(void* value, bool isArmatureDisplay) {
    _display = value;
}

void CLEngine2DSlot::_removeDisplay() {
    _display = _rawDisplay;
}

void CLEngine2DSlot::_updateZOrder() {
}

void CLEngine2DSlot::_updateFrame() {
    DBSlotVertexData* display;
    if (!_display) {
        display = new DBSlotVertexData();
        _display = display;
    } else {
        display = static_cast<DBSlotVertexData*>(_display);
    }

    _meshDisplay = display;

    if (!_textureData) {
        display->vertices.clear();
        display->indices.clear();
        display->texture = nullptr;
        return;
    }

    auto* atlasData = static_cast<CLEngine2DTextureAtlasData*>(_textureData->parent);
    display->texture = atlasData->GetRenderTexture();

    auto* vd = _deformVertices ? _deformVertices->verticesData : nullptr;
    if (!vd || !vd->data) {
        float texW = static_cast<float>(display->texture->GetWidth());
        float texH = static_cast<float>(display->texture->GetHeight());
        auto& region = _textureData->region;

        display->vertices.resize(4);
        display->vertices[0] = { -region.width / 2, -region.height / 2, 0, region.x / texW, (region.y + region.height) / texH, 0, 1,1,1,1 };
        display->vertices[1] = {  region.width / 2, -region.height / 2, 0, (region.x + region.width) / texW, (region.y + region.height) / texH, 0, 1,1,1,1 };
        display->vertices[2] = {  region.width / 2,  region.height / 2, 0, (region.x + region.width) / texW, region.y / texH, 0, 1,1,1,1 };
        display->vertices[3] = { -region.width / 2,  region.height / 2, 0, region.x / texW, region.y / texH, 0, 1,1,1,1 };

        display->indices = { 0, 1, 2, 2, 3, 0 };
        return;
    }

    auto* intArr = vd->data->intArray;
    auto* floatArr = vd->data->floatArray;
    unsigned vc = static_cast<unsigned>(intArr[vd->offset + static_cast<int>(BinaryOffset::MeshVertexCount)]);
    unsigned tc = static_cast<unsigned>(intArr[vd->offset + static_cast<int>(BinaryOffset::MeshTriangleCount)]);
    int voff = intArr[vd->offset + static_cast<int>(BinaryOffset::MeshFloatOffset)];

    display->vertices.resize(vc);
    display->indices.resize(tc * 3);

    float texW = static_cast<float>(display->texture->GetWidth());
    float texH = static_cast<float>(display->texture->GetHeight());
    auto& region = _textureData->region;

    unsigned uvOff = static_cast<unsigned>(voff) + vc * 2;
    for (unsigned i = 0; i < vc; ++i) {
        display->vertices[i] = {
            floatArr[voff + i * 2], floatArr[voff + i * 2 + 1], 0,
            (region.x + floatArr[uvOff + i * 2] * region.width) / texW,
            (region.y + floatArr[uvOff + i * 2 + 1] * region.height) / texH,
            0,
            1, 1, 1, 1
        };
    }

    unsigned idxOff = static_cast<unsigned>(intArr[vd->offset + static_cast<int>(BinaryOffset::MeshVertexIndices)]);
    for (unsigned i = 0; i < tc * 3; ++i) {
        display->indices[i] = static_cast<unsigned>(intArr[idxOff + i]);
    }
}

void CLEngine2DSlot::_updateMesh() {
    auto* display = static_cast<DBSlotVertexData*>(_display);
    if (!display || display->vertices.empty()) return;

    auto* vd = _deformVertices ? _deformVertices->verticesData : nullptr;
    if (!vd || !vd->weight) {
        _updateFrame();
        return;
    }

    auto* weight = vd->weight;
    auto* intArr = vd->data->intArray;
    auto* floatArr = vd->data->floatArray;

    unsigned vc = static_cast<unsigned>(intArr[vd->offset + static_cast<int>(BinaryOffset::MeshVertexCount)]);
    int voff = intArr[vd->offset + static_cast<int>(BinaryOffset::MeshFloatOffset)];

    unsigned uvOff = static_cast<unsigned>(voff) + vc * 2;
    float texW = static_cast<float>(display->texture->GetWidth());
    float texH = static_cast<float>(display->texture->GetHeight());
    auto& region = _textureData->region;

    int boneOffset = static_cast<int>(weight->offset);
    const auto& bones = _deformVertices->bones;

    for (unsigned i = 0; i < vc; ++i) {
        float xG = 0, yG = 0;
        float xL = floatArr[voff + i * 2];
        float yL = floatArr[voff + i * 2 + 1];

        int weightCount = intArr[boneOffset + static_cast<int>(BinaryOffset::WeigthBoneCount) + i];
        int weightFloatOff = intArr[boneOffset + static_cast<int>(BinaryOffset::WeigthFloatOffset) + i];
        int weightIndexOff = intArr[boneOffset + static_cast<int>(BinaryOffset::WeigthBoneIndices) + i];

        for (int j = 0; j < weightCount; ++j) {
            int boneIndex = intArr[weightIndexOff + j];
            float boneWeight = floatArr[weightFloatOff + j];
            auto* bone = static_cast<Bone*>(bones[static_cast<unsigned>(boneIndex)]);
            auto& m = bone->globalTransformMatrix;

            xG += (m.a * xL + m.c * yL + m.tx) * boneWeight;
            yG += (m.b * xL + m.d * yL + m.ty) * boneWeight;
        }

        display->vertices[i].Position[0] = xG;
        display->vertices[i].Position[1] = yG;
        display->vertices[i].Position[2] = 0;

        display->vertices[i].TexCoord[0] = (region.x + floatArr[uvOff + i * 2] * region.width) / texW;
        display->vertices[i].TexCoord[1] = (region.y + floatArr[uvOff + i * 2 + 1] * region.height) / texH;
    }

    auto& slotColor = _colorTransform;
    for (unsigned i = 0; i < display->vertices.size(); ++i) {
        display->vertices[i].Color[0] = slotColor.redMultiplier;
        display->vertices[i].Color[1] = slotColor.greenMultiplier;
        display->vertices[i].Color[2] = slotColor.blueMultiplier;
        display->vertices[i].Color[3] = slotColor.alphaMultiplier;
    }
}

void CLEngine2DSlot::_updateTransform() {
}

void CLEngine2DSlot::_identityTransform() {
}

void CLEngine2DSlot::_updateVisible() {
}

void CLEngine2DSlot::_updateBlendMode() {
}

void CLEngine2DSlot::_updateColor() {
}
