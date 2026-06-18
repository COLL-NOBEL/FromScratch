// src/main.cpp
// Entry point for the graphics engine demo.
// Creates an Application subclass that hosts the DemoScene.

#include "NKWindow/Core/NkMain.h"
#include "../engine/core/Application.hpp"
#include "DemoScene.hpp"

using namespace engine;

// =========================================================================
// MyApp — thin subclass of Application; just loads the demo scene.
// =========================================================================
class MyApp : public Application {
public:
    MyApp() : Application({ "Graphics Engine Demo", 1280, 720,
                             /*vsync=*/true, /*resizable=*/true,
                             /*glMajor=*/4, /*glMinor=*/6 }) {}

    void OnInit() override {
        // Hand the scene to the app — Application will call OnInit(renderer)
        SetScene(std::make_shared<demo::DemoScene>());
    }

    void OnResize(uint32_t w, uint32_t h) override {
        // Keep camera aspect ratio in sync
        if (auto cam = GetRenderer().GetCamera()) {
            cam->SetPerspective(
                NkMathUtils::deg2rad(60.0f),
                (float)w / (float)(h > 0 ? h : 1),
                0.1f, 200.0f);
        }
    }
};

// =========================================================================
// nkmain — nkentseu entry point (replaces main)
// =========================================================================
int nkmain(const nkentseu::NkEntryState& /*state*/) {
    MyApp app;
    return app.Run();
}