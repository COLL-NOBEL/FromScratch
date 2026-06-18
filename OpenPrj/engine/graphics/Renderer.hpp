#pragma once

#include "Camera.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "../core/Transform.hpp"
#include <vector>
#include <memory>

namespace engine {

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Call once after gladLoadGL
    void Initialize();

    // Call at start / end of each frame
    void BeginFrame();
    void EndFrame();

    // ---- Camera ---------------------------------------------------------
    void SetCamera(std::shared_ptr<Camera> cam) { camera = cam; }
    std::shared_ptr<Camera> GetCamera() { return camera; }

    // ---- Lights ---------------------------------------------------------
    void AddLight(const Light& light);
    void ClearLights();
    const std::vector<Light>& GetLights() const { return lights; }

    // ---- Draw calls -----------------------------------------------------
    // Renders a mesh with a material, applying model matrix from Transform
    void RenderMesh(std::shared_ptr<Mesh>     mesh,
                    std::shared_ptr<Material> material,
                    const Transform&          transform);

    // Same but with an explicit model matrix
    void RenderMesh(std::shared_ptr<Mesh>     mesh,
                    std::shared_ptr<Material> material,
                    const Mat4f&              modelMatrix);

    // ---- Factory helpers (convenience) ----------------------------------
    std::shared_ptr<Shader>   CreateShader();
    std::shared_ptr<Material> CreateMaterial();
    std::shared_ptr<Mesh>     CreateMesh();
    std::shared_ptr<Camera>   CreateCamera();
    std::shared_ptr<Texture>  CreateTexture();

    // ---- Stats ----------------------------------------------------------
    uint32_t GetDrawCalls() const { return drawCalls; }
    uint32_t GetTriCount()  const { return triCount;  }

private:
    void ResetStats();
    void UploadLightsToShader(Shader* shader);

    std::shared_ptr<Camera> camera;
    std::vector<Light>      lights;

    uint32_t drawCalls = 0;
    uint32_t triCount  = 0;
};

} // namespace engine
