#include "Project.h"
#include "Serializer.h"
#include <Logger.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

JsonValue Project::ToJson() const {
    JsonValue json;
    json.Set("version", JsonValue(version));
    json.Set("name", JsonValue(name));
    json.Set("directory", JsonValue(directory));

    JsonValue filesArr;
    for (const auto& f : files) {
        filesArr.PushBack(JsonValue(f));
    }
    json.Set("files", filesArr);
    return json;
}

Project Project::FromJson(const JsonValue& json) {
    Project proj;
    if (json.Get("version").GetType() != JsonValue::Type::Null)
        proj.version = json.Get("version").AsInt();
    if (json.Get("name").GetType() != JsonValue::Type::Null)
        proj.name = json.Get("name").AsString();
    if (json.Get("directory").GetType() != JsonValue::Type::Null)
        proj.directory = json.Get("directory").AsString();

    const JsonValue& filesArr = json.Get("files");
    if (filesArr.GetType() == JsonValue::Type::Array) {
        for (size_t i = 0; i < filesArr.Size(); ++i) {
            proj.files.push_back(filesArr[i].AsString());
        }
    }
    return proj;
}

bool Project::SaveToFile(const std::string& filepath) const {
    std::string jsonStr = ToJson().Serialize();

    std::ofstream file(filepath);
    if (!file.is_open()) {
        Logger::Error("Project::SaveToFile: cannot write {}", filepath);
        return false;
    }
    file << jsonStr;
    return true;
}

Project Project::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::Error("Project::LoadFromFile: cannot read {}", filepath);
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    JsonValue json = JsonValue::Parse(content);
    Project proj = FromJson(json);

    if (proj.directory.empty()) {
        std::filesystem::path p(filepath);
        proj.directory = p.parent_path().string();
    }

    return proj;
}

std::string Project::GetProjectFilePath() const {
    return directory + "/" + name + ".cuiproj";
}

std::string Project::GetCuiFilePath(const std::string& filename) const {
    return directory + "/" + filename;
}

void Project::AddFile(const std::string& filename) {
    if (!HasFile(filename)) {
        files.push_back(filename);
    }
}

void Project::RemoveFile(const std::string& filename) {
    auto it = std::remove(files.begin(), files.end(), filename);
    files.erase(it, files.end());
}

void Project::RenameFile(const std::string& oldName, const std::string& newName) {
    for (auto& f : files) {
        if (f == oldName) {
            f = newName;
            return;
        }
    }
}

bool Project::HasFile(const std::string& filename) const {
    return std::find(files.begin(), files.end(), filename) != files.end();
}
