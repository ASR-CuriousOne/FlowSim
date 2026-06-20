#pragma once
#include <Ghost/Core/ghostGraphicsPipeline.hpp>
#include <Ghost/Resources/vertex.hpp>
#include <memory>

namespace Ghost {
struct GhostRenderObject;

class Material {
  public:
    Material(std::shared_ptr<GhostGraphicsPipeline> pipeline,
             uint32_t pipelineTypeId);
    virtual ~Material() = default;

    virtual void bind(const vk::raii::CommandBuffer &cmd) { m_pipeline->bind(cmd); }

    virtual void updateShaderResources(const vk::raii::CommandBuffer &cmd,
                                       const GhostRenderObject &obj) = 0;

    virtual VertexType getRequiredVertexType() const = 0;

    uint32_t getPipelineTypeId() const { return m_pipelineTypeId; }

  protected:
    std::shared_ptr<GhostGraphicsPipeline> m_pipeline;
    uint32_t m_pipelineTypeId;
};
} // namespace Ghost
