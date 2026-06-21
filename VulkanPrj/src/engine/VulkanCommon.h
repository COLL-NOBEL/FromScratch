#pragma once

#include "NKPlatform/NkPlatformDetect.h"

#if defined(NKENTSEU_PLATFORM_WINDOWS)
    #ifndef VK_USE_PLATFORM_WIN32_KHR
        #define VK_USE_PLATFORM_WIN32_KHR
    #endif
#elif defined(NKENTSEU_PLATFORM_MACOS)
    #ifndef VK_USE_PLATFORM_METAL_EXT
        #define VK_USE_PLATFORM_METAL_EXT
    #endif
#elif defined(NKENTSEU_WINDOWING_XCB)
    #ifndef VK_USE_PLATFORM_XCB_KHR
        #define VK_USE_PLATFORM_XCB_KHR
    #endif
#elif defined(NKENTSEU_WINDOWING_XLIB)
    #ifndef VK_USE_PLATFORM_XLIB_KHR
        #define VK_USE_PLATFORM_XLIB_KHR
    #endif
#elif defined(NKENTSEU_WINDOWING_WAYLAND)
    #ifndef VK_USE_PLATFORM_WAYLAND_KHR
        #define VK_USE_PLATFORM_WAYLAND_KHR
    #endif
#endif

#include <vulkan/vulkan.h>
