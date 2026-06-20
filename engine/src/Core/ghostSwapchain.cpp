#include <Ghost/Core/ghostSwapchain.hpp>
#include <Ghost/Core/vulkanDevice.hpp>
#include <Ghost/Utils/swapChainDetails.hpp>
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
GhostSwapchain::GhostSwapchain(VulkanDevice &device,
                               const vk::SurfaceKHR &surface,
                               const vk::Extent2D &currentExtent) {
    SwapChainSupportDetails swapChainSupport;
    swapChainSupport.querySwapChainSupport(device, surface);

    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    auto extent =
        chooseSwapExtent(swapChainSupport.capabilities, currentExtent);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    QueueFamilyIndicies indices = device.getQueueFamilyIndices();
    std::vector<uint32_t> queueFamilyIndices = {indices.graphicsFamily.value(),
                                                indices.presentFamily.value()};
    bool isConcurrent = (indices.graphicsFamily != indices.presentFamily);

    auto createInfo =
        vk::SwapchainCreateInfoKHR()
            .setSurface(surface)
            .setMinImageCount(imageCount)
            .setImageFormat(surfaceFormat.format)
            .setImageColorSpace(surfaceFormat.colorSpace)
            .setImageExtent(extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode(isConcurrent ? vk::SharingMode::eConcurrent
                                              : vk::SharingMode::eExclusive)
            .setPreTransform(swapChainSupport.capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(presentMode)
            .setClipped(vk::True)
            .setOldSwapchain(nullptr);

    if (isConcurrent)
        createInfo.setQueueFamilyIndices(queueFamilyIndices);

    const vk::raii::Device &raiiDevice = device;

    VkSwapchainKHR rawSwapchain = nullptr;
    VkResult result = vkCreateSwapchainKHR(
        *raiiDevice,
        reinterpret_cast<const VkSwapchainCreateInfoKHR *>(&createInfo),
        nullptr, &rawSwapchain);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to create swapchain!");
    }

    m_swapchain = vk::raii::SwapchainKHR(raiiDevice, rawSwapchain);

    m_swapchainImages = m_swapchain.getImages();
    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;

    m_swapchainImageViews.reserve(m_swapchainImages.size());

    for (const auto &image : m_swapchainImages) {
        vk::ImageViewCreateInfo createInfo(
            {}, image, vk::ImageViewType::e2D, m_swapchainImageFormat, {},
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0,
                                      1));

        m_swapchainImageViews.emplace_back(device, createInfo);
    }

    m_swapchainDepthFormat = device.findDepthFormat();

    m_depthImages.reserve(imageCount);

    for (int i = 0; i < imageCount; i++) {
        m_depthImages.push_back(std::make_unique<GhostImage>(
            device, m_swapchainExtent.width, m_swapchainExtent.height,
            m_swapchainDepthFormat, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eDepthStencilAttachment,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::ImageAspectFlagBits::eDepth));
    }
}

GhostSwapchain::~GhostSwapchain() {}

vk::SurfaceFormatKHR GhostSwapchain::chooseSwapSurfaceFormat(
    std::vector<vk::SurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR GhostSwapchain::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D
GhostSwapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities,
                                 const vk::Extent2D &currentExtent) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {

        VkExtent2D actualExtent = currentExtent;

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

vk::ResultValue<uint32_t> GhostSwapchain::aquireNextImage(
    const vk::raii::Semaphore &imageAvailableSemaphore) {
    return m_swapchain.acquireNextImage(UINT32_MAX, imageAvailableSemaphore);
}

} // namespace Ghost
