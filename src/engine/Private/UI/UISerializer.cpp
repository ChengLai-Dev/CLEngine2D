#include "UI/UISerializer.h"
#include "SceneGraph/Node.h"
#include "SceneGraph/Widget.h"
#include "SceneGraph/Button.h"
#include "SceneGraph/Label.h"
#include "SceneGraph/Sprite.h"
#include "SceneGraph/Image.h"
#include "SceneGraph/CanvasPanel.h"
#include "SceneGraph/Layout.h"
#include "Render/Texture.h"
#include "AssetManager.h"
#include <fstream>
#include <sstream>

// ========== Helper serialization utilities ==========

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
    if (dynamic_cast<Image*>(node)) return "Image";
    if (dynamic_cast<Sprite*>(node)) return "Sprite";
    return "Widget";
}

static void SetTextureField(const JsonValue& json, const std::string& key,
    const std::function<void(std::shared_ptr<Texture>)>& setter)
{
    if (json.Get(key).GetType() != JsonValue::Type::Null) {
        std::string path = json.Get(key).AsString();
        if (!path.empty()) {
            auto tex = AssetManager::GetInstance().LoadTexture(path);
            if (tex) setter(tex);
        }
    }
}

// ========== Serialization ==========

JsonValue UISerializer::SerializeNode(Node* node) {
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

    json.Set("opacity", JsonValue(static_cast<double>(node->GetOpacity())));

    if (node->GetColor().x != 1.0f || node->GetColor().y != 1.0f ||
        node->GetColor().z != 1.0f || node->GetColor().w != 1.0f) {
        json.Set("color", SerializeVec4(node->GetColor()));
    }

    if (Widget* widget = dynamic_cast<Widget*>(node)) {
        json.Set("enabled", JsonValue(widget->IsEnabled()));
        json.Set("touchEnabled", JsonValue(widget->IsTouchEnabled()));
        if (widget->IsFocusable()) {
            json.Set("focusable", JsonValue(true));
        }
    }

    if (Label* label = dynamic_cast<Label*>(node)) {
        if (!label->GetText().empty()) json.Set("text", JsonValue(label->GetText()));
        if (label->GetFontSize() != 16.0f) json.Set("fontSize", JsonValue(static_cast<double>(label->GetFontSize())));
        if (label->GetTextColor().x != 1.0f || label->GetTextColor().y != 1.0f ||
            label->GetTextColor().z != 1.0f || label->GetTextColor().w != 1.0f) {
            json.Set("textColor", SerializeVec4(label->GetTextColor()));
        }
        auto bg = label->GetBackground();
        if (bg && !bg->GetFilePath().empty()) {
            json.Set("background", JsonValue(bg->GetFilePath()));
        }
    }

    if (Button* btn = dynamic_cast<Button*>(node)) {
        if (!btn->GetText().empty()) json.Set("text", JsonValue(btn->GetText()));
        if (btn->GetFontSize() != 16.0f) json.Set("fontSize", JsonValue(static_cast<double>(btn->GetFontSize())));
        if (btn->GetTextColor().x != 1.0f || btn->GetTextColor().y != 1.0f ||
            btn->GetTextColor().z != 1.0f || btn->GetTextColor().w != 1.0f) {
            json.Set("textColor", SerializeVec4(btn->GetTextColor()));
        }
        auto normalTex = btn->GetNormalImage();
        if (normalTex && !normalTex->GetFilePath().empty()) json.Set("normalTex", JsonValue(normalTex->GetFilePath()));
        auto pressedTex = btn->GetPressedImage();
        if (pressedTex && !pressedTex->GetFilePath().empty()) json.Set("pressedTex", JsonValue(pressedTex->GetFilePath()));
        auto disabledTex = btn->GetDisabledImage();
        if (disabledTex && !disabledTex->GetFilePath().empty()) json.Set("disabledTex", JsonValue(disabledTex->GetFilePath()));
    }

    if (Sprite* sp = dynamic_cast<Sprite*>(node)) {
        auto tex = sp->GetTexture();
        if (tex && !tex->GetFilePath().empty()) json.Set("texture", JsonValue(tex->GetFilePath()));
        if (sp->GetTexOffsetX() != 0.0f || sp->GetTexOffsetY() != 0.0f) {
            json.Set("texOffset", SerializeVec2(Vec2(sp->GetTexOffsetX(), sp->GetTexOffsetY())));
        }
        if (sp->GetTexScaleX() != 1.0f || sp->GetTexScaleY() != 1.0f) {
            json.Set("texScale", SerializeVec2(Vec2(sp->GetTexScaleX(), sp->GetTexScaleY())));
        }
    }

    if (Image* img = dynamic_cast<Image*>(node)) {
        if (img->IsScale9Enabled()) {
            json.Set("scale9Enabled", JsonValue(true));
            json.Set("capInsets", SerializeVec4(img->GetCapInsets()));
        }
    }

    if (Layout* layout = dynamic_cast<Layout*>(node)) {
        std::string layoutTypeStr;
        switch (layout->GetLayoutType()) {
            case Layout::Type::VERTICAL: layoutTypeStr = "VERTICAL"; break;
            case Layout::Type::HORIZONTAL: layoutTypeStr = "HORIZONTAL"; break;
            case Layout::Type::GRID: layoutTypeStr = "GRID"; break;
        }
        json.Set("layoutType", JsonValue(layoutTypeStr));
        if (layout->GetSpacing() != 4.0f) json.Set("spacing", JsonValue(static_cast<double>(layout->GetSpacing())));
        if (layout->GetPadding().x != 0.0f || layout->GetPadding().y != 0.0f ||
            layout->GetPadding().z != 0.0f || layout->GetPadding().w != 0.0f) {
            json.Set("padding", SerializeVec4(layout->GetPadding()));
        }
        if (layout->GetGridColumns() != 2) json.Set("gridColumns", JsonValue(layout->GetGridColumns()));
    }

    CanvasPanel* canvasPanel = dynamic_cast<CanvasPanel*>(node);
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

// ========== Deserialization ==========

Node* UISerializer::DeserializeNode(const JsonValue& json) {
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
    } else if (type == "Image") {
        node = std::make_unique<Image>();
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

    if (json.Get("opacity").GetType() != JsonValue::Type::Null) {
        node->SetOpacity(static_cast<float>(json.Get("opacity").AsNumber()));
    }

    if (json.Get("color").GetType() != JsonValue::Type::Null) {
        node->SetColor(DeserializeVec4(json.Get("color")));
    }

    Widget* widget = dynamic_cast<Widget*>(node.get());
    if (widget) {
        widget->SetEnabled(json.Get("enabled").AsBool());
        widget->SetTouchEnabled(json.Get("touchEnabled").AsBool());
        if (json.Get("focusable").GetType() != JsonValue::Type::Null) {
            widget->SetFocusable(json.Get("focusable").AsBool());
        }
    }

    if (Label* label = dynamic_cast<Label*>(node.get())) {
        if (json.Get("text").GetType() != JsonValue::Type::Null)
            label->SetText(json.Get("text").AsString());
        if (json.Get("fontSize").GetType() != JsonValue::Type::Null)
            label->SetFontSize(static_cast<float>(json.Get("fontSize").AsNumber()));
        if (json.Get("textColor").GetType() != JsonValue::Type::Null)
            label->SetTextColor(DeserializeVec4(json.Get("textColor")));
        SetTextureField(json, "background", [label](auto tex) { label->SetBackground(tex); });
    }

    if (Button* btn = dynamic_cast<Button*>(node.get())) {
        if (json.Get("text").GetType() != JsonValue::Type::Null)
            btn->SetText(json.Get("text").AsString());
        if (json.Get("fontSize").GetType() != JsonValue::Type::Null)
            btn->SetFontSize(static_cast<float>(json.Get("fontSize").AsNumber()));
        if (json.Get("textColor").GetType() != JsonValue::Type::Null)
            btn->SetTextColor(DeserializeVec4(json.Get("textColor")));
        SetTextureField(json, "normalTex", [btn](auto tex) { btn->SetNormalImage(tex); });
        SetTextureField(json, "pressedTex", [btn](auto tex) { btn->SetPressedImage(tex); });
        SetTextureField(json, "disabledTex", [btn](auto tex) { btn->SetDisabledImage(tex); });
    }

    if (Sprite* sp = dynamic_cast<Sprite*>(node.get())) {
        SetTextureField(json, "texture", [sp](auto tex) { sp->SetTexture(tex); });
        if (json.Get("texOffset").GetType() != JsonValue::Type::Null) {
            Vec2 off = DeserializeVec2(json.Get("texOffset"));
            sp->SetTexOffset(off.x, off.y);
        }
        if (json.Get("texScale").GetType() != JsonValue::Type::Null) {
            Vec2 sc = DeserializeVec2(json.Get("texScale"));
            sp->SetTexScale(sc.x, sc.y);
        }
    }

    if (Image* img = dynamic_cast<Image*>(node.get())) {
        if (json.Get("scale9Enabled").GetType() != JsonValue::Type::Null) {
            img->SetScale9Enabled(json.Get("scale9Enabled").AsBool());
        }
        if (json.Get("capInsets").GetType() != JsonValue::Type::Null) {
            img->SetCapInsets(DeserializeVec4(json.Get("capInsets")));
        }
    }

    if (Layout* layout = dynamic_cast<Layout*>(node.get())) {
        if (json.Get("layoutType").GetType() != JsonValue::Type::Null) {
            std::string lt = json.Get("layoutType").AsString();
            if (lt == "VERTICAL") layout->SetLayoutType(Layout::Type::VERTICAL);
            else if (lt == "HORIZONTAL") layout->SetLayoutType(Layout::Type::HORIZONTAL);
            else if (lt == "GRID") layout->SetLayoutType(Layout::Type::GRID);
        }
        if (json.Get("spacing").GetType() != JsonValue::Type::Null)
            layout->SetSpacing(static_cast<float>(json.Get("spacing").AsNumber()));
        if (json.Get("padding").GetType() != JsonValue::Type::Null)
            layout->SetPadding(DeserializeVec4(json.Get("padding")));
        if (json.Get("gridColumns").GetType() != JsonValue::Type::Null)
            layout->SetGridColumns(json.Get("gridColumns").AsInt());
    }

    if (json.Get("children").GetType() == JsonValue::Type::Array) {
        const auto& childrenArr = json.Get("children").GetArray();
        CanvasPanel* canvasPanel = dynamic_cast<CanvasPanel*>(node.get());

        for (const JsonValue& childJson : childrenArr) {
            Node* child = DeserializeNode(childJson);

            if (canvasPanel && childJson.Get("anchor").GetType() != JsonValue::Type::Null) {
                const JsonValue& anchorJson = childJson.Get("anchor");
                FAnchorData anchorData;
                anchorData.AnchorMin = DeserializeVec2(anchorJson.Get("min"));
                anchorData.AnchorMax = DeserializeVec2(anchorJson.Get("max"));
                anchorData.Alignment = DeserializeVec2(anchorJson.Get("alignment"));
                anchorData.Position = DeserializeVec2(anchorJson.Get("position"));
                anchorData.Size = DeserializeVec2(anchorJson.Get("size"));

                Widget* childWidget = dynamic_cast<Widget*>(child);
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

JsonValue UISerializer::SerializeScene(Node* root) {
    JsonValue scene;
    scene.Set("version", JsonValue(1));
    scene.Set("widgets", SerializeNode(root));
    return scene;
}

bool UISerializer::DeserializeScene(Node* root, const JsonValue& json) {
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

bool UISerializer::SaveToFile(Node* root, const std::string& filepath) {
    JsonValue scene = SerializeScene(root);
    std::string jsonStr = scene.Serialize();

    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    file << jsonStr;
    return true;
}

Node* UISerializer::LoadFromFile(const std::string& filepath) {
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
