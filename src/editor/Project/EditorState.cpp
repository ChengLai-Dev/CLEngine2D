#include "EditorState.h"
#include "Serializer.h"
#include <Logger.h>
#include <fstream>
#include <sstream>

JsonValue EditorState::ToJson() const {
    JsonValue json;
    json.Set("version", JsonValue(1));
    json.Set("lastProjectPath", JsonValue(lastProjectPath));

    JsonValue filesArr;
    for (const auto& f : openFiles) {
        filesArr.PushBack(JsonValue(f));
    }
    json.Set("openFiles", filesArr);
    json.Set("activeFile", JsonValue(activeFile));

    return json;
}

EditorState EditorState::FromJson(const JsonValue& json) {
    EditorState state;
    if (json.Get("lastProjectPath").GetType() != JsonValue::Type::Null)
        state.lastProjectPath = json.Get("lastProjectPath").AsString();
    if (json.Get("activeFile").GetType() != JsonValue::Type::Null)
        state.activeFile = json.Get("activeFile").AsString();

    const JsonValue& filesArr = json.Get("openFiles");
    if (filesArr.GetType() == JsonValue::Type::Array) {
        for (size_t i = 0; i < filesArr.Size(); ++i) {
            state.openFiles.push_back(filesArr[i].AsString());
        }
    }
    return state;
}

bool EditorState::SaveToFile(const std::string& filepath) const {
    std::string jsonStr = ToJson().Serialize();
    std::ofstream file(filepath);
    if (!file.is_open()) {
        Logger::Error("EditorState::SaveToFile: cannot write {}", filepath);
        return false;
    }
    file << jsonStr;
    return true;
}

EditorState EditorState::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::Info("EditorState::LoadFromFile: no saved state at {}", filepath);
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    JsonValue json = JsonValue::Parse(content);
    return FromJson(json);
}
