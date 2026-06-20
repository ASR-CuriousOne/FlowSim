#include <Ghost/Utils/swapChainDetails.hpp>
#include <Ghost/Core/vulkanDevice.hpp>
#include <set>
#include <stdexcept>

namespace Ghost {
VulkanDevice::VulkanDevice(const vk::raii::Instance &instance,
                           const vk::raii::SurfaceKHR &surface)
    : m_physicalDevices(instance) {
    for (size_t i = 0; i < m_physicalDevices.size(); i++) {
        const auto &device = m_physicalDevices[i];
        if (isDeviceSuitable(device, surface)) {
            m_physicalDevice = std::move(device);
            m_queueFamilyIndicies.findQueueFamily(device, surface);
            break;
        }
    }

    if (!(*m_physicalDevice)) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    std::set<uint32_t> uniqueQueueFamilyIndicies = {
        m_queueFamilyIndicies.graphicsFamily.value(),
        m_queueFamilyIndicies.presentFamily.value(),
        m_queueFamilyIndicies.transferFamily.value()};

    float queuePriority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    for (auto index : uniqueQueueFamilyIndicies) {
        queueCreateInfos.push_back({{}, index, 1, &queuePriority});
    }

    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.setSamplerAnisotropy(vk::True);

    vk::DeviceCreateInfo createInfo(
        {}, queueCreateInfos.size(), queueCreateInfos.data(), 0, nullptr,
        static_cast<uint32_t>(m_deviceExtensions.size()),
        m_deviceExtensions.data(), &deviceFeatures);

    m_device = vk::raii::Device(m_physicalDevice, createInfo);

    m_graphicsQueue = vk::raii::Queue(
        m_device, m_queueFamilyIndicies.graphicsFamily.value(), 0);
    m_presentQueue = vk::raii::Queue(
        m_device, m_queueFamilyIndicies.presentFamily.value(), 0);

    m_transferQueue = vk::raii::Queue(
        m_device, m_queueFamilyIndicies.transferFamily.value(), 0);

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient)
        .setQueueFamilyIndex(m_queueFamilyIndicies.transferFamily.value());

    m_transferCommandPool = vk::raii::CommandPool(m_device, poolInfo);
}

bool VulkanDevice::isDeviceSuitable(
    const vk::raii::PhysicalDevice &physicalDevice,
    const vk::raii::SurfaceKHR &surface) {
    auto features = physicalDevice.getFeatures();
    auto properties = physicalDevice.getProperties();

    QueueFamilyIndicies queueFamilyIndicies;
    queueFamilyIndicies.findQueueFamily(physicalDevice, surface);

    bool extensionSupported = checkExtensions(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionSupported) {
        SwapChainSupportDetails swapChainDetails;
        swapChainDetails.querySwapChainSupport(physicalDevice, surface);
        swapChainAdequate = !swapChainDetails.formats.empty() &&
                            !swapChainDetails.presentModes.empty();
    }

    return properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
           queueFamilyIndicies.isComplete() && extensionSupported &&
           swapChainAdequate && features.samplerAnisotropy;
}

bool VulkanDevice::checkExtensions(
    const vk::raii::PhysicalDevice &physicalDevice) {
    auto availableExtensions =
        physicalDevice.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(m_deviceExtensions.begin(),
                                             m_deviceExtensions.end());
    for (const auto &extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

std::string VulkanDevice::getDeviceName() {
    return m_physicalDevice.getProperties().deviceName;
}

void VulkanDevice::submitGraphicsQueue(const vk::SubmitInfo &submitInfo,
                                       const vk::Fence &fence) {
    m_graphicsQueue.submit(submitInfo, fence);
}

vk::Result
VulkanDevice::submitPresentQueue(const vk::PresentInfoKHR &presentInfo) {
    return m_presentQueue.presentKHR(presentInfo);
}

uint32_t
VulkanDevice::findMemoryType(uint32_t typeFilter,
                             vk::MemoryPropertyFlags properties) const {
    vk::PhysicalDeviceMemoryProperties memProperties =
        m_physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

vk::raii::CommandBuffer VulkanDevice::beginSingleTimeCommands() const {
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(*m_transferCommandPool)
        .setCommandBufferCount(1);

    vk::raii::CommandBuffers commandBuffers(m_device, allocInfo);
    vk::raii::CommandBuffer commandBuffer = std::move(commandBuffers[0]);

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void VulkanDevice::endSingleTimeCommands(
    const vk::raii::CommandBuffer &commandBuffer) const {
    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBufferCount(1).setPCommandBuffers(&*commandBuffer);

    m_transferQueue.submit({submitInfo}, nullptr);
    m_transferQueue.waitIdle();
}

vk::Format
VulkanDevice::findSupportedFormat(const std::vector<vk::Format> &candidates,
                                  vk::ImageTiling tiling,
                                  vk::FormatFeatureFlags features) const {
    for (vk::Format format : candidates) {
        vk::FormatProperties props =
            m_physicalDevice.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format!");
}

vk::Format VulkanDevice::findDepthFormat() const {
    return findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
         vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

} // namespace Ghost
