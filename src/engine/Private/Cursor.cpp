#include "Cursor.h"
#include <GLFW/glfw3.h>

void CursorManager::Init(GLFWwindow* nativeWindow) {
    s_window = nativeWindow;
    EnsureCursors();
    s_initialized = true;
}

void CursorManager::Set(CursorType type) {
    if (!s_initialized) return;
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < 8 && s_cursors[idx]) {
        glfwSetCursor(s_window, s_cursors[idx]);
    }
}

void CursorManager::Reset() {
    if (!s_initialized) return;
    glfwSetCursor(s_window, nullptr);
}

void CursorManager::Shutdown() {
    if (!s_initialized) return;
    for (auto& c : s_cursors) {
        if (c) {
            glfwDestroyCursor(c);
            c = nullptr;
        }
    }
    s_initialized = false;
    s_window = nullptr;
}

void CursorManager::EnsureCursors() {
    if (s_cursors[0]) return;

    s_cursors[0] = nullptr;
    s_cursors[1] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    s_cursors[2] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    s_cursors[3] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    s_cursors[4] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    s_cursors[5] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    s_cursors[6] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
    s_cursors[7] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
}
