#pragma once
#include <Ghost/Core/vulkanDevice.hpp>
#include <Ghost/Core/vulkanInstance.hpp>
#include <Ghost/Systems/ghostRenderer.hpp>
#include <Ghost/Utils/ghostRenderObject.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace Ghost {

struct ForwardRendererConfig {
    std::vector<const char *> instanceExtensions;
    std::function<vk::SurfaceKHR(VkInstance)> surfaceCreateCallback;
    uint32_t windowWidth;
    uint32_t windowHeight;
};

class ForwardRenderer {
  public:
    ForwardRenderer(ForwardRendererConfig &config);
    ~ForwardRenderer();

    ForwardRenderer(const ForwardRenderer &) = delete;
    ForwardRenderer &operator=(const ForwardRenderer &) = delete;

    vk::raii::CommandBuffer &beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(const vk::raii::CommandBuffer &commandBuffer);
    void endSwapChainRenderPass(const vk::raii::CommandBuffer &commandBuffer);

    void resize(uint32_t width, uint32_t height);

    VulkanDevice &getDevice() { return *m_device; }
    vk::RenderPass getRenderPass() const {
        return m_renderer->getSwapChainRenderPass();
    }
    float getAspectRatio() const;

    int getFrameIndex() const { return m_renderer->getFrameIndex(); }
    bool isFrameInProgress() const { return m_renderer->isFrameInProgress(); }

    const VulkanDevice &getDevice() const { return *m_device; }
    VulkanInstance &getInstance() const { return *m_instance; }

    void renderScene(const vk::raii::CommandBuffer &commandBuffer,
                     std::vector<GhostRenderObject> &renderObjects);

  private:
    std::unique_ptr<VulkanInstance> m_instance;
    std::unique_ptr<vk::raii::SurfaceKHR> m_surface;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<GhostRenderer> m_renderer;

    uint32_t m_width;
    uint32_t m_height;
    bool m_isMinimized = false;
};
} // namespace Ghost
