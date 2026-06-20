#pragma once
#include <Ghost/Resources/ghostDescriptorManager.hpp>
#include <Ghost/Resources/ghostTexture.hpp>
#include <Ghost/Resources/material.hpp>

namespace Ghost {
struct GhostRenderObject;
}

namespace Ghost {

constexpr auto PBR_LAYOUT_NAME = "PBRMaterialLayout";

struct PBRPushConstants {
    glm::mat4 modelMatrix{1.f};
    glm::vec4 albedoTint{1.f, 1.f, 1.f, 1.f};
    float metallicFactor{0.0f};
    float roughnessFactor{0.5f};
    float padding[2];
};

class PBRMaterial : public Material {
  public:
    PBRMaterial(VulkanDevice &device, GhostDescriptorManager &descriptorManager,
                std::shared_ptr<GhostGraphicsPipeline> pipeline,
                uint32_t pipelineTypeId);
    ~PBRMaterial() override = default;

    VertexType getRequiredVertexType() const override {
        return VertexType::Standard;
    }

    void updateShaderResources(const vk::raii::CommandBuffer &cmd,
                               const GhostRenderObject &obj) override;

    void setAlbedoTint(const glm::vec4 &color) {
        m_pushData.albedoTint = color;
    }
    void setMetallic(float m) { m_pushData.metallicFactor = m; }
    void setRoughness(float r) { m_pushData.roughnessFactor = r; }

    void setAlbedoMap(std::shared_ptr<GhostTexture> tex);
    void setNormalMap(std::shared_ptr<GhostTexture> tex);

  private:
    void flushDescriptorUpdates();

    VulkanDevice &m_device;
    GhostDescriptorManager &m_descriptorManager;

    PBRPushConstants m_pushData{};

    std::shared_ptr<GhostTexture> m_albedoMap;
    std::shared_ptr<GhostTexture> m_normalMap;
    std::shared_ptr<GhostTexture> m_metallicRoughnessMap;

    std::vector<vk::raii::DescriptorSet> m_descriptorSets;
};

} // namespace Ghost
