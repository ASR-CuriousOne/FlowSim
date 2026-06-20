#include <Ghost/Core/ghostBuffer.hpp>
#include <Ghost/Resources/textureBase.hpp>
#include <cmath>

namespace Ghost {
TextureBase::TextureBase(VulkanDevice &device) : m_device(device) {}

void TextureBase::createTextureImage(const void *pixels,
                                     vk::DeviceSize imageSize, uint32_t width,
                                     uint32_t height, vk::Format format) {

    GhostBuffer stagingBuffer(m_device, imageSize,
                              vk::BufferUsageFlagBits::eTransferSrc,
                              vk::MemoryPropertyFlagBits::eHostVisible |
                                  vk::MemoryPropertyFlagBits::eHostCoherent);

    stagingBuffer.writeToBuffer(std::span<const uint8_t>(
        static_cast<const uint8_t *>(pixels), static_cast<size_t>(imageSize)));

    m_mipLevels =
        static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) +
        1;
    m_ghostImage = std::make_unique<GhostImage>(
        m_device, width, height, format, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst |
            vk::ImageUsageFlagBits::eSampled |
            vk::ImageUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::ImageAspectFlagBits::eColor, m_mipLevels);

    auto commandBuffer = m_device.beginSingleTimeCommands();

    m_ghostImage->transitionImageLayout(commandBuffer,
                                        vk::ImageLayout::eTransferDstOptimal);
    copyBufferToImage(commandBuffer, stagingBuffer.getBuffer(),
                      *m_ghostImage->getImage(), width, height);
    m_ghostImage->generateMipmaps(commandBuffer, width, height);

    m_device.endSingleTimeCommands(commandBuffer);

    createTextureSampler();
}

void TextureBase::createTextureSampler() {
    vk::SamplerCreateInfo createInfo;

    createInfo.setMagFilter(vk::Filter::eLinear)
        .setMinFilter(vk::Filter::eLinear)
        .setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setAnisotropyEnable(vk::True)
        .setMaxAnisotropy(
            m_device.getPhysicalDeviceProperties().limits.maxSamplerAnisotropy)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
        .setUnnormalizedCoordinates(vk::False)
        .setCompareEnable(vk::False)
        .setCompareOp(vk::CompareOp::eAlways)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setMipLodBias(0.0f)
        .setMinLod(0.0f)
        .setMaxLod(static_cast<float>(m_mipLevels));

    m_sampler = vk::raii::Sampler(m_device, createInfo);
}

void TextureBase::copyBufferToImage(vk::raii::CommandBuffer &commandBuffer,
                                    const vk::Buffer &buffer,
                                    const vk::Image &image, uint32_t width,
                                    uint32_t height) {
    vk::BufferImageCopy region;

    region.setBufferOffset(0)
        .setBufferImageHeight(0)
        .setBufferRowLength(0)
        .imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setMipLevel(0)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    region.imageExtent.setWidth(width).setHeight(height).setDepth(1);

    commandBuffer.copyBufferToImage(
        buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}

} // namespace Ghost
