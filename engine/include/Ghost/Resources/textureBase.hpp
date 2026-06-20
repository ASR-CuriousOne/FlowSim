#pragma once
#include <Ghost/Core/ghostImage.hpp>
#include <Ghost/Core/vulkanDevice.hpp>

namespace Ghost {
class TextureBase {
  public:
    virtual ~TextureBase() = default;

    TextureBase(const TextureBase &) = delete;
    TextureBase &operator=(const TextureBase &) = delete;

    const vk::raii::ImageView &getImageView() const {
        return m_ghostImage->getImageView();
    }
    const vk::raii::Sampler &getSampler() const { return m_sampler; }
    vk::ImageLayout getImageLayout() const {
        return vk::ImageLayout::eShaderReadOnlyOptimal;
    }

    vk::DescriptorImageInfo descriptorInfo() {
        return vk::DescriptorImageInfo(m_sampler, m_ghostImage->getImageView(),
                                       vk::ImageLayout::eShaderReadOnlyOptimal);
    }

  protected:
    TextureBase(VulkanDevice &device);

    VulkanDevice &m_device;
    std::unique_ptr<GhostImage> m_ghostImage;
    vk::raii::Sampler m_sampler = nullptr;
	uint32_t m_mipLevels = 1;

    void createTextureImage(const void *pixels, vk::DeviceSize imageSize,
                            uint32_t width, uint32_t height, vk::Format format);

    void createTextureSampler();
    void copyBufferToImage(vk::raii::CommandBuffer &commandBuffer,
                           const vk::Buffer &buffer, const vk::Image &image,
                           uint32_t width, uint32_t height);
};
} // namespace Ghost
