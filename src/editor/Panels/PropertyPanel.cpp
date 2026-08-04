#include "PropertyPanel.h"
#include "TextEditBox.h"
#include "PropertyFieldRegistry.h"
#include "UndoRedo.h"
#include "FileDialog.h"
#include <SceneGraph/Node.h>
#include <Render/Renderer.h>
#include <TextRenderer.h>
#include <Input/RawInput.h>
#include <Input/InputCodes.h>
#include <algorithm>
#include <format>

static constexpr float FIELD_HEIGHT = 24.0f;
static constexpr float LABEL_LEFT = 8.0f;
static constexpr float VALUE_LEFT = 102.0f;
static constexpr float VALUE_RIGHT = 290.0f;
static constexpr float VALUE_PADDING = 6.0f;
static constexpr float SECTION_TOP_MARGIN = 12.0f;
static constexpr float CONTENT_TOP = 44.0f;
static constexpr float TEXTURE_BUTTON_WIDTH = 20.0f;
static constexpr float ENUM_BUTTON_WIDTH = 20.0f;

PropertyPanel::PropertyPanel() {
    m_rectWidth = 300.0f;
    m_rectHeight = 720.0f;
}

PropertyPanel::~PropertyPanel() = default;

void PropertyPanel::SetTarget(Node* target) {
    if (m_target == target) return;
    if (m_activeFieldIndex >= 0) {
        m_fields[m_activeFieldIndex].editBox.Deactivate();
        m_activeFieldIndex = -1;
    }
    m_target = target;
    m_scrollOffset = 0.0f;
    m_openEnumField = -1;
    m_fields.clear();
    BuildFields();
}

Node* PropertyPanel::GetTarget() const {
    return m_target;
}

void PropertyPanel::OnPropertyChanged(PropertyChangedCallback cb) {
    m_onPropertyChanged = std::move(cb);
}

void PropertyPanel::OnNameChanged(std::function<void()> cb) {
    m_onNameChanged = std::move(cb);
}

void PropertyPanel::BuildFields() {
    m_fields.clear();
    if (!m_target) return;

    float y = CONTENT_TOP;

    for (const PropertySectionDef& section : PropertyFieldRegistry::GetAll()) {
        if (section.condition && !section.condition(m_target)) continue;

        y += SECTION_TOP_MARGIN;
        FieldInfo headerInfo;
        headerInfo.type = FieldType::Float;
        headerInfo.label = section.title;
        headerInfo.virtualY = y;
        headerInfo.getter = nullptr;
        headerInfo.setter = nullptr;
        m_fields.push_back(std::move(headerInfo));
        y += FIELD_HEIGHT;

        for (const PropertyFieldDef& field : section.fields) {
            FieldInfo info;
            info.type = field.type;
            info.label = field.label;
            info.virtualY = y;
            info.minVal = field.minVal;
            info.maxVal = field.maxVal;
            info.options = field.options;
            info.readOnly = field.readOnly;
            info.getter = [this, &field]() { return field.getter(m_target); };
            if (field.setter) {
                info.setter = [this, &field](const std::string& v) { field.setter(m_target, v); };
            }
            m_fields.push_back(std::move(info));
            y += FIELD_HEIGHT;
        }
    }

    m_contentHeight = y;
}

void PropertyPanel::DrawSectionHeader(Renderer& renderer, const char* title, float y) {
    if (!m_fontRenderer) return;
    if (y + FIELD_HEIGHT < 0.0f || y > m_rectHeight) return;

    float headerColor[4] = { 0.9f, 0.6f, 0.2f, 1.0f };
    float textH = m_fontRenderer->GetLineHeight(1.0f);
    float base = m_fontRenderer->GetBaselineOffset(1.0f);
    float centerY = y + (FIELD_HEIGHT - textH) * 0.5f + base;
    m_fontRenderer->RenderString(renderer, title,
        10.0f, centerY, 1.0f, headerColor, TextRenderer::Align::Left);
}

void PropertyPanel::DrawFieldLabel(Renderer& renderer, const char* label, float y) {
    if (!m_fontRenderer || !label) return;

    float labelColor[4] = { 0.6f, 0.6f, 0.6f, 1.0f };
    float textH = m_fontRenderer->GetLineHeight(1.0f);
    float base = m_fontRenderer->GetBaselineOffset(1.0f);
    float centerY = y + (FIELD_HEIGHT - textH) * 0.5f + base;
    m_fontRenderer->RenderString(renderer, label,
        LABEL_LEFT + 2.0f, centerY, 1.0f, labelColor, TextRenderer::Align::Left);
}

void PropertyPanel::DrawFieldBackground(Renderer& renderer, float y, bool isActive, bool isBool) {
    float bgColor[4];
    if (isActive) {
        bgColor[0] = 0.18f; bgColor[1] = 0.35f; bgColor[2] = 0.55f; bgColor[3] = 1.0f;
    } else if (isBool) {
        bgColor[0] = 0.16f; bgColor[1] = 0.16f; bgColor[2] = 0.18f; bgColor[3] = 1.0f;
    } else {
        bgColor[0] = 0.13f; bgColor[1] = 0.13f; bgColor[2] = 0.15f; bgColor[3] = 1.0f;
    }

    float cx = (VALUE_LEFT + VALUE_RIGHT) * 0.5f;
    float cy = y + FIELD_HEIGHT * 0.5f;
    float cw = VALUE_RIGHT - VALUE_LEFT;
    renderer.DrawQuad(Vec3(cx, cy, 0.0f),
                      Vec3(cw, FIELD_HEIGHT - 2.0f, 1.0f),
                      Color(bgColor[0], bgColor[1], bgColor[2], bgColor[3]));
}

void PropertyPanel::DrawFields(Renderer& renderer) {
    if (!m_fontRenderer) return;

    for (int i = 0; i < static_cast<int>(m_fields.size()); ++i) {
        FieldInfo& field = m_fields[i];
        float renderY = field.virtualY - m_scrollOffset;

        if (renderY + FIELD_HEIGHT < 0.0f || renderY > m_rectHeight) continue;

        bool isSectionHeader = (field.getter == nullptr);

        if (isSectionHeader) {
            DrawSectionHeader(renderer, field.label.c_str(), renderY);
            continue;
        }

        bool isActive = (i == m_activeFieldIndex);
        bool isBoolType = (field.type == FieldType::Bool);
        bool isEnumType = (field.type == FieldType::Enum);

        DrawFieldLabel(renderer, field.label.c_str(), renderY);

        if (field.readOnly) {
            DrawFieldBackground(renderer, renderY, false, true);
            std::string text = field.getter ? field.getter() : "";
            if (!text.empty() && m_fontRenderer) {
                float textH = m_fontRenderer->GetLineHeight(1.0f);
                float base = m_fontRenderer->GetBaselineOffset(1.0f);
                float centerY = renderY + (FIELD_HEIGHT - textH) * 0.5f + base;
                float valColor[4] = { 0.55f, 0.55f, 0.58f, 1.0f };
                m_fontRenderer->RenderString(renderer, text,
                    VALUE_LEFT + VALUE_PADDING, centerY, 1.0f, valColor, TextRenderer::Align::Left);
            }
        } else if (isEnumType) {
            bool isOpen = (i == m_openEnumField);
            DrawFieldBackground(renderer, renderY, isOpen, true);
            std::string text = field.getter ? field.getter() : "";
            if (!text.empty() && m_fontRenderer) {
                float textH = m_fontRenderer->GetLineHeight(1.0f);
                float base = m_fontRenderer->GetBaselineOffset(1.0f);
                float centerY = renderY + (FIELD_HEIGHT - textH) * 0.5f + base;
                float valColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
                m_fontRenderer->RenderString(renderer, text,
                    VALUE_LEFT + VALUE_PADDING, centerY, 1.0f, valColor, TextRenderer::Align::Left);
            }
            float btnLeft = VALUE_RIGHT - ENUM_BUTTON_WIDTH;
            float btnColor[4] = { 0.25f, 0.25f, 0.28f, 1.0f };
            float btnCx = btnLeft + ENUM_BUTTON_WIDTH * 0.5f;
            float btnCy = renderY + FIELD_HEIGHT * 0.5f;
            renderer.DrawQuad(Vec3(btnCx, btnCy, 0.0f),
                              Vec3(ENUM_BUTTON_WIDTH - 1.0f, FIELD_HEIGHT - 2.0f, 1.0f),
                              Color(btnColor[0], btnColor[1], btnColor[2], btnColor[3]));
            if (m_fontRenderer) {
                float txtColor[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
                float textH = m_fontRenderer->GetLineHeight(1.0f);
                float base = m_fontRenderer->GetBaselineOffset(1.0f);
                float txtY = renderY + (FIELD_HEIGHT - textH) * 0.5f + base;
                m_fontRenderer->RenderString(renderer, "▾",
                    btnLeft + 2.0f, txtY, 1.0f, txtColor, TextRenderer::Align::Left);
            }
            if (isOpen) {
                int optIdx = 0;
                for (const std::string& opt : field.options) {
                    float optY = renderY + FIELD_HEIGHT + optIdx * FIELD_HEIGHT;
                    if (optY + FIELD_HEIGHT < 0.0f || optY > m_rectHeight) { ++optIdx; continue; }
                    bool isCurrent = (opt == text);
                    float optBg[4];
                    if (isCurrent) {
                        optBg[0] = 0.18f; optBg[1] = 0.35f; optBg[2] = 0.55f; optBg[3] = 1.0f;
                    } else {
                        optBg[0] = 0.16f; optBg[1] = 0.16f; optBg[2] = 0.18f; optBg[3] = 1.0f;
                    }
                    float optCx = (VALUE_LEFT + VALUE_RIGHT) * 0.5f;
                    float optCy = optY + FIELD_HEIGHT * 0.5f;
                    renderer.DrawQuad(Vec3(optCx, optCy, 0.0f),
                                      Vec3(VALUE_RIGHT - VALUE_LEFT, FIELD_HEIGHT - 1.0f, 1.0f),
                                      Color(optBg[0], optBg[1], optBg[2], optBg[3]));
                    if (m_fontRenderer) {
                        float textH = m_fontRenderer->GetLineHeight(1.0f);
                        float base = m_fontRenderer->GetBaselineOffset(1.0f);
                        float centerY = optY + (FIELD_HEIGHT - textH) * 0.5f + base;
                        float optColor[4];
                        if (isCurrent) {
                            optColor[0] = 1.0f; optColor[1] = 1.0f; optColor[2] = 1.0f; optColor[3] = 1.0f;
                        } else {
                            optColor[0] = 0.75f; optColor[1] = 0.75f; optColor[2] = 0.78f; optColor[3] = 1.0f;
                        }
                        m_fontRenderer->RenderString(renderer, opt,
                            VALUE_LEFT + VALUE_PADDING, centerY, 1.0f, optColor, TextRenderer::Align::Left);
                    }
                    ++optIdx;
                }
            }
        } else if (isBoolType) {
            DrawFieldBackground(renderer, renderY, false, true);
            std::string text = field.getter ? field.getter() : "";
            if (!text.empty() && m_fontRenderer) {
                float textH = m_fontRenderer->GetLineHeight(1.0f);
                float base = m_fontRenderer->GetBaselineOffset(1.0f);
                float centerY = renderY + (FIELD_HEIGHT - textH) * 0.5f + base;
                float valColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
                m_fontRenderer->RenderString(renderer, text,
                    VALUE_LEFT + VALUE_PADDING, centerY, 1.0f, valColor, TextRenderer::Align::Left);
            }
        } else if (field.type == FieldType::TextureAsset) {
            float editWidth = VALUE_RIGHT - VALUE_LEFT - TEXTURE_BUTTON_WIDTH;
            if (isActive) {
                field.editBox.Draw(renderer, m_fontRenderer,
                    VALUE_LEFT, renderY,
                    editWidth, FIELD_HEIGHT, true);
            } else {
                field.editBox.SetValue(field.getter ? field.getter() : "");
                field.editBox.Draw(renderer, m_fontRenderer,
                    VALUE_LEFT, renderY,
                    editWidth, FIELD_HEIGHT, false);
            }
            float btnLeft = VALUE_LEFT + editWidth;
            float btnColor[4] = { 0.25f, 0.25f, 0.28f, 1.0f };
            float btnCx = btnLeft + TEXTURE_BUTTON_WIDTH * 0.5f;
            float btnCy = renderY + FIELD_HEIGHT * 0.5f;
            renderer.DrawQuad(Vec3(btnCx, btnCy, 0.0f),
                              Vec3(TEXTURE_BUTTON_WIDTH - 1.0f, FIELD_HEIGHT - 2.0f, 1.0f),
                              Color(btnColor[0], btnColor[1], btnColor[2], btnColor[3]));
            if (m_fontRenderer) {
                float txtColor[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
                float textH = m_fontRenderer->GetLineHeight(1.0f);
                float base = m_fontRenderer->GetBaselineOffset(1.0f);
                float txtY = renderY + (FIELD_HEIGHT - textH) * 0.5f + base;
                m_fontRenderer->RenderString(renderer, "...",
                    btnLeft + 2.0f, txtY, 1.0f, txtColor, TextRenderer::Align::Left);
            }
        } else {
            if (isActive) {
                field.editBox.Draw(renderer, m_fontRenderer,
                    VALUE_LEFT, renderY,
                    VALUE_RIGHT - VALUE_LEFT, FIELD_HEIGHT, true);
            } else {
                field.editBox.SetValue(field.getter ? field.getter() : "");
                field.editBox.Draw(renderer, m_fontRenderer,
                    VALUE_LEFT, renderY,
                    VALUE_RIGHT - VALUE_LEFT, FIELD_HEIGHT, false);
            }
        }
    }
}

void PropertyPanel::OnRender(Renderer& renderer) {
    Color bgColor(0.12f, 0.12f, 0.14f, 1.0f);
    renderer.DrawQuad(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f),
                      Vec3(m_rectWidth, m_rectHeight, 1.0f), bgColor);

    if (!m_target || !m_fontRenderer) return;

    std::string titleStr = std::format("{}  ({:.0f} x {:.0f})",
        m_target->GetName(),
        m_target->GetContentSize().x,
        m_target->GetContentSize().y);

    float titleColor[4] = { 0.9f, 0.6f, 0.2f, 1.0f };
    m_fontRenderer->RenderString(renderer, titleStr,
        10.0f, 20.0f, 1.0f, titleColor, TextRenderer::Align::Left);

    DrawFields(renderer);
}

bool PropertyPanel::OnMouseEvent(const MouseEvent& event) {
    switch (event.type) {
        case MouseEvent::Press: {
            if (event.button != MouseEvent::Left) return false;

            float localX = event.screenPos.x - m_rectLeft;
            float localY = event.screenPos.y - m_rectTop;

            if (m_openEnumField >= 0) {
                const FieldInfo& openField = m_fields[m_openEnumField];
                float fieldRenderY = openField.virtualY - m_scrollOffset;
                int optIdx = 0;
                bool hitOption = false;
                for (const std::string& opt : openField.options) {
                    float optY = fieldRenderY + FIELD_HEIGHT + optIdx * FIELD_HEIGHT;
                    if (localX >= VALUE_LEFT && localX <= VALUE_RIGHT &&
                        localY >= optY && localY < optY + FIELD_HEIGHT) {
                        hitOption = true;
                        if (m_activeFieldIndex >= 0) CommitEdit();
                        std::string oldVal = openField.getter ? openField.getter() : "";
                        if (oldVal != opt) {
                            auto setter = openField.setter;
                            UndoRedoStack::GetInstance().ExecuteCommand(
                                std::make_unique<PropertyChangeCommand>(
                                    "Set " + openField.label,
                                    [setter, opt]() { setter(opt); },
                                    [setter, oldVal]() { setter(oldVal); }
                                ));
                            if (m_onPropertyChanged) m_onPropertyChanged();
                        }
                        m_openEnumField = -1;
                        return true;
                    }
                    ++optIdx;
                }
                if (!hitOption) {
                    m_openEnumField = -1;
                }
            }

            for (int i = 0; i < static_cast<int>(m_fields.size()); ++i) {
                float fieldRenderY = m_fields[i].virtualY - m_scrollOffset;
                if (localX >= LABEL_LEFT && localX <= VALUE_RIGHT &&
                    localY >= fieldRenderY && localY < fieldRenderY + FIELD_HEIGHT) {

                    if (m_fields[i].readOnly) {
                        return true;
                    }

                    if (m_fields[i].type == FieldType::Bool) {
                        if (m_activeFieldIndex >= 0) CommitEdit();
                        std::string oldVal = m_fields[i].getter ? m_fields[i].getter() : "";
                        std::string newVal = (oldVal == "True") ? "False" : "True";
                        auto setter = m_fields[i].setter;
                        if (setter) {
                            UndoRedoStack::GetInstance().ExecuteCommand(
                                std::make_unique<PropertyChangeCommand>(
                                    "Set " + m_fields[i].label,
                                    [setter, newVal]() { setter(newVal); },
                                    [setter, oldVal]() { setter(oldVal); }
                                ));
                        }
                        if (m_onPropertyChanged) m_onPropertyChanged();
                        return true;
                    }

                    if (m_fields[i].type == FieldType::TextureAsset) {
                        float btnLeft = VALUE_RIGHT - TEXTURE_BUTTON_WIDTH;
                        if (localX >= btnLeft && localX <= VALUE_RIGHT) {
                            if (m_activeFieldIndex >= 0) CommitEdit();
                            std::string result = FileDialog::OpenFile(
                                "Select Texture",
                                "Image Files\0*.png;*.jpg;*.bmp;*.tga\0All Files\0*.*\0",
                                m_parentHwnd);
                            if (!result.empty()) {
                                std::string oldVal = m_fields[i].getter ? m_fields[i].getter() : "";
                                auto setter = m_fields[i].setter;
                                if (setter) {
                                    UndoRedoStack::GetInstance().ExecuteCommand(
                                        std::make_unique<PropertyChangeCommand>(
                                            "Set " + m_fields[i].label,
                                            [setter, result]() { setter(result); },
                                            [setter, oldVal]() { setter(oldVal); }
                                        ));
                                }
                                if (m_onPropertyChanged) m_onPropertyChanged();
                            }
                            return true;
                        }
                    }

                    if (m_fields[i].type == FieldType::Enum) {
                        if (m_activeFieldIndex >= 0) CommitEdit();
                        m_openEnumField = (m_openEnumField == i) ? -1 : i;
                        return true;
                    }

                    if (i == m_activeFieldIndex) {
                        m_fields[i].editBox.OnMouseDown(m_fontRenderer, localX, VALUE_LEFT, VALUE_PADDING);
                    } else {
                        if (m_activeFieldIndex >= 0) CommitEdit();
                        StartEdit(i);
                    }
                    return true;
                }
            }

            if (m_activeFieldIndex >= 0) CancelEdit();
            return false;
        }

        case MouseEvent::Move: {
            if (m_activeFieldIndex >= 0 && event.button == MouseEvent::Left) {
                float localX = event.screenPos.x - m_rectLeft;
                m_fields[m_activeFieldIndex].editBox.OnMouseDrag(
                    m_fontRenderer, localX, VALUE_LEFT, VALUE_PADDING);
                return true;
            }
            return false;
        }

        case MouseEvent::Release: {
            if (m_activeFieldIndex >= 0) {
                m_fields[m_activeFieldIndex].editBox.OnMouseRelease();
            }
            return false;
        }

        case MouseEvent::Scroll: {
            if (m_openEnumField >= 0) {
                m_openEnumField = -1;
            }
            float scrollAmount = event.scrollDelta * 36.0f;
            m_scrollOffset -= scrollAmount;

            float maxScroll = (std::max)(0.0f, m_contentHeight - m_rectHeight);
            m_scrollOffset = (std::max)(0.0f, (std::min)(m_scrollOffset, maxScroll));
            return true;
        }

        default:
            return false;
    }
}

void PropertyPanel::OnUpdate(float deltaTime) {
    if (m_activeFieldIndex < 0) {
        RawInput::ConsumeCharBuffer();
        return;
    }

    m_fields[m_activeFieldIndex].editBox.OnUpdate(deltaTime);

    if (RawInput::IsKeyPressed(KeyCode::Enter)) {
        CommitEdit();
        return;
    }

    if (RawInput::IsKeyPressed(KeyCode::Escape)) {
        CancelEdit();
        return;
    }
}

void PropertyPanel::StartEdit(int fieldIndex) {
    if (fieldIndex < 0 || fieldIndex >= static_cast<int>(m_fields.size())) return;

    m_activeFieldIndex = fieldIndex;
    m_fields[fieldIndex].editBox.Activate(
        m_fields[fieldIndex].getter ? m_fields[fieldIndex].getter() : "");
}

void PropertyPanel::CommitEdit() {
    if (m_activeFieldIndex < 0 || m_activeFieldIndex >= static_cast<int>(m_fields.size())) {
        m_activeFieldIndex = -1;
        return;
    }

    auto& field = m_fields[m_activeFieldIndex];
    TextEditBox& eb = field.editBox;
    const std::string& newValue = eb.GetValue();
    const std::string& oldValue = eb.GetOldValue();

    if (newValue != oldValue) {
        auto setter = field.setter;
        if (setter) {
            UndoRedoStack::GetInstance().ExecuteCommand(
                std::make_unique<PropertyChangeCommand>(
                    "Set " + field.label,
                    [setter, newValue]() { setter(newValue); },
                    [setter, oldValue]() { setter(oldValue); }
                ));
        }

        if (field.label == "Name" && m_onNameChanged) {
            m_onNameChanged();
        }
        if (m_onPropertyChanged) m_onPropertyChanged();
    }

    eb.Deactivate();
    m_activeFieldIndex = -1;
}

void PropertyPanel::CancelEdit() {
    if (m_activeFieldIndex >= 0 && m_activeFieldIndex < static_cast<int>(m_fields.size())) {
        m_fields[m_activeFieldIndex].editBox.Revert();
        m_fields[m_activeFieldIndex].editBox.Deactivate();
    }

    m_activeFieldIndex = -1;
}
