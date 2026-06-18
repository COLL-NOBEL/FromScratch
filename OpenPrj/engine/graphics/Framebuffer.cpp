// engine/graphics/Framebuffer.cpp

#include "Framebuffer.hpp"
#include "NKLogger/NkLog.h"

namespace engine {

using namespace nkentseu;

Framebuffer::Framebuffer() {}

Framebuffer::~Framebuffer() { Destroy(); }

void Framebuffer::Destroy() {
    if (depthRbo != 0) { glDeleteRenderbuffers(1, &depthRbo); depthRbo = 0; }
    if (colorTex != 0) { glDeleteTextures(1, &colorTex);      colorTex = 0; }
    if (fbo      != 0) { glDeleteFramebuffers(1, &fbo);       fbo      = 0; }
}

bool Framebuffer::Create(uint32_t w, uint32_t h) {
    Destroy();
    width = w; height = h;

    // --- Colour texture ---
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)w, (GLsizei)h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // --- Depth renderbuffer ---
    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)w, (GLsizei)h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // --- FBO ---
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

    bool complete = IsComplete();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (complete)
        logger.Infof("[Framebuffer] Created %ux%u FBO (handle=%u)\n", w, h, fbo);
    else
        logger.Error("[Framebuffer] FBO incomplete!\n");

    return complete;
}

bool Framebuffer::Resize(uint32_t newW, uint32_t newH) {
    if (newW == width && newH == height) return true;
    return Create(newW, newH);
}

void Framebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void Framebuffer::Unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::IsComplete() const {
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

} // namespace engine
