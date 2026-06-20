#include <Ghost/Resources/skyboxMaterial.hpp>
#include <Ghost/Utils/ghostRenderObject.hpp>

namespace Ghost {
SkyboxMaterial::SkyboxMaterial(VulkanDevice &device,
                               GhostDescriptorManager &descriptorManager,
                               std::shared_ptr<GhostGraphicsPipeline> pipeline,
                               uint32_t typeId)
    : Material(pipeline, typeId), m_device(device),
      m_descriptorManager(descriptorManager) {
    m_descriptorSet =
        std::move(m_descriptorManager.allocateSets("SkyboxLayout", 1)[0]);
};

void SkyboxMaterial::updateShaderResources(
    const vk::raii::CommandBuffer &commandBuffer,
    const GhostRenderObject &obj) {
    m_pushData.modelMatrix = obj.transformMatrix;
    commandBuffer.pushConstants<PBRPushConstants>(
        m_pipeline->getPipelineLayout(),
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, m_pushData);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     m_pipeline->getPipelineLayout(), 1,
                                     {*m_descriptorSet}, nullptr);
}
void SkyboxMaterial::buildDescriptorSet() {
    auto imageInfo = m_hdriMap->descriptorInfo();
    GhostDescriptorWriter(m_descriptorManager.getLayout("SkyboxLayout"))
        .writeImage(0, &imageInfo)
        .build(m_descriptorSet, m_device);
}

} // namespace Ghost
