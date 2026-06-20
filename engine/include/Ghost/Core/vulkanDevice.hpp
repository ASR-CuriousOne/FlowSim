#pragma once
#include <Ghost/Utils/queueFamilyIndicies.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {

class VulkanDevice {
  public:
    VulkanDevice(const vk::raii::Instance &instance,
                 const vk::raii::SurfaceKHR &surface);
    ~VulkanDevice() = default;

    VulkanDevice(const VulkanDevice &) = delete;
    VulkanDevice &operator=(const VulkanDevice &) = delete;

    VulkanDevice(VulkanDevice &&) = default;
    VulkanDevice &operator=(VulkanDevice &&) = default;

    operator const vk::raii::Device &() const { return m_device; }
    const vk::raii::Device *operator->() const { return &m_device; }

	vk::Device getRawDeviceHandle() { return m_device; }

    operator const vk::PhysicalDevice &() const { return *m_physicalDevice; }

    vk::PhysicalDeviceProperties getPhysicalDeviceProperties() {
        return m_physicalDevice.getProperties();
    };

    const QueueFamilyIndicies &getQueueFamilyIndices() const {
        return m_queueFamilyIndicies;
    }

    uint32_t findMemoryType(uint32_t typeFilter,
                            vk::MemoryPropertyFlags properties) const;

    void submitGraphicsQueue(const vk::SubmitInfo &submitInfo,
                             const vk::Fence &fence);
    vk::Queue getGraphicsQueue() { return m_graphicsQueue; }

    vk::Result submitPresentQueue(const vk::PresentInfoKHR &presentInfo);

    std::string getDeviceName();

    vk::raii::CommandBuffer beginSingleTimeCommands() const;
    void
    endSingleTimeCommands(const vk::raii::CommandBuffer &commandBuffer) const;

    vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates,
                                   vk::ImageTiling tiling,
                                   vk::FormatFeatureFlags features) const;
    vk::Format findDepthFormat() const;

  private:
    vk::raii::PhysicalDevices m_physicalDevices = nullptr;
    vk::raii::PhysicalDevice m_physicalDevice = nullptr;

    static constexpr std::array<const char *, 1> m_deviceExtensions = {
        vk::KHRSwapchainExtensionName};

    QueueFamilyIndicies m_queueFamilyIndicies;

    vk::raii::Device m_device = nullptr;

    vk::raii::Queue m_graphicsQueue = nullptr;
    vk::raii::Queue m_presentQueue = nullptr;
    vk::raii::Queue m_transferQueue = nullptr;

    vk::raii::CommandPool m_transferCommandPool = nullptr;

    bool isDeviceSuitable(const vk::raii::PhysicalDevice &physicalDevice,
                          const vk::raii::SurfaceKHR &surface);

    bool checkExtensions(const vk::raii::PhysicalDevice &physicalDevice);
};

} // namespace Ghost
