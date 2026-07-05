#pragma once
#include <Ghost/Resources/textureBase.hpp>

namespace Ghost {
class StorageTexture : public TextureBase {
  public:
    StorageTexture(VulkanDevice &device, uint32_t width, uint32_t height,
                   vk::Format format);
    ~StorageTexture() override = default;

    vk::ImageLayout getImageLayout() const override;
    vk::DescriptorImageInfo descriptorInfo() override;
};

} // namespace Ghost
