#include <Ghost/Systems/forwardRenderer.hpp>

namespace Ghost {

ForwardRenderer::ForwardRenderer(ForwardRendererConfig &config)
    : m_width(config.windowWidth), m_height(config.windowHeight) {
    m_instance = std::make_unique<VulkanInstance>(config.instanceExtensions);

    VkSurfaceKHR c_surface = config.surfaceCreateCallback(**m_instance);
    m_surface = std::make_unique<vk::raii::SurfaceKHR>(*m_instance, c_surface);

    m_device = std::make_unique<VulkanDevice>(*m_instance, *m_surface);

    m_renderer = std::make_unique<GhostRenderer>(*m_device, **m_surface,
                                                 m_width, m_height);
}

ForwardRenderer::~ForwardRenderer() { (*m_device)->waitIdle(); }

void ForwardRenderer::resize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    m_isMinimized = (width == 0 || height == 0);

    if (!m_isMinimized) {
        m_renderer->flagFramebufferResized(width, height);
    }
}

float ForwardRenderer::getAspectRatio() const {
    return static_cast<float>(m_width) / static_cast<float>(m_height);
}

vk::raii::CommandBuffer &ForwardRenderer::beginFrame() {
    return m_renderer->beginFrame();
}

void ForwardRenderer::endFrame() { m_renderer->endFrame(); }

void ForwardRenderer::beginSwapChainRenderPass(
    const vk::raii::CommandBuffer &commandBuffer) {
    m_renderer->beginSwapChainRenderPass(commandBuffer);
}

void ForwardRenderer::endSwapChainRenderPass(
    const vk::raii::CommandBuffer &commandBuffer) {
    m_renderer->endSwapChainRenderPass(commandBuffer);
}

void ForwardRenderer::renderScene(
    const vk::raii::CommandBuffer &commandBuffer,
    std::vector<GhostRenderObject> &renderObjects) {

    std::sort(renderObjects.begin(), renderObjects.end(),
              [](const GhostRenderObject &a, const GhostRenderObject &b) {
                  return a.material->getPipelineTypeId() <
                         b.material->getPipelineTypeId();
              });

    uint32_t currentBoundPipelineId = 0;

    for (const auto &obj : renderObjects) {

        if (!obj.mesh || !obj.material)
            continue;

        assert(obj.mesh->getVertexType() ==
                   obj.material->getRequiredVertexType() &&
               "Mesh and Material vertex types do not match!");

        if (obj.material->getPipelineTypeId() != currentBoundPipelineId) {
            obj.material->bind(commandBuffer);
            currentBoundPipelineId = obj.material->getPipelineTypeId();
        }

        obj.material->updateShaderResources(commandBuffer, obj);

        obj.mesh->bind(commandBuffer);
        obj.mesh->draw(commandBuffer);
    }
}

} // namespace Ghost
