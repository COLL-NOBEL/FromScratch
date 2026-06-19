#pragma once

#if defined(_WIN32) || defined(NKENTSEU_PLATFORM_WINDOWS)
    #ifndef VK_USE_PLATFORM_WIN32_KHR
        #define VK_USE_PLATFORM_WIN32_KHR
    #endif
#elif defined(__APPLE__) || defined(NKENTSEU_PLATFORM_MACOS)
    #ifndef VK_USE_PLATFORM_METAL_EXT
        #define VK_USE_PLATFORM_METAL_EXT
    #endif
#elif defined(__linux__) || defined(NKENTSEU_PLATFORM_LINUX)
    #if !defined(VK_USE_PLATFORM_XCB_KHR) && !defined(VK_USE_PLATFORM_XLIB_KHR) && !defined(VK_USE_PLATFORM_WAYLAND_KHR)
        #define VK_USE_PLATFORM_XCB_KHR
    #endif
#endif

#include <vulkan/vulkan.h>
