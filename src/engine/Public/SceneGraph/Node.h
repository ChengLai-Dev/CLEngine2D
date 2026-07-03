#pragma once

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "Math/Vec4.h"
#include "Math/Mat4.h"

#include <memory>
#include <string>
#include <vector>

class Renderer;

class Node {
public:
    Node();
    Node(Node&&) = default;
    Node& operator=(Node&&) = default;
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    virtual ~Node();

    // === Transform (Vec3 position, Euler XYZ rotation, Vec3 scale) ===
    void SetPosition(const Vec3& pos);
    const Vec3& GetPosition() const;

    void SetRotation(const Vec3& eulerRad);
    const Vec3& GetRotation() const;

    void SetScale(const Vec3& scale);
    const Vec3& GetScale() const;

    void SetRotationZ(float rad);
    float GetRotationZ() const;

    // === Anchor (normalized 0-1, default 0.5=center) ===
    void SetAnchor(const Vec2& anchor);
    const Vec2& GetAnchor() const;

    void SetContentSize(const Vec2& size);
    const Vec2& GetContentSize() const;

    // === Hierarchy ===
    void AddChild(std::unique_ptr<Node> child);
    std::unique_ptr<Node> RemoveChild(Node* child);
    Node* GetParent() const;

    size_t GetChildCount() const;
    Node* GetChild(size_t index) const;
    Node* FindChild(const std::string& name);

    // === State ===
    void SetVisible(bool visible);
    bool IsVisible() const;

    void SetOpacity(float opacity);
    float GetOpacity() const;

    void SetColor(const Vec4& color);
    const Vec4& GetColor() const;

    void SetZOrder(int zOrder);
    int GetZOrder() const;

    void SetName(const std::string& name);
    const std::string& GetName() const;

    // === Computed transform ===
    const Mat4& GetLocalTransform();
    const Mat4& GetWorldTransform();

    // === Lifecycle ===
    virtual void OnUpdate(float deltaTime);
    virtual void Visit(Renderer& renderer, const Mat4& parentTransform, float parentOpacity);

protected:
    virtual void OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity);

    Vec3 m_position;
    Vec3 m_rotation;
    Vec3 m_scale = Vec3(1.0f);

    Vec2 m_anchor = Vec2(0.5f, 0.5f);
    Vec2 m_contentSize = Vec2(1.0f, 1.0f);

    Node* m_parent = nullptr;
    std::vector<std::unique_ptr<Node>> m_children;

    bool m_visible = true;
    float m_opacity = 1.0f;
    Vec4 m_color = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    int m_localZOrder = 0;

    std::string m_name;

    mutable Mat4 m_localTransform;
    mutable Mat4 m_worldTransform;
    mutable bool m_transformDirty = true;

private:
    void MarkDirty();
    void SortChildrenByZOrder();
};

class NodeIterator {
public:
    explicit NodeIterator(Node* root);

    Node* Next();

private:
    std::vector<Node*> m_stack;
};
