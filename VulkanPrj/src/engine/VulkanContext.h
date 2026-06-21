#pragma once

#include "engine/VulkanInstance.h"
#include "engine/VulkanRenderer.h"

#include "../../nkMath/NkMat4x4.h"
#include "../../nkMath/NkVec3.h"

namespace nkentseu {
class NkWindow;
}

namespace graphics::vulkan {

struct CameraState {
    NkVec3<float> position = NkVec3<float>(0.0f, 1.0f, 4.0f);
    NkVec3<float> forward = NkVec3<float>(0.0f, 0.0f, -1.0f);
    NkVec3<float> up = NkVec3<float>(0.0f, 1.0f, 0.0f);

    float yawRadians = 0.0f;
    float pitchRadians = 0.0f;

    float verticalFovRadians = 1.0471976f; // 60 degrees
    float nearClip = 0.1f;
    float farClip = 100.0f;

    NkMat4x4<float> viewMatrix;
    NkMat4x4<float> projectionMatrix;
};

struct VulkanContextCreateInfo {
    const char* applicationName = "VKGraphicsEngine";
    const char* engineName = "SimpleVulkanEngine";
    bool enableValidationLayers = true;
    bool enableDebugUtils = true;
};

class VulkanContext {
public:
    bool Initialize(nkentseu::NkWindow& window, const VulkanContextCreateInfo& createInfo);
    void Update(float deltaSeconds);
    void RenderFrame(float sceneTimeSeconds);
    void Shutdown();

    [[nodiscard]] bool IsInitialized() const {
        return mInstance.IsValid() && mRenderer.IsInitialized();
    }

    [[nodiscard]] VkInstance GetInstance() const { return mInstance.GetHandle(); }
    [[nodiscard]] const CameraState& GetCamera() const { return mCamera; }

private:
    void RebuildViewMatrix();
    void RebuildProjectionMatrix(float aspectRatio);

    nkentseu::NkWindow* mWindow = nullptr;

    VulkanInstance mInstance;
    VulkanRenderer mRenderer;
    CameraState mCamera;
    float mCameraAnimationTimeSeconds = 0.0f;
};

} // namespace graphics::vulkan
