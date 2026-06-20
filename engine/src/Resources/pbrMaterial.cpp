#include <Ghost/Resources/pbrMaterial.hpp>
#include <Ghost/Utils/ghostRenderObject.hpp>

namespace Ghost {
PBRMaterial::PBRMaterial(VulkanDevice &device,
                         GhostDescriptorManager &descriptorManager,
                         std::shared_ptr<GhostGraphicsPipeline> pipeline,
                         uint32_t pipelineId)
    : Material(pipeline, pipelineId), m_device(device),
      m_descriptorManager(descriptorManager) {
    m_descriptorSets = m_descriptorManager.allocateSets(PBR_LAYOUT_NAME, 1);
}

void PBRMaterial::updateShaderResources(const vk::raii::CommandBuffer &cmd,
                                        const GhostRenderObject &obj) {
    m_pushData.modelMatrix = obj.transformMatrix;
    cmd.pushConstants<PBRPushConstants>(m_pipeline->getPipelineLayout(),
                                        vk::ShaderStageFlagBits::eVertex |
                                            vk::ShaderStageFlagBits::eFragment,
                                        0, m_pushData);

    if (m_albedoMap && m_normalMap) {
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                               m_pipeline->getPipelineLayout(), 1,
                               {*m_descriptorSets[0]}, nullptr);
    }
}

void PBRMaterial::setAlbedoMap(std::shared_ptr<GhostTexture> tex) {
    m_albedoMap = std::move(tex);
    flushDescriptorUpdates();
}

void PBRMaterial::setNormalMap(std::shared_ptr<GhostTexture> tex) {
    m_normalMap = std::move(tex);
    flushDescriptorUpdates();
}

void PBRMaterial::flushDescriptorUpdates() {
    if (!m_albedoMap || !m_normalMap)
        return;

    auto &setLayout = m_descriptorManager.getLayout(PBR_LAYOUT_NAME);

    vk::DescriptorImageInfo albedoInfo = m_albedoMap->descriptorInfo();
    vk::DescriptorImageInfo normalInfo = m_normalMap->descriptorInfo();

    GhostDescriptorWriter writer(setLayout);
    writer.writeImage(0, &albedoInfo);
    writer.writeImage(1, &normalInfo);

    writer.build(m_descriptorSets[0], m_device);
}

} // namespace Ghost
