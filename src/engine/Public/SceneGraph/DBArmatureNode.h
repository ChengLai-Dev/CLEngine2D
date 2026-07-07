#pragma once

#include "SceneGraph/Node.h"
#include "dragonBones/DragonBonesHeaders.h"

DRAGONBONES_USING_NAME_SPACE;

class CLEngine2DFactory;

class DBArmatureNode : public Node {
public:
    DBArmatureNode();
    ~DBArmatureNode() override;

    bool LoadFromFile(const std::string& skeletonPath,
                      const std::string& atlasPath,
                      const std::string& texturePath,
                      const std::string& armatureName = "",
                      float scale = 1.0f);

    void OnUpdate(float deltaTime) override;

    Animation* GetAnimation() const;
    Armature* GetArmature() const { return m_armature; }

    void Play(const std::string& animName, int loopCount = 0);
    void Stop();

protected:
    void OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) override;

private:
    CLEngine2DFactory* m_factory = nullptr;
    Armature* m_armature = nullptr;
    WorldClock* m_clock = nullptr;
};
