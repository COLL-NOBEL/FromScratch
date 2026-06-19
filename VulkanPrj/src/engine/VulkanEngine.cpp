#include "engine/VulkanEngine.h"

#include "NKLogger/NkLog.h"

namespace graphics::vulkan {

bool VulkanEngine::Initialize() {
    VulkanContextCreateInfo contextInfo{};

    if (!mContext.Initialize(contextInfo)) {
        logger.Error("[NkVulkan] VulkanContext initialization failed");
        return false;
    }

    mUptimeSeconds = 0.0f;

    logger.Infof(
        "[NkVulkan] Simple Vulkan engine initialized (instance=%p)\n",
        static_cast<void*>(mContext.GetInstance())
    );

    return true;
}

void VulkanEngine::Tick(float deltaSeconds) {
    if (!mContext.IsInitialized()) {
        return;
    }

    mUptimeSeconds += deltaSeconds;
    mContext.Update(deltaSeconds);
}

void VulkanEngine::Shutdown() {
    if (!mContext.IsInitialized()) {
        return;
    }

    mContext.Shutdown();
    logger.Infof("[NkVulkan] Engine shutdown after %.2f seconds\n", mUptimeSeconds);
}

} // namespace graphics::vulkan
