#include "WidgetTreePanel.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <Render/Renderer.h>
#include <TextRenderer.h>
#include <Input/RawInput.h>
#include <cmath>

static constexpr float ITEM_HEIGHT = 22.0f;
static constexpr float DRAG_THRESHOLD = 6.0f;

WidgetTreePanel::WidgetTreePanel() {
    m_rectWidth = 250.0f;
    m_rectHeight = 720.0f;
}

WidgetTreePanel::~WidgetTreePanel() = default;

void WidgetTreePanel::SetRoot(Node* root) {
    m_root = root;
}

void WidgetTreePanel::OnSelectionChanged(SelectionChangedCallback cb) {
    m_onSelectionChanged = std::move(cb);
}

Node* WidgetTreePanel::GetSelectedNode() const {
    return m_selectedNode;
}

void WidgetTreePanel::DrawWidgetTree(Node* node, Renderer& renderer, float& y, int depth) {
    if (!node) return;

    float indent = static_cast<float>(depth) * 16.0f;

    Color bgColor(0.15f, 0.15f, 0.17f, 1.0f);
    Color selColor(0.3f, 0.4f, 0.6f, 1.0f);
    Color dragColor(0.4f, 0.5f, 0.7f, 1.0f);

    const Color& useColor = (node == m_selectedNode) ? selColor
                           : (node == m_dragNode && m_isDragging) ? dragColor
                           : bgColor;

    float centerX = indent + (m_rectWidth - indent) * 0.5f;
    float centerY = y + ITEM_HEIGHT * 0.5f;
    Mat4 itemBg = Mat4::Translate(Vec3(centerX, centerY, 0.0f));
    renderer.DrawQuad(itemBg, Vec2(m_rectWidth - indent - 4.0f, ITEM_HEIGHT),
                      useColor);

    if (m_fontRenderer && !node->GetName().empty()) {
        float textColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
        float textH = m_fontRenderer->GetLineHeight(1.0f);
        float base = m_fontRenderer->GetBaselineOffset(1.0f);
        m_fontRenderer->RenderString(renderer, node->GetName(),
            indent + 6.0f, y + (ITEM_HEIGHT - textH) * 0.5f + base,
            1.0f, textColor, TextRenderer::Align::Left);
    }

    y += ITEM_HEIGHT;

    for (size_t i = 0; i < node->GetChildCount(); ++i) {
        DrawWidgetTree(node->GetChild(i), renderer, y, depth + 1);
    }
}

void WidgetTreePanel::DrawDropIndicator(Renderer& renderer, float y) {
    Color lineColor(0.6f, 0.8f, 1.0f, 1.0f);
    float cx = m_rectWidth * 0.5f;
    Mat4 xform = Mat4::Translate(Vec3(cx, y, 0.0f));
    renderer.DrawQuad(xform, Vec2(m_rectWidth - 8.0f, 2.0f), lineColor);
}

Node* WidgetTreePanel::HitTest(Node* node, float& y, float my) const {
    if (!node) return nullptr;

    float itemTop = y;
    float itemBottom = y + ITEM_HEIGHT;
    y = itemBottom;

    if (my >= itemTop && my < itemBottom) {
        return node;
    }

    for (size_t i = 0; i < node->GetChildCount(); ++i) {
        Node* found = HitTest(node->GetChild(i), y, my);
        if (found) return found;
    }

    return nullptr;
}

void WidgetTreePanel::CollectPositions(Node* node, float& y, int depth,
                                        std::vector<NodePos>& out) const {
    if (!node) return;

    out.push_back({ node, y, static_cast<float>(depth) });
    y += ITEM_HEIGHT;

    for (size_t i = 0; i < node->GetChildCount(); ++i) {
        CollectPositions(node->GetChild(i), y, depth + 1, out);
    }
}

void WidgetTreePanel::CommitDrag() {
    if (!m_dragNode || !m_dropTarget) return;
    if (m_dragNode == m_dropTarget) return;

    Node* parent = m_dragNode->GetParent();
    if (!parent) return;

    // Determine the target sibling index in the parent
    size_t targetIndex = parent->GetChildIndex(m_dropTarget);
    if (targetIndex == static_cast<size_t>(-1)) return;

    // If dropping after, increment index
    if (!m_dropBefore) {
        targetIndex += 1;
    }

    // If the dragged node comes before the target in the vector,
    // removing it shifts the target index down by one
    size_t dragIndex = parent->GetChildIndex(m_dragNode);
    if (dragIndex < targetIndex) {
        targetIndex -= 1;
    }

    auto dragged = parent->RemoveChild(m_dragNode);
    parent->InsertChildAt(std::move(dragged), targetIndex);

    ReindexZOrder(parent);
}

void WidgetTreePanel::ReindexZOrder(Node* parent) {
    for (size_t i = 0; i < parent->GetChildCount(); ++i) {
        parent->GetChild(i)->SetZOrder(static_cast<int>(i));
    }
}

bool WidgetTreePanel::OnMouseEvent(const MouseEvent& event) {
    IEditorPanel::OnMouseEvent(event);

    float localX = event.screenPos.x - m_rectLeft;
    float localY = event.screenPos.y - m_rectTop;

    switch (event.type) {
        case MouseEvent::Press: {
            if (event.button != MouseEvent::Left) return false;
            if (localY < 0.0f || localY >= m_rectHeight) return false;

            if (m_root) {
                float y = 0.0f;
                Node* hit = HitTest(m_root, y, localY);
                if (hit && hit->GetParent()) {
                    m_isDragPending = true;
                    m_dragNode = hit;
                    m_dragStartMouse = event.screenPos;
                    return true;
                }
                if (hit) {
                    m_selectedNode = hit;
                    if (m_onSelectionChanged) m_onSelectionChanged(hit);
                    return true;
                }
            }
            return false;
        }

        case MouseEvent::Move: {
            if (m_isDragging) {
                // Find drop target
                if (m_root) {
                    std::vector<NodePos> positions;
                    float y = 0.0f;
                    CollectPositions(m_root, y, 0, positions);

                    m_dropTarget = nullptr;
                    m_dropBefore = false;

                    for (size_t i = 0; i < positions.size(); ++i) {
                        const auto& pos = positions[i];
                        if (pos.node->GetParent() == nullptr) continue;
                        if (pos.node == m_dragNode) continue;

                        // Check if the drag node is an ancestor of this node
                        bool isDescendant = false;
                        Node* p = pos.node->GetParent();
                        while (p) {
                            if (p == m_dragNode) { isDescendant = true; break; }
                            p = p->GetParent();
                        }
                        if (isDescendant) continue;

                        float itemTop = pos.y;
                        float itemBot = pos.y + ITEM_HEIGHT;

                        if (localY >= itemTop && localY < itemBot) {
                            m_dropTarget = pos.node;
                            m_dropBefore = (localY - itemTop) < (ITEM_HEIGHT * 0.5f);
                            break;
                        }
                    }
                }
                return true;
            }

            if (m_isDragPending) {
                Vec2 delta = event.screenPos - m_dragStartMouse;
                if (std::abs(delta.x) > DRAG_THRESHOLD ||
                    std::abs(delta.y) > DRAG_THRESHOLD) {
                    m_isDragPending = false;
                    m_isDragging = true;
                }
                return true;
            }

            return false;
        }

        case MouseEvent::Release: {
            if (event.button != MouseEvent::Left) return false;

            if (m_isDragging) {
                if (m_dropTarget) {
                    CommitDrag();
                }
                m_isDragging = false;
                m_dragNode = nullptr;
                m_dropTarget = nullptr;
                return true;
            }

            if (m_isDragPending) {
                m_isDragPending = false;
                // Didn't move enough — treat as click
                if (localY >= 0.0f && localY < m_rectHeight && m_root) {
                    float y = 0.0f;
                    Node* hit = HitTest(m_root, y, localY);
                    if (hit) {
                        m_selectedNode = hit;
                        if (m_onSelectionChanged) m_onSelectionChanged(hit);
                    }
                }
                m_dragNode = nullptr;
                return true;
            }

            return false;
        }

        default:
            return false;
    }
}

void WidgetTreePanel::SelectNode(Node* node) {
    m_selectedNode = node;
}

void WidgetTreePanel::OnRender(Renderer& renderer) {
    Color bgColor(0.1f, 0.1f, 0.12f, 1.0f);

    Mat4 bgTransform = Mat4::Translate(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bgTransform, Vec2(m_rectWidth, m_rectHeight));

    if (m_root) {
        float y = 0.0f;

        // Collect all node positions for drop indicator
        std::vector<NodePos> positions;
        if (m_isDragging) {
            CollectPositions(m_root, y, 0, positions);

            // Draw tree items manually so we can interleave the drop indicator
            for (size_t i = 0; i < positions.size(); ++i) {
                const auto& pos = positions[i];

                // Draw drop indicator before this item
                if (m_dropTarget == pos.node && m_dropBefore) {
                    DrawDropIndicator(renderer, pos.y);
                }

                float indent = pos.depth * 16.0f;
                Color color(0.15f, 0.15f, 0.17f, 1.0f);
                if (pos.node == m_selectedNode) {
                    color = Color(0.3f, 0.4f, 0.6f, 1.0f);
                } else if (pos.node == m_dragNode) {
                    color = Color(0.4f, 0.5f, 0.7f, 1.0f);
                }

                float centerX = indent + (m_rectWidth - indent) * 0.5f;
                float centerY = pos.y + ITEM_HEIGHT * 0.5f;
                Mat4 xform = Mat4::Translate(Vec3(centerX, centerY, 0.0f));
                renderer.DrawQuad(xform, Vec2(m_rectWidth - indent - 4.0f, ITEM_HEIGHT), color);

                if (m_fontRenderer && !pos.node->GetName().empty()) {
                    float textColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
                    float textH = m_fontRenderer->GetLineHeight(1.0f);
                    float base = m_fontRenderer->GetBaselineOffset(1.0f);
                    m_fontRenderer->RenderString(renderer, pos.node->GetName(),
                        indent + 6.0f, pos.y + (ITEM_HEIGHT - textH) * 0.5f + base,
                        1.0f, textColor, TextRenderer::Align::Left);
                }
            }

            // Draw drop indicator after the last item if dropping after
            if (m_dropTarget && !m_dropBefore) {
                for (const auto& pos : positions) {
                    if (pos.node == m_dropTarget) {
                        DrawDropIndicator(renderer, pos.y + ITEM_HEIGHT);
                        break;
                    }
                }
            }
        } else {
            y = 0.0f;
            DrawWidgetTree(m_root, renderer, y, 0);
        }
    }
}
