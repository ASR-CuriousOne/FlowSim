#include <Ghost/Core/vulkanDevice.hpp>
#include <vulkan/vulkan_to_string.hpp>

namespace Ghost {
void QueueFamilyIndicies::findQueueFamily(
    const vk::raii::PhysicalDevice &physicalDevice,
    const vk::raii::SurfaceKHR &surface) {
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    uint32_t i = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlags::BitsType::eGraphics) {
            graphicsFamily = i;

            if (physicalDevice.getSurfaceSupportKHR(graphicsFamily.value(),
                                                    surface)) {
                presentFamily = i;
            }
        } else if (queueFamily.queueFlags &
                   vk::QueueFlags::BitsType::eTransfer) {
            transferFamily = i;
        }

        if (!transferFamily.has_value() && graphicsFamily.has_value()) {
            transferFamily = graphicsFamily;
        }

        if (isComplete()) {
            break;
        }

        i++;
    }

    if (!presentFamily.has_value()) {
        for (uint32_t presentFamilyIndex = 0;
             presentFamilyIndex < queueFamilies.size(); presentFamilyIndex++) {
            if (physicalDevice.getSurfaceSupportKHR(graphicsFamily.value(),
                                                    surface)) {
                presentFamily = presentFamilyIndex;
            }

            if (isComplete()) {
                break;
            }
        }
    }
}

bool QueueFamilyIndicies::isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value() &&
           transferFamily.has_value();
}
} // namespace Ghost
