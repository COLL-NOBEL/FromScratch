#include "engine/VulkanRenderer.h"

#include "engine/VulkanShaderData.h"

#include "../../nkMath/NkMathUtils.h"

#include "NKLogger/NkLog.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <set>

namespace graphics::vulkan {

namespace {

constexpr std::array<const char*, 1> kRequiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

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
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
        default: return "VK_ERROR_UNKNOWN";
    }
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(kRequiredDeviceExtensions.begin(), kRequiredDeviceExtensions.end());

    for (const VkExtensionProperties& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VkShaderModule CreateShaderModule(VkDevice device, const uint32_t* code, size_t sizeInBytes) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = sizeInBytes;
    createInfo.pCode = code;

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

const std::array<VulkanRenderer::Vertex, 8> kCubeVertices = {{
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.2f, 0.2f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.2f, 1.0f, 0.2f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.2f, 0.2f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.2f}},
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.2f, 1.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.2f, 1.0f, 1.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.8f, 0.2f}},
    {{-0.5f,  0.5f,  0.5f}, {0.8f, 0.8f, 0.8f}},
}};

const std::array<uint32_t, 36> kCubeIndices = {{
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    0, 1, 5, 5, 4, 0,
    2, 3, 7, 7, 6, 2,
    1, 2, 6, 6, 5, 1,
    3, 0, 4, 4, 7, 3,
}};

} // namespace

VkVertexInputBindingDescription VulkanRenderer::Vertex::MakeBindingDescription() {
    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(Vertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding;
}

std::array<VkVertexInputAttributeDescription, 2> VulkanRenderer::Vertex::MakeAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributes{};

    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[0].offset = offsetof(Vertex, position);

    attributes[1].binding = 0;
    attributes[1].location = 1;
    attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[1].offset = offsetof(Vertex, color);

    return attributes;
}

bool VulkanRenderer::Initialize(
    VkInstance instance,
    const nkentseu::NkSurfaceDesc& surfaceDesc,
    uint32_t windowWidth,
    uint32_t windowHeight
) {
    mInstance = instance;

    if (mInstance == VK_NULL_HANDLE) {
        logger.Error("[NkVulkan] Renderer initialization failed: invalid VkInstance");
        return false;
    }

    if (!CreateSurface(surfaceDesc)) {
        Shutdown();
        return false;
    }

    if (!PickPhysicalDevice()) {
        Shutdown();
        return false;
    }

    if (!CreateLogicalDevice()) {
        Shutdown();
        return false;
    }

    if (!CreateCommandPool()) {
        Shutdown();
        return false;
    }

    if (!CreatePipelineLayout()) {
        Shutdown();
        return false;
    }

    if (!CreateMeshBuffers()) {
        Shutdown();
        return false;
    }

    if (!CreateSwapchain(windowWidth, windowHeight) ||
        !CreateImageViews() ||
        !CreateRenderPass() ||
        !CreateGraphicsPipeline() ||
        !CreateDepthResources() ||
        !CreateFramebuffers() ||
        !CreateCommandBuffers() ||
        !CreateSyncObjects()) {
        Shutdown();
        return false;
    }

    logger.Info("[NkVulkan] Vulkan renderer initialized with swapchain and graphics pipeline\n");
    return true;
}

void VulkanRenderer::Render(
    float sceneTimeSeconds,
    const NkMat4x4<float>& viewMatrix,
    const NkMat4x4<float>& projectionMatrix,
    uint32_t windowWidth,
    uint32_t windowHeight
) {
    if (!IsInitialized()) {
        return;
    }

    if (windowWidth == 0 || windowHeight == 0) {
        return;
    }

    vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(
        mDevice,
        mSwapchain,
        UINT64_MAX,
        mImageAvailableSemaphores[mCurrentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain(windowWidth, windowHeight);
        return;
    }

    if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
        logger.Infof(
            "[NkVulkan] vkAcquireNextImageKHR failed: %s (%d)\n",
            VkResultToString(acquireResult),
            static_cast<int>(acquireResult)
        );
        return;
    }

    vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);

    vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], 0);
    RecordCommandBuffer(
        mCommandBuffers[mCurrentFrame],
        imageIndex,
        sceneTimeSeconds,
        viewMatrix,
        projectionMatrix
    );

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentFrame]};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[mCurrentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    const VkResult submitResult = vkQueueSubmit(
        mGraphicsQueue,
        1,
        &submitInfo,
        mInFlightFences[mCurrentFrame]
    );

    if (submitResult != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkQueueSubmit failed: %s (%d)\n",
            VkResultToString(submitResult),
            static_cast<int>(submitResult)
        );
        return;
    }

    VkSwapchainKHR swapchains[] = {mSwapchain};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    const VkResult presentResult = vkQueuePresentKHR(mPresentQueue, &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        RecreateSwapchain(windowWidth, windowHeight);
    } else if (presentResult != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkQueuePresentKHR failed: %s (%d)\n",
            VkResultToString(presentResult),
            static_cast<int>(presentResult)
        );
    }

    mCurrentFrame = (mCurrentFrame + 1) % kMaxFramesInFlight;
}

void VulkanRenderer::Shutdown() {
    if (mDevice != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(mDevice);
    }

    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
        if (mImageAvailableSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
            mImageAvailableSemaphores[i] = VK_NULL_HANDLE;
        }

        if (mRenderFinishedSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
            mRenderFinishedSemaphores[i] = VK_NULL_HANDLE;
        }

        if (mInFlightFences[i] != VK_NULL_HANDLE) {
            vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
            mInFlightFences[i] = VK_NULL_HANDLE;
        }
    }

    if (!mCommandBuffers.empty() && mCommandPool != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(
            mDevice,
            mCommandPool,
            static_cast<uint32_t>(mCommandBuffers.size()),
            mCommandBuffers.data()
        );
    }
    mCommandBuffers.clear();

    CleanupSwapchain();

    DestroyBuffer(mCubeMesh.vertexBuffer, mCubeMesh.vertexMemory);
    DestroyBuffer(mCubeMesh.indexBuffer, mCubeMesh.indexMemory);
    mCubeMesh.indexCount = 0;

    if (mPipelineLayout != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
        mPipelineLayout = VK_NULL_HANDLE;
    }

    if (mCommandPool != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
        mCommandPool = VK_NULL_HANDLE;
    }

    if (mDevice != VK_NULL_HANDLE) {
        vkDestroyDevice(mDevice, nullptr);
        mDevice = VK_NULL_HANDLE;
    }

    if (mSurface != VK_NULL_HANDLE && mInstance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        mSurface = VK_NULL_HANDLE;
    }

    mPhysicalDevice = VK_NULL_HANDLE;
    mGraphicsQueue = VK_NULL_HANDLE;
    mPresentQueue = VK_NULL_HANDLE;
    mCurrentFrame = 0;
    mInstance = VK_NULL_HANDLE;
}

bool VulkanRenderer::CreateSurface(const nkentseu::NkSurfaceDesc& surfaceDesc) {
#if defined(NKENTSEU_PLATFORM_WINDOWS)
    if (surfaceDesc.hwnd == nullptr || surfaceDesc.hinstance == nullptr) {
        logger.Error("[NkVulkan] Win32 surface data is invalid");
        return false;
    }

    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = surfaceDesc.hwnd;
    createInfo.hinstance = surfaceDesc.hinstance;

    const VkResult result = vkCreateWin32SurfaceKHR(mInstance, &createInfo, nullptr, &mSurface);
#elif defined(NKENTSEU_WINDOWING_XCB)
    if (surfaceDesc.connection == nullptr || surfaceDesc.window == 0) {
        logger.Error("[NkVulkan] XCB surface data is invalid");
        return false;
    }

    VkXcbSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.connection = surfaceDesc.connection;
    createInfo.window = surfaceDesc.window;

    const VkResult result = vkCreateXcbSurfaceKHR(mInstance, &createInfo, nullptr, &mSurface);
#elif defined(NKENTSEU_WINDOWING_XLIB)
    if (surfaceDesc.display == nullptr || surfaceDesc.window == 0) {
        logger.Error("[NkVulkan] Xlib surface data is invalid");
        return false;
    }

    VkXlibSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.dpy = surfaceDesc.display;
    createInfo.window = surfaceDesc.window;

    const VkResult result = vkCreateXlibSurfaceKHR(mInstance, &createInfo, nullptr, &mSurface);
#elif defined(NKENTSEU_WINDOWING_WAYLAND)
    if (surfaceDesc.display == nullptr || surfaceDesc.surface == nullptr) {
        logger.Error("[NkVulkan] Wayland surface data is invalid");
        return false;
    }

    VkWaylandSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.display = surfaceDesc.display;
    createInfo.surface = surfaceDesc.surface;

    const VkResult result = vkCreateWaylandSurfaceKHR(mInstance, &createInfo, nullptr, &mSurface);
#elif defined(NKENTSEU_PLATFORM_MACOS)
    if (surfaceDesc.metalLayer == nullptr) {
        logger.Error("[NkVulkan] Metal layer is missing for macOS Vulkan surface");
        return false;
    }

    VkMetalSurfaceCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    createInfo.pLayer = surfaceDesc.metalLayer;

    const VkResult result = vkCreateMetalSurfaceEXT(mInstance, &createInfo, nullptr, &mSurface);
#else
    logger.Error("[NkVulkan] Unsupported platform for Vulkan surface creation");
    return false;
#endif

    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] Surface creation failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    return true;
}

bool VulkanRenderer::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        logger.Error("[NkVulkan] No Vulkan physical devices available");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

    for (VkPhysicalDevice device : devices) {
        QueueFamilyIndices indices = FindQueueFamilies(device);
        const bool extensionSupported = CheckDeviceExtensionSupport(device);

        bool swapchainAdequate = false;
        if (extensionSupported) {
            SwapchainSupportDetails details = QuerySwapchainSupport(device);
            swapchainAdequate = !details.formats.empty() && !details.presentModes.empty();
        }

        if (indices.IsComplete() && extensionSupported && swapchainAdequate) {
            mPhysicalDevice = device;
            break;
        }
    }

    if (mPhysicalDevice == VK_NULL_HANDLE) {
        logger.Error("[NkVulkan] Failed to find a suitable physical device");
        return false;
    }

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);
    logger.Infof("[NkVulkan] Using GPU: %s\n", properties.deviceName);

    return true;
}

bool VulkanRenderer::CreateLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);

    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value(),
    };

    const float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(kRequiredDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = kRequiredDeviceExtensions.data();

    const VkResult result = vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice);
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkCreateDevice failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);

    return true;
}

bool VulkanRenderer::CreateCommandPool() {
    const QueueFamilyIndices queueFamilies = FindQueueFamilies(mPhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();

    const VkResult result = vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool);
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkCreateCommandPool failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    return true;
}

bool VulkanRenderer::CreateSwapchain(uint32_t windowWidth, uint32_t windowHeight) {
    const SwapchainSupportDetails support = QuerySwapchainSupport(mPhysicalDevice);

    if (support.formats.empty() || support.presentModes.empty()) {
        logger.Error("[NkVulkan] Swapchain support is incomplete for selected device");
        return false;
    }

    const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(support.formats);
    const VkPresentModeKHR presentMode = ChoosePresentMode(support.presentModes);
    const VkExtent2D extent = ChooseSwapExtent(support.capabilities, windowWidth, windowHeight);

    uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount) {
        imageCount = support.capabilities.maxImageCount;
    }

    const QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
    const uint32_t queueFamilyIndices[] = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value(),
    };

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    const VkResult result = vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain);
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkCreateSwapchainKHR failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
    mSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

    mSwapchainImageFormat = surfaceFormat.format;
    mSwapchainExtent = extent;

    return true;
}

bool VulkanRenderer::CreateImageViews() {
    mSwapchainImageViews.resize(mSwapchainImages.size());

    for (size_t i = 0; i < mSwapchainImages.size(); ++i) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = mSwapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = mSwapchainImageFormat;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        const VkResult result = vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapchainImageViews[i]);
        if (result != VK_SUCCESS) {
            logger.Infof(
                "[NkVulkan] vkCreateImageView failed: %s (%d)\n",
                VkResultToString(result),
                static_cast<int>(result)
            );
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::CreateRenderPass() {
    mDepthFormat = FindDepthFormat();
    if (mDepthFormat == VK_FORMAT_UNDEFINED) {
        logger.Error("[NkVulkan] Failed to find supported depth format");
        return false;
    }

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = mSwapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = mDepthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    const std::array<VkAttachmentDescription, 2> attachments = {
        colorAttachment,
        depthAttachment,
    };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    const VkResult result = vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass);
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkCreateRenderPass failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    return true;
}

bool VulkanRenderer::CreatePipelineLayout() {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    const VkResult result = vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout);
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkCreatePipelineLayout failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    return true;
}

bool VulkanRenderer::CreateGraphicsPipeline() {
    VkShaderModule vertexShader = CreateShaderModule(
        mDevice,
        shaderdata::kCubeVertexShaderSpv,
        sizeof(shaderdata::kCubeVertexShaderSpv)
    );

    VkShaderModule fragmentShader = CreateShaderModule(
        mDevice,
        shaderdata::kCubeFragmentShaderSpv,
        sizeof(shaderdata::kCubeFragmentShaderSpv)
    );

    if (vertexShader == VK_NULL_HANDLE || fragmentShader == VK_NULL_HANDLE) {
        if (vertexShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mDevice, vertexShader, nullptr);
        }
        if (fragmentShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(mDevice, fragmentShader, nullptr);
        }
        logger.Error("[NkVulkan] Failed to create shader modules");
        return false;
    }

    VkPipelineShaderStageCreateInfo vertexStageInfo{};
    vertexStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageInfo.module = vertexShader;
    vertexStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentStageInfo{};
    fragmentStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageInfo.module = fragmentShader;
    fragmentStageInfo.pName = "main";

    const VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertexStageInfo,
        fragmentStageInfo,
    };

    const VkVertexInputBindingDescription bindingDescription = Vertex::MakeBindingDescription();
    const std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = Vertex::MakeAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(mSwapchainExtent.width);
    viewport.height = static_cast<float>(mSwapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = mSwapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    const std::array<VkDynamicState, 2> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = mPipelineLayout;
    pipelineInfo.renderPass = mRenderPass;
    pipelineInfo.subpass = 0;

    const VkResult result = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline);

    vkDestroyShaderModule(mDevice, fragmentShader, nullptr);
    vkDestroyShaderModule(mDevice, vertexShader, nullptr);

    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkCreateGraphicsPipelines failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    return true;
}

bool VulkanRenderer::CreateDepthResources() {
    if (!CreateImage(
            mSwapchainExtent.width,
            mSwapchainExtent.height,
            mDepthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mDepthImage,
            mDepthImageMemory
        )) {
        return false;
    }

    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (HasStencilComponent(mDepthFormat)) {
        aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = mDepthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = mDepthFormat;
    viewInfo.subresourceRange.aspectMask = aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    const VkResult result = vkCreateImageView(mDevice, &viewInfo, nullptr, &mDepthImageView);
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkCreateImageView(depth) failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    return true;
}

bool VulkanRenderer::CreateFramebuffers() {
    mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

    for (size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
        const std::array<VkImageView, 2> attachments = {
            mSwapchainImageViews[i],
            mDepthImageView,
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = mRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = mSwapchainExtent.width;
        framebufferInfo.height = mSwapchainExtent.height;
        framebufferInfo.layers = 1;

        const VkResult result = vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]);
        if (result != VK_SUCCESS) {
            logger.Infof(
                "[NkVulkan] vkCreateFramebuffer failed: %s (%d)\n",
                VkResultToString(result),
                static_cast<int>(result)
            );
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::CreateMeshBuffers() {
    const VkDeviceSize vertexBufferSize = sizeof(kCubeVertices[0]) * kCubeVertices.size();
    const VkDeviceSize indexBufferSize = sizeof(kCubeIndices[0]) * kCubeIndices.size();

    if (!CreateBuffer(
            vertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            mCubeMesh.vertexBuffer,
            mCubeMesh.vertexMemory
        )) {
        return false;
    }

    void* mappedData = nullptr;
    vkMapMemory(mDevice, mCubeMesh.vertexMemory, 0, vertexBufferSize, 0, &mappedData);
    std::memcpy(mappedData, kCubeVertices.data(), static_cast<size_t>(vertexBufferSize));
    vkUnmapMemory(mDevice, mCubeMesh.vertexMemory);

    if (!CreateBuffer(
            indexBufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            mCubeMesh.indexBuffer,
            mCubeMesh.indexMemory
        )) {
        return false;
    }

    vkMapMemory(mDevice, mCubeMesh.indexMemory, 0, indexBufferSize, 0, &mappedData);
    std::memcpy(mappedData, kCubeIndices.data(), static_cast<size_t>(indexBufferSize));
    vkUnmapMemory(mDevice, mCubeMesh.indexMemory);

    mCubeMesh.indexCount = static_cast<uint32_t>(kCubeIndices.size());
    return true;
}

bool VulkanRenderer::CreateCommandBuffers() {
    mCommandBuffers.resize(kMaxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

    const VkResult result = vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data());
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkAllocateCommandBuffers failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    return true;
}

bool VulkanRenderer::CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS) {
            logger.Error("[NkVulkan] Failed to create synchronization objects");
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::RecreateSwapchain(uint32_t windowWidth, uint32_t windowHeight) {
    if (windowWidth == 0 || windowHeight == 0) {
        return false;
    }

    vkDeviceWaitIdle(mDevice);
    CleanupSwapchain();

    if (!CreateSwapchain(windowWidth, windowHeight) ||
        !CreateImageViews() ||
        !CreateRenderPass() ||
        !CreateGraphicsPipeline() ||
        !CreateDepthResources() ||
        !CreateFramebuffers()) {
        logger.Error("[NkVulkan] Swapchain recreation failed");
        return false;
    }

    return true;
}

void VulkanRenderer::CleanupSwapchain() {
    for (VkFramebuffer framebuffer : mSwapchainFramebuffers) {
        if (framebuffer != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
        }
    }
    mSwapchainFramebuffers.clear();

    DestroyImage(mDepthImage, mDepthImageMemory, mDepthImageView);

    if (mGraphicsPipeline != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
        mGraphicsPipeline = VK_NULL_HANDLE;
    }

    if (mRenderPass != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
        mRenderPass = VK_NULL_HANDLE;
    }

    for (VkImageView imageView : mSwapchainImageViews) {
        if (imageView != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
            vkDestroyImageView(mDevice, imageView, nullptr);
        }
    }
    mSwapchainImageViews.clear();

    if (mSwapchain != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
        mSwapchain = VK_NULL_HANDLE;
    }
    mSwapchainImages.clear();
}

void VulkanRenderer::RecordCommandBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex,
    float sceneTimeSeconds,
    const NkMat4x4<float>& viewMatrix,
    const NkMat4x4<float>& projectionMatrix
) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.06f, 0.08f, 0.12f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass;
    renderPassInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = mSwapchainExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(mSwapchainExtent.width);
    viewport.height = static_cast<float>(mSwapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = mSwapchainExtent;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mCubeMesh.vertexBuffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, mCubeMesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    for (uint32_t instance = 0; instance < 2; ++instance) {
        const NkMat4x4<float> modelMatrix = BuildCubeModelMatrix(sceneTimeSeconds, instance);
        const NkMat4x4<float> mvp = projectionMatrix * viewMatrix * modelMatrix;

        PushConstants push{};
        MatrixToColumnMajorArray(mvp, push.mvp);

        vkCmdPushConstants(
            commandBuffer,
            mPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(PushConstants),
            &push
        );

        vkCmdDrawIndexed(commandBuffer, mCubeMesh.indexCount, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);
    vkEndCommandBuffer(commandBuffer);
}

VulkanRenderer::QueueFamilyIndices VulkanRenderer::FindQueueFamilies(VkPhysicalDevice device) const {
    QueueFamilyIndices indices{};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
        if (presentSupport == VK_TRUE) {
            indices.presentFamily = i;
        }

        if (indices.IsComplete()) {
            break;
        }
    }

    return indices;
}

VulkanRenderer::SwapchainSupportDetails VulkanRenderer::QuerySwapchainSupport(VkPhysicalDevice device) const {
    SwapchainSupportDetails details{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            mSurface,
            &presentModeCount,
            details.presentModes.data()
        );
    }

    return details;
}

VkSurfaceFormatKHR VulkanRenderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const {
    for (const VkSurfaceFormatKHR& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return formats.front();
}

VkPresentModeKHR VulkanRenderer::ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) const {
    for (const VkPresentModeKHR mode : presentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    uint32_t windowWidth,
    uint32_t windowHeight
) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    VkExtent2D actualExtent{};
    actualExtent.width = std::clamp(windowWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(windowHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actualExtent;
}

VkFormat VulkanRenderer::FindDepthFormat() const {
    const std::array<VkFormat, 3> candidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };

    for (VkFormat format : candidates) {
        VkFormatProperties properties{};
        vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &properties);

        if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

bool VulkanRenderer::HasStencilComponent(VkFormat format) const {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

bool VulkanRenderer::CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& outBuffer,
    VkDeviceMemory& outMemory
) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(mDevice, &bufferInfo, nullptr, &outBuffer);
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkCreateBuffer failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(mDevice, outBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(mDevice, &allocInfo, nullptr, &outMemory);
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkAllocateMemory(buffer) failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        vkDestroyBuffer(mDevice, outBuffer, nullptr);
        outBuffer = VK_NULL_HANDLE;
        return false;
    }

    vkBindBufferMemory(mDevice, outBuffer, outMemory, 0);
    return true;
}

bool VulkanRenderer::CreateImage(
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage& outImage,
    VkDeviceMemory& outMemory
) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(mDevice, &imageInfo, nullptr, &outImage);
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkCreateImage failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        return false;
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(mDevice, outImage, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(mDevice, &allocInfo, nullptr, &outMemory);
    if (result != VK_SUCCESS) {
        logger.Infof(
            "[NkVulkan] vkAllocateMemory(image) failed: %s (%d)\n",
            VkResultToString(result),
            static_cast<int>(result)
        );
        vkDestroyImage(mDevice, outImage, nullptr);
        outImage = VK_NULL_HANDLE;
        return false;
    }

    vkBindImageMemory(mDevice, outImage, outMemory, 0);
    return true;
}

uint32_t VulkanRenderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        const bool isTypeSupported = (typeFilter & (1u << i)) != 0;
        const bool hasRequestedFlags = (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;

        if (isTypeSupported && hasRequestedFlags) {
            return i;
        }
    }

    logger.Error("[NkVulkan] Failed to find suitable memory type");
    return 0;
}

void VulkanRenderer::DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& memory) {
    if (buffer != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkDestroyBuffer(mDevice, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
    }

    if (memory != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkFreeMemory(mDevice, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::DestroyImage(VkImage& image, VkDeviceMemory& memory, VkImageView& imageView) {
    if (imageView != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkDestroyImageView(mDevice, imageView, nullptr);
        imageView = VK_NULL_HANDLE;
    }

    if (image != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkDestroyImage(mDevice, image, nullptr);
        image = VK_NULL_HANDLE;
    }

    if (memory != VK_NULL_HANDLE && mDevice != VK_NULL_HANDLE) {
        vkFreeMemory(mDevice, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::MatrixToColumnMajorArray(const NkMat4x4<float>& matrix, float out[16]) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            out[col * 4 + row] = matrix(row, col);
        }
    }
}

NkMat4x4<float> VulkanRenderer::BuildCubeModelMatrix(float sceneTimeSeconds, uint32_t instanceIndex) {
    const float phaseOffset = (instanceIndex == 0) ? 0.0f : 1.7f;
    const float orbitSpeed = (instanceIndex == 0) ? 1.05f : 0.8f;
    const float spinSpeedY = (instanceIndex == 0) ? 58.0f : 42.0f;
    const float spinSpeedX = (instanceIndex == 0) ? 28.0f : 36.0f;

    const float x = std::sin(sceneTimeSeconds * orbitSpeed + phaseOffset) * 0.9f;
    const float y = std::sin(sceneTimeSeconds * (orbitSpeed * 1.35f) + phaseOffset) * 0.45f;
    const float z = -0.45f + std::cos(sceneTimeSeconds * (orbitSpeed * 0.7f) + phaseOffset) * 0.4f;

    const float rotationY = sceneTimeSeconds * NkMathUtils::deg2rad(spinSpeedY);
    const float rotationX = sceneTimeSeconds * NkMathUtils::deg2rad(spinSpeedX);
    const float scale = (instanceIndex == 0) ? 1.0f : 0.78f;

    return NkMat4x4<float>::Translation(x, y, z)
         * NkMat4x4<float>::RotationY(rotationY)
         * NkMat4x4<float>::RotationX(rotationX)
         * NkMat4x4<float>::Scale(scale, scale, scale);
}

} // namespace graphics::vulkan
