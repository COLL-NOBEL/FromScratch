#include "engine/VulkanContext.h"

#include "../../nkMath/NkMathUtils.h"

#include <cmath>

namespace graphics::vulkan {

bool VulkanContext::Initialize(const VulkanContextCreateInfo& createInfo) {
    VulkanInstanceConfig instanceConfig{};
    instanceConfig.applicationName = createInfo.applicationName;
    instanceConfig.engineName = createInfo.engineName;
    instanceConfig.enableValidationLayers = createInfo.enableValidationLayers;
    instanceConfig.enableDebugUtils = createInfo.enableDebugUtils;

    if (!mInstance.Create(instanceConfig)) {
        return false;
    }

    mCamera.position = NkVec3<float>(0.0f, 1.0f, 4.0f);
    mCamera.up = NkVec3<float>(0.0f, 1.0f, 0.0f).Normalize();
    mCamera.yawRadians = NkMathUtils::deg2rad(-90.0f);
    mCamera.pitchRadians = NkMathUtils::deg2rad(-12.0f);

    RebuildViewMatrix();
    return true;
}

void VulkanContext::Update(float deltaSeconds) {
    if (!mInstance.IsValid()) {
        return;
    }

    const float dt = NkMathUtils::clamp(deltaSeconds, 0.0f, 0.1f);

    // Minimal animated camera state to actively use nkMath vectors/matrices.
    mCamera.yawRadians += NkMathUtils::deg2rad(20.0f) * dt;

    const float pitchSwing = std::sin(mCamera.yawRadians * 0.5f) * NkMathUtils::deg2rad(6.0f);
    mCamera.pitchRadians = NkMathUtils::clamp(
        pitchSwing,
        NkMathUtils::deg2rad(-30.0f),
        NkMathUtils::deg2rad(30.0f)
    );

    RebuildViewMatrix();
}

void VulkanContext::Shutdown() {
    mInstance.Destroy();
}

void VulkanContext::RebuildViewMatrix() {
    const float cosPitch = std::cos(mCamera.pitchRadians);

    NkVec3<float> forward(
        std::cos(mCamera.yawRadians) * cosPitch,
        std::sin(mCamera.pitchRadians),
        std::sin(mCamera.yawRadians) * cosPitch
    );

    mCamera.forward = forward.Normalize();

    const NkMat4x4<float> rotation =
        NkMat4x4<float>::RotationY(-mCamera.yawRadians) *
        NkMat4x4<float>::RotationX(-mCamera.pitchRadians);

    const NkMat4x4<float> translation = NkMat4x4<float>::Translation(
        -mCamera.position[0],
        -mCamera.position[1],
        -mCamera.position[2]
    );

    mCamera.viewMatrix = rotation * translation;
}

} // namespace graphics::vulkan
