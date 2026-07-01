#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>

struct Mat4;

class Shader {
public:
    Shader(const std::string& filepath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetMat4(const std::string& name, const Mat4& value);
    void SetIntArray(const std::string& name, const int* values, unsigned int count);

private:
    GLuint m_rendererID;
    std::unordered_map<std::string, int> m_uniformLocationCache;

    GLuint CompileShader(GLenum type, const std::string& source);
    GLuint CreateProgram(const std::string& vertexSrc, const std::string& fragmentSrc);
    int GetUniformLocation(const std::string& name);
};
