#pragma once
#include <Ghost/Core/ghostCommandPool.hpp>
#include <Ghost/Core/ghostSwapchain.hpp>
#include <Ghost/Core/vulkanDevice.hpp>
#include <cstdint>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
const int MAX_FRAMES_IN_FLIGHT = 3;
class GhostRenderer {
  public:
    GhostRenderer(VulkanDevice &device, vk::SurfaceKHR surface, uint32_t width,
                  uint32_t height);
    ~GhostRenderer();

    vk::raii::CommandBuffer &beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(const vk::raii::CommandBuffer &commandBuffer);
    void endSwapChainRenderPass(const vk::raii::CommandBuffer &commandBuffer);

    int getFrameIndex() const { return m_currentFrame; }
    vk::RenderPass getSwapChainRenderPass() const { return m_renderPass; }

    bool isFrameInProgress() { return m_isFrameStarted; }

    vk::Extent2D getSwapchainExtent() const {
        return m_swapchain->getSwapchainExtent();
    }
    void flagFramebufferResized(uint32_t width, uint32_t height);

  private:
    void createCommandBuffers();
    void createSyncObjects();
    void recreateSwapchain();

    void createRenderPass();
    void createFramebuffers();

    VulkanDevice &m_device;
    vk::Extent2D m_extent;
    vk::SurfaceKHR m_surface;

    std::unique_ptr<GhostSwapchain> m_swapchain;
    GhostCommandPool m_commandPool;
    std::vector<vk::raii::CommandBuffer> m_commandBuffers;

    vk::raii::RenderPass m_renderPass = nullptr;
    std::vector<vk::raii::Framebuffer> m_framebuffers;

    std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_inFlightFences;

    uint32_t m_currentFrame = 0;
    uint32_t m_currentImageIndex = 0;
    bool m_isFrameStarted = false;
    bool m_framebufferResized = false;
};
} // namespace Ghost
