#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>

class JsonValue {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    JsonValue() : m_type(Type::Null), m_data(false) {}
    JsonValue(bool v) : m_type(Type::Bool), m_data(v) {}
    JsonValue(double v) : m_type(Type::Number), m_data(v) {}
    JsonValue(int v) : m_type(Type::Number), m_data(static_cast<double>(v)) {}
    JsonValue(const std::string& v) : m_type(Type::String), m_data(v) {}
    JsonValue(const char* v) : m_type(Type::String), m_data(std::string(v)) {}

    Type GetType() const { return m_type; }

    bool AsBool() const;
    double AsNumber() const;
    int AsInt() const;
    const std::string& AsString() const;

    JsonValue& operator[](const std::string& key);
    JsonValue& operator[](size_t index);
    const JsonValue& Get(const std::string& key) const;

    void PushBack(const JsonValue& val);
    void Set(const std::string& key, const JsonValue& val);

    size_t Size() const;
    const JsonValue& operator[](size_t index) const;

    using ObjectType = std::unordered_map<std::string, JsonValue>;
    using ArrayType = std::vector<JsonValue>;

    const ObjectType& GetObject() const;
    const ArrayType& GetArray() const;

    std::string Serialize(int indent = 0) const;

    static JsonValue Parse(const std::string& json);

private:
    Type m_type;
    std::variant<bool, double, std::string, ArrayType, ObjectType> m_data;
};
