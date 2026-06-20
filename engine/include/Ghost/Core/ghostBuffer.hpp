#pragma once
#include <Ghost/Core/vulkanDevice.hpp>

namespace Ghost {
class GhostBuffer {
  public:
    GhostBuffer(VulkanDevice &device, vk::DeviceSize bufferSize,
                vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    ~GhostBuffer();

    GhostBuffer(const GhostBuffer &) = delete;
    GhostBuffer &operator=(const GhostBuffer &) = delete;

    vk::Result map(vk::DeviceSize size = vk::WholeSize,
                   vk::DeviceSize offset = 0);

    template <typename T>
    void writeToBuffer(std::span<const T> data, vk::DeviceSize offset = 0) {
        vk::DeviceSize size = data.size_bytes();

        if (m_mapped) {
            std::memcpy(static_cast<char *>(m_mapped) + offset, data.data(),
                        size);
        } else {
            map(size, offset);
            std::memcpy(static_cast<char *>(m_mapped) + offset, data.data(),
                        size);
            unmap();
        }
    }

    void unmap();

    vk::Buffer getBuffer() const { return *m_buffer; }
    vk::DeviceSize getBufferSize() const { return m_bufferSize; }
    vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = vk::WholeSize,
                                            vk::DeviceSize offset = 0) const {
        return vk::DescriptorBufferInfo(*m_buffer, offset, size);
    }

  private:
	std::reference_wrapper<VulkanDevice> m_device;

    vk::raii::Buffer m_buffer = nullptr;
    vk::raii::DeviceMemory m_memory = nullptr;

    vk::DeviceSize m_bufferSize;
    void *m_mapped = nullptr;
};
} // namespace Ghost
