#pragma once

#include "dragonBones/DragonBonesHeaders.h"
#include "internal.h"

DRAGONBONES_USING_NAME_SPACE;

class CLEngine2DSlot : public Slot {
    BIND_CLASS_TYPE_A(CLEngine2DSlot);

public:

protected:
    void _onClear() override;

    void _initDisplay(void* value, bool isRetain) override;
    void _disposeDisplay(void* value, bool isRelease) override;
    void _onUpdateDisplay() override;
    void _addDisplay() override;
    void _replaceDisplay(void* value, bool isArmatureDisplay) override;
    void _removeDisplay() override;
    void _updateZOrder() override;
    void _updateFrame() override;
    void _updateMesh() override;
    void _updateTransform() override;
    void _identityTransform() override;
    void _updateVisible() override;
    void _updateBlendMode() override;
    void _updateColor() override;
};
