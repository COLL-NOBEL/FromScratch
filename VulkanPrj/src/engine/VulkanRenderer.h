#pragma once

#include "engine/VulkanCommon.h"

#include "../../nkMath/NkMat4x4.h"

#include "NKWindow/Core/NkSurface.h"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace graphics::vulkan {

class VulkanRenderer {
public:
    bool Initialize(
        VkInstance instance,
        const nkentseu::NkSurfaceDesc& surfaceDesc,
        uint32_t windowWidth,
        uint32_t windowHeight
    );

    void Render(
        float sceneTimeSeconds,
        const NkMat4x4<float>& viewMatrix,
        const NkMat4x4<float>& projectionMatrix,
        uint32_t windowWidth,
        uint32_t windowHeight
    );

    void Shutdown();

    [[nodiscard]] bool IsInitialized() const {
        return mDevice != VK_NULL_HANDLE && mSwapchain != VK_NULL_HANDLE;
    }

public:
    struct Vertex {
        float position[3];
        float color[3];

        static VkVertexInputBindingDescription MakeBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 2> MakeAttributeDescriptions();
    };

private:
    static constexpr uint32_t kMaxFramesInFlight = 2;

    struct MeshBuffers {
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory indexMemory = VK_NULL_HANDLE;
        uint32_t indexCount = 0;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool IsComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct PushConstants {
        float mvp[16];
    };

    bool CreateSurface(const nkentseu::NkSurfaceDesc& surfaceDesc);
    bool PickPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateCommandPool();
    bool CreateSwapchain(uint32_t windowWidth, uint32_t windowHeight);
    bool CreateImageViews();
    bool CreateRenderPass();
    bool CreatePipelineLayout();
    bool CreateGraphicsPipeline();
    bool CreateDepthResources();
    bool CreateFramebuffers();
    bool CreateMeshBuffers();
    bool CreateCommandBuffers();
    bool CreateSyncObjects();

    bool RecreateSwapchain(uint32_t windowWidth, uint32_t windowHeight);
    void CleanupSwapchain();

    void RecordCommandBuffer(
        VkCommandBuffer commandBuffer,
        uint32_t imageIndex,
        float sceneTimeSeconds,
        const NkMat4x4<float>& viewMatrix,
        const NkMat4x4<float>& projectionMatrix
    );

    [[nodiscard]] QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
    [[nodiscard]] SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device) const;
    [[nodiscard]] VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const;
    [[nodiscard]] VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) const;
    [[nodiscard]] VkExtent2D ChooseSwapExtent(
        const VkSurfaceCapabilitiesKHR& capabilities,
        uint32_t windowWidth,
        uint32_t windowHeight
    ) const;

    [[nodiscard]] VkFormat FindDepthFormat() const;
    [[nodiscard]] bool HasStencilComponent(VkFormat format) const;

    bool CreateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& outBuffer,
        VkDeviceMemory& outMemory
    );

    bool CreateImage(
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& outImage,
        VkDeviceMemory& outMemory
    );

    [[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    void DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& memory);
    void DestroyImage(VkImage& image, VkDeviceMemory& memory, VkImageView& imageView);

    static void MatrixToColumnMajorArray(const NkMat4x4<float>& matrix, float out[16]);
    static NkMat4x4<float> BuildCubeModelMatrix(float sceneTimeSeconds);

    VkInstance mInstance = VK_NULL_HANDLE;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;

    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;

    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    std::vector<VkImage> mSwapchainImages;
    std::vector<VkImageView> mSwapchainImageViews;
    std::vector<VkFramebuffer> mSwapchainFramebuffers;
    VkFormat mSwapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D mSwapchainExtent{};

    VkRenderPass mRenderPass = VK_NULL_HANDLE;
    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;

    VkImage mDepthImage = VK_NULL_HANDLE;
    VkDeviceMemory mDepthImageMemory = VK_NULL_HANDLE;
    VkImageView mDepthImageView = VK_NULL_HANDLE;
    VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;

    MeshBuffers mCubeMesh;

    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> mCommandBuffers;

    std::array<VkSemaphore, kMaxFramesInFlight> mImageAvailableSemaphores{};
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::array<VkFence, kMaxFramesInFlight> mInFlightFences{};

    uint32_t mCurrentFrame = 0;
};

} // namespace graphics::vulkan
