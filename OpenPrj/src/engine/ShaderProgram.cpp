#include "engine/ShaderProgram.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace graphics {

bool ShaderProgram::ReadTextWithFallbacks(const std::string& relativePath,
                                          std::string& outText,
                                          std::string& outResolvedPath) const {
    const std::vector<std::string> prefixes = {
        "",
        "./",
        "../",
        "../../",
        "../../../",
        "../../../../",
        "../../../../../",
        "../../../../../../"
    };

    for (const std::string& prefix : prefixes) {
        const std::string candidate = prefix + relativePath;
        std::ifstream file(candidate, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            continue;
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();

        outText = buffer.str();
        outResolvedPath = candidate;
        return true;
    }

    return false;
}

GLuint ShaderProgram::Compile(GLenum shaderType,
                              const std::string& source,
                              const std::string& debugName) const {
    const GLuint shader = glCreateShader(shaderType);
    if (shader == 0) {
        std::cerr << "[ShaderProgram] Failed to create shader: " << debugName << '\n';
        return 0;
    }

    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    GLint compileOk = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileOk);

    if (compileOk == GL_TRUE) {
        return shader;
    }

    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::string log;
    if (logLength > 1) {
        log.resize(static_cast<size_t>(logLength), '\0');
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    }

    std::cerr << "[ShaderProgram] Compilation failed for " << debugName << "\n";
    if (!log.empty()) {
        std::cerr << log << '\n';
    }

    glDeleteShader(shader);
    return 0;
}

bool ShaderProgram::Link(GLuint vertexShader, GLuint fragmentShader) {
    mProgram = glCreateProgram();
    if (mProgram == 0) {
        std::cerr << "[ShaderProgram] Failed to create program\n";
        return false;
    }

    glAttachShader(mProgram, vertexShader);
    glAttachShader(mProgram, fragmentShader);
    glLinkProgram(mProgram);

    GLint linkOk = GL_FALSE;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linkOk);

    if (linkOk == GL_TRUE) {
        return true;
    }

    GLint logLength = 0;
    glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &logLength);
    std::string log;
    if (logLength > 1) {
        log.resize(static_cast<size_t>(logLength), '\0');
        glGetProgramInfoLog(mProgram, logLength, nullptr, log.data());
    }

    std::cerr << "[ShaderProgram] Program link failed\n";
    if (!log.empty()) {
        std::cerr << log << '\n';
    }

    glDeleteProgram(mProgram);
    mProgram = 0;
    return false;
}

bool ShaderProgram::LoadFromFiles(const std::string& vertexRelativePath,
                                  const std::string& fragmentRelativePath) {
    Shutdown();

    std::string vertexSource;
    std::string fragmentSource;
    std::string vertexResolvedPath;
    std::string fragmentResolvedPath;

    if (!ReadTextWithFallbacks(vertexRelativePath, vertexSource, vertexResolvedPath)) {
        std::cerr << "[ShaderProgram] Vertex shader not found: " << vertexRelativePath << '\n';
        return false;
    }

    if (!ReadTextWithFallbacks(fragmentRelativePath, fragmentSource, fragmentResolvedPath)) {
        std::cerr << "[ShaderProgram] Fragment shader not found: " << fragmentRelativePath << '\n';
        return false;
    }

    const GLuint vertexShader = Compile(GL_VERTEX_SHADER, vertexSource, vertexResolvedPath);
    if (vertexShader == 0) {
        return false;
    }

    const GLuint fragmentShader = Compile(GL_FRAGMENT_SHADER, fragmentSource, fragmentResolvedPath);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }

    const bool linked = Link(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return linked;
}

void ShaderProgram::Bind() const {
    glUseProgram(mProgram);
}

GLint ShaderProgram::FindUniform(const char* uniformName) const {
    if (mProgram == 0 || uniformName == nullptr) {
        return -1;
    }
    return glGetUniformLocation(mProgram, uniformName);
}

void ShaderProgram::Shutdown() {
    if (mProgram != 0) {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
}

} // namespace graphics
