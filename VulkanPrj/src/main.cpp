#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"
#include "NKLogger/NkLog.h"

#include "engine/VulkanEngine.h"

#include "../nkMath/NkMathUtils.h"

using namespace nkentseu;

int nkmain(const NkEntryState& /*state*/) {
    NkWindowConfig cfg;
    cfg.title = "VKGraphicsEngine";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.centered = true;
    cfg.resizable = true;
    cfg.dropEnabled = true;

    NkWindow window(cfg);
    if (!window.IsOpen()) {
        logger.Error("[NkVulkan] Window creation failed");
        NkContextShutdown();
        return -2;
    }

    graphics::vulkan::VulkanEngine engine;
    if (!engine.Initialize(window)) {
        logger.Error("[NkVulkan] Engine initialization failed");
        window.Close();
        return -3;
    }

    auto& events = NkEvents();
    NkChrono frameChrono;
    bool running = true;

    while (running) {
        const NkElapsedTime frameElapsed = frameChrono.Reset();
        const float deltaSeconds = NkMathUtils::clamp(static_cast<float>(frameElapsed.seconds), 0.0f, 0.1f);

        while (NkEvent* event = events.PollEvent()) {
            if (event->Is<NkWindowCloseEvent>()) {
                running = false;
                break;
            }
        }

        if (!running) {
            break;
        }

        engine.Tick(deltaSeconds);

        const float sleepMilliseconds = 16.0f - static_cast<float>(frameElapsed.milliseconds);
        if (sleepMilliseconds > 0.0f) {
            NkChrono::Sleep(static_cast<double>(sleepMilliseconds));
        } else {
            NkChrono::YieldThread();
        }
    }

    engine.Shutdown();
    window.Close();

    return 0;
}
