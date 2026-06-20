#include <Ghost/Core/vulkanInstance.hpp>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace Ghost {

vk::DebugUtilsMessengerCreateInfoEXT VulkanInstance::populateDebugCreateInfo() {
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    return vk::DebugUtilsMessengerCreateInfoEXT(
        {}, severityFlags, messageTypeFlags, &debugMessageFunc);
}

VulkanInstance::VulkanInstance(
    const std::vector<const char *> &requiredExtensions)
    : m_context(), m_instance(nullptr) {
    if (m_enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error(
            "Validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo("GhostRenderEngine", vk::makeVersion(1, 0, 0),
                                "NoEngine", vk::makeVersion(1, 0, 0),
                                vk::ApiVersion10);

    std::vector<const char *> allRequiredExtensions =
        checkAndReturnRequiredExtensions(requiredExtensions);

    auto debugCreateInfo = populateDebugCreateInfo();

    vk::InstanceCreateInfo createInfo(
        {}, &appInfo,
        m_enableValidationLayers
            ? static_cast<uint32_t>(m_validationLayers.size())
            : 0,
        m_enableValidationLayers ? m_validationLayers.data() : nullptr,
        static_cast<uint32_t>(allRequiredExtensions.size()),
        allRequiredExtensions.data(),
        m_enableValidationLayers ? &debugCreateInfo : nullptr);

    m_instance = vk::raii::Instance(m_context, createInfo);

    if (m_enableValidationLayers) {
        auto debugCreateInfo = populateDebugCreateInfo();
        m_debugMessenger.emplace(m_instance, debugCreateInfo);
    }
}

std::vector<const char *> VulkanInstance::checkAndReturnRequiredExtensions(
    const std::vector<const char *> &requiredExtensions) {
    std::vector<vk::ExtensionProperties> supportedExtensions =
        m_context.enumerateInstanceExtensionProperties();

    std::vector<const char *> allRequiredExtensions;
    if (m_enableValidationLayers) {
        allRequiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    allRequiredExtensions.append_range(requiredExtensions);

    for (const char *requiredExt : allRequiredExtensions) {
        bool extensionFound = false;

        for (const auto &supportedExt : supportedExtensions) {
            if (std::strcmp(requiredExt, supportedExt.extensionName) == 0) {
                extensionFound = true;
                break;
            }
        }

        if (!extensionFound) {
            throw std::runtime_error(
                std::string("Missing required Vulkan extension: ") +
                requiredExt);
        }
    }
    std::clog << "All GLFW required Vulkan extensions are supported\n";
    return allRequiredExtensions;
}

bool VulkanInstance::checkValidationLayerSupport() {
    std::vector<vk::LayerProperties> availableLayers =
        m_context.enumerateInstanceLayerProperties();
    for (auto layerName : m_validationLayers) {
        bool layerFound = true;
        for (const auto &layerProperties : availableLayers) {
            if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

VulkanInstance::~VulkanInstance() {}
} // namespace Ghost
