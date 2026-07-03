add_executable(editor
    main.cpp
    Public/EditorApp.h
    Private/EditorApp.cpp
    Public/CanvasView.h
    Private/CanvasView.cpp
    Public/PropertyPanel.h
    Private/PropertyPanel.cpp
    Public/WidgetTreePanel.h
    Private/WidgetTreePanel.cpp
    Public/Toolbar.h
    Private/Toolbar.cpp
    Public/Gizmo.h
    Private/Gizmo.cpp
    Public/Serializer.h
    Private/Serializer.cpp
    Public/UndoRedo.h
    Private/UndoRedo.cpp
)

target_include_directories(editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(editor PRIVATE engine)
target_copy_assets(editor)
