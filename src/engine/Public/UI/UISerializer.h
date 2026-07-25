#pragma once

#include <string>
#include "UI/JsonValue.h"

class Node;
class Widget;
class CanvasPanel;

struct FAnchorData;

class UISerializer {
public:
    static JsonValue SerializeNode(Node* node);
    static Node* DeserializeNode(const JsonValue& json);

    static JsonValue SerializeScene(Node* root);
    static bool DeserializeScene(Node* root, const JsonValue& json);

    static bool SaveToFile(Node* root, const std::string& filepath);
    static Node* LoadFromFile(const std::string& filepath);
};
