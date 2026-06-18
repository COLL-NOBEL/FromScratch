#pragma once

#include <string>

#include "NKGlad/include/glad/gl.h"

namespace graphics {

class ShaderProgram {
public:
    bool LoadFromFiles(const std::string& vertexRelativePath,
                       const std::string& fragmentRelativePath);

    void Bind() const;
    GLint FindUniform(const char* uniformName) const;
    void Shutdown();

private:
    bool ReadTextWithFallbacks(const std::string& relativePath,
                               std::string& outText,
                               std::string& outResolvedPath) const;

    GLuint Compile(GLenum shaderType,
                   const std::string& source,
                   const std::string& debugName) const;

    bool Link(GLuint vertexShader, GLuint fragmentShader);

private:
    GLuint mProgram = 0;
};

} // namespace graphics
