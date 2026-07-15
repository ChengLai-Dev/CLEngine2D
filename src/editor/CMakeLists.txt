add_executable(editor
    main.cpp
    Public/EditorApp.h
    Private/EditorApp.cpp
    Public/IEditorPanel.h
    Public/EditorUISystem.h
    Private/EditorUISystem.cpp
    Public/CanvasView.h
    Private/CanvasView.cpp
    Public/MenuBar.h
    Private/MenuBar.cpp
    Public/WidgetPalette.h
    Private/WidgetPalette.cpp
    Public/PropertyPanel.h
    Private/PropertyPanel.cpp
    Public/PropertyEditBox.h
    Private/PropertyEditBox.cpp
    Public/WidgetTreePanel.h
    Private/WidgetTreePanel.cpp
    Public/Gizmo.h
    Private/Gizmo.cpp
    Public/Serializer.h
    Private/Serializer.cpp
    Public/EditorLayoutConfig.h
    Private/EditorLayoutConfig.cpp
    Public/UndoRedo.h
    Private/UndoRedo.cpp
    Public/PropertyFieldRegistry.h
    Private/PropertyFieldRegistry.cpp
    Public/FileDialog.h
    Private/FileDialog.cpp
    Public/PanelDivider.h
    Private/PanelDivider.cpp
    Public/TabBar.h
    Private/TabBar.cpp
)

target_include_directories(editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(editor PRIVATE engine)
target_copy_assets(editor)
