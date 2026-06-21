#include "engine/VulkanContext.h"

#include "../../nkMath/NkMathUtils.h"

#include "NKWindow/NkWindow.h"

#include <cmath>

namespace graphics::vulkan {

bool VulkanContext::Initialize(nkentseu::NkWindow& window, const VulkanContextCreateInfo& createInfo) {
    mWindow = &window;

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

    const auto windowSize = mWindow->GetSize();
    const float aspectRatio =
        (windowSize.height > 0)
            ? static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height)
            : 16.0f / 9.0f;

    RebuildProjectionMatrix(aspectRatio);

    if (!mRenderer.Initialize(
            mInstance.GetHandle(),
            mWindow->GetSurfaceDesc(),
            windowSize.width,
            windowSize.height
        )) {
        mInstance.Destroy();
        mWindow = nullptr;
        return false;
    }

    return true;
}

void VulkanContext::Update(float deltaSeconds) {
    if (!IsInitialized()) {
        return;
    }

    const float dt = NkMathUtils::clamp(deltaSeconds, 0.0f, 0.1f);

    mCamera.yawRadians += NkMathUtils::deg2rad(20.0f) * dt;

    const float pitchSwing = std::sin(mCamera.yawRadians * 0.5f) * NkMathUtils::deg2rad(6.0f);
    mCamera.pitchRadians = NkMathUtils::clamp(
        pitchSwing,
        NkMathUtils::deg2rad(-30.0f),
        NkMathUtils::deg2rad(30.0f)
    );

    RebuildViewMatrix();

    if (mWindow != nullptr) {
        const auto windowSize = mWindow->GetSize();
        if (windowSize.width > 0 && windowSize.height > 0) {
            const float aspectRatio = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);
            RebuildProjectionMatrix(aspectRatio);
        }
    }
}

void VulkanContext::RenderFrame(float sceneTimeSeconds) {
    if (!IsInitialized() || mWindow == nullptr) {
        return;
    }

    const auto windowSize = mWindow->GetSize();

    mRenderer.Render(
        sceneTimeSeconds,
        mCamera.viewMatrix,
        mCamera.projectionMatrix,
        windowSize.width,
        windowSize.height
    );
}

void VulkanContext::Shutdown() {
    mRenderer.Shutdown();
    mInstance.Destroy();
    mWindow = nullptr;
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

void VulkanContext::RebuildProjectionMatrix(float aspectRatio) {
    const float safeAspectRatio = aspectRatio > 0.001f ? aspectRatio : (16.0f / 9.0f);
    const float tanHalfFov = std::tan(mCamera.verticalFovRadians * 0.5f);

    const float yScale = (tanHalfFov > 0.0001f) ? (1.0f / tanHalfFov) : 1.0f;
    const float xScale = yScale / safeAspectRatio;

    const float nearPlane = mCamera.nearClip;
    const float farPlane = mCamera.farClip;

    mCamera.projectionMatrix = NkMat4x4<float>(
        xScale, 0.0f, 0.0f, 0.0f,
        0.0f, -yScale, 0.0f, 0.0f,
        0.0f, 0.0f, farPlane / (nearPlane - farPlane), (farPlane * nearPlane) / (nearPlane - farPlane),
        0.0f, 0.0f, -1.0f, 0.0f
    );
}

} // namespace graphics::vulkan
