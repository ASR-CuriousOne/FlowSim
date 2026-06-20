#pragma once
#include <cstdint>
#include <optional>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {

struct QueueFamilyIndicies {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

    void findQueueFamily(const vk::raii::PhysicalDevice &physicalDevice,
                         const vk::raii::SurfaceKHR &surface);
    bool isComplete();
};
} // namespace Ghost
