#pragma once
#include <Ghost/Resources/textureBase.hpp>
#include <filesystem>

namespace Ghost {
class HDRITexture : public TextureBase {
  public:
    HDRITexture(VulkanDevice &device, const std::filesystem::path &imagePath);
    ~HDRITexture() override = default;
};
} // namespace Ghost
