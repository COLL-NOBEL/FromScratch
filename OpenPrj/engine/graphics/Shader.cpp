#include "Shader.hpp"
#include "NKLogger/NkLog.h"

namespace engine {

using namespace nkentseu;

Shader::Shader() : program(0) {}

Shader::~Shader() {
    if (program != 0) {
        glDeleteProgram(program);
    }
}

bool Shader::Compile(const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc) {
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    if (vertexShader == 0) return false;

    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }

    GLuint geometryShader = 0;
    if (!geometrySrc.empty()) {
        geometryShader = CompileShader(GL_GEOMETRY_SHADER, geometrySrc);
        if (geometryShader == 0) {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return false;
        }
    }

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    if (geometryShader != 0) {
        glAttachShader(program, geometryShader);
    }

    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        logger.Errorf("[Shader] Link failed: %s\n", infoLog);
        glDeleteProgram(program);
        program = 0;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        if (geometryShader != 0) glDeleteShader(geometryShader);
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryShader != 0) glDeleteShader(geometryShader);

    logger.Infof("[Shader] Program compiled successfully: %u\n", program);
    return true;
}

void Shader::Use() const {
    glUseProgram(program);
}

void Shader::Unuse() const {
    glUseProgram(0);
}

GLuint Shader::CompileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        const char* typeStr = (type == GL_VERTEX_SHADER) ? "Vertex" :
                              (type == GL_FRAGMENT_SHADER) ? "Fragment" : "Geometry";
        logger.Errorf("[Shader] %s compilation failed: %s\n", typeStr, infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLint Shader::GetUniformLocation(const std::string& name) {
    auto it = uniformCache.find(name);
    if (it != uniformCache.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(program, name.c_str());
    if (location == -1) {
        logger.Warnf("[Shader] Uniform '%s' not found\n", name.c_str());
    }
    uniformCache[name] = location;
    return location;
}

void Shader::SetUniform1f(const std::string& name, float value) {
    Use();
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform1i(const std::string& name, int value) {
    Use();
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform2f(const std::string& name, float x, float y) {
    Use();
    glUniform2f(GetUniformLocation(name), x, y);
}

void Shader::SetUniform3f(const std::string& name, float x, float y, float z) {
    Use();
    glUniform3f(GetUniformLocation(name), x, y, z);
}

void Shader::SetUniform3f(const std::string& name, const Vec3f& vec) {
    SetUniform3f(name, vec[0], vec[1], vec[2]);
}

void Shader::SetUniform4f(const std::string& name, float x, float y, float z, float w) {
    Use();
    glUniform4f(GetUniformLocation(name), x, y, z, w);
}

void Shader::SetUniformMat4f(const std::string& name, const Mat4f& matrix) {
    Use();
    float mat[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            mat[i * 4 + j] = matrix(i, j);
        }
    }
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, mat);
}

}
