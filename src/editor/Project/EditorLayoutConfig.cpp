#include "EditorLayoutConfig.h"
#include <fstream>
#include <sstream>

JsonValue EditorLayoutConfig::ToJson() const {
    JsonValue json;
    json.Set("version", JsonValue(1));
    json.Set("menuBarHeight", JsonValue(static_cast<double>(menuBarHeight)));
    json.Set("leftPanelWidth", JsonValue(static_cast<double>(leftPanelWidth)));
    json.Set("rightPanelWidth", JsonValue(static_cast<double>(rightPanelWidth)));
    json.Set("leftPanelDivider1Y", JsonValue(static_cast<double>(leftPanelDivider1Y)));
    json.Set("leftPanelDivider2Y", JsonValue(static_cast<double>(leftPanelDivider2Y)));
    return json;
}

EditorLayoutConfig EditorLayoutConfig::FromJson(const JsonValue& json) {
    EditorLayoutConfig config;
    if (json.Get("menuBarHeight").GetType() != JsonValue::Type::Null)
        config.menuBarHeight = static_cast<float>(json.Get("menuBarHeight").AsNumber());
    if (json.Get("leftPanelWidth").GetType() != JsonValue::Type::Null)
        config.leftPanelWidth = static_cast<float>(json.Get("leftPanelWidth").AsNumber());
    if (json.Get("rightPanelWidth").GetType() != JsonValue::Type::Null)
        config.rightPanelWidth = static_cast<float>(json.Get("rightPanelWidth").AsNumber());
    if (json.Get("leftPanelDivider1Y").GetType() != JsonValue::Type::Null)
        config.leftPanelDivider1Y = static_cast<float>(json.Get("leftPanelDivider1Y").AsNumber());
    if (json.Get("leftPanelDivider2Y").GetType() != JsonValue::Type::Null)
        config.leftPanelDivider2Y = static_cast<float>(json.Get("leftPanelDivider2Y").AsNumber());
    return config;
}

bool EditorLayoutConfig::SaveToFile(const std::string& filepath) const {
    std::string jsonStr = ToJson().Serialize();

    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    file << jsonStr;
    return true;
}

bool EditorLayoutConfig::LoadFromFile(const std::string& filepath, EditorLayoutConfig& out) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    JsonValue json = JsonValue::Parse(content);
    if (json.Get("version").AsInt() != 1) return false;

    out = FromJson(json);
    return true;
}
