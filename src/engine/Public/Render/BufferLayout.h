#pragma once

#include <string>
#include <vector>
#include <glad/glad.h>

enum class ShaderDataType {
    None = 0,
    Float,
    Float2,
    Float3,
    Float4,
    Int,
    Int2,
    Int3,
    Int4,
    Mat3,
    Mat4
};

inline unsigned int ShaderDataTypeSize(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::None:   return 0;
        case ShaderDataType::Float:  return sizeof(float);
        case ShaderDataType::Float2: return sizeof(float) * 2;
        case ShaderDataType::Float3: return sizeof(float) * 3;
        case ShaderDataType::Float4: return sizeof(float) * 4;
        case ShaderDataType::Int:    return sizeof(int);
        case ShaderDataType::Int2:   return sizeof(int) * 2;
        case ShaderDataType::Int3:   return sizeof(int) * 3;
        case ShaderDataType::Int4:   return sizeof(int) * 4;
        case ShaderDataType::Mat3:   return sizeof(float) * 3 * 3;
        case ShaderDataType::Mat4:   return sizeof(float) * 4 * 4;
    }
    return 0;
}

inline unsigned int ShaderDataTypeComponentCount(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::None:   return 0;
        case ShaderDataType::Float:  return 1;
        case ShaderDataType::Float2: return 2;
        case ShaderDataType::Float3: return 3;
        case ShaderDataType::Float4: return 4;
        case ShaderDataType::Int:    return 1;
        case ShaderDataType::Int2:   return 2;
        case ShaderDataType::Int3:   return 3;
        case ShaderDataType::Int4:   return 4;
        case ShaderDataType::Mat3:   return 3;
        case ShaderDataType::Mat4:   return 4;
    }
    return 0;
}

inline GLenum ShaderDataTypeToGLType(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float:
        case ShaderDataType::Float2:
        case ShaderDataType::Float3:
        case ShaderDataType::Float4:
        case ShaderDataType::Mat3:
        case ShaderDataType::Mat4:
            return GL_FLOAT;
        case ShaderDataType::Int:
        case ShaderDataType::Int2:
        case ShaderDataType::Int3:
        case ShaderDataType::Int4:
            return GL_INT;
    }
    return 0;
}

struct BufferElement {
    std::string Name;
    ShaderDataType Type;
    unsigned int Size;
    unsigned int Offset;
    bool Normalized;

    BufferElement(ShaderDataType type, std::string name, bool normalized = false)
        : Name(std::move(name))
        , Type(type)
        , Size(ShaderDataTypeSize(type))
        , Offset(0)
        , Normalized(normalized)
    {
    }
};

class BufferLayout {
public:
    BufferLayout() = default;

    BufferLayout(std::initializer_list<BufferElement> elements)
        : m_elements(elements)
    {
        CalculateOffsetsAndStride();
    }

    unsigned int GetStride() const { return m_stride; }
    const std::vector<BufferElement>& GetElements() const { return m_elements; }

private:
    void CalculateOffsetsAndStride() {
        m_stride = 0;
        for (BufferElement& element : m_elements) {
            element.Offset = m_stride;
            m_stride += element.Size;
        }
    }

    std::vector<BufferElement> m_elements;
    unsigned int m_stride = 0;
};
