#pragma once

#include "NKGlad/include/glad/gl.h"
#include <string>
#include <cstdint>

namespace engine {

enum class TextureFilter {
    Nearest = GL_NEAREST,
    Linear = GL_LINEAR,
};

enum class TextureWrap {
    Clamp = GL_CLAMP_TO_EDGE,
    Repeat = GL_REPEAT,
    Mirror = GL_MIRRORED_REPEAT,
};

class Texture {
public:
    Texture();
    ~Texture();

    bool Create(uint32_t width, uint32_t height, const uint8_t* data);
    void SetFilter(TextureFilter min, TextureFilter mag);
    void SetWrap(TextureWrap u, TextureWrap v);

    void Bind(uint32_t slot = 0) const;
    void Unbind() const;

    GLuint GetHandle() const { return handle; }
    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }

private:
    GLuint handle = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

}
