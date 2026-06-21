#include "engine/VulkanInstance.h"

#include "NKLogger/NkLog.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace graphics::vulkan {

namespace {

constexpr const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";

const char* VkResultToString(VkResult result) {
    switch (result) {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        default: return "VK_ERROR_UNKNOWN";
    }
}

} // namespace

VulkanInstance::~VulkanInstance() {
    Destroy();
}

bool VulkanInstance::Create(const VulkanInstanceConfig& config) {
    Destroy();

    mEnabledExtensions.clear();
    mEnabledLayers.clear();

    if (!TryAddExtension(mEnabledExtensions, VK_KHR_SURFACE_EXTENSION_NAME)) {
        logger.Error("[NkVulkan] Missing required extension VK_KHR_surface");
        return false;
    }

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!TryAddExtension(mEnabledExtensions, VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
        logger.Error("[NkVulkan] Missing required extension VK_KHR_win32_surface");
        return false;
    }
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    if (!TryAddExtension(mEnabledExtensions, VK_EXT_METAL_SURFACE_EXTENSION_NAME)) {
        logger.Error("[NkVulkan] Missing required extension VK_EXT_metal_surface");
        return false;
    }
    TryAddExtension(mEnabledExtensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    TryAddExtension(mEnabledExtensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    if (!TryAddExtension(mEnabledExtensions, VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
        logger.Info("[NkVulkan] VK_KHR_xcb_surface unavailable; proceeding without surface creation support\n");
    }
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    if (!TryAddExtension(mEnabledExtensions, VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) {
        logger.Info("[NkVulkan] VK_KHR_xlib_surface unavailable; proceeding without surface creation support\n");
    }
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (!TryAddExtension(mEnabledExtensions, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)) {
        logger.Info("[NkVulkan] VK_KHR_wayland_surface unavailable; proceeding without surface creation support\n");
    }
#endif

    if (config.enableValidationLayers) {
        if (IsInstanceLayerSupported(kValidationLayer)) {
            mEnabledLayers.push_back(kValidationLayer);
            logger.Info("[NkVulkan] Validation layer enabled\n");

            if (config.enableDebugUtils) {
                TryAddExtension(mEnabledExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
        } else {
            logger.Info("[NkVulkan] Validation layer unavailable; continuing without it\n");
        }
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = config.applicationName;
    appInfo.applicationVersion = config.applicationVersion;
    appInfo.pEngineName = config.engineName;
    appInfo.engineVersion = config.engineVersion;
    appInfo.apiVersion = config.apiVersion;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(mEnabledExtensions.size());
    createInfo.ppEnabledExtensionNames = mEnabledExtensions.empty() ? nullptr : mEnabledExtensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(mEnabledLayers.size());
    createInfo.ppEnabledLayerNames = mEnabledLayers.empty() ? nullptr : mEnabledLayers.data();

#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (IsInstanceExtensionSupported(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    const VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);
    if (result != VK_SUCCESS) {
        logger.Infof("[NkVulkan] vkCreateInstance failed: %s (%d)\n", VkResultToString(result), static_cast<int>(result));
        mEnabledExtensions.clear();
        mEnabledLayers.clear();
        return false;
    }

    logger.Infof(
        "[NkVulkan] VkInstance created (API %u.%u.%u)\n",
        VK_API_VERSION_MAJOR(config.apiVersion),
        VK_API_VERSION_MINOR(config.apiVersion),
        VK_API_VERSION_PATCH(config.apiVersion)
    );

    return true;
}

void VulkanInstance::Destroy() {
    if (mInstance != VK_NULL_HANDLE) {
        vkDestroyInstance(mInstance, nullptr);
        mInstance = VK_NULL_HANDLE;
        logger.Info("[NkVulkan] VkInstance destroyed\n");
    }

    mEnabledExtensions.clear();
    mEnabledLayers.clear();
}

bool VulkanInstance::IsInstanceLayerSupported(const char* layerName) {
    uint32_t propertyCount = 0;
    if (vkEnumerateInstanceLayerProperties(&propertyCount, nullptr) != VK_SUCCESS || propertyCount == 0) {
        return false;
    }

    std::vector<VkLayerProperties> properties(propertyCount);
    if (vkEnumerateInstanceLayerProperties(&propertyCount, properties.data()) != VK_SUCCESS) {
        return false;
    }

    return std::any_of(properties.begin(), properties.end(), [layerName](const VkLayerProperties& property) {
        return std::strcmp(property.layerName, layerName) == 0;
    });
}

bool VulkanInstance::IsInstanceExtensionSupported(const char* extensionName) {
    uint32_t propertyCount = 0;
    if (vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr) != VK_SUCCESS || propertyCount == 0) {
        return false;
    }

    std::vector<VkExtensionProperties> properties(propertyCount);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.data()) != VK_SUCCESS) {
        return false;
    }

    return std::any_of(properties.begin(), properties.end(), [extensionName](const VkExtensionProperties& property) {
        return std::strcmp(property.extensionName, extensionName) == 0;
    });
}

bool VulkanInstance::TryAddExtension(std::vector<const char*>& extensions, const char* extensionName) {
    if (extensionName == nullptr) {
        return false;
    }

    if (!IsInstanceExtensionSupported(extensionName)) {
        logger.Infof("[NkVulkan] Instance extension unavailable: %s\n", extensionName);
        return false;
    }

    extensions.push_back(extensionName);
    return true;
}

} // namespace graphics::vulkan
