#include "Serializer.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <SceneGraph/Button.h>
#include <SceneGraph/Label.h>
#include <SceneGraph/Sprite.h>
#include <SceneGraph/CanvasPanel.h>
#include <SceneGraph/Layout.h>
#include <fstream>
#include <sstream>
#include <stack>
#include <cctype>

// ========== JsonValue implementation ==========

bool JsonValue::AsBool() const {
    return std::get<bool>(m_data);
}

double JsonValue::AsNumber() const {
    return std::get<double>(m_data);
}

int JsonValue::AsInt() const {
    return static_cast<int>(std::get<double>(m_data));
}

const std::string& JsonValue::AsString() const {
    return std::get<std::string>(m_data);
}

JsonValue& JsonValue::operator[](const std::string& key) {
    if (m_type != Type::Object) {
        m_type = Type::Object;
        m_data = ObjectType();
    }
    auto& obj = std::get<ObjectType>(m_data);
    return obj[key];
}

JsonValue& JsonValue::operator[](size_t index) {
    auto& arr = std::get<ArrayType>(m_data);
    if (index >= arr.size()) arr.resize(index + 1);
    return arr[index];
}

const JsonValue& JsonValue::Get(const std::string& key) const {
    static JsonValue nullVal;
    if (m_type != Type::Object) return nullVal;
    const auto& obj = std::get<ObjectType>(m_data);
    auto it = obj.find(key);
    return it != obj.end() ? it->second : nullVal;
}

void JsonValue::PushBack(const JsonValue& val) {
    if (m_type != Type::Array) {
        m_type = Type::Array;
        m_data = ArrayType();
    }
    std::get<ArrayType>(m_data).push_back(val);
}

void JsonValue::Set(const std::string& key, const JsonValue& val) {
    (*this)[key] = val;
}

size_t JsonValue::Size() const {
    if (m_type == Type::Array) return std::get<ArrayType>(m_data).size();
    if (m_type == Type::Object) return std::get<ObjectType>(m_data).size();
    return 0;
}

const JsonValue& JsonValue::operator[](size_t index) const {
    static JsonValue nullVal;
    if (m_type != Type::Array) return nullVal;
    const auto& arr = std::get<ArrayType>(m_data);
    return index < arr.size() ? arr[index] : nullVal;
}

const JsonValue::ObjectType& JsonValue::GetObject() const {
    static ObjectType empty;
    return m_type == Type::Object ? std::get<ObjectType>(m_data) : empty;
}

const JsonValue::ArrayType& JsonValue::GetArray() const {
    static ArrayType empty;
    return m_type == Type::Array ? std::get<ArrayType>(m_data) : empty;
}

static std::string EscapeString(const std::string& s) {
    std::string result;
    result.reserve(s.size() + 2);
    result.push_back('"');
    for (char c : s) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\t': result += "\\t"; break;
            case '\r': result += "\\r"; break;
            default: result.push_back(c);
        }
    }
    result.push_back('"');
    return result;
}

std::string JsonValue::Serialize(int indent) const {
    std::string prefix(indent, ' ');
    std::string childPrefix(indent + 2, ' ');

    switch (m_type) {
    case Type::Null:
        return "null";
    case Type::Bool:
        return AsBool() ? "true" : "false";
    case Type::Number: {
        double v = AsNumber();
        if (v == static_cast<int>(v)) return std::to_string(static_cast<int>(v));
        return std::to_string(v);
    }
    case Type::String:
        return EscapeString(AsString());
    case Type::Array: {
        const auto& arr = std::get<ArrayType>(m_data);
        if (arr.empty()) return "[]";
        std::string result = "[\n";
        for (size_t i = 0; i < arr.size(); ++i) {
            result += childPrefix + arr[i].Serialize(indent + 2);
            if (i + 1 < arr.size()) result += ",";
            result += "\n";
        }
        result += prefix + "]";
        return result;
    }
    case Type::Object: {
        const auto& obj = std::get<ObjectType>(m_data);
        if (obj.empty()) return "{}";
        std::string result = "{\n";
        bool first = true;
        for (const auto& [key, val] : obj) {
            if (!first) result += ",\n";
            first = false;
            result += childPrefix + EscapeString(key) + ": " + val.Serialize(indent + 2);
        }
        result += "\n" + prefix + "}";
        return result;
    }
    }
    return "null";
}

// ========== JSON Parser (minimal recursive descent) ==========

struct JsonParser {
    const std::string& input;
    size_t pos = 0;

    JsonParser(const std::string& s) : input(s) {}

    void SkipWhitespace() {
        while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos]))) ++pos;
    }

    char Peek() {
        SkipWhitespace();
        return pos < input.size() ? input[pos] : '\0';
    }

    char Advance() {
        return pos < input.size() ? input[pos++] : '\0';
    }

    bool Expect(char c) {
        SkipWhitespace();
        if (pos < input.size() && input[pos] == c) { ++pos; return true; }
        return false;
    }

    JsonValue ParseValue() {
        char c = Peek();
        switch (c) {
            case '{': return ParseObject();
            case '[': return ParseArray();
            case '"': return ParseString();
            case 't': case 'f': return ParseBool();
            case 'n': return ParseNull();
            default:
                if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return ParseNumber();
                return JsonValue();
        }
    }

    JsonValue ParseObject() {
        JsonValue obj;
        Expect('{');
        if (Expect('}')) return obj;

        while (true) {
            std::string key = ParseString().AsString();
            Expect(':');
            obj.Set(key, ParseValue());
            if (!Expect(',')) break;
        }
        Expect('}');
        return obj;
    }

    JsonValue ParseArray() {
        JsonValue arr;
        Expect('[');
        if (Expect(']')) return arr;

        while (true) {
            arr.PushBack(ParseValue());
            if (!Expect(',')) break;
        }
        Expect(']');
        return arr;
    }

    JsonValue ParseString() {
        Expect('"');
        std::string result;
        while (pos < input.size() && input[pos] != '"') {
            if (input[pos] == '\\') {
                ++pos;
                if (pos >= input.size()) break;
                switch (input[pos]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case 'r': result += '\r'; break;
                    default: result += input[pos];
                }
            } else {
                result += input[pos];
            }
            ++pos;
        }
        Expect('"');
        return JsonValue(result);
    }

    JsonValue ParseNumber() {
        size_t start = pos;
        if (input[pos] == '-') ++pos;
        while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) ++pos;
        if (pos < input.size() && input[pos] == '.') {
            ++pos;
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) ++pos;
        }
        if (pos < input.size() && (input[pos] == 'e' || input[pos] == 'E')) {
            ++pos;
            if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) ++pos;
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) ++pos;
        }
        double val = std::stod(input.substr(start, pos - start));
        return JsonValue(val);
    }

    JsonValue ParseBool() {
        if (input.substr(pos, 4) == "true") { pos += 4; return JsonValue(true); }
        if (input.substr(pos, 5) == "false") { pos += 5; return JsonValue(false); }
        return JsonValue();
    }

    JsonValue ParseNull() {
        if (input.substr(pos, 4) == "null") { pos += 4; return JsonValue(); }
        return JsonValue();
    }
};

JsonValue JsonValue::Parse(const std::string& json) {
    JsonParser parser(json);
    return parser.ParseValue();
}

// ========== Serializer implementation ==========

static JsonValue SerializeVec2(const Vec2& v) {
    JsonValue obj;
    obj.Set("x", JsonValue(static_cast<double>(v.x)));
    obj.Set("y", JsonValue(static_cast<double>(v.y)));
    return obj;
}

static Vec2 DeserializeVec2(const JsonValue& json) {
    return Vec2(
        static_cast<float>(json.Get("x").AsNumber()),
        static_cast<float>(json.Get("y").AsNumber())
    );
}

static JsonValue SerializeVec4(const Vec4& v) {
    JsonValue obj;
    obj.Set("x", JsonValue(static_cast<double>(v.x)));
    obj.Set("y", JsonValue(static_cast<double>(v.y)));
    obj.Set("z", JsonValue(static_cast<double>(v.z)));
    obj.Set("w", JsonValue(static_cast<double>(v.w)));
    return obj;
}

static Vec4 DeserializeVec4(const JsonValue& json) {
    return Vec4(
        static_cast<float>(json.Get("x").AsNumber()),
        static_cast<float>(json.Get("y").AsNumber()),
        static_cast<float>(json.Get("z").AsNumber()),
        static_cast<float>(json.Get("w").AsNumber())
    );
}

static std::string GetWidgetType(Node* node) {
    if (dynamic_cast<CanvasPanel*>(node)) return "CanvasPanel";
    if (dynamic_cast<Button*>(node)) return "Button";
    if (dynamic_cast<Label*>(node)) return "Label";
    if (dynamic_cast<Layout*>(node)) return "Layout";
    if (dynamic_cast<Sprite*>(node)) return "Sprite";
    return "Widget";
}

JsonValue Serializer::SerializeNode(Node* node) {
    JsonValue json;
    if (!node) return json;

    json.Set("type", JsonValue(GetWidgetType(node)));
    json.Set("name", JsonValue(node->GetName()));

    Vec3 pos = node->GetPosition();
    JsonValue posJson;
    posJson.Set("x", JsonValue(static_cast<double>(pos.x)));
    posJson.Set("y", JsonValue(static_cast<double>(pos.y)));
    posJson.Set("z", JsonValue(static_cast<double>(pos.z)));
    json.Set("position", posJson);

    Vec2 size = node->GetContentSize();
    JsonValue sizeJson;
    sizeJson.Set("x", JsonValue(static_cast<double>(size.x)));
    sizeJson.Set("y", JsonValue(static_cast<double>(size.y)));
    json.Set("size", sizeJson);

    Vec3 scale = node->GetScale();
    if (scale.x != 1.0f || scale.y != 1.0f || scale.z != 1.0f) {
        JsonValue scaleJson;
        scaleJson.Set("x", JsonValue(static_cast<double>(scale.x)));
        scaleJson.Set("y", JsonValue(static_cast<double>(scale.y)));
        scaleJson.Set("z", JsonValue(static_cast<double>(scale.z)));
        json.Set("scale", scaleJson);
    }

    Vec2 anchor = node->GetAnchor();
    if (anchor.x != 0.5f || anchor.y != 0.5f) {
        json.Set("anchor", SerializeVec2(anchor));
    }

    json.Set("visible", JsonValue(node->IsVisible()));
    json.Set("zOrder", JsonValue(node->GetZOrder()));

    if (auto* widget = dynamic_cast<Widget*>(node)) {
        json.Set("enabled", JsonValue(widget->IsEnabled()));
        json.Set("touchEnabled", JsonValue(widget->IsTouchEnabled()));
    }

    auto* canvasPanel = dynamic_cast<CanvasPanel*>(node);
    if (canvasPanel && node->GetChildCount() > 0) {
        JsonValue childrenJson;
        for (size_t i = 0; i < node->GetChildCount(); ++i) {
            Node* child = node->GetChild(i);
            JsonValue childJson = SerializeNode(child);

            const FAnchorData* anchorData = canvasPanel->GetChildAnchor(
                dynamic_cast<Widget*>(child));
            if (anchorData) {
                JsonValue anchorJson;
                anchorJson.Set("min", SerializeVec2(anchorData->AnchorMin));
                anchorJson.Set("max", SerializeVec2(anchorData->AnchorMax));
                anchorJson.Set("alignment", SerializeVec2(anchorData->Alignment));
                anchorJson.Set("position", SerializeVec2(anchorData->Position));
                anchorJson.Set("size", SerializeVec2(anchorData->Size));
                childJson.Set("anchor", anchorJson);
            }

            childrenJson.PushBack(childJson);
        }
        json.Set("children", childrenJson);
    } else if (node->GetChildCount() > 0) {
        JsonValue childrenJson;
        for (size_t i = 0; i < node->GetChildCount(); ++i) {
            childrenJson.PushBack(SerializeNode(node->GetChild(i)));
        }
        json.Set("children", childrenJson);
    }

    return json;
}

Node* Serializer::DeserializeNode(const JsonValue& json) {
    std::string type = json.Get("type").AsString();
    std::string name = json.Get("name").AsString();

    std::unique_ptr<Node> node;

    if (type == "CanvasPanel") {
        node = std::make_unique<CanvasPanel>();
    } else if (type == "Button") {
        node = std::make_unique<Button>();
    } else if (type == "Label") {
        node = std::make_unique<Label>();
    } else if (type == "Layout") {
        node = std::make_unique<Layout>();
    } else if (type == "Sprite") {
        node = std::make_unique<Sprite>();
    } else {
        node = std::make_unique<Widget>();
    }

    node->SetName(name);

    const JsonValue& posJson = json.Get("position");
    node->SetPosition(Vec3(
        static_cast<float>(posJson.Get("x").AsNumber()),
        static_cast<float>(posJson.Get("y").AsNumber()),
        static_cast<float>(posJson.Get("z").AsNumber())
    ));

    const JsonValue& sizeJson = json.Get("size");
    node->SetContentSize(Vec2(
        static_cast<float>(sizeJson.Get("x").AsNumber()),
        static_cast<float>(sizeJson.Get("y").AsNumber())
    ));

    if (json.Get("scale").GetType() != JsonValue::Type::Null) {
        const JsonValue& s = json.Get("scale");
        node->SetScale(Vec3(
            static_cast<float>(s.Get("x").AsNumber()),
            static_cast<float>(s.Get("y").AsNumber()),
            static_cast<float>(s.Get("z").AsNumber())
        ));
    }

    if (json.Get("anchor").GetType() != JsonValue::Type::Null) {
        node->SetAnchor(DeserializeVec2(json.Get("anchor")));
    }

    node->SetVisible(json.Get("visible").AsBool());
    node->SetZOrder(json.Get("zOrder").AsInt());

    auto* widget = dynamic_cast<Widget*>(node.get());
    if (widget) {
        widget->SetEnabled(json.Get("enabled").AsBool());
        widget->SetTouchEnabled(json.Get("touchEnabled").AsBool());
    }

    if (json.Get("children").GetType() == JsonValue::Type::Array) {
        const auto& childrenArr = json.Get("children").GetArray();
        auto* canvasPanel = dynamic_cast<CanvasPanel*>(node.get());

        for (const auto& childJson : childrenArr) {
            Node* child = DeserializeNode(childJson);

            if (canvasPanel && childJson.Get("anchor").GetType() != JsonValue::Type::Null) {
                const JsonValue& anchorJson = childJson.Get("anchor");
                FAnchorData anchorData;
                anchorData.AnchorMin = DeserializeVec2(anchorJson.Get("min"));
                anchorData.AnchorMax = DeserializeVec2(anchorJson.Get("max"));
                anchorData.Alignment = DeserializeVec2(anchorJson.Get("alignment"));
                anchorData.Position = DeserializeVec2(anchorJson.Get("position"));
                anchorData.Size = DeserializeVec2(anchorJson.Get("size"));

                auto* childWidget = dynamic_cast<Widget*>(child);
                if (childWidget) {
                    canvasPanel->AddChildWithAnchor(
                        std::unique_ptr<Widget>(static_cast<Widget*>(child)), anchorData);
                    continue;
                }
            }

            node->AddChild(std::unique_ptr<Node>(child));
        }
    }

    return node.release();
}

JsonValue Serializer::SerializeScene(Node* root) {
    JsonValue scene;
    scene.Set("version", JsonValue(1));
    scene.Set("widgets", SerializeNode(root));
    return scene;
}

bool Serializer::DeserializeScene(Node* root, const JsonValue& json) {
    if (json.Get("version").AsInt() != 1) return false;
    const JsonValue& widgetsJson = json.Get("widgets");
    if (widgetsJson.GetType() != JsonValue::Type::Object) return false;

    while (root->GetChildCount() > 0) {
        root->RemoveChild(root->GetChild(0));
    }

    Node* loaded = DeserializeNode(widgetsJson);
    if (loaded) {
        while (loaded->GetChildCount() > 0) {
            root->AddChild(std::unique_ptr<Node>(loaded->RemoveChild(loaded->GetChild(0))));
        }
        delete loaded;
    }

    return true;
}

bool Serializer::SaveToFile(Node* root, const std::string& filepath) {
    JsonValue scene = SerializeScene(root);
    std::string jsonStr = scene.Serialize();

    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    file << jsonStr;
    return true;
}

Node* Serializer::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return nullptr;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    JsonValue json = JsonValue::Parse(content);
    if (json.Get("version").AsInt() != 1) return nullptr;

    const JsonValue& widgetsJson = json.Get("widgets");
    return DeserializeNode(widgetsJson);
}
