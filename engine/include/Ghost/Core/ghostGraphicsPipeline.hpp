#pragma once
#include <Ghost/Core/vulkanDevice.hpp>
#include <Ghost/Utils/pipelineConfig.hpp>

namespace Ghost {

class GhostGraphicsPipeline {

    const VulkanDevice &m_device;

    vk::PipelineLayout m_pipelineLayout;
    vk::raii::ShaderModule m_vertShaderModule = nullptr;
    vk::raii::ShaderModule m_fragShaderModule = nullptr;
    vk::raii::Pipeline m_graphicsPipeline = nullptr;

  public:
    GhostGraphicsPipeline(const VulkanDevice &device,
                          const std::vector<std::byte> &vertFilepath,
                          const std::vector<std::byte> &fragFilepath,
                          const PipelineConfigInfo &configInfo);
    ~GhostGraphicsPipeline();

    vk::Pipeline operator*() const { return *m_graphicsPipeline; }

    vk::PipelineLayout getPipelineLayout() const { return m_pipelineLayout; }

    void bind(const vk::raii::CommandBuffer &buf) {
        buf.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);
    }
};

} // namespace Ghost
