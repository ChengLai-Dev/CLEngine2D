#include "UI/JsonValue.h"
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
