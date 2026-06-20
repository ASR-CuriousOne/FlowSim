#pragma once
#include <Ghost/Core/ghostImage.hpp>
#include <Ghost/Core/vulkanDevice.hpp>
#include <cstdint>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
class GhostSwapchain {
    vk::raii::SwapchainKHR m_swapchain = nullptr;
    std::vector<vk::Image> m_swapchainImages;
    std::vector<vk::raii::ImageView> m_swapchainImageViews;

    vk::Format m_swapchainImageFormat;
    vk::Extent2D m_swapchainExtent;

    vk::Format m_swapchainDepthFormat;
    std::vector<std::unique_ptr<GhostImage>> m_depthImages;

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
        std::vector<vk::SurfaceFormatKHR> &availableFormats);

    vk::PresentModeKHR chooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR> &availablePresentModes);

    vk::Extent2D
    chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities,
                     const vk::Extent2D &currentExtent);

  public:
    GhostSwapchain(VulkanDevice &device, const vk::SurfaceKHR &surface,
                   const vk::Extent2D &currentExtent);
    ~GhostSwapchain();

    vk::SwapchainKHR operator*() const { return m_swapchain; }

    size_t getImageCount() const { return m_swapchainImages.size(); }
    const vk::Extent2D &getSwapchainExtent() { return m_swapchainExtent; }

    const vk::raii::ImageView &getDepthImageView(int index) const {
        return m_depthImages[index]->getImageView();
    }

    vk::Format getSwapchainImageFormat() const {
        return m_swapchainImageFormat;
    }

    const vk::raii::ImageView &getImageView(uint32_t index) const {
        return m_swapchainImageViews[index];
    }

    vk::ResultValue<uint32_t>
    aquireNextImage(const vk::raii::Semaphore &imageAvailableSemaphore);
};
} // namespace Ghost
