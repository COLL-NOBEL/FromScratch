#pragma once
// src/DemoScene.hpp
// A complete demo scene showing:
//   - Rotating cube, sphere, pyramid on a ground plane
//   - Directional + point light
//   - Fly-cam controlled with WASD + mouse drag

#include "../engine/core/Scene.hpp"
#include "../engine/core/Input.hpp"
#include "../engine/core/Time.hpp"
#include "../engine/graphics/ShaderLoader.hpp"
#include "NKEvent/NkKeycodeMap.h"

using namespace nkentseu;

namespace demo {

class DemoScene : public engine::Scene {
public:
    DemoScene() : engine::Scene("Demo") {}

    void OnInit(engine::Renderer& renderer) override;
    void OnUpdate(float dt) override;
    void OnRender(engine::Renderer& renderer) override;

private:
    // Meshes
    std::shared_ptr<engine::Mesh> cubeMesh;
    std::shared_ptr<engine::Mesh> sphereMesh;
    std::shared_ptr<engine::Mesh> pyramidMesh;
    std::shared_ptr<engine::Mesh> planeMesh;

    // Materials
    std::shared_ptr<engine::Material> litMaterial;
    std::shared_ptr<engine::Material> redMaterial;
    std::shared_ptr<engine::Material> blueMaterial;
    std::shared_ptr<engine::Material> greenMaterial;
    std::shared_ptr<engine::Material> unlitMaterial; // light-bulb indicator

    // Shared shader
    std::shared_ptr<engine::Shader> litShader;
    std::shared_ptr<engine::Shader> unlitShader;

    // Camera (shared with renderer)
    std::shared_ptr<engine::Camera> camera;

    // Lights
    engine::Light dirLight;
    engine::Light pointLight;

    // Rotation accumulators
    float cubeAngle    = 0.0f;
    float sphereAngle  = 0.0f;
    float pyramidAngle = 0.0f;
    float totalTime    = 0.0f;

    // Fly-cam state
    float camYaw   = 0.0f;
    float camPitch = 0.0f;

    // Helper
    std::shared_ptr<engine::Material> MakeLitMaterial(
        engine::Renderer& r,
        engine::Vec3f colour, float shininess = 64.0f);
};

} // namespace demo
