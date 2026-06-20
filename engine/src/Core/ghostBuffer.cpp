#include <Ghost/Core/ghostBuffer.hpp>

namespace Ghost {
GhostBuffer::GhostBuffer(VulkanDevice &device, vk::DeviceSize bufferSize,
                         vk::BufferUsageFlags usage,
                         vk::MemoryPropertyFlags properties)
    : m_device(device), m_bufferSize(bufferSize) {

    vk::BufferCreateInfo bufferInfo({}, bufferSize, usage,
                                    vk::SharingMode::eExclusive);
    m_buffer = vk::raii::Buffer(m_device.get(), bufferInfo);

    vk::MemoryRequirements memRequirements = m_buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo(
        memRequirements.size, m_device.get().findMemoryType(
                                  memRequirements.memoryTypeBits, properties));
    m_memory = vk::raii::DeviceMemory(m_device.get(), allocInfo);

    m_buffer.bindMemory(*m_memory, 0);
}

GhostBuffer::~GhostBuffer() {
    if (m_mapped) {
        unmap();
    }
}

vk::Result GhostBuffer::map(vk::DeviceSize size, vk::DeviceSize offset) {
    if (m_mapped)
        return vk::Result::eSuccess;

    m_mapped = m_memory.mapMemory(offset, size);
    return m_mapped ? vk::Result::eSuccess : vk::Result::eErrorMemoryMapFailed;
}

void GhostBuffer::unmap() {
    if (m_mapped) {
        m_memory.unmapMemory();
        m_mapped = nullptr;
    }
}

} // namespace Ghost
