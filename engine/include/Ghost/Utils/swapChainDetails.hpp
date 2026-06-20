#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;

    void querySwapChainSupport(const vk::PhysicalDevice &physicalDevice,
                               const vk::SurfaceKHR &surface) {
        capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
        formats = physicalDevice.getSurfaceFormatsKHR(surface);
        presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
    }
};
} // namespace Ghost
