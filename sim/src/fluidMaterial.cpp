#include <Sim/fluidMaterial.hpp>

namespace FluidSim {
FluidMaterial::FluidMaterial(
    Ghost::VulkanDevice &device,
    Ghost::GhostDescriptorManager &descriptorManager,
    std::shared_ptr<Ghost::GhostGraphicsPipeline> pipeline, uint32_t pipelineId)
    : Material(std::move(pipeline), pipelineId), m_device(device),
      m_descriptorManager(descriptorManager) {

    m_descriptorSets =
        m_descriptorManager.allocateSets("FluidMaterialLayout", 2);
}

void FluidMaterial::updateShaderResources(const vk::raii::CommandBuffer &cmd,
                                          const Ghost::GhostRenderObject &obj) {

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           m_pipeline->getPipelineLayout(), 0,
                           {*m_descriptorSets[m_activeIndex]}, nullptr);
}

void FluidMaterial::setTextures(std::shared_ptr<Ghost::StorageTexture> vel0,
                                std::shared_ptr<Ghost::StorageTexture> den0,
                                std::shared_ptr<Ghost::StorageTexture> vel1,
                                std::shared_ptr<Ghost::StorageTexture> den1) {
    auto &setLayout = m_descriptorManager.getLayout("FluidMaterialLayout");
    vk::DescriptorImageInfo vel0Info = vel0->descriptorInfo();
    vk::DescriptorImageInfo den0Info = den0->descriptorInfo();
    Ghost::GhostDescriptorWriter(setLayout)
        .writeImage(0, &vel0Info)
        .writeImage(1, &den0Info)
        .build(m_descriptorSets[0], m_device);

    vk::DescriptorImageInfo vel1Info = vel1->descriptorInfo();
    vk::DescriptorImageInfo den1Info = den1->descriptorInfo();
    Ghost::GhostDescriptorWriter(setLayout)
        .writeImage(0, &vel1Info)
        .writeImage(1, &den1Info)
        .build(m_descriptorSets[1], m_device);
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
