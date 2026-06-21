#pragma once

#include "engine/VulkanCommon.h"

#include <cstdint>
#include <vector>

namespace graphics::vulkan {

struct VulkanInstanceConfig {
    const char* applicationName = "VKGraphicsEngine";
    const char* engineName = "SimpleVulkanEngine";
    uint32_t applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    uint32_t engineVersion = VK_MAKE_VERSION(1, 0, 0);
    uint32_t apiVersion = VK_API_VERSION_1_2;
    bool enableValidationLayers = true;
    bool enableDebugUtils = true;
};

class VulkanInstance {
public:
    VulkanInstance() = default;
    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;
    ~VulkanInstance();

    bool Create(const VulkanInstanceConfig& config);
    void Destroy();

    [[nodiscard]] bool IsValid() const { return mInstance != VK_NULL_HANDLE; }
    [[nodiscard]] VkInstance GetHandle() const { return mInstance; }

private:
    static bool IsInstanceLayerSupported(const char* layerName);
    static bool IsInstanceExtensionSupported(const char* extensionName);
    static bool TryAddExtension(std::vector<const char*>& extensions, const char* extensionName);

    VkInstance mInstance = VK_NULL_HANDLE;
    std::vector<const char*> mEnabledExtensions;
    std::vector<const char*> mEnabledLayers;
};

} // namespace graphics::vulkan
