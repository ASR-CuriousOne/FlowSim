#include <Ghost/Utils/pipelineConfig.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Ghost {

PipelineConfigInfo
PipelineConfigInfo::defaultConfig(PipelineConfigInfo &defaultConfig) {

    defaultConfig.vertexInputInfo.setVertexBindingDescriptionCount(0)
        .setVertexBindingDescriptions(nullptr)
        .setVertexAttributeDescriptionCount(0)
        .setVertexAttributeDescriptions(nullptr);

    defaultConfig.inputAssemblyInfo
        .setTopology(vk::PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(vk::False);

    defaultConfig.dynamicStateEnables = {vk::DynamicState::eViewport,
                                         vk::DynamicState::eScissor};

    defaultConfig.dynamicStateCreateInfo
        .setDynamicStateCount(defaultConfig.dynamicStateEnables.size())
        .setDynamicStates(defaultConfig.dynamicStateEnables);

    defaultConfig.viewportInfo.setViewportCount(1).setScissorCount(1);

    defaultConfig.rasterizationInfo.setDepthBiasClamp(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1.0f)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eCounterClockwise)
        .setDepthBiasEnable(vk::False);

    defaultConfig.multisampleInfo.setSampleShadingEnable(vk::False)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(vk::False)
        .setAlphaToOneEnable(vk::False);

    defaultConfig.colorBlendAttachment.setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    defaultConfig.colorBlendInfo.setLogicOpEnable(vk::False)
        .setLogicOp(vk::LogicOp::eCopy)
        .setAttachmentCount(1)
        .setAttachments(defaultConfig.colorBlendAttachment);

    defaultConfig.depthStencilInfo.setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLess)
        .setDepthBoundsTestEnable(vk::False)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f)
        .setStencilTestEnable(vk::False);

    return defaultConfig;
}
} // namespace Ghost
