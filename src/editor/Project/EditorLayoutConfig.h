#pragma once

#include <UI/JsonValue.h>
#include <string>

class EditorLayoutConfig {
public:
    float menuBarHeight = 24.0f;
    float leftPanelWidth = 250.0f;
    float rightPanelWidth = 300.0f;

    float leftPanelDivider1Y = 200.0f;
    float leftPanelDivider2Y = 400.0f;

    JsonValue ToJson() const;
    static EditorLayoutConfig FromJson(const JsonValue& json);

    bool SaveToFile(const std::string& filepath) const;
    static bool LoadFromFile(const std::string& filepath, EditorLayoutConfig& out);
};
 