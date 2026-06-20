#include <Ghost/Core/ghostCommandPool.hpp>

namespace Ghost {
GhostCommandPool::GhostCommandPool(const VulkanDevice &device) {
    auto queueFamilyIndices = device.getQueueFamilyIndices();

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

    m_commandPool = vk::raii::CommandPool(device, poolInfo);
}

GhostCommandPool::~GhostCommandPool() {}
} // namespace Ghost
