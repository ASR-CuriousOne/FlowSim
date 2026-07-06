#pragma once
#include <Ghost/Resources/ghostDescriptorManager.hpp>
#include <Ghost/Resources/material.hpp>
#include <Ghost/Resources/storageTexture.hpp>
#include <Sim/pushConstants.hpp>

namespace FluidSim {

class FluidMaterial : public Ghost::Material {
  public:
    FluidMaterial(Ghost::VulkanDevice &device,
                  Ghost::GhostDescriptorManager &descriptorManager,
                  std::shared_ptr<Ghost::GhostGraphicsPipeline> pipeline,
                  uint32_t pipelineTypeId);

    ~FluidMaterial() override = default;

    Ghost::VertexType getRequiredVertexType() const override {
        return Ghost::VertexType::Standard;
    }

    void updateShaderResources(const vk::raii::CommandBuffer &cmd,
                               const Ghost::GhostRenderObject &obj) override;

    void setTextures(std::shared_ptr<Ghost::StorageTexture> vel0,
                     std::shared_ptr<Ghost::StorageTexture> den0,
                     std::shared_ptr<Ghost::StorageTexture> vel1,
                     std::shared_ptr<Ghost::StorageTexture> den1);
    
    void setActiveIndex(uint32_t index) { m_activeIndex = index; }

  private:
    void flushDescriptorUpdates();

    Ghost::VulkanDevice &m_device;
    Ghost::GhostDescriptorManager &m_descriptorManager;
    SimGraphicsPushConstant m_pushData{};

    std::shared_ptr<Ghost::StorageTexture> m_velocityTexture;
    std::shared_ptr<Ghost::StorageTexture> m_densityTexture;
    std::vector<vk::raii::DescriptorSet> m_descriptorSets;

	uint32_t m_activeIndex = 0;
};

} // namespace FluidSim
