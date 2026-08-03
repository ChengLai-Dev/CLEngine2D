#pragma once

#include <string>
#include <vector>
#include <functional>

class Node;

enum class FieldType { Float, Int, Bool, String, TextureAsset, Enum };

struct PropertyFieldDef {
    std::string label;
    FieldType type;
    std::function<std::string(Node*)> getter;
    std::function<void(Node*, const std::string&)> setter;
    float minVal = -10000.0f;
    float maxVal = 10000.0f;
    std::vector<std::string> options;
};

struct PropertySectionDef {
    std::string title;
    std::function<bool(Node*)> condition;
    std::vector<PropertyFieldDef> fields;
};

class PropertyFieldRegistry {
public:
    static const std::vector<PropertySectionDef>& GetAll();
};
