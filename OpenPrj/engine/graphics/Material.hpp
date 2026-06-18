#pragma once

#include "Shader.hpp"
#include "Texture.hpp"
#include <memory>
#include <unordered_map>

namespace engine {

class Material {
public:
    Material();
    ~Material();

    void SetShader(std::shared_ptr<Shader> shdr) { shader = shdr; }
    std::shared_ptr<Shader> GetShader() { return shader; }

    void SetTexture(const std::string& name, std::shared_ptr<Texture> tex);
    std::shared_ptr<Texture> GetTexture(const std::string& name);

    void SetUniform1f(const std::string& name, float value);
    void SetUniform3f(const std::string& name, const Vec3f& value);
    void SetUniformMat4f(const std::string& name, const Mat4f& value);

    void Use();
    void Unuse();

private:
    std::shared_ptr<Shader> shader;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
    std::unordered_map<std::string, float> uniforms1f;
    std::unordered_map<std::string, Vec3f> uniforms3f;
    std::unordered_map<std::string, Mat4f> uniformsMat4;

    uint32_t textureSlot = 0;
};

}
