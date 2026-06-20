#include <Ghost/Resources/hdriTexture.hpp>
#include <iostream>
#include <vendor/stb_image.h>

namespace Ghost {
HDRITexture::HDRITexture(VulkanDevice &device,
                         const std::filesystem::path &imagePath)
    : TextureBase(device) {
    stbi_set_flip_vertically_on_load(true);

    int texWidth, texHeight, texChannels;
    float *pixels = stbi_loadf(imagePath.string().c_str(), &texWidth,
                               &texHeight, &texChannels, STBI_rgb_alpha);

    vk::DeviceSize imageSize = texWidth * texHeight * 4 * sizeof(float);

    std::clog << std::format("Width: {} Height: {} Channels: {}", texWidth,
                             texHeight, texChannels)
              << std::endl;

    if (!pixels) {
        throw std::runtime_error("failed to load HDRI texture!");
    }

    createTextureImage(pixels, imageSize, texWidth, texHeight,
                       vk::Format::eR32G32B32A32Sfloat);

    stbi_image_free(pixels);

    stbi_set_flip_vertically_on_load(false);
}
} // namespace Ghost
