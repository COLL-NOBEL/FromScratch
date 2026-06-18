#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"
#include "NKLogger/NkLog.h"

#include "NKGlad/include/glad/wgl.h"
#include "NKGlad/include/glad/gl.h"

#if defined(Bool)
    #undef Bool
#endif

#include "NKContext/Factory/NkContextFactory.h"
#include "NKContext/Core/NkContextDesc.h"
#include "NKContext/Core/NkNativeContextAccess.h"
#include "NKContext/Core/NkOpenGLDesc.h"

using namespace nkentseu;

// Load OpenGL function pointers using GLAD
static bool LoadGL(NkIGraphicsContext* ctx) {
    auto loader = NkNativeContext::GetOpenGLProcAddressLoader(ctx);
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
    desc.api    = NkGraphicsApi::NK_API_OPENGL;
    desc.opengl.majorVersion = 4;
    desc.opengl.minorVersion = 6;
    desc.opengl.profile = NkGLProfile::Core;
    desc.opengl.contextFlags = NkGLContextFlags::ForwardCompat | NkGLContextFlags::Debug;
    desc.opengl.runtime.installDebugCallback = true;
    desc.opengl.msaaSamples        = 4;
    desc.opengl.srgbFramebuffer    = true;
    desc.opengl.swapInterval       = NkGLSwapInterval::AdaptiveVSync;
    desc.opengl.runtime.autoLoadEntryPoints  = true;
    desc.opengl.runtime.validateVersion      = true;

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

    // Step 5: Initialize OpenGL state
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // Step 6: Main loop
    bool running = true;
    auto& events = NkEvents();

    while (running) {
        while (NkEvent* ev = events.PollEvent()) {
            if (ev->Is<NkWindowCloseEvent>()) {
                running = false;
                break;
            }
        }

        if (!running) break;

        if (ctx->BeginFrame()) {
            glViewport(0, 0, window.GetSize().width, window.GetSize().height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Your rendering code here
            
            ctx->EndFrame();
            ctx->Present();
        }
    }

    // Step 7: Cleanup
    ctx->Shutdown();
    window.Close();

    return 0;
}