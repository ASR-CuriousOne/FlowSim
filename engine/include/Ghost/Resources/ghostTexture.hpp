#pragma once
#include <Ghost/Core/ghostImage.hpp>
#include <Ghost/Resources/textureBase.hpp>
#include <filesystem>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
class GhostTexture : public TextureBase {
  public:
    GhostTexture(VulkanDevice &device, const std::filesystem::path &imagePath,
                 vk::Format format = vk::Format::eR8G8B8A8Srgb);
    ~GhostTexture() override = default;
};
} // namespace Ghost
