#include "Texture.hpp"
#include "NKLogger/NkLog.h"

namespace engine {

using namespace nkentseu;

Texture::Texture() : handle(0), width(0), height(0) {}

Texture::~Texture() {
    if (handle != 0) {
        glDeleteTextures(1, &handle);
    }
}

bool Texture::Create(uint32_t w, uint32_t h, const uint8_t* data) {
    width = w;
    height = h;

    glGenTextures(1, &handle);
    Bind();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    Unbind();

    logger.Infof("[Texture] Created texture %u (%ux%u)\n", handle, width, height);
    return true;
}

void Texture::SetFilter(TextureFilter min, TextureFilter mag) {
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)mag);
    Unbind();
}

void Texture::SetWrap(TextureWrap u, TextureWrap v) {
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)u);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)v);
    Unbind();
}

void Texture::Bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, handle);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

}
