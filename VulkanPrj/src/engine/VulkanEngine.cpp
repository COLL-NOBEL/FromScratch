#include "engine/VulkanEngine.h"

#include "NKLogger/NkLog.h"

namespace graphics::vulkan {

bool VulkanEngine::Initialize(nkentseu::NkWindow& window) {
    VulkanContextCreateInfo contextInfo{};

    if (!mContext.Initialize(window, contextInfo)) {
        logger.Error("[NkVulkan] VulkanContext initialization failed");
        return false;
    }

    mUptimeSeconds = 0.0f;

    logger.Infof(
        "[NkVulkan] Vulkan graphics engine initialized (instance=%p)\n",
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
    mContext.RenderFrame(mUptimeSeconds);
}

void VulkanEngine::Shutdown() {
    if (!mContext.IsInitialized()) {
        return;
    }

    mContext.Shutdown();
    logger.Infof("[NkVulkan] Engine shutdown after %.2f seconds\n", mUptimeSeconds);
}

} // namespace graphics::vulkan
