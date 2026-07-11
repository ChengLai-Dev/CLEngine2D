#pragma once

struct GLFWwindow;
struct GLFWcursor;

enum class CursorType {
    Default,
    Hand,
    IBeam,
    Crosshair,
    HResize,
    VResize,
    DiagResize1,
    DiagResize2,
};

class CursorManager {
public:
    static void Init(GLFWwindow* nativeWindow);
    static void Set(CursorType type);
    static void Reset();
    static void Shutdown();

private:
    static void EnsureCursors();

    static inline GLFWwindow* s_window = nullptr;
    static inline GLFWcursor* s_cursors[8] = {};
    static inline bool s_initialized = false;
};
