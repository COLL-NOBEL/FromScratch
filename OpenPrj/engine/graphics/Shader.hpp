#pragma once

#include "../core/Math.hpp"
#include "NKGlad/include/glad/gl.h"
#include <string>
#include <unordered_map>

namespace engine {

class Shader {
public:
    Shader();
    ~Shader();

    bool Compile(const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc = "");
    void Use() const;
    void Unuse() const;

    void SetUniform1f(const std::string& name, float value);
    void SetUniform1i(const std::string& name, int value);
    void SetUniform2f(const std::string& name, float x, float y);
    void SetUniform3f(const std::string& name, float x, float y, float z);
    void SetUniform3f(const std::string& name, const Vec3f& vec);
    void SetUniform4f(const std::string& name, float x, float y, float z, float w);
    void SetUniformMat4f(const std::string& name, const Mat4f& matrix);

    GLuint GetProgram() const { return program; }

private:
    GLuint CompileShader(GLenum type, const std::string& source);
    GLint GetUniformLocation(const std::string& name);

    GLuint program = 0;
    std::unordered_map<std::string, GLint> uniformCache;
};

}
