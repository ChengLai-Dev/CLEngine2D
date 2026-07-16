#pragma once

#include <string>
#include <vector>

class JsonValue;

class EditorState {
public:
    std::string lastProjectPath;
    std::vector<std::string> openFiles;
    std::string activeFile;

    JsonValue ToJson() const;
    static EditorState FromJson(const JsonValue& json);

    bool SaveToFile(const std::string& filepath) const;
    static EditorState LoadFromFile(const std::string& filepath);
};
