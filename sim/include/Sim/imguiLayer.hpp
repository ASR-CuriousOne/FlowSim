#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {
class VulkanDevice;
class GhostRenderer;
} // namespace Ghost

struct WindowGLFW;

class ImGuiLayer {
  public:
    ImGuiLayer(Ghost::VulkanDevice &device, WindowGLFW &window,
               vk::RenderPass renderPass, vk::Instance instance);
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer &) = delete;
    ImGuiLayer &operator=(const ImGuiLayer &) = delete;

    void beginFrame();

    void render(const vk::raii::CommandBuffer &commandBuffer);

  private:
    Ghost::VulkanDevice &m_device;
    vk::raii::DescriptorPool m_descriptorPool = nullptr;
};
