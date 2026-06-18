#pragma once
// engine/graphics/Framebuffer.hpp
// Off-screen render target (color + depth).

#include "Texture.hpp"
#include "NKGlad/include/glad/gl.h"
#include <cstdint>

namespace engine {

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    // No copy
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    // Creates the FBO with a color texture + depth-renderbuffer attachment.
    // Call after gladLoadGL.
    bool Create(uint32_t width, uint32_t height);

    // Resize (recreates attachments)
    bool Resize(uint32_t newWidth, uint32_t newHeight);

    void Bind()   const;   // render into this FBO
    void Unbind() const;   // back to default framebuffer (0)

    GLuint  GetHandle()       const { return fbo; }
    GLuint  GetColorTexture() const { return colorTex; }
    uint32_t GetWidth()       const { return width; }
    uint32_t GetHeight()      const { return height; }

    bool IsComplete() const;

private:
    void Destroy();

    GLuint   fbo      = 0;
    GLuint   colorTex = 0;
    GLuint   depthRbo = 0;
    uint32_t width    = 0;
    uint32_t height   = 0;
};

} // namespace engine
