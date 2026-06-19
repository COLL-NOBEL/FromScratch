#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"

// 1. Activer les extensions de plateforme Vulkan pour Windows avant l'inclusion
#if defined(_WIN32) || defined(NKENTSEU_PLATFORM_WINDOWS)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

using namespace nkentseu;

// Définir une macro de secours si NK_VK_CHECK n'est pas encore définie dans ton framework
#ifndef NK_VK_CHECK
    #define NK_VK_CHECK(x) x
#endif

// =============================================================================
// Point d'entrée — nkmain
// =============================================================================
int nkmain(const NkEntryState& state) {

    // -------------------------------------------------------------------------
    // 2. Fenêtre
    // -------------------------------------------------------------------------
    NkWindowConfig cfg;
    cfg.title       = "VKGraphicsEngine";
    cfg.width       = 1280;
    cfg.height      = 720;
    cfg.centered    = true;
    cfg.resizable   = true;
    cfg.dropEnabled = true;

    NkWindow window(cfg);
    if (!window.IsOpen()) {
        logger.Error("[Sandbox] Window creation FAILED");
        NkContextShutdown();
        return -2;
    }

    // Extraction correcte (par copie ou commente si inutile pour l'instant)
    auto surf = window.GetSurfaceDesc();

    // -------------------------------------------------------------------------
    // 3. Création de l'instance Vulkan
    // -------------------------------------------------------------------------
    VkInstance instance = VK_NULL_HANDLE;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VKGraphicsEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "WandaVKGE";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    

    NkVector<const char*> extensions;
    extensions.PushBack(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(NKENTSEU_PLATFORM_WINDOWS) || defined(_WIN32)
    extensions.PushBack(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(NKENTSEU_WINDOWING_XLIB)
    extensions.PushBack(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(NKENTSEU_WINDOWING_XCB)
    extensions.PushBack(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(NKENTSEU_WINDOWING_WAYLAND)
    extensions.PushBack(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(NKENTSEU_PLATFORM_ANDROID)
    extensions.PushBack(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(NKENTSEU_PLATFORM_MACOS)
    extensions.PushBack(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    extensions.PushBack(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.PushBack(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    // Remplacement des variables 'd.' par des valeurs explicites pour le TP01
    bool enableValidationLayers = true; // Mets à 'false' si tu veux désactiver temporairement
    bool enableDebugUtils = true;

    if (enableDebugUtils) {
        extensions.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    NkVector<const char*> layers;
    if (enableValidationLayers) {
        layers.PushBack("VK_LAYER_KHRONOS_validation");
        logger.Infof("[NkVulkan] Validation layers enabled");
    }

    VkInstanceCreateInfo info{};
    info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo        = &appInfo;
    info.enabledExtensionCount   = (uint32)extensions.Size();
    info.ppEnabledExtensionNames = extensions.Data();
    info.enabledLayerCount       = (uint32)layers.Size();
    info.ppEnabledLayerNames     = layers.Data();

#if defined(NKENTSEU_PLATFORM_MACOS)
    info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    NK_VK_CHECK(vkCreateInstance(&info, nullptr, &instance));
    logger.Infof("[NkVulkan] VkInstance created successfully!");

    // -------------------------------------------------------------------------
    // 5. Boucle principale
    // -------------------------------------------------------------------------
    auto& eventSystem = NkEvents();

    bool running = true;
    NkChrono chrono;
    NkElapsedTime elapsed;

    while (running)
    {
        NkElapsedTime e = chrono.Reset();

        while (NkEvent* event = eventSystem.PollEvent())
        {
            if (auto wcl = event->As<NkWindowCloseEvent>()) {
                running = false;
                break;
            }
        }

        if (!running) break;

        // --- Cap 60 fps ---
        elapsed = chrono.Elapsed();
        if (elapsed.milliseconds < 16)
            NkChrono::Sleep(16 - elapsed.milliseconds);
        else
            NkChrono::YieldThread();
    }

    // Ne pas oublier de détruire l'instance à la fin avant de fermer la fenêtre
    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        logger.Infof("[NkVulkan] VkInstance destroyed");
    }

    window.Close();
    return 0;
}