#include <Ghost/Resources/storageTexture.hpp>

namespace Ghost {
StorageTexture::StorageTexture(VulkanDevice &device, uint32_t width,
                               uint32_t height, vk::Format format)
    : TextureBase(device) {
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eStorage |
                                vk::ImageUsageFlagBits::eSampled |
                                vk::ImageUsageFlagBits::eTransferDst;

    m_ghostImage = std::make_unique<GhostImage>(
        m_device, width, height, format, vk::ImageTiling::eOptimal, usage,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::ImageAspectFlagBits::eColor);

    createTextureSampler();
}
vk::ImageLayout StorageTexture::getImageLayout() const {
    return vk::ImageLayout::eGeneral;
}

vk::DescriptorImageInfo StorageTexture::descriptorInfo() {
    return vk::DescriptorImageInfo(m_sampler, m_ghostImage->getImageView(),
                                   vk::ImageLayout::eGeneral);
}

} // namespace Ghost
