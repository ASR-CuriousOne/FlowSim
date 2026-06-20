#include <Ghost/Core/ghostComputePipeline.hpp>

namespace Ghost {
GhostComputePipeline::GhostComputePipeline(
    const VulkanDevice &device, const std::vector<std::byte> &compShaderCode,
    vk::PipelineLayout pipelineLayout)
    : m_device(device), m_pipelineLayout(pipelineLayout) {

    vk::ShaderModuleCreateInfo compCreateInfo(
        {}, compShaderCode.size(),
        reinterpret_cast<const uint32_t *>(compShaderCode.data()));
    m_compShaderModule = vk::raii::ShaderModule(m_device, compCreateInfo);

    vk::PipelineShaderStageCreateInfo shaderStage(
        {}, vk::ShaderStageFlagBits::eCompute, *m_compShaderModule, "main");

    vk::ComputePipelineCreateInfo createInfo({}, shaderStage, m_pipelineLayout);

    m_computePipeline = vk::raii::Pipeline(m_device, nullptr, createInfo);
}
} // namespace Ghost
