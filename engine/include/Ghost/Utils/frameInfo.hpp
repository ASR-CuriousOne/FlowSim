#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
struct FrameInfo {
    uint32_t frameIndex;
    float frameTime;
    const vk::raii::CommandBuffer &commandBuffer;
};
} // namespace Ghost
