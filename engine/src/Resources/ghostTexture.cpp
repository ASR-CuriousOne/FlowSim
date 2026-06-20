#include <Ghost/Core/ghostBuffer.hpp>
#include <Ghost/Resources/ghostTexture.hpp>
#include <format>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <vendor/stb_image.h>

namespace Ghost {
GhostTexture::GhostTexture(VulkanDevice &device,
                           const std::filesystem::path &imagePath,
                           vk::Format format)
    : TextureBase(device) {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(imagePath.string().c_str(), &texWidth,
                                &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) {
        std::string failiureReason = stbi_failure_reason();
        throw std::runtime_error("Unable to open image " + imagePath.string() +
                                 " : " + failiureReason);
    }

    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    std::clog << std::format("Width: {} Height: {} Channels: {}", texWidth,
                             texHeight, texChannels)
              << std::endl;

    createTextureImage(pixels, imageSize, texWidth, texHeight, format);

    stbi_image_free(pixels);
}

} // namespace Ghost
