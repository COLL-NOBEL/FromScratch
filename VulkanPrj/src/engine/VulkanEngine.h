#pragma once

#include "engine/VulkanContext.h"

namespace graphics::vulkan {

class VulkanEngine {
public:
    bool Initialize();
    void Tick(float deltaSeconds);
    void Shutdown();

    [[nodiscard]] bool IsInitialized() const { return mContext.IsInitialized(); }
    [[nodiscard]] const CameraState& GetCameraState() const { return mContext.GetCamera(); }

private:
    VulkanContext mContext;
    float mUptimeSeconds = 0.0f;
};

} // namespace graphics::vulkan
