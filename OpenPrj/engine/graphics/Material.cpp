#include "Material.hpp"

namespace engine {

Material::Material() : textureSlot(0) {}

Material::~Material() {}

void Material::SetTexture(const std::string& name, std::shared_ptr<Texture> tex) {
    textures[name] = tex;
}

std::shared_ptr<Texture> Material::GetTexture(const std::string& name) {
    auto it = textures.find(name);
    return (it != textures.end()) ? it->second : nullptr;
}

void Material::SetUniform1f(const std::string& name, float value) {
    uniforms1f[name] = value;
}

void Material::SetUniform3f(const std::string& name, const Vec3f& value) {
    uniforms3f[name] = value;
}

void Material::SetUniformMat4f(const std::string& name, const Mat4f& value) {
    uniformsMat4[name] = value;
}

void Material::Use() {
    if (!shader) return;

    shader->Use();

    uint32_t slot = 0;
    for (auto& [name, texture] : textures) {
        if (texture) {
            texture->Bind(slot);
            shader->SetUniform1i(name, slot);
            slot++;
        }
    }

    for (auto& [name, value] : uniforms1f) {
        shader->SetUniform1f(name, value);
    }

    for (auto& [name, value] : uniforms3f) {
        shader->SetUniform3f(name, value);
    }

    for (auto& [name, value] : uniformsMat4) {
        shader->SetUniformMat4f(name, value);
    }
}

void Material::Unuse() {
    if (shader) {
        shader->Unuse();
    }
}

}
