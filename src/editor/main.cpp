#include "EditorApp.h"
#include <windows.h>
#include <ole2.h>

int main() {
    OleInitialize(nullptr);
    DisableProcessWindowsGhosting();

    EditorApp app;
    app.Run();
    return 0;
}
