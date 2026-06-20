#pragma once
#include <Ghost/Core/vulkanDevice.hpp>
#include <functional>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
class GhostImage {
  public:
    GhostImage(VulkanDevice &device, uint32_t width, uint32_t height,
               vk::Format format, vk::ImageTiling tiling,
               vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
               vk::ImageAspectFlags aspectFlags, uint32_t mipLevels = 1,
               uint32_t arrayLayers = 1, bool isCubeMap = false);
    ~GhostImage() = default;

    GhostImage(const GhostImage &) = delete;
    GhostImage &operator=(const GhostImage &) = delete;

    GhostImage(GhostImage &&) = default;
    GhostImage &operator=(GhostImage &&) = default;

    const vk::raii::Image &getImage() const { return m_image; }
    const vk::raii::ImageView &getImageView() const { return m_imageView; }
    const vk::raii::DeviceMemory &getMemory() const { return m_memory; }

    vk::Extent3D getExtent() const { return m_extent; }
    vk::Format getFormat() const { return m_format; }
    uint32_t getMipLevels() const { return m_mipLevels; }
    uint32_t getArrayLayers() const { return m_arrayLayers; }
    vk::ImageAspectFlags getAspect() const { return m_aspectFlags; }

    void transitionImageLayout(vk::raii::CommandBuffer &cmd,
                               vk::ImageLayout newLayout);
    void generateMipmaps(vk::raii::CommandBuffer &cmd, uint32_t width,
                         uint32_t height);

  private:
    std::reference_wrapper<VulkanDevice> m_device;
    vk::raii::Image m_image = nullptr;
    vk::raii::ImageView m_imageView = nullptr;
    vk::raii::DeviceMemory m_memory = nullptr;

    vk::ImageLayout m_imageLayout = vk::ImageLayout::eUndefined;
    vk::Extent3D m_extent;
    vk::Format m_format;

    vk::ImageAspectFlags m_aspectFlags;
    uint32_t m_mipLevels = 1;
    uint32_t m_arrayLayers = 1;
};
} // namespace Ghost
