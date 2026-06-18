// src/DemoScene.cpp

#include "DemoScene.hpp"
#include "../engine/core/Math.hpp"
#include "NKLogger/NkLog.h"
#include <cmath>

using namespace engine;
using namespace nkentseu;

namespace demo {

// ---- Helper: build a lit material with a given albedo colour ------------
std::shared_ptr<Material> DemoScene::MakeLitMaterial(
    Renderer& r, Vec3f colour, float shininess)
{
    auto mat = r.CreateMaterial();
    mat->SetShader(litShader);
    mat->SetUniform3f("uAlbedo",     colour);
    mat->SetUniform1f("uShininess",  shininess);
    mat->SetUniform1i("uUseTexture", 0);
    return mat;
}

// ---- OnInit -------------------------------------------------------------
void DemoScene::OnInit(Renderer& renderer) {
    logger.Info("[DemoScene] Initialising...\n");

    // ---- Shaders ----
    auto litSrc = ShaderLoader::LoadFiles(
        "shaders/lit.vert", "shaders/lit.frag");
    auto unlitSrc = ShaderLoader::LoadFiles(
        "shaders/unlit.vert", "shaders/unlit.frag");

    litShader   = renderer.CreateShader();
    unlitShader = renderer.CreateShader();

    if (!litSrc.ok || !litShader->Compile(litSrc.vertex, litSrc.fragment)) {
        logger.Error("[DemoScene] Failed to compile lit shader\n");
    }
    if (!unlitSrc.ok || !unlitShader->Compile(unlitSrc.vertex, unlitSrc.fragment)) {
        logger.Error("[DemoScene] Failed to compile unlit shader\n");
    }

    // ---- Meshes ----
    cubeMesh    = renderer.CreateMesh();
    sphereMesh  = renderer.CreateMesh();
    pyramidMesh = renderer.CreateMesh();
    planeMesh   = renderer.CreateMesh();

    cubeMesh   ->SetData(Mesh::CreateCube());
    sphereMesh ->SetData(Mesh::CreateSphere(32, 16));
    pyramidMesh->SetData(Mesh::CreatePyramid());
    planeMesh  ->SetData(Mesh::CreatePlane(12.0f, 12.0f, 4));

    cubeMesh   ->Init();
    sphereMesh ->Init();
    pyramidMesh->Init();
    planeMesh  ->Init();

    // ---- Materials ----
    //  Cube  = warm orange
    litMaterial   = MakeLitMaterial(renderer, Vec3f(0.95f, 0.55f, 0.15f), 128.0f);
    //  Sphere = cool blue
    blueMaterial  = MakeLitMaterial(renderer, Vec3f(0.20f, 0.50f, 0.95f),  64.0f);
    //  Pyramid = vivid green
    greenMaterial = MakeLitMaterial(renderer, Vec3f(0.20f, 0.85f, 0.35f),  32.0f);
    //  Ground plane = desaturated grey
    redMaterial   = MakeLitMaterial(renderer, Vec3f(0.35f, 0.35f, 0.40f),  16.0f);

    // Unlit bright-white indicator for point light position
    unlitMaterial = renderer.CreateMaterial();
    unlitMaterial->SetShader(unlitShader);
    unlitMaterial->SetUniform3f("uAlbedo",     Vec3f(1.0f, 1.0f, 0.7f));
    unlitMaterial->SetUniform1i("uUseTexture", 0);

    // ---- Camera ----
    camera = renderer.CreateCamera();
    camera->SetPosition(Vec3f(0.0f, 2.5f, 8.0f));
    camera->SetPerspective(
        NkMathUtils::deg2rad(60.0f),
        1280.0f / 720.0f,
        0.1f, 200.0f);
    renderer.SetCamera(camera);

    // ---- Lights ----
    dirLight = Light::CreateDirectional(
        Vec3f(-0.4f, -1.0f, -0.6f),   // direction (down + forward)
        Vec3f(1.0f,  0.95f, 0.85f),   // warm white
        0.7f);

    pointLight = Light::CreatePoint(
        Vec3f(3.0f, 3.0f, 3.0f),      // position (animated)
        Vec3f(1.0f, 0.6f, 0.3f),      // orange glow
        12.0f,                          // range
        2.0f);                          // intensity

    renderer.AddLight(dirLight);
    renderer.AddLight(pointLight);

    logger.Info("[DemoScene] Ready\n");
}

// ---- OnUpdate -----------------------------------------------------------
void DemoScene::OnUpdate(float dt) {
    totalTime    += dt;
    cubeAngle    += dt * 0.8f;
    sphereAngle  += dt * 0.5f;
    pyramidAngle += dt * 1.2f;

    // ---- Fly-camera: WASD + QE + mouse drag --------------------------
    const float speed    = 5.0f;
    const float sensitivity = 0.003f;

    if (Input::IsMouseButtonDown(1)) {  // right mouse held
        Vec2f delta = Input::GetMouseDelta();
        camYaw   -= delta[0] * sensitivity;
        camPitch -= delta[1] * sensitivity;
        // Clamp pitch to avoid gimbal flip
        const float maxPitch = NkMathUtils::deg2rad(89.0f);
        if (camPitch >  maxPitch) camPitch =  maxPitch;
        if (camPitch < -maxPitch) camPitch = -maxPitch;
        camera->Rotate(camYaw, camPitch, 0.0f);
    }

    float moveSpeed = speed * dt;
    if (Input::IsKeyDown(NkKeycode::NK_KEY_LEFT_SHIFT)) moveSpeed *= 3.0f;

    if (Input::IsKeyDown(NkKeycode::NK_KEY_W)) camera->MoveForward( moveSpeed);
    if (Input::IsKeyDown(NkKeycode::NK_KEY_S)) camera->MoveForward(-moveSpeed);
    if (Input::IsKeyDown(NkKeycode::NK_KEY_A)) camera->MoveRight  (-moveSpeed);
    if (Input::IsKeyDown(NkKeycode::NK_KEY_D)) camera->MoveRight  ( moveSpeed);
    if (Input::IsKeyDown(NkKeycode::NK_KEY_Q)) camera->MoveUp     (-moveSpeed);
    if (Input::IsKeyDown(NkKeycode::NK_KEY_E)) camera->MoveUp     ( moveSpeed);

    // Scroll zoom
    float scroll = Input::GetScrollDelta();
    if (scroll != 0.0f) camera->MoveForward(scroll * 0.5f);

    // Orbit point light around scene
    pointLight.position = Vec3f(
        std::cos(totalTime * 0.7f) * 4.0f,
        3.0f + std::sin(totalTime * 0.4f),
        std::sin(totalTime * 0.7f) * 4.0f);
}

// ---- OnRender -----------------------------------------------------------
void DemoScene::OnRender(Renderer& renderer) {
    // Rebuild light list each frame (point light moves)
    renderer.ClearLights();
    renderer.AddLight(dirLight);
    renderer.AddLight(pointLight);

    // ---- Ground plane (static) ----
    {
        Transform t;
        t.position = Vec3f(0.0f, -1.0f, 0.0f);
        renderer.RenderMesh(planeMesh, redMaterial, t);
    }

    // ---- Spinning cube ----
    {
        Transform t;
        t.position = Vec3f(-2.5f, 0.0f, 0.0f);
        t.RotateAround(Vec3f(0.3f, 1.0f, 0.2f).Normalize(), cubeAngle);
        renderer.RenderMesh(cubeMesh, litMaterial, t);
    }

    // ---- Bouncing sphere ----
    {
        Transform t;
        float bounce = std::abs(std::sin(sphereAngle * 1.5f)) * 0.8f;
        t.position = Vec3f(0.0f, bounce, 0.0f);
        t.RotateAround(Vec3f(0, 1, 0), sphereAngle);
        renderer.RenderMesh(sphereMesh, blueMaterial, t);
    }

    // ---- Spinning pyramid ----
    {
        Transform t;
        t.position = Vec3f(2.5f, 0.0f, 0.0f);
        t.RotateAround(Vec3f(0, 1, 0), pyramidAngle);
        renderer.RenderMesh(pyramidMesh, greenMaterial, t);
    }

    // ---- Small unlit sphere at point-light position ----
    {
        Transform t;
        t.position = pointLight.position;
        t.scale    = Vec3f(0.15f, 0.15f, 0.15f);
        renderer.RenderMesh(sphereMesh, unlitMaterial, t);
    }
}

} // namespace demo
