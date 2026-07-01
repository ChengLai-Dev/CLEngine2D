#include "Render/Shader.h"
#include "Math/Mat4.h"
#include "Logger.h"

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <format>
#include <vector>

static std::string ReadFile(const std::string& filepath) {
    std::ifstream stream(filepath, std::ios::in | std::ios::binary);
    if (!stream) {
        Logger::Error(std::format("Failed to open shader file: {}", filepath));
        return "";
    }
    std::stringstream ss;
    ss << stream.rdbuf();
    return ss.str();
}

static void ParseShader(const std::string& source, std::string& vertexSrc, std::string& fragmentSrc) {
    std::istringstream stream(source);
    std::string line;
    std::string* current = nullptr;

    while (std::getline(stream, line)) {
        if (line.find("#type vertex") != std::string::npos) {
            current = &vertexSrc;
            continue;
        }
        else if (line.find("#type fragment") != std::string::npos) {
            current = &fragmentSrc;
            continue;
        }
        if (current) {
            *current += line + "\n";
        }
    }
}

Shader::Shader(const std::string& filepath) {
    std::string source = ReadFile(filepath);
    std::string vertexSrc, fragmentSrc;
    ParseShader(source, vertexSrc, fragmentSrc);

    m_rendererID = CreateProgram(vertexSrc, fragmentSrc);
}

Shader::~Shader() {
    glDeleteProgram(m_rendererID);
}

void Shader::Bind() const {
    glUseProgram(m_rendererID);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

void Shader::SetInt(const std::string& name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetMat4(const std::string& name, const Mat4& value) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, value.Data());
}

void Shader::SetIntArray(const std::string& name, const int* values, unsigned int count) {
    glUniform1iv(GetUniformLocation(name), static_cast<int>(count), values);
}

GLuint Shader::CompileShader(GLenum type, const std::string& source) {
    GLuint id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> message(static_cast<size_t>(length));
        glGetShaderInfoLog(id, length, &length, message.data());

        const char* typeName = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        Logger::Error(std::format("Shader compilation failed ({}): {}", typeName, message.data()));
        glDeleteShader(id);
        return 0;
    }

    return id;
}

GLuint Shader::CreateProgram(const std::string& vertexSrc, const std::string& fragmentSrc) {
    GLuint program = glCreateProgram();
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    int result;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> message(static_cast<size_t>(length));
        glGetProgramInfoLog(program, length, &length, message.data());
        Logger::Error(std::format("Shader linking failed: {}", message.data()));
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

int Shader::GetUniformLocation(const std::string& name) {
    auto it = m_uniformLocationCache.find(name);
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }

    int location = glGetUniformLocation(m_rendererID, name.c_str());
    if (location == -1) {
        Logger::Warn(std::format("Uniform '{}' not found in shader", name));
    }
    m_uniformLocationCache[name] = location;
    return location;
}
