#include "SceneGraph/DBArmatureNode.h"
#include "SceneGraph/DragonBones/CLEngine2DFactory.h"
#include "SceneGraph/DragonBones/CLEngine2DSlot.h"
#include "SceneGraph/DragonBones/internal.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"
#include "Logger.h"

#include <fstream>
#include <sstream>

DBArmatureNode::DBArmatureNode()
    : m_factory(new CLEngine2DFactory())
    , m_clock(new WorldClock()) {
}

DBArmatureNode::~DBArmatureNode() {
    if (m_armature) {
        m_armature->dispose();
        m_armature = nullptr;
    }
    delete m_clock;
    delete m_factory;
}

static std::string ReadFileContent(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool DBArmatureNode::LoadFromFile(const std::string& skeletonPath,
                                   const std::string& atlasPath,
                                   const std::string& texturePath,
                                   const std::string& armatureName,
                                   float scale) {
    auto skeletonJson = ReadFileContent(skeletonPath);
    if (skeletonJson.empty()) {
        Logger::Error("DBArmatureNode: Failed to read skeleton file: {}", skeletonPath);
        return false;
    }

    auto atlasJson = ReadFileContent(atlasPath);
    if (atlasJson.empty()) {
        Logger::Error("DBArmatureNode: Failed to read atlas file: {}", atlasPath);
        return false;
    }

    auto* dragonBonesData = m_factory->parseDragonBonesData(skeletonJson.c_str(), "", scale);
    if (!dragonBonesData) {
        Logger::Error("DBArmatureNode: Failed to parse skeleton data");
        return false;
    }

    auto* texture = new Texture(texturePath);
    auto* texAtlasData = m_factory->parseTextureAtlasData(atlasJson.c_str(), texture, "", scale);
    if (!texAtlasData) {
        Logger::Error("DBArmatureNode: Failed to parse texture atlas data");
        delete texture;
        return false;
    }

    std::string armName = armatureName.empty() ? dragonBonesData->getArmatureNames()[0] : armatureName;

    m_armature = m_factory->buildArmature(armName);
    if (!m_armature) {
        Logger::Error("DBArmatureNode: Failed to build armature: {}", armName);
        return false;
    }

    m_clock->add(m_armature);
    m_armature->getAnimation()->play();

    return true;
}

void DBArmatureNode::OnUpdate(float deltaTime) {
    Node::OnUpdate(deltaTime);

    if (m_clock) {
        m_clock->advanceTime(deltaTime);
    }
}

Animation* DBArmatureNode::GetAnimation() const {
    return m_armature ? m_armature->getAnimation() : nullptr;
}

void DBArmatureNode::Play(const std::string& animName, int loopCount) {
    if (m_armature) {
        m_armature->getAnimation()->play(animName, loopCount);
    }
}

void DBArmatureNode::Stop() {
    if (m_armature) {
        m_armature->getAnimation()->reset();
    }
}

void DBArmatureNode::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
    if (!m_armature) return;

    auto& slots = m_armature->getSlots();
    for (auto* slot : slots) {
        auto* data = static_cast<DBSlotVertexData*>(slot->getDisplay());
        if (!data || data->indices.empty() || !data->texture) continue;

        renderer.DrawTriangles(
            data->vertices.data(),
            static_cast<unsigned>(data->vertices.size()),
            data->indices.data(),
            static_cast<unsigned>(data->indices.size()),
            data->texture);
    }
}
