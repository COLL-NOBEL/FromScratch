#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"
#include "NKLogger/NkLog.h"

#if defined(_WIN32)
#include "NKGlad/include/glad/wgl.h"
#endif
#include "NKGlad/include/glad/gl.h"

#if defined(_WIN32) && defined(Bool)
    #undef Bool
#endif

#include "NKContext/Factory/NkContextFactory.h"
#include "NKContext/Core/NkContextDesc.h"
#include "NKContext/Core/NkIGraphicsContext.h"
#include "NKContext/Core/NkOpenGLDesc.h"
#include "NKContext/Graphics/OpenGL/NkOpenGLContextData.h"

#include "engine/Renderer.h"

#include <chrono>

using namespace nkentseu;

// Load OpenGL function pointers using GLAD
static bool LoadGL(NkIGraphicsContext* ctx) {
    if (!ctx || ctx->GetApi() != NkGraphicsApi::NK_API_OPENGL) {
        logger.Info("[OpenGL] Invalid graphics context API\n");
        return false;
    }

    auto* nativeData = static_cast<NkOpenGLContextData*>(ctx->GetNativeContextData());
    auto loader = (nativeData != nullptr) ? nativeData->getProcAddress : nullptr;
    if (!loader) {
        logger.Info("[OpenGL] Function loader not found\n");
        return false;
    }

    int ver = gladLoadGL((GLADloadfunc)loader);

    if (!ver) {
        logger.Error("[OpenGL] gladLoadGL failed\n");
        return false;
    }

    logger.Infof("[OpenGL] OpenGL %s  GLSL %s\n",
        glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    return true;
}

int nkmain(const nkentseu::NkEntryState& /*state*/) {
    // Step 1: Create window
    NkWindowConfig cfg;
    cfg.title = "My OpenGL App";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.centered = true;
    cfg.resizable = true;

    NkWindow window;
    if (!window.Create(cfg)) {
        logger.Error("Failed to create window");
        return -2;
    }

    // Step 2: Configure OpenGL context
    NkContextDesc desc;
    desc.api = NkGraphicsApi::NK_API_OPENGL;
    desc.opengl.majorVersion = 4;
    desc.opengl.minorVersion = 6;
    desc.opengl.profile = NkGLProfile::Core;
    desc.opengl.contextFlags = NkGLContextFlags::ForwardCompat | NkGLContextFlags::Debug;
    desc.opengl.runtime.installDebugCallback = true;
    desc.opengl.msaaSamples = 4;
    desc.opengl.srgbFramebuffer = true;
    desc.opengl.swapInterval = NkGLSwapInterval::AdaptiveVSync;
    desc.opengl.runtime.autoLoadEntryPoints = true;
    desc.opengl.runtime.validateVersion = true;

    // Step 3: Create graphics context
    auto ctx = NkContextFactory::Create(window, desc);
    if (!ctx) {
        logger.Error("[OpenGL] Context creation failed");
        window.Close();
        return -3;
    }

    // Step 4: Load OpenGL functions
    if (!LoadGL(ctx)) {
        ctx->Shutdown();
        window.Close();
        return -4;
    }

    // Step 5: Initialize graphics-engine scaffold
    graphics::Renderer renderer;
    const auto initialSize = window.GetSize();

    if (!renderer.Init(initialSize.width, initialSize.height)) {
        logger.Error("[OpenGL] Renderer initialization failed");
        ctx->Shutdown();
        window.Close();
        return -5;
    }

    // Step 6: Main loop
    bool running = true;
    auto& events = NkEvents();
    const auto appStartTime = std::chrono::steady_clock::now();

    while (running) {
        while (NkEvent* ev = events.PollEvent()) {
            if (ev->Is<NkWindowCloseEvent>()) {
                running = false;
                break;
            }
        }

        if (!running) {
            break;
        }

        if (ctx->BeginFrame()) {
            const auto currentSize = window.GetSize();
            renderer.Resize(currentSize.width, currentSize.height);

            const auto now = std::chrono::steady_clock::now();
            const float elapsedSeconds = std::chrono::duration<float>(now - appStartTime).count();
            renderer.Render(elapsedSeconds);

            ctx->EndFrame();
            ctx->Present();
        }
    }

    // Step 7: Cleanup
    renderer.Shutdown();
    ctx->Shutdown();
    window.Close();

    return 0;
}
