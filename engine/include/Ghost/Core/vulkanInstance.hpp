#pragma once
#include <iostream>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
class VulkanInstance {
  private:
    vk::raii::Context m_context;
    vk::raii::Instance m_instance;
    std::optional<vk::raii::DebugUtilsMessengerEXT> m_debugMessenger;

    const std::vector<const char *> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    vk::DebugUtilsMessengerCreateInfoEXT populateDebugCreateInfo();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageFunc(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
        vk::DebugUtilsMessengerCallbackDataEXT const *pCallbackData,
        void *pUserData) {

        std::cerr << "Validation Layer: " << pCallbackData->pMessage
                  << std::endl;
        return VK_FALSE;
    }

#ifdef NDEBUG
    const bool m_enableValidationLayers = false;
#else
    const bool m_enableValidationLayers = true;
#endif

    bool checkValidationLayerSupport();

    std::vector<const char *> checkAndReturnRequiredExtensions(
        const std::vector<const char *> &requiredExtensions);

  public:
    VulkanInstance(const std::vector<const char *> &requiredExtensions);
    ~VulkanInstance();
    operator const vk::raii::Instance &() const { return m_instance; }
    vk::Instance operator*() const { return *m_instance; }
};
} // namespace Ghost
