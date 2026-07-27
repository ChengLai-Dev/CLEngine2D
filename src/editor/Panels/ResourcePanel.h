#pragma once

#include "IEditorPanel.h"
#include "Project.h"
#include <functional>
#include <vector>
#include <string>

class ResourcePanel : public IEditorPanel {
public:
    ResourcePanel();

    void SetProject(const Project* project);
    const Project* GetProject() const { return m_project; }
    void ClearProject();
    void Refresh();

    void SetSelectedFile(const std::string& filename);
    std::string GetSelectedFile() const { return m_selectedFile; }
    int GetHoveredItemIndex() const { return m_hoveredIndex; }

    // IEditorPanel
    void OnRender(Renderer& renderer) override;
    bool OnMouseEvent(const MouseEvent& event) override;
    void OnUpdate(float deltaTime) override;

    using FileClickCallback = std::function<void(const std::string& filename)>;
    void OnFileClick(FileClickCallback cb);
    void OnFileDelete(std::function<void(const std::string& filename)> cb);
    void OnFileRename(std::function<void(const std::string& oldName, const std::string& newName)> cb);

    using ProjectActionCallback = std::function<void(int action)>;
    void OnProjectAction(ProjectActionCallback cb);

private:
    static constexpr float ITEM_HEIGHT = 22.0f;
    static constexpr float HEADER_HEIGHT = 24.0f;

    int HitTest(float localY) const;

    const Project* m_project = nullptr;

    std::string m_selectedFile;
    int m_hoveredIndex = -1;

    FileClickCallback m_onFileClick;
    std::function<void(const std::string&)> m_onFileDelete;
    std::function<void(const std::string&, const std::string&)> m_onFileRename;
    ProjectActionCallback m_onProjectAction;
};
