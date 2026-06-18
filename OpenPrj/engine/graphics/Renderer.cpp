// engine/graphics/Renderer.cpp

#include "Renderer.hpp"
#include "NKLogger/NkLog.h"
#include "NKGlad/include/glad/gl.h"
#include <algorithm>

namespace engine {

using namespace nkentseu;

Renderer::Renderer() : camera(nullptr) {}
Renderer::~Renderer() {}

void Renderer::Initialize() {
    if (!camera) camera = std::make_shared<Camera>();
    logger.Info("[Renderer] Initialized\n");
}

void Renderer::BeginFrame() {
    ResetStats();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame() {
    // future post-processing hook
}

void Renderer::ResetStats() {
    drawCalls = 0;
    triCount  = 0;
}

void Renderer::AddLight(const Light& light)  { lights.push_back(light); }
void Renderer::ClearLights()                  { lights.clear(); }

// ---- Upload up to 4 lights to a shader (uniform arrays) -----------------
void Renderer::UploadLightsToShader(Shader* shader) {
    if (!shader) return;

    int count = (int)std::min(lights.size(), (size_t)4);
    shader->SetUniform1i("uLightCount", count);

    for (int i = 0; i < count; i++) {
        const Light& L = lights[i];
        std::string base = "uLights[" + std::to_string(i) + "]";
        shader->SetUniform1i(base + ".type",      (int)L.type);
        shader->SetUniform3f(base + ".position",  L.position);
        shader->SetUniform3f(base + ".direction", L.direction);
        shader->SetUniform3f(base + ".color",     L.color);
        shader->SetUniform1f(base + ".intensity",  L.intensity);
        shader->SetUniform1f(base + ".range",      L.range);
    }
}

// ---- Render with Transform ----------------------------------------------
void Renderer::RenderMesh(std::shared_ptr<Mesh>     mesh,
                           std::shared_ptr<Material> material,
                           const Transform&          transform)
{
    RenderMesh(mesh, material, transform.GetMatrix());
}

// ---- Render with explicit matrix ----------------------------------------
void Renderer::RenderMesh(std::shared_ptr<Mesh>     mesh,
                           std::shared_ptr<Material> material,
                           const Mat4f&              modelMatrix)
{
    if (!mesh || !material || !camera) return;

    // Ensure GPU buffers are uploaded
    if (!mesh->GetVertexBuffer().IsInitialized()) mesh->Init();

    material->Use();

    if (auto shader = material->GetShader()) {
        Mat4f view       = camera->GetViewMatrix();
        Mat4f projection = camera->GetProjectionMatrix();

        shader->SetUniformMat4f("uModel",      modelMatrix);
        shader->SetUniformMat4f("uView",       view);
        shader->SetUniformMat4f("uProjection", projection);
        shader->SetUniform3f  ("uCamPos",      camera->GetPosition());
        shader->SetUniform1f  ("uTime",        0.0f); // patched by app

        UploadLightsToShader(shader.get());
    }

    mesh->Render();
    material->Unuse();

    drawCalls++;
    triCount += (uint32_t)(mesh->GetVertexBuffer().GetIndexCount() / 3);
}

// ---- Factory helpers ----------------------------------------------------
std::shared_ptr<Shader>   Renderer::CreateShader()   { return std::make_shared<Shader>(); }
std::shared_ptr<Material> Renderer::CreateMaterial()  { return std::make_shared<Material>(); }
std::shared_ptr<Mesh>     Renderer::CreateMesh()      { return std::make_shared<Mesh>(); }
std::shared_ptr<Camera>   Renderer::CreateCamera()    { return std::make_shared<Camera>(); }
std::shared_ptr<Texture>  Renderer::CreateTexture()   { return std::make_shared<Texture>(); }

} // namespace engine
