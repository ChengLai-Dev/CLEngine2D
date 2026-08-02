#include "ResourcePanel.h"
#include "PopupMenu.h"
#include <Math/Vec2.h>
#include <Render/Renderer.h>
#include <TextRenderer.h>
#include <Input/RawInput.h>
#include <Input/InputCodes.h>

ResourcePanel::ResourcePanel() {
    m_rectWidth = 250.0f;
    m_rectHeight = 200.0f;
}

void ResourcePanel::SetProject(const Project* project) {
    m_project = project;
    m_selectedFile.clear();
    m_hoveredIndex = -1;
}

void ResourcePanel::ClearProject() {
    m_project = nullptr;
    m_selectedFile.clear();
    m_hoveredIndex = -1;
}

void ResourcePanel::Refresh() {
    m_hoveredIndex = -1;
}

void ResourcePanel::SetSelectedFile(const std::string& filename) {
    m_selectedFile = filename;
}

void ResourcePanel::OnFileClick(FileClickCallback cb) {
    m_onFileClick = std::move(cb);
}

void ResourcePanel::OnFileDelete(std::function<void(const std::string&)> cb) {
    m_onFileDelete = std::move(cb);
}

void ResourcePanel::OnFileRename(std::function<void(const std::string&, const std::string&)> cb) {
    m_onFileRename = std::move(cb);
}

void ResourcePanel::OnProjectAction(ProjectActionCallback cb) {
    m_onProjectAction = std::move(cb);
}

int ResourcePanel::HitTest(float localY) const {
    if (!m_project) return -1;

    float y = HEADER_HEIGHT;
    for (size_t i = 0; i < m_project->files.size(); ++i) {
        float itemTop = y;
        float itemBottom = y + ITEM_HEIGHT;
        if (localY >= itemTop && localY < itemBottom) {
            return static_cast<int>(i);
        }
        y += ITEM_HEIGHT;
    }
    return -1;
}

void ResourcePanel::OnRender(Renderer& renderer) {
    Color bgColor(0.1f, 0.1f, 0.12f, 1.0f);
    renderer.DrawQuad(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f),
                      Vec3(m_rectWidth, m_rectHeight, 1.0f), bgColor);

    if (!m_fontRenderer) return;

    float textH = m_fontRenderer->GetLineHeight(1.0f);
    float base = m_fontRenderer->GetBaselineOffset(1.0f);

    if (!m_project) {
        float noProjColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
        m_fontRenderer->RenderString(renderer, "No project opened",
            8.0f, 8.0f + base, 1.0f, noProjColor, TextRenderer::Align::Left);
        return;
    }

    // Project header
    Color headerBg(0.15f, 0.15f, 0.18f, 1.0f);
    renderer.DrawQuad(Vec3(m_rectWidth * 0.5f, HEADER_HEIGHT * 0.5f, 0.0f),
                      Vec3(m_rectWidth, HEADER_HEIGHT, 1.0f), headerBg);

    float headerColor[4] = { 0.6f, 0.8f, 0.6f, 1.0f };
    m_fontRenderer->RenderString(renderer, m_project->name.c_str(),
        8.0f, 4.0f + base, 1.0f, headerColor, TextRenderer::Align::Left);

    Color selColor(0.3f, 0.4f, 0.6f, 1.0f);
    Color hoverColor(0.2f, 0.25f, 0.35f, 1.0f);
    Color itemBg(0.12f, 0.12f, 0.14f, 1.0f);

    float y = HEADER_HEIGHT;
    for (size_t i = 0; i < m_project->files.size(); ++i) {
        const std::string& filename = m_project->files[i];

        const Color& useColor = (filename == m_selectedFile) ? selColor
                               : (static_cast<int>(i) == m_hoveredIndex) ? hoverColor
                               : itemBg;

        float cx = (m_rectWidth - 4.0f) * 0.5f;
        float cy = y + ITEM_HEIGHT * 0.5f;
        renderer.DrawQuad(Vec3(cx, cy, 0.0f),
                          Vec3(m_rectWidth - 4.0f, ITEM_HEIGHT, 1.0f), useColor);

        float itemColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
        m_fontRenderer->RenderString(renderer, filename,
            12.0f, y + (ITEM_HEIGHT - textH) * 0.5f + base,
            1.0f, itemColor, TextRenderer::Align::Left);

        y += ITEM_HEIGHT;
    }
}

void ResourcePanel::OnUpdate(float deltaTime) {
    (void)deltaTime;
}

bool ResourcePanel::OnMouseEvent(const MouseEvent& event) {
    float localX = event.screenPos.x - m_rectLeft;
    float localY = event.screenPos.y - m_rectTop;

    if (event.type == MouseEvent::Move) {
        m_hoveredIndex = HitTest(localY);
        return true;
    }

    if (event.type == MouseEvent::Press && event.button == MouseEvent::Right) {
        if (!m_project) return true;

        if (localY >= 0 && localY < HEADER_HEIGHT) {
            static const PopupMenu::Item projectItems[] = {
                { "New CUI File",    0 },
                { "Import CUI File", 1 },
            };
            m_popupMenu.Open(
                Vec2(event.screenPos.x, event.screenPos.y),
                std::vector<PopupMenu::Item>(projectItems, projectItems + 2),
                [this](int d) {
                    if (d == 0 && m_onProjectAction) m_onProjectAction(0);
                    if (d == 1 && m_onProjectAction) m_onProjectAction(1);
                });
        } else {
            int hit = HitTest(localY);
            if (hit >= 0) {
                int fileIndex = hit;
                static const PopupMenu::Item fileItems[] = {
                    { "Rename", 0 },
                    { "Delete", 1 },
                };
                m_popupMenu.Open(
                    Vec2(event.screenPos.x, event.screenPos.y),
                    std::vector<PopupMenu::Item>(fileItems, fileItems + 2),
                    [this, fileIndex](int d) {
                        if (m_project && fileIndex < static_cast<int>(m_project->files.size())) {
                            const std::string& fn = m_project->files[fileIndex];
                            if (d == 0 && m_onFileRename) m_onFileRename(fn, fn);
                            if (d == 1 && m_onFileDelete) m_onFileDelete(fn);
                        }
                    });
            }
        }
        return true;
    }

    if (event.type == MouseEvent::Press && event.button == MouseEvent::Left) {
        int hit = HitTest(localY);
        if (hit >= 0 && m_project &&
            static_cast<size_t>(hit) < m_project->files.size()) {
            const std::string& filename = m_project->files[static_cast<size_t>(hit)];
            m_selectedFile = filename;
            if (m_onFileClick) {
                m_onFileClick(filename);
            }
            return true;
        }
        return true;
    }

    return false;
}
