#pragma once
#include <Ghost/Resources/ghostDescriptorManager.hpp>
#include <Ghost/Resources/hdriTexture.hpp>
#include <Ghost/Resources/material.hpp>
#include <Ghost/Resources/pbrMaterial.hpp>

namespace Ghost {
class SkyboxMaterial : public Material {
  public:
    SkyboxMaterial(VulkanDevice &device,
                   GhostDescriptorManager &descriptorManager,
                   std::shared_ptr<GhostGraphicsPipeline> pipeline,
                   uint32_t typeId);
    ~SkyboxMaterial() = default;

    void setHDRIMap(std::shared_ptr<HDRITexture> hdriMap) {
        m_hdriMap = hdriMap;
        buildDescriptorSet();
    }

    void updateShaderResources(const vk::raii::CommandBuffer &commandBuffer,
                               const GhostRenderObject &obj) override;

    VertexType getRequiredVertexType() const override {
        return VertexType::Standard;
    }

  private:
    void buildDescriptorSet();
    VulkanDevice &m_device;
    GhostDescriptorManager &m_descriptorManager;
    std::shared_ptr<HDRITexture> m_hdriMap;
    vk::raii::DescriptorSet m_descriptorSet = nullptr;
    PBRPushConstants m_pushData{};
};
} // namespace Ghost
