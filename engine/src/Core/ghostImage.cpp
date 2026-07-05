#include <Ghost/Core/ghostImage.hpp>

namespace Ghost {
GhostImage::GhostImage(VulkanDevice &device, uint32_t width, uint32_t height,
                       vk::Format format, vk::ImageTiling tiling,
                       vk::ImageUsageFlags usage,
                       vk::MemoryPropertyFlags properties,
                       vk::ImageAspectFlags aspectFlags, uint32_t mipLevels,
                       uint32_t arrayLayers, bool isCubeMap)
    : m_device(device), m_format(format), m_aspectFlags(aspectFlags),
      m_mipLevels(mipLevels), m_arrayLayers(arrayLayers) {
    m_extent.setWidth(width).setHeight(height).setDepth(1);

    vk::ImageCreateInfo imageInfo{};
    imageInfo.setImageType(vk::ImageType::e2D)
        .setExtent(m_extent)
        .setMipLevels(m_mipLevels)
        .setArrayLayers(m_arrayLayers)
        .setFormat(m_format)
        .setTiling(tiling)
        .setInitialLayout(m_imageLayout)
        .setUsage(usage)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setSharingMode(vk::SharingMode::eExclusive);
    if (isCubeMap) {
        imageInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
    }

    m_image = vk::raii::Image(m_device.get(), imageInfo);

    vk::MemoryRequirements memRequirements = m_image.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(m_device.get().findMemoryType(
            memRequirements.memoryTypeBits, properties));

    m_memory = vk::raii::DeviceMemory(m_device.get(), allocInfo);

    m_image.bindMemory(m_memory, 0);

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.setImage(*m_image).setFormat(m_format);

    if (isCubeMap && m_arrayLayers == 6) {
        viewInfo.setViewType(vk::ImageViewType::eCube);
    } else if (m_arrayLayers > 1) {
        viewInfo.setViewType(vk::ImageViewType::e2DArray);
    } else {
        viewInfo.setViewType(vk::ImageViewType::e2D);
    }

    viewInfo.subresourceRange.setAspectMask(m_aspectFlags)
        .setBaseMipLevel(0)
        .setLevelCount(m_mipLevels)
        .setBaseArrayLayer(0)
        .setLayerCount(m_arrayLayers);

    m_imageView = vk::raii::ImageView(m_device.get(), viewInfo);
}

void GhostImage::transitionImageLayout(vk::raii::CommandBuffer &cmd,
                                       vk::ImageLayout newLayout) {
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = m_imageLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

    barrier.setImage(m_image);

    barrier.subresourceRange.aspectMask = m_aspectFlags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = m_mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = m_arrayLayers;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (m_imageLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (m_imageLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (m_imageLayout == vk::ImageLayout::eUndefined &&
               newLayout == vk::ImageLayout::eGeneral) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask =
            vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eComputeShader;
    } else {
        throw std::invalid_argument(
            "GhostImage: Unsupported layout transition requested!");
    }

    cmd.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(),
                        nullptr, nullptr, barrier);

    m_imageLayout = newLayout;
}

void GhostImage::generateMipmaps(vk::raii::CommandBuffer &cmd, uint32_t width,
                                 uint32_t height) {
    vk::ImageMemoryBarrier barrier{};
    barrier.setImage(m_image);
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.subresourceRange.aspectMask = m_aspectFlags;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = m_arrayLayers;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    for (uint32_t i = 1; i < m_mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                            vk::PipelineStageFlagBits::eTransfer,
                            vk::DependencyFlags(), nullptr, nullptr, barrier);

        vk::ImageBlit blit{};
        blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
        blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = m_aspectFlags;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = m_arrayLayers;

        blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
        blit.dstOffsets[1] = vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1,
                                          mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = m_aspectFlags;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = m_arrayLayers;

        cmd.blitImage(*m_image, vk::ImageLayout::eTransferSrcOptimal, *m_image,
                      vk::ImageLayout::eTransferDstOptimal, blit,
                      vk::Filter::eLinear);

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                            vk::PipelineStageFlagBits::eFragmentShader,
                            vk::DependencyFlags(), nullptr, nullptr, barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = m_mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                        vk::PipelineStageFlagBits::eFragmentShader,
                        vk::DependencyFlags(), nullptr, nullptr, barrier);

    m_imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

} // namespace Ghost
