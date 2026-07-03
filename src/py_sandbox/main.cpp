// CLEngine2D Python Sandbox
// Entry point that embeds Python and runs scripts/main.py

#include <BindApp.h>
#include <Logger.h>
#include <string>
int main(int argc, char* argv[])
{
    std::string scriptDir = "scripts";
    std::string moduleName = "main";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--dir" && i + 1 < argc) {
            scriptDir = argv[++i];
        } else if (arg == "--module" && i + 1 < argc) {
            moduleName = argv[++i];
        }
    }

    Logger::Info("Starting Python sandbox: dir='{}' module='{}'", scriptDir, moduleName);

    PythonScriptApp app(scriptDir, moduleName);
    app.Run();

    return 0;
}
