#pragma once

#include <string>
#include <vector>

class JsonValue;

class Project {
public:
    std::string name;
    std::string directory;
    std::vector<std::string> files;
    int version = 1;

    bool IsValid() const { return !name.empty() && !directory.empty(); }

    JsonValue ToJson() const;
    static Project FromJson(const JsonValue& json);

    bool SaveToFile(const std::string& filepath) const;
    static Project LoadFromFile(const std::string& filepath);

    std::string GetProjectFilePath() const;
    std::string GetCuiFilePath(const std::string& filename) const;

    void AddFile(const std::string& filename);
    void RemoveFile(const std::string& filename);
    void RenameFile(const std::string& oldName, const std::string& newName);
    bool HasFile(const std::string& filename) const;
};
