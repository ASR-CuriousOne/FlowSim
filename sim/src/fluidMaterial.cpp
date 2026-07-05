#include <Sim/fluidMaterial.hpp>

namespace FluidSim {
FluidMaterial::FluidMaterial(
    Ghost::VulkanDevice &device,
    Ghost::GhostDescriptorManager &descriptorManager,
    std::shared_ptr<Ghost::GhostGraphicsPipeline> pipeline, uint32_t pipelineId)
    : Material(std::move(pipeline), pipelineId), m_device(device),
      m_descriptorManager(descriptorManager) {

    m_descriptorSets =
        m_descriptorManager.allocateSets("FluidMaterialLayout", 1);
}

void FluidMaterial::updateShaderResources(const vk::raii::CommandBuffer &cmd,
                                          const Ghost::GhostRenderObject &obj) {

    if (m_velocityTexture && m_densityTexture) {
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                               m_pipeline->getPipelineLayout(), 0,
                               {*m_descriptorSets[0]}, nullptr);
    }
}

void FluidMaterial::setTextures(
    std::shared_ptr<Ghost::StorageTexture> velocityTex,
    std::shared_ptr<Ghost::StorageTexture> densityTex) {
    m_velocityTexture = std::move(velocityTex);
    m_densityTexture = std::move(densityTex);
    flushDescriptorUpdates();
}

void FluidMaterial::flushDescriptorUpdates() {
    if (!m_velocityTexture || !m_densityTexture)
        return;

    auto &setLayout = m_descriptorManager.getLayout("FluidMaterialLayout");

    vk::DescriptorImageInfo velocityInfo = m_velocityTexture->descriptorInfo();
    vk::DescriptorImageInfo densityInfo = m_densityTexture->descriptorInfo();

    Ghost::GhostDescriptorWriter writer(setLayout);
    writer.writeImage(0, &velocityInfo);
    writer.writeImage(1, &densityInfo);
    writer.build(m_descriptorSets[0], m_device);
}

} // namespace FluidSim
