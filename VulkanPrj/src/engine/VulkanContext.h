#pragma once

#include "engine/VulkanInstance.h"

#include "../../nkMath/NkMat4x4.h"
#include "../../nkMath/NkVec3.h"

namespace graphics::vulkan {

struct CameraState {
    NkVec3<float> position = NkVec3<float>(0.0f, 1.0f, 4.0f);
    NkVec3<float> forward = NkVec3<float>(0.0f, 0.0f, -1.0f);
    NkVec3<float> up = NkVec3<float>(0.0f, 1.0f, 0.0f);

    float yawRadians = 0.0f;
    float pitchRadians = 0.0f;

    NkMat4x4<float> viewMatrix;
};

struct VulkanContextCreateInfo {
    const char* applicationName = "VKGraphicsEngine";
    const char* engineName = "SimpleVulkanEngine";
    bool enableValidationLayers = true;
    bool enableDebugUtils = true;
};

class VulkanContext {
public:
    bool Initialize(const VulkanContextCreateInfo& createInfo);
    void Update(float deltaSeconds);
    void Shutdown();

    [[nodiscard]] bool IsInitialized() const { return mInstance.IsValid(); }
    [[nodiscard]] VkInstance GetInstance() const { return mInstance.GetHandle(); }
    [[nodiscard]] const CameraState& GetCamera() const { return mCamera; }

private:
    void RebuildViewMatrix();

    VulkanInstance mInstance;
    CameraState mCamera;
};

} // namespace graphics::vulkan
