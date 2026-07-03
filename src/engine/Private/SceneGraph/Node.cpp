#include "SceneGraph/Node.h"
#include "Render/Renderer.h"

#include <algorithm>

Node::Node()
    : m_position(0.0f, 0.0f, 0.0f)
    , m_rotation(0.0f, 0.0f, 0.0f)
{
}

Node::~Node() = default;

void Node::SetPosition(const Vec3& pos) {
    m_position = pos;
    MarkDirty();
}

const Vec3& Node::GetPosition() const {
    return m_position;
}

void Node::SetRotation(const Vec3& eulerRad) {
    m_rotation = eulerRad;
    MarkDirty();
}

const Vec3& Node::GetRotation() const {
    return m_rotation;
}

void Node::SetScale(const Vec3& scale) {
    m_scale = scale;
    MarkDirty();
}

const Vec3& Node::GetScale() const {
    return m_scale;
}

void Node::SetRotationZ(float rad) {
    m_rotation.x = 0.0f;
    m_rotation.y = 0.0f;
    m_rotation.z = rad;
    MarkDirty();
}

float Node::GetRotationZ() const {
    return m_rotation.z;
}

void Node::SetAnchor(const Vec2& anchor) {
    m_anchor = anchor;
    MarkDirty();
}

const Vec2& Node::GetAnchor() const {
    return m_anchor;
}

void Node::SetContentSize(const Vec2& size) {
    m_contentSize = size;
    MarkDirty();
}

const Vec2& Node::GetContentSize() const {
    return m_contentSize;
}

void Node::AddChild(std::unique_ptr<Node> child) {
    if (!child) return;
    child->m_parent = this;
    m_children.push_back(std::move(child));
}

std::unique_ptr<Node> Node::RemoveChild(Node* child) {
    auto it = std::find_if(m_children.begin(), m_children.end(),
        [child](const std::unique_ptr<Node>& ptr) { return ptr.get() == child; });
    if (it != m_children.end()) {
        auto removed = std::move(*it);
        removed->m_parent = nullptr;
        m_children.erase(it);
        return removed;
    }
    return nullptr;
}

Node* Node::GetParent() const {
    return m_parent;
}

size_t Node::GetChildCount() const {
    return m_children.size();
}

Node* Node::GetChild(size_t index) const {
    return (index < m_children.size()) ? m_children[index].get() : nullptr;
}

Node* Node::FindChild(const std::string& name) {
    for (auto& child : m_children) {
        if (child->m_name == name) return child.get();
        Node* found = child->FindChild(name);
        if (found) return found;
    }
    return nullptr;
}

void Node::SetVisible(bool visible) {
    m_visible = visible;
}

bool Node::IsVisible() const {
    return m_visible;
}

void Node::SetOpacity(float opacity) {
    m_opacity = opacity;
}

float Node::GetOpacity() const {
    return m_opacity;
}

void Node::SetColor(const Vec4& color) {
    m_color = color;
}

const Vec4& Node::GetColor() const {
    return m_color;
}

void Node::SetZOrder(int zOrder) {
    m_localZOrder = zOrder;
}

int Node::GetZOrder() const {
    return m_localZOrder;
}

void Node::SetName(const std::string& name) {
    m_name = name;
}

const std::string& Node::GetName() const {
    return m_name;
}

const Mat4& Node::GetLocalTransform() {
    if (m_transformDirty) {
        Vec3 anchorOffset(
            (0.5f - m_anchor.x) * m_contentSize.x,
            (0.5f - m_anchor.y) * m_contentSize.y,
            0.0f
        );

        m_localTransform =
            Mat4::Translate(m_position) *
            Mat4::Translate(anchorOffset) *
            Mat4::RotateZ(m_rotation.z) *
            Mat4::RotateY(m_rotation.y) *
            Mat4::RotateX(m_rotation.x) *
            Mat4::Scale(m_scale);

        m_transformDirty = false;
    }
    return m_localTransform;
}

const Mat4& Node::GetWorldTransform() {
    if (m_parent) {
        m_worldTransform = m_parent->GetWorldTransform() * GetLocalTransform();
        return m_worldTransform;
    }
    return GetLocalTransform();
}

void Node::OnUpdate(float deltaTime) {
    for (auto& child : m_children) {
        child->OnUpdate(deltaTime);
    }
}

void Node::Visit(Renderer& renderer, const Mat4& parentTransform, float parentOpacity) {
    if (!m_visible) return;

    float worldOpacity = parentOpacity * m_opacity;
    Mat4 worldTransform = parentTransform * GetLocalTransform();

    OnDraw(renderer, worldTransform, worldOpacity);

    SortChildrenByZOrder();

    for (auto& child : m_children) {
        if (child->m_localZOrder < 0) {
            child->Visit(renderer, worldTransform, worldOpacity);
        }
    }

    for (auto& child : m_children) {
        if (child->m_localZOrder >= 0) {
            child->Visit(renderer, worldTransform, worldOpacity);
        }
    }
}

void Node::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
}

void Node::MarkDirty() {
    m_transformDirty = true;
    for (auto& child : m_children) {
        child->MarkDirty();
    }
}

void Node::SortChildrenByZOrder() {
    std::sort(m_children.begin(), m_children.end(),
        [](const std::unique_ptr<Node>& a, const std::unique_ptr<Node>& b) {
            return a->m_localZOrder < b->m_localZOrder;
        });
}

// === NodeIterator ===

NodeIterator::NodeIterator(Node* root) {
    if (root) m_stack.push_back(root);
}

Node* NodeIterator::Next() {
    if (m_stack.empty()) return nullptr;
    Node* current = m_stack.back();
    m_stack.pop_back();
    for (size_t i = current->GetChildCount(); i > 0; --i) {
        m_stack.push_back(current->GetChild(i - 1));
    }
    return current;
}
