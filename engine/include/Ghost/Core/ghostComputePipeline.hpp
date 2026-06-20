#pragma once
#include <Ghost/Core/vulkanDevice.hpp>

namespace Ghost {
class GhostComputePipeline {
  public:
    GhostComputePipeline(const VulkanDevice &device,
                         const std::vector<std::byte> &compShaderCode,
                         vk::PipelineLayout pipelineLayout);
    ~GhostComputePipeline() = default;

    vk::Pipeline operator*() const { return *m_computePipeline; }

    vk::PipelineLayout getPipelineLayout() const { return m_pipelineLayout; }

    void bind(const vk::raii::CommandBuffer &buf) {
        buf.bindPipeline(vk::PipelineBindPoint::eCompute, m_computePipeline);
    }

  private:
    const VulkanDevice &m_device;

    vk::PipelineLayout m_pipelineLayout;
    vk::raii::ShaderModule m_compShaderModule = nullptr;
    vk::raii::Pipeline m_computePipeline = nullptr;
};
} // namespace Ghost
