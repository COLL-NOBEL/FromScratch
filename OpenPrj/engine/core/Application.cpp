// engine/core/Application.cpp

#include "Application.hpp"
#include "NKLogger/NkLog.h"

namespace engine {

using namespace nkentseu;

Application::Application(const AppConfig& c)
    : cfg(c), width(c.width), height(c.height) {}

Application::~Application() {}

int Application::Run() {
    if (!InitWindow())  return -1;
    if (!LoadGL())      { Shutdown(); return -2; }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.08f, 0.08f, 0.12f, 1.0f);

    renderer.Initialize();

    OnInit();
    if (activeScene) activeScene->OnInit(renderer);

    MainLoop();

    if (activeScene) activeScene->OnShutdown();
    OnShutdown();
    Shutdown();
    return 0;
}

bool Application::InitWindow() {
    NkWindowConfig wc;
    wc.title     = cfg.title;
    wc.width     = cfg.width;
    wc.height    = cfg.height;
    wc.centered  = true;
    wc.resizable = cfg.resizable;

    if (!window.Create(wc)) {
        logger.Error("[Application] Failed to create window\n");
        return false;
    }

    NkContextDesc desc;
    desc.api                             = NkGraphicsApi::NK_API_OPENGL;
    desc.opengl.majorVersion             = cfg.glMajor;
    desc.opengl.minorVersion             = cfg.glMinor;
    desc.opengl.profile                  = NkGLProfile::Core;
    desc.opengl.contextFlags             = NkGLContextFlags::ForwardCompat | NkGLContextFlags::Debug;
    desc.opengl.runtime.installDebugCallback = true;
    desc.opengl.msaaSamples              = 4;
    desc.opengl.srgbFramebuffer          = true;
    desc.opengl.swapInterval             = cfg.vsync
        ? NkGLSwapInterval::AdaptiveVSync
        : NkGLSwapInterval::Immediate;
    desc.opengl.runtime.autoLoadEntryPoints = true;
    desc.opengl.runtime.validateVersion     = true;

    ctx = NkContextFactory::Create(window, desc);
    if (!ctx) {
        logger.Error("[Application] Context creation failed\n");
        window.Close();
        return false;
    }
    return true;
}

bool Application::LoadGL() {
    auto loader = NkNativeContext::GetOpenGLProcAddressLoader(ctx);
    if (!loader) { logger.Error("[Application] GL loader not found\n"); return false; }

    if (!gladLoadGL((GLADloadfunc)loader)) {
        logger.Error("[Application] gladLoadGL failed\n");
        return false;
    }
    logger.Infof("[Application] OpenGL %s  GLSL %s\n",
        glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    return true;
}

void Application::MainLoop() {
    auto& events = NkEvents();

    while (running) {
        Input::BeginFrame();
        time.Tick();

        // ---- Event polling ------------------------------------------
        while (NkEvent* ev = events.PollEvent()) {
            Input::ProcessEvent(ev);

            if (ev->Is<NkWindowCloseEvent>()) {
                running = false;
                break;
            }
            if (auto* re = ev->Cast<NkWindowResizeEvent>()) {
                width  = (uint32_t)re->GetWidth();
                height = (uint32_t)re->GetHeight();
                OnResize(width, height);
            }
        }
        if (!running) break;

        // ---- Update -------------------------------------------------
        float dt = time.GetDeltaTime();
        if (activeScene) activeScene->OnUpdate(dt);
        OnUpdate(dt);

        // ---- Render -------------------------------------------------
        if (ctx->BeginFrame()) {
            glViewport(0, 0, (GLsizei)width, (GLsizei)height);
            renderer.BeginFrame();

            if (activeScene) activeScene->OnRender(renderer);
            OnRender();

            renderer.EndFrame();
            ctx->EndFrame();
            ctx->Present();
        }
    }
}

void Application::Shutdown() {
    if (ctx)  { ctx->Shutdown(); ctx = nullptr; }
    window.Close();
}

void Application::SetScene(std::shared_ptr<Scene> scene) {
    if (activeScene) activeScene->OnShutdown();
    activeScene = std::move(scene);
    // If GL is already loaded (mid-run), init immediately
    if (ctx && activeScene) activeScene->OnInit(renderer);
}

} // namespace engine
