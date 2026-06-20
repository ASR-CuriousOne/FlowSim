#pragma once
#include <Ghost/Core/vulkanDevice.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
class GhostCommandPool {
    vk::raii::CommandPool m_commandPool = nullptr;

  public:
    GhostCommandPool(const VulkanDevice &device);
    ~GhostCommandPool();

    operator const vk::raii::CommandPool &() const { return m_commandPool; }
    vk::CommandPool operator*() const { return *m_commandPool; }
};
} // namespace Ghost
