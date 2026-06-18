#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"
#include "NKLogger/NkLog.h"

#include "NKContext/Factory/NkContextFactory.h"
#include "NKContext/Core/NkContextDesc.h"
#include "NKContext/Core/NkIGraphicsContext.h"

using namespace nkentseu;

int nkmain(const nkentseu::NkEntryState& /*state*/) {
    // Step 1: Create a plain window.
    NkWindowConfig cfg;
    cfg.title = "My Vulkan App";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.centered = true;
    cfg.resizable = true;

    NkWindow window;
    if (!window.Create(cfg)) {
        logger.Error("[Vulkan] Failed to create window");
        return -2;
    }

    // Step 2: Configure a minimal Vulkan context.
#if !defined(NDEBUG)
    constexpr bool kEnableValidation = true;
#else
    constexpr bool kEnableValidation = false;
#endif

    NkContextDesc desc = NkContextDesc::MakeVulkan(kEnableValidation);
    desc.vulkan.appName = "MyVulkanApp";
    desc.vulkan.engineName = "Nkentseu";
    desc.vulkan.vsync = true;
    desc.vulkan.swapchainImages = 3;
    desc.vulkan.msaaSamples = 1;

    auto* ctx = NkContextFactory::Create(window, desc);
    if (!ctx || !ctx->IsValid()) {
        logger.Error("[Vulkan] Context creation failed");
        if (ctx) {
            ctx->Shutdown();
            delete ctx;
        }
        window.Close();
        return -3;
    }

    logger.Info("[Vulkan] Context created. Entering main loop");

    // Step 3: Minimal event/render loop (no drawing yet).
    bool running = true;
    auto& events = NkEvents();

    auto lastSize = window.GetSize();
    if (lastSize.width > 0 && lastSize.height > 0) {
        ctx->OnResize(lastSize.width, lastSize.height);
    }

    while (running && window.IsOpen()) {
        while (NkEvent* ev = events.PollEvent()) {
            if (ev->Is<NkWindowCloseEvent>()) {
                running = false;
                break;
            }
        }

        if (!running) {
            break;
        }

        const auto currentSize = window.GetSize();
        if ((currentSize.width != lastSize.width || currentSize.height != lastSize.height)
            && currentSize.width > 0 && currentSize.height > 0) {
            ctx->OnResize(currentSize.width, currentSize.height);
            lastSize = currentSize;
        }

        if (ctx->BeginFrame()) {
            // Intentionally empty frame for now.
            ctx->EndFrame();
            ctx->Present();
        }
    }

    // Step 4: Clean shutdown.
    ctx->Shutdown();
    delete ctx;
    window.Close();

    logger.Info("[Vulkan] Shutdown complete");
    return 0;
}
